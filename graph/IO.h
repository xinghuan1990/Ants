// This code is part of the project "Ligra: A Lightweight Graph Processing
// Framework for Shared Memory", presented at Principles and Practice of 
// Parallel Programming, 2013.
// Copyright (c) 2013 Julian Shun and Guy Blelloch
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cmath>
#include "parallel.h"
#include "blockRadixSort.h"
#include "utils.h"
#include "graph.h"

#include <hbwmalloc.h>

using namespace std;

typedef pair<uintE,uintE> intPair;
typedef pair<uintE, pair<uintE,intE> > intTriple;

#define grainsize 64

template <class E>
struct pairFirstCmp {
  bool operator() (pair<uintE,E> a, pair<uintE,E> b) {
    return a.first < b.first; }
};

template <class E>
struct getFirst {uintE operator() (pair<uintE,E> a) {return a.first;} };

template <class IntType>
struct pairBothCmp {
  bool operator() (pair<uintE,IntType> a, pair<uintE,IntType> b) {
    if (a.first != b.first) return a.first < b.first;
    return a.second < b.second;
  }
};

// A structure that keeps a sequence of strings all allocated from
// the same block of memory
struct words {
  long n; // total number of characters
  char* Chars;  // array storing all strings
  long m; // number of substrings
  char** Strings; // pointers to strings (all should be null terminated)
  words() {}
words(char* C, long nn, char** S, long mm)
: Chars(C), n(nn), Strings(S), m(mm) {}
  void del() {free(Chars); free(Strings);}
};
 
inline bool isSpace(char c) {
  switch (c)  {
  case '\r': 
  case '\t': 
  case '\n': 
  case 0:
  case ' ' : return true;
  default : return false;
  }
}

_seq<char> readStringFromFile(char *fileName) {
  ifstream file (fileName, ios::in | ios::binary | ios::ate);
  if (!file.is_open()) {
    std::cout << "Unable to open file: " << fileName << std::endl;
    abort();
  }
  long end = file.tellg();
  file.seekg (0, ios::beg);
  long n = end - file.tellg();
  char* bytes = newA(char,n+1);
  if(bytes==NULL){
	  cout<<"bytes malloc failed"<<endl;
	  exit(0);
  }
  file.read (bytes,n);
  file.close();
  return _seq<char>(bytes,n);
}

// parallel code for converting a string to words
words stringToWords(char *Str, long n) {
 
  parallel_for_64(long i=0;i<n;i++)
	if(isSpace(Str[i]))	Str[i]=0;
 
  // mark start of words
  bool *FL = newA(bool,n);
  FL[0] = Str[0];
  
  parallel_for_64(long i=0;i<n;i++)
	  FL[i]=Str[i]&&!Str[i-1];
    
  // offset for each start of word
  _seq<long> Off = sequence::packIndex<long>(FL, n);
  long m = Off.n;
  long *offsets = Off.A;

  // pointer to each start of word
  char **SA = newA(char*, m);
  {parallel_for_8(long j=0; j < m; j++) SA[j] = Str+offsets[j];}

  free(offsets); free(FL);
  return words(Str,n,SA,m);
}

template <class vertex>
struct Uncompressed_Mem : public Deletable {
public:

  vertex* V;
  long n;
  long m;
  long real_n;
  void* allocatedInplace, * inEdges;
  uintE* flags;
  bool is_in_hbw;

  Uncompressed_Mem(vertex* VV, long nn, long real_nn, long mm, bool t_f, void* ai, void* _inEdges = NULL) 
  : V(VV), n(nn), real_n(real_nn), m(mm), is_in_hbw(t_f), allocatedInplace(ai), inEdges(_inEdges),  flags(NULL) {}

  void del() {
    if (flags != NULL) free(flags);
    if (allocatedInplace == NULL) 
      for (long i=0; i < n; i++) V[i].del();
    else{
		if(is_in_hbw)
			hbw_free(allocatedInplace);
		else
			free(allocatedInplace);
	}
    //free(V);
	hbw_free(V);
    if(inEdges != NULL){
		if(is_in_hbw)
			hbw_free(inEdges);
		else
			free(inEdges);
	}
  }
};

