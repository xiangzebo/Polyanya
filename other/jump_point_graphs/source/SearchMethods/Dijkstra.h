/*
 * Dijkstra.h
 *
 *  Created on: Oct 28, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_UTILS_DIJKSTRA_H_
#define APPS_SUBGOALGRAPH_UTILS_DIJKSTRA_H_
#include "../Utils/CPUTimer.h"
#include "../Utils/LevelManager.h"
#include "../Utils/Statistics/QueryStatistic.h"
#include "PreallocatedPriorityQueue.h"
#include "SearchIterationCounter.h"

// A simple dijkstra implementation that can be used in bidirectional dijkstra.
// Provides methods for single expansion and querying the g-value of a node.

template <class G>
class Dijkstra {
 public:
  Dijkstra(G* g)
      : g_(g),
        max_node_id_(g->GetNumAllNodes() + 2),
        search_counter_(max_node_id_) {

    g_val_.resize(max_node_id_);
    parent_.resize(max_node_id_);
    stalled_.resize(max_node_id_);
    num_expanded_ = 0;
    num_generated_ = 0;
    num_settled_arcs_ = 0;
    search_time_ = 0;
    num_stalled_ = 0;
    ppq_.SetupQueue(max_node_id_);
    start_ = kNonNode;
  }
  ~Dijkstra() {}

  bool IsFinished() {
    return ppq_.IsEmpty();
  }
  nodeId GetNext() {
    return ppq_.GetMin();
  }
  Distance GetRadius() {
    if (ppq_.IsEmpty())
      return kMaxDistance;
    else
      return g_val_[ppq_.GetMin()];
  }
  bool PathToExists(nodeId n) {
    return search_counter_.IsGenerated(n);
  }
  Distance GetTentativeDistance(nodeId n) {
    if (search_counter_.IsGenerated(n))
      return g_val_[n];
    else
      return kMaxDistance;
  }
  bool IsTentativeDistanceCertain(nodeId n) {
    return search_counter_.IsGenerated(n) // TODO: Just added this fix.
        && GetTentativeDistance(n) <= GetRadius() + 0.001;
  }

  nodeId GetParent(nodeId n) {
    return parent_[n];
  }

  void ResetStatistics() {
    num_expanded_ = 0;
    num_generated_ = 0;
    num_settled_arcs_ = 0;
    search_time_ = 0;
    num_stalled_ = 0;
  }

  void Reset() {
    assert(g_->GetNumAllNodes() <= max_node_id_);
    search_counter_.Reset();
    ppq_.Reset();
    ResetStatistics();
  }

  void AddStart(nodeId start) {
    GenerateNode(start);
    g_val_[start] = 0;
    ppq_.InsertOrDecreaseKey(start, 0);
    start_ = start;
  }

  // If the search is already initialized (with AddStart), expands until
  // goal is expanded or queue is empty.
  Distance FindGoal(nodeId goal) {
    GenerateNode(goal);
    g_val_[goal] = kMaxDistance;

    while(!ppq_.IsEmpty() && g_val_[goal] > ppq_.GetMinCost() + 0.001)
      ExpandNext();

    return g_val_[goal];
  }

  template<class ArcHead>
  void SettleArc(nodeId n, ArcHead & a) {
    nodeId succ = a.target;
    GenerateNode(succ);
    Distance new_g_val = g_val_[n] + g_->GetWeight(n, a);
    if (new_g_val + 0.001 < g_val_[succ]) {
      g_val_[succ] = new_g_val;
      parent_[succ] = n;
      ppq_.InsertOrDecreaseKey(succ, g_val_[succ]);
    }
    num_settled_arcs_++;
  }

  nodeId ExpandNext() {
    nodeId curr = ppq_.PopMin();
    for (auto a:g_->GetSuccessors(curr))
      SettleArc(curr, a);
    num_expanded_++;
    return curr;
  }

  template<class ArcHead>
  void SettleArcPreferMostRecenParent(nodeId n, ArcHead & a, Distance bound =
                                          kMaxDistance) {
    nodeId succ = a.target;
    GenerateNode(succ);
    Distance new_g_val = g_val_[n] + g_->GetWeight(n, a);
    if (new_g_val < bound && new_g_val <= g_val_[succ] + kEpsDistance) {
      g_val_[succ] = new_g_val;
      parent_[succ] = n;
      ppq_.InsertOrDecreaseKey(succ, g_val_[succ]);
    }
    num_settled_arcs_++;
  }

  nodeId ExpandNextPreferMostRecentParent(Distance bound = kMaxDistance) {
    nodeId curr = ppq_.PopMin();
    for (auto a:g_->GetSuccessors(curr))
      SettleArcPreferMostRecenParent(curr, a, bound);
    num_expanded_++;
    return curr;
  }



  nodeId ExpandNextGenerateHigherLevel(LevelManager* l) {
    nodeId curr = ppq_.PopMin();
    for (auto a:g_->GetSuccessors(curr))
      if (l->IsStrictlyLowerLevel(curr, a.target))
        SettleArc(curr,a);
    return curr;
  }

  // Not necessary?
  nodeId ExpandNextGenerateSameOrHigherLevel(LevelManager* l) {
    nodeId curr = ppq_.PopMin();
    for (auto a:g_->GetSuccessors(curr))
      if (!(l->IsStrictlyLowerLevel(a.target, curr)))
        SettleArc(curr,a);
    return curr;
  }

  nodeId StallNext() {
    num_stalled_++;
    stalled_[ppq_.GetMin()] = true;
    return ppq_.PopMin();
  }

  Distance FindPath(nodeId start, nodeId goal, std::vector<nodeId> & path) {
    CPUTimer t;
    t.StartTimer();

    Reset();
    AddStart(start);
    FindGoal(goal);
    Distance d = ExtractPathToNode(goal, path);

    search_time_ = t.EndTimer();
    return d;
  }

  bool DoesPathUpToLengthExist(nodeId start, nodeId goal, Distance b) {
    Reset();
    AddStart(start);
    GenerateNode(goal);
    g_val_[goal] = kMaxDistance;

    while (!ppq_.IsEmpty()) {
      if (ppq_.GetMinCost() > b + 0.01)
        return false;
      if (g_val_[goal] < b + 0.01)
        return true;
      ExpandNext();
    }
    return false;
    /*
    while (!ppq_.IsEmpty() && ppq_.GetMinCost() < b + 0.01
        && g_val_[goal] > b + 0.01)
      ExpandNext();
    return g_val_[goal] <= b + 0.01;
    */
  }

  void AddStatistics(QueryStatistic & s, bool backward = false) const {
    s.num_expanded += num_expanded_;
    s.num_stalled += num_stalled_;
    s.num_generated += num_generated_;
    s.num_relaxed_arcs += num_settled_arcs_;
    s.num_percolations += ppq_.GetNumPercolations();
    s.num_update_key += ppq_.GetNumUpdateKey();
    s.search_time += search_time_;
    if (backward)
      s.num_backward_expanded = num_expanded_;
  }

  Distance ExtractPathToNode(nodeId goal, std::vector<nodeId> & path) {
    path.clear();
    if (g_val_[goal] < kMaxDistance) {
      nodeId curr = goal;
      while (curr != start_) {
        path.push_back(curr);
        curr = parent_[curr];
      }
      path.push_back(curr);
      std::reverse(path.begin(), path.end());
    }
    return g_val_[goal];
  }

  // Returns a list of all arcs that appear on all the symmetric shortest paths
  // between two nodes. Mainly used for visualization.
  void ExtractAllEdgesOnShortestPaths(nodeId start, nodeId goal,
                                      std::vector<nodeId> & from,
                                      std::vector<nodeId> & to) {
    from.clear();
    to.clear();

    std::vector<nodeId> dummy_path;
    FindPath(start, goal, dummy_path);

    // If no path is found, return.
    if (dummy_path.size() == 0)
      return;

    std::vector<bool> visited(max_node_id_, false);
    std::vector<nodeId> stack;
    stack.push_back(goal);
    visited[goal] = true;

    std::vector<WeightedArcHead> predecessors;
    while (!stack.empty()) {
      nodeId curr = stack.back();
      stack.pop_back();
      g_->GetPredecessors(curr, predecessors);

      for (unsigned int i = 0; i < predecessors.size(); i++) {
        nodeId pred = predecessors[i].target;
        // If the predecessor is on a shortest path to the current node, extract the edge, and add the predecessor to the stack (if not already visited).
        if (fabs(g_val_[pred] + predecessors[i].weight - g_val_[curr])
            < 0.001) {
          from.push_back(pred);
          to.push_back(curr);
          if (!visited[pred]) {
            visited[pred] = true;
            stack.push_back(pred);
          }
        }
      }
    }
  }

  void GetSearchTree(std::vector<Arc> & f_exp,
                     std::vector<Arc> & f_stall,
                     std::vector<Arc> & f_gen,
                     std::vector<Arc> & b_exp,
                     std::vector<Arc> & b_stall,
                     std::vector<Arc> & b_gen) {
    for (nodeId n = 0; n < max_node_id_; n++) {
      if (search_counter_.IsGenerated(n)) {
        Arc a(n, parent_[n]);
        if (ppq_.Contains(n))
          f_gen.push_back(a);
        else if (stalled_[n])
          f_stall.push_back(a);
        else
          f_exp.push_back(a);
      }
    }
  }

 private:
  G* g_;
  unsigned int max_node_id_;
  nodeId start_;

  SearchIterationCounter search_counter_;
  F_Val_PPQ ppq_;

  std::vector<Distance> g_val_;
  std::vector<nodeId> parent_;
  std::vector<bool> stalled_;

  int num_expanded_;
  int num_generated_;
  int num_settled_arcs_;
  int num_stalled_;
  double search_time_;

  // Generates a node if it hasn't been generated before.
  void GenerateNode(nodeId node) {
    if (search_counter_.IsGenerated(node))
      return;

    num_generated_ ++;
    search_counter_.GenerateNode(node);
    parent_[node] = kNonNode;
    g_val_[node] = kMaxDistance;
    stalled_[node] = false;
  }
};



#endif /* APPS_SUBGOALGRAPH_UTILS_DIJKSTRA_H_ */
