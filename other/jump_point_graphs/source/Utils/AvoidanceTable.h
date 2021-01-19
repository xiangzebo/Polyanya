/*
 * AvoidanceTable.h
 *
 *  Created on: Nov 13, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_UTILS_AVOIDANCETABLE_H_
#define APPS_SUBGOALGRAPH_UTILS_AVOIDANCETABLE_H_
#include <vector>

#include "../Graphs/GraphDefinitions.h"
// Mainly for jump-point variants.
// Suppose we are doing a forward search.
// If a node does not have any successors, we can avoid generating it.
// We can technically do this pretty efficiently during search.
// But, for the CH version, we need to do this at the level of the original
// JP-graph (since nodes at higher levels may have 0 successors, but they
// are important because the backward search needs to connect to them).
// Therefore, for CH variants, we can precompute which nodes to avoid by looking
// at the *original* jump point graph (before any contractions).
// During queries, if one of these nodes become connected to the goal, we need
// to make an exception for it (that is all these nodes are useful for anyway).

class AvoidanceTable {
 public:
  AvoidanceTable() {}

  template<class G>
  AvoidanceTable(G* g, bool forward) {
    if (forward)
      PopulateGivenGraph(g);
    else
      PopulateGivenReverseGraph(g);
  }

  template<class G>
  void PopulateGivenGraph(G* g) {
    avoid_.resize(g->GetNumAllNodes()+2, false);  // +2 for query nodes
    for (nodeId n = 0; n < avoid_.size()-2; n++) {
      if (g->HasNoSuccessors(n)) {
        avoid_[n] = true;
      }
    }
  }

  template<class G>
  void PopulateGivenReverseGraph(G* g) {
    avoid_.resize(g->GetNumAllNodes()+2, true);   // +2 for query nodes
    for (nodeId n = 0; n < avoid_.size()-2; n++) {
      for (auto a : g->GetSuccessors(n)) {
        avoid_[a.target] = false;
      }
    }

    avoid_[avoid_.size()-1] = false;
    avoid_[avoid_.size()-2] = false;
  }

  bool CanAvoid(nodeId n) {
    return avoid_[n];
  }

  void AddException(nodeId n) {
    if (CanAvoid(n)) {
      exceptions_.push_back(n);
      avoid_[n] = false;
    }
  }

  void AddException(std::vector<nodeId> & nodes) {
    for (auto n : nodes)
      AddException(n);
  }

  template<class Arc>
  void AddExceptionForTargets(std::vector<Arc> & arcs) {
    for (auto a: arcs)
      AddException(a.target);
  }

  void ClearExceptions() {
    for (auto n : exceptions_)
      avoid_[n] = true;
    exceptions_.clear();
  }

 private:
  std::vector<bool> avoid_;
  std::vector<nodeId> exceptions_;
};


#endif /* APPS_SUBGOALGRAPH_UTILS_AVOIDANCETABLE_H_ */
