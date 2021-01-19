/*
 * CHSearchGraph.h
 *
 *  Created on: Nov 5, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_CONTRACTIONHIERARCHY_CHSEARCHGRAPH_H_
#define APPS_SUBGOALGRAPH_CONTRACTIONHIERARCHY_CHSEARCHGRAPH_H_
#include "../Graphs/ArcListQueryGraph.h"
#include "../Utils/FileReadWrite.h"

// The resulting CH.
template <class SearchGraphType>
class CHSearchGraph {
 public:
  CHSearchGraph() {
    is_undirected_ = false;
  }
  ~CHSearchGraph() {}

  void SetIsUndirected(bool is_undirected) {
    is_undirected_ = is_undirected;
  }
  bool IsUndirected() {
    return is_undirected_;
  }

  long GetNumAllNodes() {
    return forward_graph_.GetNumAllNodes();
  }
  long GetNumArcs() {
    int num_arcs = forward_graph_.GetNumArcs();
    if (!is_undirected_)
      num_arcs += backward_graph_.GetNumArcs();
    return num_arcs;
  }

  template<class OnNode, class OnArc>
  void IterateOverArcsAndDo (OnNode on_node, OnArc on_arc) {
    IterateOverArcsAndDo(forward_graph_, on_node, on_arc, false);
    if (!is_undirected_)
      IterateOverArcsAndDo(backward_graph_, on_node, on_arc, true);
  }

  SearchGraphType* GetForwardGraph() {
    return &forward_graph_;
  }
  SearchGraphType* GetBackwardGraph() {
    if (is_undirected_)
      return &forward_graph_;
    else
      return &backward_graph_;
  }

  void Write(FileReadWrite & rw) {
    rw.Write(is_undirected_);
    rw.WriteGraph(GetForwardGraph());
    if (!is_undirected_)
      rw.WriteGraph(GetBackwardGraph());
  }
  void Read(FileReadWrite & rw) {
    rw.Read(is_undirected_);
    rw.ReadGraph(GetForwardGraph());
    if (!is_undirected_)
      rw.ReadGraph(GetBackwardGraph());
  }

  void Write(std::string filename) {
    FileReadWrite rw;
    rw.StartWrite(filename);
    Write(rw);
    rw.CloseWrite();
  }
  void Read(std::string filename) {
    FileReadWrite rw;
    rw.StartRead(filename);
    Read(rw);
    rw.CloseRead();
  }

 private:
  bool is_undirected_;
  SearchGraphType forward_graph_;
  SearchGraphType backward_graph_;

  template<class OnNode, class OnArc>
  void IterateOverArcsAndDo(SearchGraphType & graph, OnNode on_node,
                            OnArc on_arc, bool backward) {
    int count = 0;
    for (nodeId n = 0; n < graph.GetNumAllNodes(); n++) {
      if (!backward)
        on_node(n);
      for (auto a : graph.GetSuccessors(n))
        on_arc(n,a,backward);
    }
  }
};



#endif /* APPS_SUBGOALGRAPH_CONTRACTIONHIERARCHY_CHSEARCHGRAPH_H_ */
