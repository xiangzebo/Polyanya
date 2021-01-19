/*
 * FileReadWrite.h
 *
 *  Created on: Feb 11, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_UTILS_FILEREADWRITE_H_
#define APPS_SUBGOALGRAPH_UTILS_FILEREADWRITE_H_
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "GraphUtils.h"
class FileReadWrite {
 public:
  FileReadWrite() {}
  ~FileReadWrite() {}

  // Open/close
  bool StartWrite(std::string filename) {
    out.open(filename.c_str(), std::ios::out | std::ios::binary);
    if (!out.is_open()) {
//      std::cout<<"Cannot open file for writing: "<<filename<<std::endl;
      return false;
    }
//    std::cout<<"Writing to "<<filename<<std::endl;
    return true;
  }
  bool StartRead(std::string filename) {
    in.open(filename.c_str(), std::ios::in | std::ios::binary);
    if (!in.is_open()) {
//      std::cout<<"Cannot open file for reading: "<<filename<<std::endl;
      return false;
    }
//    std::cout<<"Reading from "<<filename<<std::endl;
    return true;
  }
  void CloseWrite() {
    assert(out.is_open());
    out.close();
  }
  void CloseRead() {
    assert(in.is_open());
    in.close();
  }

  template <class T>
  void Write(T n) {
    out.write((char*) & n, sizeof(T));
  }
  template <class T>
  void Read(T & n) {
    in.read((char*) & n, sizeof(T));
  }

  template <class T>
  void WriteVector(const std::vector<T> & v){
    Write((int)v.size());
    for (unsigned int i = 0; i < v.size(); i++) {
      Write(v[i]);
    }
  }
  template <class T>
  void ReadVector(std::vector<T> & v){
    int n;
    Read(n);
    v.resize(n);
    for (unsigned int i = 0; i < v.size(); i++) {
      Read(v[i]);
    }
  }

  template <class G>
  void WriteGraph(G* g) {
    int num_nodes;
    typedef typename G::SuccessorType ArcHeadType;
    typedef typename type_of_arc<ArcHeadType>::type ArcType;
    std::vector<ArcType> arcs;
    ExplicitGraphExtractor graph_extractor;
    graph_extractor.ExtractGraph(g, num_nodes, arcs);
    Write(num_nodes);
    WriteVector(arcs);
  }
  template <class G>
  void ReadGraph(G* g) {
    int num_nodes;
    typedef typename G::SuccessorType ArcHeadType;
    typedef typename type_of_arc<ArcHeadType>::type ArcType;
    std::vector<ArcType> arcs;
    Read(num_nodes);
    ReadVector(arcs);
    assert (g != NULL);
    g->CreateGraph(num_nodes, arcs);
  }

 private:
  std::ifstream in;
  std::ofstream out;
};


#endif /* APPS_SUBGOALGRAPH_UTILS_FILEREADWRITE_H_ */
