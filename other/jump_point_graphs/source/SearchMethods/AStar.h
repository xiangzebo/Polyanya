#ifndef A_STAR_H
#define A_STAR_H

#include <stddef.h>
#include <algorithm>
#include <cassert>
#include <vector>

#include "../Graphs/GraphDefinitions.h"
#include "../Utils/AvoidanceTable.h"
#include "../Utils/CPUTimer.h"
#include "../Utils/LevelManager.h"
#include "../Utils/PreallocatedPriorityQueue.h"
#include "../Utils/Statistics/QueryStatistic.h"
#include "SearchIterationCounter.h"


class NullHeuristic {
 public:
  NullHeuristic() {
  }
  ~NullHeuristic() {
  }
  Distance GetHeuristicDistance(nodeId from, nodeId to) {
    return 0;
  }
  bool IsConsistent() {return true;}
};

template<class G, class H = NullHeuristic>
class AStar {
 public:
  AStar(G* g, H* h, double w = 1.0)
      : g_(g),
        h_(h),
        w_(w),
        max_node_id_(g->GetNumAllNodes() + 2),
        search_counter_(max_node_id_) {
    assert (h != NULL);

    // Hack: Inflate the number of nodes by 2 to account for query subgoals.
    // Since this is mainly a subgoal graph code, this seems justified.
    g_val_.resize(max_node_id_);
    h_val_.resize(max_node_id_);
    parent_.resize(max_node_id_);
    stalled_.resize(max_node_id_);
    num_expanded_ = 0;
    num_stalled_ = 0;
    num_generated_ = 0;
    num_settled_arcs_ = 0;
    search_time_ = 0;
    path_time_ = 0;
    dijkstra_ = false;
    start_ = kNonNode;
    goal_ = kNonNode;
    ppq_.SetupQueue(max_node_id_);

    avoid_ = NULL;  // FIXME: Hacky add.
  }
  ~AStar() {
  }

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
      return g_val_[ppq_.GetMin()] + h_val_[ppq_.GetMin()];
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
  nodeId GetParent(nodeId n) {
    return parent_[n];
  }

  void Reset() {
    search_counter_.Reset();
    ppq_.Reset();
    num_expanded_ = 0;
    num_stalled_ = 0;
    num_generated_ = 0;
    num_settled_arcs_ = 0;
    search_time_ = 0;
    path_time_ = 0;
  }

  void StartNewSearch(nodeId start, nodeId goal) {
    assert(g_->GetNumAllNodes() <= max_node_id_);

    Reset();

    start_ = start;
    goal_ = goal; // Heuristic is determined based on this.
    GenerateNode(start);
    GenerateNode(goal);

    g_val_[start] = 0;
    ppq_.InsertOrDecreaseKey(start, FG_Cost(h_val_[start], 0));
  }

  template<class ArcHead>
  void SettleArc(nodeId n, ArcHead & a) {
    nodeId succ = a.target;

    GenerateNode(succ);
    Distance new_g_val = g_val_[n] + g_->GetWeight(n, a);
    if (new_g_val + 0.001 < g_val_[succ]) {
      g_val_[succ] = new_g_val;
      parent_[succ] = n;
      ppq_.InsertOrDecreaseKey(
          succ, FG_Cost(g_val_[succ] + h_val_[succ], g_val_[succ]));
    }
    num_settled_arcs_++;
  }

  template<class ArcHead, class B>
  void SettleArcWithoutPushingThenUpdateMid(nodeId n, ArcHead & a, B* b) {
    nodeId succ = a.target;
    GenerateNode(succ);
    Distance new_g_val = g_val_[n] + g_->GetWeight(n, a);
    if (new_g_val + 0.001 < g_val_[succ]) {
      g_val_[succ] = new_g_val;
      parent_[succ] = n;
      b->UpdateMid(succ);
    }
    num_settled_arcs_++;
  }

  nodeId ExpandNext() {
    nodeId curr = ppq_.PopMin();
    for (auto a : g_->GetSuccessors(curr))
      SettleArc(curr, a);
    num_expanded_++;
    return curr;
  }

  nodeId ExpandNextGenerateHigherLevel(LevelManager* l) {
    nodeId curr = ppq_.PopMin();
    for (auto a:g_->GetSuccessors(curr))
      if (l->IsStrictlyLowerLevel(curr, a.target))
        SettleArc(curr,a);
    num_expanded_++;
    return curr;
  }

  nodeId ExpandNextGenerateSameOrHigherLevel(LevelManager* l) {
    nodeId curr = ppq_.PopMin();
    for (auto a:g_->GetSuccessors(curr))
      if (!(l->IsStrictlyLowerLevel(a.target, curr)))
        SettleArc(curr,a);
    num_expanded_++;
    return curr;
  }

  // R-CH backward search.
  template<class B>
  nodeId ExpandNextDontPushHighestLevel(LevelManager* l, B* b) {
    nodeId curr = ppq_.PopMin();
    for (auto a:g_->GetSuccessors(curr))
      if (l->IsHighestLevel(a.target)) {
        SettleArcWithoutPushingThenUpdateMid(curr, a, b);
      }
      else
        SettleArc(curr,a);
    num_expanded_++;
    return curr;
  }

  // Heavy-R backward search
  template<class B>
  nodeId ExpandNextGenerateHigherLevelDontPushHighestLevel(LevelManager* l, B* b) {
    nodeId curr = ppq_.PopMin();
    for (auto a:g_->GetSuccessors(curr))
      // Highest level must be higher level if we dont push highest level
      // nodes into the open list.
      if (l->IsHighestLevel(a.target)) {
        SettleArcWithoutPushingThenUpdateMid(curr, a, b);
      }
      else if ((l->IsStrictlyLowerLevel(curr, a.target)))
        SettleArc(curr,a);
    num_expanded_++;
    return curr;
  }

