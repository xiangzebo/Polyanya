/*
 * DynamicGraph.h
 *
 *  Created on: Oct 29, 2016
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_DYNAMICGRAPH_H_
#define APPS_SUBGOALGRAPH_DYNAMICGRAPH_H_

#include <iostream>
#include <cassert>

#include "../Utils/range.h"
#include "GraphDefinitions.h"

// A graph implemented as a list of lists with various features:
// - Can add/remove nodes/arcs
// - Can add query nodes/arcs and clear them
// - Can 'hide' a node by removing all its arcs and then restoring it.

// Implements the following safety features:
// - Cannot add/remove regular nodes or arcs if query nodes/arcs are present.
// - Cannot hide a node if query nodes/arcs are present.
// - Cannot add/remove regular nodes/arcs if a node is hidden.
// - Overall allowed flow: permanent -> hide (not necesary) -> query

// Main uses:
// - Core graph for CH
// - Meant to be extended for constructing an R-SPC via contractions
//   - Modify base graph during contractions
//   - Hide bypass node during witness searches
//   - Connect start and/or goal to graph as query edges if they are local
// - Might be useful for dynamic subgoal graphs if we ever get to that?

// Template assumptions:
// 	- ArcHeadType has the public member nodeId id.

// TODO: It seems like, for heavy R-contractions, arcs will be hidden before
// query arcs are added. Make sure to not add a query arc to the node being
// bypassed (either here, or in the Contraction Core).
// TODO: Perform the safety checks before calling these functions.
// TODO: Finish implementing the above functions and test it by using it as
// a subgoal graph (hide feature won't be tested).

enum kDynamicGraphMode {
  kDynamicGraphBaseMode = 0,
  kDynamicGraphBypassMode = 1,
  kDynamicGraphQueryMode = 2
};

template<class ArcHeadType = WeightedArcHead>
class DynamicGraph {
 public:
  typedef ArcHeadType SuccessorType;

  DynamicGraph();
  ~DynamicGraph();

  template <class ArcType>
  void CreateGraph(int num_nodes, std::vector<ArcType> & arcs);

  void SetNumNodes(int num_nodes) {
    assert(num_nodes_ == 0);
    assert(arcs_.size() == 0);
    num_nodes_ = num_nodes;
    arcs_.resize(num_nodes_+2);
  }
  void AddArc(nodeId n, ArcHeadType arc, bool reverse = false);
  void RemoveArc(nodeId from, nodeId to);

  // Extend for query.
  void AddQueryNode(nodeId n, std::vector<ArcHeadType> & arcs, bool reverse);
  void ClearQuery();

  // Bypass node.
  void HideArc(nodeId from, nodeId to);
  void RestoreHiddenArcs();

  void GetSuccessors(nodeId n, std::vector<ArcHeadType> & neighbors) const {
    neighbors = arcs_[n];
  }
  void GetSuccessors(nodeId n, ArcHeadType* & neighbors, int & num_neighbors) {
    neighbors = &arcs_[n][0];
    num_neighbors = arcs_[n].size();
  }
  Range<typename std::vector<ArcHeadType>::iterator> GetSuccessors (nodeId n) {
    return vector_to_reference_range(arcs_[n]);
  }
  Distance GetWeight(nodeId n, ArcHeadType & h);

  int OutDegree(nodeId n) {
    return arcs_[n].size();
  }

  int GetNumAllNodes() const {
    return num_nodes_;
  }
  int GetNumArcs() const {
    return num_arcs_;
  }

  double EstimateStorageMB() {
    return (sizeof(ArcHeadType)*num_arcs_ + 8*num_nodes_)/(1024.0*1024.0);
  }

  // TODO: Maybe extract the arcs to a vector if one is specified?
  // Can help with memory peaks when moving arcs from one graph to another?
  void RemoveAllArcs() {
    assert(!query_mode_ && !bypass_mode_);
    for (nodeId n = 0; n < arcs_.size(); n++)
      arcs_[n].clear();
    num_arcs_ = 0;
  }

 private:
  int num_nodes_;
  int num_arcs_;

  std::vector<std::vector<ArcHeadType> > arcs_;

  bool query_mode_;
  std::vector<nodeId> nodes_with_query_arcs_;

  bool bypass_mode_;
  std::vector<nodeId> hidden_arc_sources_;
  std::vector<ArcHeadType> hidden_arc_heads_;

  // Returns -1 if the arc is not found. Otherwise, returns its index in
  // arcs_[from].
  int FindArc(nodeId from, nodeId to);
  // Add an arc to the graph (asserts that arc doesn't exist).
  void AddArcSubroutine(nodeId from, ArcHeadType to, bool reverse);
  // Remove an edge from the graph (asserts that arc exists).
  void RemoveArcSubroutine(nodeId from, nodeId to);

  void AddQueryArc(nodeId from, ArcHeadType to, bool reverse);
};


#include "DynamicGraph.inc"

#endif /* APPS_SUBGOALGRAPH_DYNAMICGRAPH_H_ */
