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
#ifndef LIGRA_H
#define LIGRA_H
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <algorithm>
#include "parallel.h"
#include "gettime.h"
#include "utils.h"
#include "vertex.h"
//#include "compressedVertex.h"
#include "vertexSubset.h"
#include "graph.h"
#include "IO.h"
#include "parseCommandLine.h"
#include "gettime.h"
#include <omp.h>
using namespace std;

//*****START FRAMEWORK*****

//options to edgeMap for different versions of dense edgeMap (default is DENSE)
enum options { DENSE, DENSE_FORWARD };

/*
//pagerank
#define parallel_for_emd _Pragma("cilk grainsize = 8") cilk_for
#define parallel_for_ems _Pragma("cilk grainsize = 1") cilk_for
#define parallel_for_x _Pragma("cilk grainsize = 1") cilk_for
#define parallel_for_vm _Pragma("cilk grainsize = 8") cilk_for
*/

//bfs
#define parallel_for_emd _Pragma("cilk grainsize = 16") cilk_for
#define parallel_for_ems _Pragma("cilk grainsize = 1") cilk_for
#define parallel_for_x _Pragma("cilk grainsize = 1") cilk_for
#define parallel_for_vm _Pragma("cilk grainsize = 1") cilk_for
/*
//CC
#define parallel_for_emd _Pragma("cilk grainsize = 16") cilk_for
#define parallel_for_x _Pragma("cilk grainsize = 1") cilk_for
#define parallel_for_ems _Pragma("cilk grainsize = 1") cilk_for
#define parallel_for_vm _Pragma("cilk grainsize = 16") cilk_for

//Radii
#define parallel_for_emd _Pragma("cilk grainsize = 16") cilk_for
#define parallel_for_x _Pragma("cilk grainsize = 1") cilk_for
#define parallel_for_ems _Pragma("cilk grainsize = 1") cilk_for
#define parallel_for_vm _Pragma("cilk grainsize = 8") cilk_for

//Trangle 
#define parallel_for_emd _Pragma("cilk grainsize = 8") cilk_for
#define parallel_for_x _Pragma("cilk grainsize = 1") cilk_for
#define parallel_for_ems _Pragma("cilk grainsize = 1") cilk_for
#define parallel_for_vm _Pragma("cilk grainsize = 8") cilk_for
*/

template <class vertex, class F>
bool* edgeMapDense(graph<vertex> GA, bool* vertexSubset, F &f, bool parallel = 0) {
  long numVertices = GA.n;
  long up_limit=GA.real_n;
  vertex *G = GA.V;
  bool* next = newA(bool,numVertices);
  memset(next,0,numVertices*sizeof(bool));
  
  {parallel_for_emd(long i=0; i<numVertices; i++) {
	//next[i] = 0;
	if (f.cond(i)&&i<up_limit) {
		G[i].decodeInNghBreakEarly(i, vertexSubset, f, next, parallel);
	}
	
	if(i==1000000) cout<<i<<endl;
  }}
 
  return next;
}

template <class vertex, class F>
bool* edgeMapDenseForward(graph<vertex> GA, bool* vertexSubset, F &f) {//never used
  long numVertices = GA.n;
  vertex *G = GA.V;
  bool* next = newA(bool,numVertices);
  {parallel_for_64(long i=0;i<numVertices;i++) next[i] = 0;}
  
  {parallel_for (long i=0; i<numVertices; i++){
    if (vertexSubset[i]) {
      G[i].decodeOutNgh(i, vertexSubset, f, next);
    }
  }}
  return next;
}

template <class vertex, class F>
pair<long,uintE*> edgeMapSparse(graph<vertex> GA, void* g_mem, vertex* frontierVertices, uintE* indices, 
        uintT* degrees, uintT m, F &f, 
        long remDups=0, uintE* flags=NULL) {
  uintT* offsets = degrees;
  long outEdgeCount = sequence::plusScan(offsets, degrees, m);
  uintE* outEdges = newA(uintE,outEdgeCount);
  
  {parallel_for(long i=0;i<outEdgeCount;i++)
	outEdges[i]=UINT_E_MAX;
  }
  
  {parallel_for_ems (long i = 0; i < m; i++) {
	uintT v = indices[i], o = offsets[i];
	vertex vert = frontierVertices[i]; 
	vert.decodeOutNghSparse(v, o, f, outEdges);
    }
  }
  uintE* nextIndices = newA(uintE, outEdgeCount);
  if(remDups) remDuplicates(outEdges,flags,outEdgeCount,remDups);
  // Filter out the empty slots (marked with -1)
  long nextM = sequence::filter(g_mem,outEdges,nextIndices,outEdgeCount,nonMaxF());
  free(outEdges);
  return pair<long,uintE*>(nextM, nextIndices);
}