  // Heavy-R forward search
  template<class B>
  nodeId ExpandNextDontPushSameLocalLevel(LevelManager* l, B* b) {
    nodeId curr = ppq_.PopMin();
    for (auto a:g_->GetSuccessors(curr))
      if (!l->IsHighestLevel(curr) && l->SameLevel(curr, a.target)) {
        SettleArcWithoutPushingThenUpdateMid(curr, a, b);
      }
      else if (!(l->IsStrictlyLowerLevel(a.target, curr)))
        SettleArc(curr,a);
    num_expanded_++;
    return curr;
  }

  nodeId StallNext() {
    num_stalled_++;
    stalled_[ppq_.GetMin()] = true;
    return ppq_.PopMin();
  }

  Distance ExtractPathToNode(nodeId goal, std::vector<nodeId> & path,
                             bool append = false,
                             bool reverse = false) {
    if (!append)
      path.clear();

    int initial_size = path.size();
    if (g_val_[goal] < kMaxDistance) {
      nodeId curr = goal;
      while (curr != start_) {
        path.push_back(curr);
        curr = parent_[curr];
      }
      path.push_back(curr);
    }
    if (!reverse) { // Reverse once: get the correct order.
                    // Reverse twice (or don't reverse): get the reverse order.
        std::reverse(std::next(path.begin(), initial_size), path.end());
    }
    return g_val_[goal];
  }

  // Hacky implementation that uses 'stalled_[n]' as closed list to not expand
  // nodes more than once
  void WAStar(nodeId start, nodeId goal) {
    StartNewSearch(start, goal);
    while (!ppq_.IsEmpty() && g_val_[goal] > ppq_.GetMinCost().f_val + 0.001) {
      nodeId curr = ppq_.PopMin();
      assert(!stalled_[curr]);
      stalled_[curr] = true;

      for (auto a : g_->GetSuccessors(curr)) {
        nodeId succ = a.target;
        GenerateNode(succ);

        if (stalled_[succ])
          continue;
        Distance new_g_val = g_val_[curr] + g_->GetWeight(curr, a);
        if (new_g_val + 0.001 < g_val_[succ]) {
          g_val_[succ] = new_g_val;
          parent_[succ] = curr;
          ppq_.InsertOrDecreaseKey(
              succ, FG_Cost(g_val_[succ] + h_val_[succ], g_val_[succ]));
        }
        num_settled_arcs_++;
      }
      num_expanded_++;
    }
  }

  Distance FindPath(nodeId start, nodeId goal, std::vector<nodeId> & path) {
    CPUTimer t;
    t.StartTimer();

    if (h_ == NULL || (w_ < 1.001 && h_->IsConsistent())) {
      StartNewSearch(start, goal);
      while (!ppq_.IsEmpty() && g_val_[goal] > ppq_.GetMinCost().f_val + 0.001)
        ExpandNext();
    }
    else
      WAStar(start, goal);

    Distance d = ExtractPathToNode(goal, path);
    path_time_ = t.EndTimer();
    return d;
  }

  void UseDijkstra() {
    dijkstra_ = true;
  }
  void UseAstar() {
    dijkstra_ = false;
  }

  void AddStatistics(QueryStatistic & s, bool backward = false) const {
    s.num_expanded += num_expanded_;
    s.num_stalled += num_stalled_;
    s.num_generated += num_generated_;
    s.num_relaxed_arcs += num_settled_arcs_;
    s.num_percolations += ppq_.GetNumPercolations();
    s.num_update_key += ppq_.GetNumUpdateKey();
    s.search_time += search_time_ + path_time_;
    if (backward) {
      s.num_backward_expanded = num_expanded_;
      s.num_backward_stalled = num_stalled_;
      s.num_backward_generated += num_generated_;
      s.num_backward_relaxed_arcs += num_settled_arcs_;
      //s.num_backward_pruned_arcs += num_pruned_arcs_;
    }
//    s.finalize_time += path_time_;
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
        else if (stalled_[n] && w_ <1.001)  // Hack: For weighted A*, stalled
                                            // flags are used as closed list.
          f_stall.push_back(a);
        else
          f_exp.push_back(a);
      }
    }
  }

 private:
  G* g_;
  H* h_;
  FG_Val_PPQ_Higher_G ppq_;

  double w_;

  unsigned int max_node_id_;
  nodeId start_;
  nodeId goal_;

  bool dijkstra_;
  SearchIterationCounter search_counter_;

  std::vector<Distance> g_val_;
  std::vector<Distance> h_val_;
  std::vector<nodeId> parent_;
  std::vector<bool> stalled_; // TODO: type. stalled not pushed etc.

  AvoidanceTable* avoid_;


  int num_expanded_;
  int num_stalled_;
  int num_generated_;
  int num_settled_arcs_;
  double search_time_;
  double path_time_;


  void GenerateNode(nodeId node) {
    if (search_counter_.IsGenerated(node))
      return;

    num_generated_++;
    search_counter_.GenerateNode(node);
    parent_[node] = kNonNode;
    g_val_[node] = kMaxDistance;
    stalled_[node] = false;

    if (dijkstra_ || h_ == NULL)
      h_val_[node] = 0;
    else
      h_val_[node] = w_*h_->GetHeuristicDistance(node, goal_);
  }
};

#endif