template <class vertex>
graph<vertex> readGraphFromFile(char* fname, bool isSymmetric) {
  _seq<char> S = readStringFromFile(fname);
  words W = stringToWords(S.A, S.n);
#ifndef WEIGHTED
  if (W.Strings[0] != (string) "AdjacencyGraph") {
#else
  if (W.Strings[0] != (string) "WeightedAdjacencyGraph") {
#endif
    cout << "Bad input file" << endl;
    abort();
  }

  long len = W.m -1;
  long n = atol(W.Strings[1]);
  long n_plus=0;
  
  
  for(int i=0;i<30;i++){
	if(grainsize*pow(2,i)==n){
		n_plus=n;
		break;
	}
	else if(grainsize*pow(2,i)<n&&grainsize*pow(2,i+1)>n){
		n_plus=grainsize*pow(2,i+1);
		break;
	}
  }
  
  cout<<"n: "<<n<<"  n_plus: "<<n_plus<<endl;
  
  //n_plus=n;
  
  long m = atol(W.Strings[2]);
#ifndef WEIGHTED
  if (len != n + m + 2) {
#else
  if (len != n + 2*m + 2) {
#endif
    cout << "Bad input file" << endl;
    abort();
  }
  
  bool all_in_hbw=false;
  //long hbw_size=(long)16*1024*1024*1024;
  long hbw_size=0;
  long m_size=n*sizeof(uintT)*4+m*sizeof(uintE)*2+n*sizeof(vertex);
  //cout<<"m_size: "<<m_size<<endl;
  if(m_size<(long)(hbw_size*0.8))
	  all_in_hbw=true;

  uintT* offsets = newA_h(uintT,n_plus);//offsets malloc on MCDRAM
  
  uintE* edges;
#ifndef WEIGHTED
  if(all_in_hbw)
	  edges = newA_h(uintE,m);
  else
	  edges = newA(uintE,m);
#else
  if(all_in_hbw)
	  edges = newA_h(intE,2*m);
  else
	  edges = newA(intE,2*m);
#endif


  {parallel_for(long i=0;i<n;i++)
    offsets[i]=atol(W.Strings[i+3]);}

  {parallel_for(long i=n;i<n_plus;i++)
	offsets[i]=offsets[n-1];}
  
  {parallel_for(long i=0;i<m;i++){
#ifndef WEIGHTED
      edges[i] = atol(W.Strings[i+n+3]); 
#else
      edges[2*i] = atol(W.Strings[i+n+3]); 
      edges[2*i+1] = atol(W.Strings[i+n+m+3]);
#endif
  }}
  //W.del(); // to deal with performance bug in malloc
    
  //vertex* v = newA(vertex,n);
  vertex* v=newA_h(vertex,n_plus);//v malloc on MCDRAM

  {parallel_for(uintT i=0;i<n;i++){
    uintT o = offsets[i];
    uintT l = ((i == n-1) ? m : offsets[i+1])-offsets[i];
    v[i].setOutDegree(l); 
#ifndef WEIGHTED
    v[i].setOutNeighbors(edges+o);     
#else
    v[i].setOutNeighbors(edges+2*o);
#endif
  }}
	
	{parallel_for(uintT i=n;i<n_plus;i++){
		v[i].setOutDegree(0);
	}}

  if(!isSymmetric) {
    //uintT* tOffsets = newA(uintT,n);
	uintT* tOffsets=newA_h(uintT,n_plus);//malloc on MCDRAM
    {parallel_for(long i=0;i<n_plus;i++) tOffsets[i] = INT_T_MAX;}
	
	uintE* inEdges;
#ifndef WEIGHTED
	if(all_in_hbw)
		inEdges = newA_h(uintE,m);
	else
		inEdges = newA(uintE,m);
	
    intPair* temp = newA(intPair,m);
#else
	if(all_in_hbw)
		inEdges = newA_h(intE,2*m);
	else
		inEdges = newA(intE,2*m);
	
    intTriple* temp = newA(intTriple,m);
#endif
  
	
	{parallel_for(long i=0;i<n;i++){
      uintT o = offsets[i];
      for(uintT j=0;j<v[i].getOutDegree();j++){	  
#ifndef WEIGHTED
	temp[o+j] = make_pair(v[i].getOutNeighbor(j),i);
#else
	temp[o+j] = make_pair(v[i].getOutNeighbor(j),make_pair(i,v[i].getOutWeight(j)));
#endif
      }
	}}
    hbw_free(offsets);

#ifndef WEIGHTED
    intSort::iSort(temp,m,n+1,getFirst<uintE>());
#else
    intSort::iSort(temp,m,n+1,getFirst<intPair>());
#endif

    tOffsets[temp[0].first] = 0; 
#ifndef WEIGHTED
    inEdges[0] = temp[0].second;
#else
    inEdges[0] = temp[0].second.first;
    inEdges[1] = temp[0].second.second;
#endif
	
    {parallel_for(long i=1;i<m;i++) {
#ifndef WEIGHTED
      inEdges[i] = temp[i].second;
#else
      inEdges[2*i] = temp[i].second.first; 
      inEdges[2*i+1] = temp[i].second.second;
#endif
      if(temp[i].first != temp[i-1].first) {
		tOffsets[temp[i].first] = i;
      }
    }}

    free(temp);
 
    //fill in offsets of degree 0 vertices by taking closest non-zero
    //offset to the right
    sequence::scanIBack(tOffsets,tOffsets,n,minF<uintT>(),(uintT)m);
	{parallel_for(long i=n;i<n_plus;i++)
	  tOffsets[i]=tOffsets[n-1];}
	
    {parallel_for(long i=0;i<n;i++){
      uintT o = tOffsets[i];
      uintT l = ((i == n-1) ? m : tOffsets[i+1])-tOffsets[i];
      v[i].setInDegree(l);
#ifndef WEIGHTED
      v[i].setInNeighbors(inEdges+o);
#else
      v[i].setInNeighbors(inEdges+2*o);
#endif
	}}

	{parallel_for(long i=n;i<n_plus;i++)
	  v[i].setInDegree(0);}

	//cout<<"in neighbors init success"<<endl;

    //free(tOffsets);
	hbw_free(tOffsets);
    Uncompressed_Mem<vertex>* mem = new Uncompressed_Mem<vertex>(v,n_plus,n,m,all_in_hbw,edges,inEdges);
    return graph<vertex>(v,n_plus,n,m,mem);
  }
  else {
    //free(offsets);
	hbw_free(offsets);
    Uncompressed_Mem<vertex>* mem = new Uncompressed_Mem<vertex>(v,n_plus,n,m,all_in_hbw,edges);
    return graph<vertex>(v,n_plus,n,m,mem);
  }
}

/*
template <class vertex>
graph<vertex> readGraphFromBinary(char* iFile, bool isSymmetric) {
  char* config = (char*) ".config";
  char* adj = (char*) ".adj";
  char* idx = (char*) ".idx";
  char configFile[strlen(iFile)+strlen(config)+1];
  char adjFile[strlen(iFile)+strlen(adj)+1];
  char idxFile[strlen(iFile)+strlen(idx)+1];
  *configFile = *adjFile = *idxFile = '\0'; 
  strcat(configFile,iFile);
  strcat(adjFile,iFile);
  strcat(idxFile,iFile);
  strcat(configFile,config);
  strcat(adjFile,adj);
  strcat(idxFile,idx);

  ifstream in(configFile, ifstream::in);
  long n;
  in >> n;
  in.close();

  ifstream in2(adjFile,ifstream::in | ios::binary); //stored as uints
  in2.seekg(0, ios::end);
  long size = in2.tellg();
  in2.seekg(0);
#ifdef WEIGHTED
  long m = size/(2*sizeof(uint));
#else
  long m = size/sizeof(uint);
#endif
  char* s = (char *) malloc(size);
  in2.read(s,size);
  in2.close();
  uintE* edges = (uintE*) s;

  ifstream in3(idxFile,ifstream::in | ios::binary); //stored as longs
  in3.seekg(0, ios::end);
  size = in3.tellg();
  in3.seekg(0);
  if(n != size/sizeof(intT)) { cout << "File size wrong\n"; abort(); }

  char* t = (char *) malloc(size);
  in3.read(t,size);
  in3.close();
  uintT* offsets = (uintT*) t;

  vertex* v = newA(vertex,n);
#ifdef WEIGHTED
  intE* edgesAndWeights = newA(intE,2*m);
  {parallel_for(long i=0;i<m;i++) {
    edgesAndWeights[2*i] = edges[i];
    edgesAndWeights[2*i+1] = edges[i+m]; 
    }}
  //free(edges);
#endif
  {parallel_for(long i=0;i<n;i++) {
    uintT o = offsets[i];
    uintT l = ((i==n-1) ? m : offsets[i+1])-offsets[i];
      v[i].setOutDegree(l); 
#ifndef WEIGHTED
      v[i].setOutNeighbors((uintE*)edges+o); 
#else
      v[i].setOutNeighbors(edgesAndWeights+2*o);
#endif
    }}
  if(!isSymmetric) {
    uintT* tOffsets = newA(uintT,n);
    {parallel_for(long i=0;i<n;i++) tOffsets[i] = INT_T_MAX;}
#ifndef WEIGHTED
    uintE* inEdges = newA(uintE,m);
    intPair* temp = newA(intPair,m);
#else
    intE* inEdges = newA(intE,2*m);
    intTriple* temp = newA(intTriple,m);
#endif
    {parallel_for(intT i=0;i<n;i++){
      uintT o = offsets[i];
      for(uintT j=0;j<v[i].getOutDegree();j++){
#ifndef WEIGHTED
	temp[o+j] = make_pair(v[i].getOutNeighbor(j),i);
#else
	temp[o+j] = make_pair(v[i].getOutNeighbor(j),make_pair(i,v[i].getOutWeight(j)));
#endif
      }
      }}
    free(offsets);
#ifndef WEIGHTED
    intSort::iSort(temp,m,n+1,getFirst<uintE>());
#else
    intSort::iSort(temp,m,n+1,getFirst<intPair>());
#endif
    tOffsets[temp[0].first] = 0; 
#ifndef WEIGHTED
    inEdges[0] = temp[0].second;
#else
    inEdges[0] = temp[0].second.first;
    inEdges[1] = temp[0].second.second;
#endif
    {parallel_for(long i=1;i<m;i++) {
#ifndef WEIGHTED
      inEdges[i] = temp[i].second;
#else
      inEdges[2*i] = temp[i].second.first;
      inEdges[2*i+1] = temp[i].second.second;
#endif
      if(temp[i].first != temp[i-1].first) {
	tOffsets[temp[i].first] = i;
      }
      }}
    free(temp);
    //fill in offsets of degree 0 vertices by taking closest non-zero
    //offset to the right
    sequence::scanIBack(tOffsets,tOffsets,n,minF<uintT>(),(uintT)m);
    {parallel_for(long i=0;i<n;i++){
      uintT o = tOffsets[i];
      uintT l = ((i == n-1) ? m : tOffsets[i+1])-tOffsets[i];
      v[i].setInDegree(l);
#ifndef WEIGHTED
      v[i].setInNeighbors((uintE*)inEdges+o);
#else
      v[i].setInNeighbors((intE*)(inEdges+2*o));
#endif
      }}
    free(tOffsets);
#ifndef WEIGHTED
    Uncompressed_Mem<vertex>* mem = new Uncompressed_Mem<vertex>(v,n,m,edges,inEdges);
    return graph<vertex>(v,n,m,mem);
#else
    Uncompressed_Mem<vertex>* mem = new Uncompressed_Mem<vertex>(v,n,m,edgesAndWeights,inEdges);
    return graph<vertex>(v,n,m,mem);
#endif
  }
  free(offsets);
#ifndef WEIGHTED  
  Uncompressed_Mem<vertex>* mem = new Uncompressed_Mem<vertex>(v,n,m,edges);
  return graph<vertex>(v,n,m,mem);
#else
  Uncompressed_Mem<vertex>* mem = new Uncompressed_Mem<vertex>(v,n,m,edgesAndWeights);
  return graph<vertex>(v,n,m,mem);
#endif
}

template <class vertex>
graph<vertex> readGraph(char* iFile, bool compressed, bool symmetric, bool binary) {
  if(binary) return readGraphFromBinary<vertex>(iFile,symmetric); 
  else return readGraphFromFile<vertex>(iFile,symmetric);
}
*/
template <class vertex>
graph<vertex> readGraph(char* iFile, bool compressed, bool symmetric, bool binary) {
  return readGraphFromFile<vertex>(iFile,symmetric);
}
