/*
 * DynamicGraph.h
 *
 *  Created on: Oct 29, 2016
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_ARC_LIST_QUERY_GRAPH_H_
#define APPS_SUBGOALGRAPH_ARC_LIST_QUERY_GRAPH_H_

#include <iostream>
#include <algorithm>
#include <cassert>

#include "../Utils/range.h"
#include "GraphDefinitions.h"

// A graph implementation that:
// - Stores the arcs as a vector of vectors.
// - Can be extended by adding a start and goal node.
// - Kinda optimized with queries in mind, where each vector is allocated space
//   for one extra arc.

// Supports the following operations on the graph:
// - Add node / set the number of nodes (removing nodes is not supported).
// - Add/remove edges.
// - Add 'temporary' nodes and edges, which can be cleared by calling a single function.
// 		(Useful when connecting start and goal to the subgoal graph.)

// Assumptions:
// - Start or goal queries are only added if they are not already in the graph.
// - If start is added, it is added before the goal.

// Template assumptions:
// - ArcHead has the public member nodeId id.

template<class ArcHeadType = WeightedArcHead>
class ArcListQueryGraph {
 public:
  typedef ArcHeadType SuccessorType;

  ArcListQueryGraph()
      : num_nodes_(0),
        num_arcs_(0),
        num_query_nodes_(0) {
  }

  template <class ArcType>
  ArcListQueryGraph(int num_nodes, std::vector<ArcType> & arcs);
  ~ArcListQueryGraph();

  template <class ArcType>
  void CreateGraph(int num_nodes, std::vector<ArcType> & arcs);

  // Extend for query.
  void AddQueryNode(nodeId n, std::vector<ArcHeadType> & arcs, bool reverse) {
    AddQueryNode(n);
    AddQueryArcs(n, arcs, reverse);
  }
  void ClearQuery();

  bool HasNoSuccessors(nodeId n) {
    return first_out_[n+1] - first_out_[n] - no_extra_arc_[n] == 0;
  }
  void GetSuccessors(nodeId n, std::vector<ArcHeadType> & neighbors) const {
    // FIXME: There is probably a better version for this?
    neighbors = std::vector<ArcHeadType>(
        arcs_.begin() + first_out_[n],
        arcs_.begin() + first_out_[n + 1] - no_extra_arc_[n]);
  }
  void GetSuccessors(nodeId n, ArcHeadType* & neighbors, int & num_neighbors) {
    neighbors = &arcs_[0] + first_out_[n];
    num_neighbors = first_out_[n+1] -no_extra_arc_[n] - first_out_[n];
  }
  Range<typename std::vector<ArcHeadType>::iterator> GetSuccessors (nodeId n) {
    return make_range(arcs_.begin() + first_out_[n],
                      arcs_.begin() + first_out_[n + 1] - no_extra_arc_[n]);
  }
  Distance GetWeight(nodeId n, ArcHeadType & h);

  int GetNumAllNodes() const {
    return num_nodes_;
  }
  int GetNumArcs() const {
    return num_arcs_;
  }
  double EstimateStorageMB() {
    return (sizeof(ArcHeadType)*num_arcs_ + 8*num_nodes_)/(1024.0*1024.0);
  }

  std::vector<ArcHeadType>* GetArcList() {return &arcs_;}
  arcId GetArcId(nodeId from, nodeId to) {
    assert(from < first_out_.size()-1);
    for (int i = first_out_[from]; i < first_out_[from+1]; i++)
      if (arcs_[i].target == to)
        return i;
    return kNonArc;
  }

 private:
  int num_nodes_;
  int num_arcs_;

  std::vector<int> first_out_;
  std::vector<bool> no_extra_arc_;
  std::vector<ArcHeadType> arcs_;

  int num_query_nodes_; // Includes num_nodes_; only used for asserts atm.
  std::vector<nodeId> nodes_with_query_arcs_;

  void AddQueryNode(nodeId n);
  void AddQueryArcs(nodeId n, std::vector<ArcHeadType> & arcs, bool reverse);
};

// TODO: Reachable: Convert from weighted to unweighted.
typedef ArcListQueryGraph<WeightedArcHead> ReachableArcListQueryGraph;
typedef ArcListQueryGraph<WeightedArcHead> WeightedArcListQueryGraph;
typedef ArcListQueryGraph<ShortcutArcHead> ShortcutArcListQueryGraph;

#include "ArcListQueryGraph.inc"

#endif /* APPS_SUBGOALGRAPH_ARC_LIST_QUERY_GRAPH_H_ */
