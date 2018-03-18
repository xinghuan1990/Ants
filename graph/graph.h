#ifndef GRAPH_H
#define GRAPH_H
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "vertex.h"
//#include "compressedVertex.h"
#include "parallel.h"
#include <omp.h>
using namespace std;

// **************************************************************
//    ADJACENCY ARRAY REPRESENTATION
// **************************************************************

// Class that handles implementation specific freeing of memory 
// owned by the graph 
struct Deletable {
public:
  virtual void del() = 0;
};

template <class vertex>
struct graph {
  vertex *V;
  long n;
  long real_n;
  long m;
  bool transposed;
  uintE* flags;
  Deletable *D;
graph(vertex* VV, long nn, long real_nn, long mm, Deletable* DD) : V(VV), n(nn), real_n(real_nn), m(mm), D(DD), flags(NULL), transposed(0) {}
graph(vertex* VV, long nn, long real_nn, long mm, Deletable* DD, uintE* _flags) : V(VV), n(nn), real_n(real_nn), m(mm), D(DD), flags(_flags), transposed(0) {}

  void del() {
    D->del();
    free(D);
  }

  void transpose() {
    if ((sizeof(vertex) == sizeof(asymmetricVertex))) {
      //#pragam omp parallel for
	  {parallel_for(long i=0;i<n;i++) {
        V[i].flipEdges();
      }}
      transposed = !transposed;
    }
  }
};
#endif