// decides on sparse or dense base on number of nonzeros in the active vertices
template <class vertex, class F>
vertexSubset edgeMap(void* g_mem, graph<vertex> GA, vertexSubset &V, F f, intT threshold = -1, 
		 char option=DENSE, bool remDups=false) {
  long numVertices = GA.n, numEdges = GA.m;
  if(threshold == -1) threshold = numEdges/20; //default threshold
  vertex *G = GA.V;
  long m = V.numNonzeros();
  if (numVertices != V.numRows()) {
    cout << "edgeMap: Sizes Don't match" << endl;
    abort();
  }
  
  if(threshold==0){
	  V.toDense();
	  
	  bool* R = (option == DENSE_FORWARD) ? 
        edgeMapDenseForward(GA,V.d,f) : 
        edgeMapDense(GA, V.d, f, option);
      vertexSubset v1 = vertexSubset(numVertices, R);
      //cout << "size (D) = " << v1.m << endl;
      return v1;
  }
  // used to generate nonzero indices to get degrees
  uintT* degrees = newA(uintT, m);
  vertex* frontierVertices;
  V.toSparse(g_mem);
  frontierVertices = newA(vertex,m);
  {parallel_for (long i=0; i < m; i++){
    vertex v = G[V.s[i]];
    degrees[i] = v.getOutDegree();
    frontierVertices[i] = v;
    }
  }
	
  uintT outDegrees = sequence::plusReduce(degrees, m);
  if (outDegrees == 0) return vertexSubset(numVertices);
  if (m + outDegrees > threshold) { 
    V.toDense();
    free(degrees);
    free(frontierVertices);
    bool* R = (option == DENSE_FORWARD) ? 
      edgeMapDenseForward(GA,V.d,f) : 
      edgeMapDense(GA, V.d, f, option);
    vertexSubset v1 = vertexSubset(numVertices, R);
    //cout << "size (D) = " << v1.m << endl;
    return v1;
  } else { 
    pair<long,uintE*> R = 
      remDups ? 
      edgeMapSparse(GA, g_mem, frontierVertices, V.s, degrees, V.numNonzeros(), f, 
		    numVertices, GA.flags) :
      edgeMapSparse(GA, g_mem, frontierVertices, V.s, degrees, V.numNonzeros(), f);
    //cout << "size (S) = " << R.first << endl;
    free(degrees);
    free(frontierVertices);
    return vertexSubset(numVertices, R.first, R.second);
  }
}

//*****VERTEX FUNCTIONS*****

//Note: this is the optimized version of vertexMap which does not
//perform a filter
template <class F>
void vertexMap(vertexSubset V, F add) {
  long n = V.numRows(), m = V.numNonzeros();
  if(V.isDense) {
	  {parallel_for_vm(long i=0;i<n;i++)
	    if(V.d[i])	add(i);}
	
  } else {
	  {parallel_for_vm(long i=0;i<m;i++)
	    add(V.s[i]);}
    
  }
}

//Note: this is the version of vertexMap in which only a subset of the
//input vertexSubset is returned
template <class F>
vertexSubset vertexFilter(vertexSubset V, F filter) {
  long n = V.numRows(), m = V.numNonzeros();
  V.toDense();
  bool* d_out = newA_h(bool,n);
  {parallel_for_64(long i=0;i<n;i++) d_out[i] = 0;}
  
  {parallel_for_64(long i=0;i<n;i++)
      if(V.d[i]) d_out[i] = filter(i);}
  
  return vertexSubset(n,d_out);
}

//cond function that always returns true
inline bool cond_true (intT d) { return 1; }

template<class vertex>
void Compute(graph<vertex>&, commandLine,void*);

int main(int argc, char* argv[]) {
  commandLine P(argc,argv," [-s] <inFile>");
  char* iFile = P.getArgument(0);
  bool symmetric = P.getOptionValue("-s");
  bool compressed = P.getOptionValue("-c");
  bool binary = P.getOptionValue("-b");
  long rounds = P.getOptionLongValue("-rounds",3);
  
  if (symmetric) {
    graph<symmetricVertex> G =
      readGraph<symmetricVertex>(iFile,compressed,symmetric,binary); //symmetric graph
	  
	char* g_mem=(char*)newA_h(char,G.n);
	//uintE* outEdges=(uintE*)newA_h(uintE,G.m/20);
    
    cout<<"read Graph success"<<endl;
    
    Compute(G,P,(void*)g_mem);
	
    cout<<"computation success"<<endl;
	
    if(G.transposed) G.transpose();
    for(int r=0;r<rounds;r++) {
      startTime();
      Compute(G,P,(void*)g_mem);
      nextTime("Running time");
      if(G.transposed) G.transpose();
    }
    G.del();
  
    hbw_free(g_mem);
	//hbw_free(outEdges);
  } else {
    graph<asymmetricVertex> G =
      readGraph<asymmetricVertex>(iFile,compressed,symmetric,binary); //asymmetric graph
	  
	char* g_mem=(char*)newA_h(char,G.n);
	//uintE* outEdges=(uintE*)newA_h(uintE,G.m/20);
    
    cout<<"read Graph success"<<endl;
    
    Compute(G,P,(void*)g_mem);
	
    cout<<"computation success"<<endl;
	
    if(G.transposed) G.transpose();
    for(int r=0;r<rounds;r++) {
      startTime();
      Compute(G,P,(void*)g_mem);
      nextTime("Running time");
      if(G.transposed) G.transpose();
    }
    G.del();
  
    hbw_free(g_mem);
	//hbw_free(outEdges);
  }
}
#endif
