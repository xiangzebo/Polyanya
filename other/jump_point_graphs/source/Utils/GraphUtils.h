/*
 * GraphUtils.h
 *
 *  Created on: Oct 29, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_UTILS_GRAPHUTILS_H_
#define APPS_SUBGOALGRAPH_UTILS_GRAPHUTILS_H_
#include <cassert>
#include <iostream>
#include <vector>
#include "../Graphs/GraphDefinitions.h"
#include "range.h"

class ExplicitGraphExtractor {
 public:
  ExplicitGraphExtractor() {}
  ~ExplicitGraphExtractor() {}

  // Extracts all arcs of a graph. Uses the graph's arc type.
  // Assumes make_arc function is implemented for the graph's arc type.
  template <class G, class ArcType>
  void ExtractGraph(G* g, int & num_nodes, std::vector<ArcType> & arcs) {
    num_nodes = g->GetNumAllNodes();
    arcs.clear();
    for (nodeId n = 0; n < num_nodes; n++)
      for (auto arc: g->GetSuccessors(n))
        arcs.push_back(make_arc(n, arc));
  }

  // Extracts all the arcs of a graph. Extracts arcs as weighted arcs.
  // Assumes that G implements GetWeight(a) function for its own arc type.
  // Assumes that arcs have a the field 'target'.
  template<class G>
  void ExtractWeightedGraph(G* g, int & num_nodes,
                            std::vector<WeightedArc> & arcs) {
    num_nodes = g->GetNumAllNodes();
    arcs.clear();
    for (nodeId n = 0; n < num_nodes; n++)
      for (auto arc : g->GetSuccessors(n))
        arcs.push_back(WeightedArc(n, arc.target, g->GetWeight(n,arc)));
  }
  template<class G>
  void ExtractUnweightedGraph(G* g, int & num_nodes, std::vector<Arc> & arcs) {
    num_nodes = g->GetNumAllNodes();
    arcs.clear();
    for (nodeId n = 0; n < num_nodes; n++)
      for (auto arc : g->GetSuccessors(n))
        arcs.push_back(Arc(n, arc.target));
  }

 private:
};

// Very hacky. Just implemented to check for connected components in subgoal graphs.
template<class G>
class DirectedGraph {
  public:
  typedef typename G::SuccessorType tArcHead;
  typedef typename type_of_arc<tArcHead>::type tArc;

  DirectedGraph(G* g) {
    std::vector<tArc> arcs;
    int num_nodes;
    ExplicitGraphExtractor e;
    e.ExtractGraph(g, num_nodes, arcs);
    forward_.CreateGraph(num_nodes, arcs);

    for (unsigned int i = 0; i < arcs.size(); i++)
      std::swap(arcs[i].source, arcs[i].target);

    backward_.CreateGraph(num_nodes, arcs);
  }

  int GetNumAllNodes() {
    return forward_.GetNumAllNodes();
  }

  Range<typename std::vector<tArcHead>::iterator> GetSuccessors (nodeId n) {
    return forward_.GetSuccessors(n);
  }
  Range<typename std::vector<tArcHead>::iterator> GetPredecessors (nodeId n) {
    return backward_.GetSuccessors(n);
  }

  bool IsValidNode(nodeId n) {
    return true;
    int num_arcs = 0;
    for (auto a : GetSuccessors(n))
      num_arcs++;
    for (auto a : GetPredecessors(n))
      num_arcs++;
    return num_arcs != 0;
  }

 private:
  G forward_;
  G backward_;
};

class UndirectedGraphCheck {
 public:
  UndirectedGraphCheck() {}

  template<class G>
  bool IsUndirected(G* g) {
    bool undirected = true;
    ExplicitGraphExtractor ext;
    ext.ExtractUnweightedGraph(g, num_nodes_, arcs_);

    assert(arcs_.size() == g->GetNumArcs());

    std::sort(arcs_.begin(), arcs_.end());

    directed_arcs_.clear();

    for (auto a: arcs_) {
      if (!std::binary_search(arcs_.begin(), arcs_.end(),
                              Arc(a.target, a.source))) {
        directed_arcs_.push_back(a);
        std::cout << "Arc " << a.source << "-" << a.target
                  << " does not have a reverse!" << std::endl;
        undirected = false;
      }
    }
    return undirected;
  }

  std::vector<Arc>* GetSingleDirectionArcs() {
    return &directed_arcs_;
  }
 private:
  int num_nodes_;
  std::vector<Arc> arcs_;
  std::vector<Arc> directed_arcs_;
  std::vector<Arc> duplicate_arcs_;
};

#endif /* APPS_SUBGOALGRAPH_UTILS_GRAPHUTILS_H_ */
