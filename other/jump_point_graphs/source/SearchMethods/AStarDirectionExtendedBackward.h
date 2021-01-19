#ifndef A_STAR_DIRECTION_EXTENDED_BACKWARD_H
#define A_STAR_DIRECTION_EXTENDED_BACKWARD_H

#include <stddef.h>
#include <algorithm>
#include <cassert>
#include <vector>

#include "../Graphs/GraphDefinitions.h"
#include "../Utils/AvoidanceTable.h"
#include "../Utils/CPUTimer.h"
#include "../Utils/GroupingMapper.h"
#include "../Utils/LevelManager.h"
#include "../Utils/PreallocatedPriorityQueue.h"
#include "../Utils/Statistics/QueryStatistic.h"
#include "AStar.h"
#include "SearchIterationCounter.h"

// TODO: M: Mapper: Can map a group of nodes into a single one that will share g-values.
template<class G,  class H = NullHeuristic>
class AStarDirectionExtendedBackward {
 public:
  AStarDirectionExtendedBackward(G* g, H* h, GroupingMapper* gm, AvoidanceTable* avoid = NULL)
      : g_(g),
        h_(h),
        gm_(gm),
        avoid_(avoid),
        num_nodes_(g->GetNumAllNodes() + 2),
        num_groups_(gm_->GetNumGroups()),
        search_counter_(num_nodes_) {
    assert (h != NULL);

    g_val_.resize(num_nodes_);
    h_val_.resize(num_nodes_);
    parent_.resize(num_nodes_);
    stalled_.resize(num_nodes_);
    representative_.resize(num_groups_); // FIXME: For the bidirectional search
    for (groupId g = 0; g < num_groups_; g++) {
      representative_[g] = gm_->GetRepresentative(g);
    }

    num_expanded_ = 0;
    num_stalled_ = 0;
    num_generated_ = 0;
    num_relaxed_arcs_ = 0;
    num_pruned_arcs_ = 0;
    search_time_ = 0;
    path_time_ = 0;
    dijkstra_ = false;
    start_ = kNonNode;
    goal_ = kNonNode;
    ppq_.SetupQueue(num_nodes_);
  }
  ~AStarDirectionExtendedBackward() {
  }

  bool IsFinished() {
    return ppq_.IsEmpty();
  }
  Distance GetRadius() {
    if (ppq_.IsEmpty())
      return kMaxDistance;
    else
      return g_val_[ppq_.GetMin()] + h_val_[ppq_.GetMin()];
  }
  bool PathToGroupExists(groupId g) {
    return search_counter_.IsGenerated(GetGroupRepresentative(g));
  }
  Distance GetTentativeGroupDistance(groupId g) {
    nodeId n = GetGroupRepresentative(g);
    if (search_counter_.IsGenerated(n))
      return g_val_[n];
    else
      return kMaxDistance;
  }

  void StartNewSearch(nodeId start, nodeId goal) {
   // assert(g_->GetNumAllNodes() <= num_nodes_);
    search_counter_.Reset();
    ppq_.Reset();
    num_expanded_ = 0;
    num_stalled_ = 0;
    num_generated_ = 0;
    num_relaxed_arcs_ = 0;
    num_pruned_arcs_ = 0;
    search_time_ = 0;
    path_time_ = 0;

    start_ = start;
    goal_ = goal; // Heuristic is determined based on this.

    SetAsGroupRepresentative(start_);
    SetAsGroupRepresentative(goal_);
    GenerateNode(start_);
    GenerateNode(goal_);

    g_val_[start_] = 0;
    ppq_.InsertOrDecreaseKey(start_, FG_Cost(h_val_[start_], 0));
  }

  template<class ArcHead>
  void RelaxArc(nodeId curr, ArcHead & a) {
    nodeId succ = a.target;

    if (avoid_ != NULL && avoid_->CanAvoid(succ)) {
      num_pruned_arcs_++;
      return;
    }

    Distance new_g_val = g_val_[curr] + g_->GetWeight(curr, a);
    nodeId group_rep = GetGroupRepresentative(GetGroup(succ));
    if (!search_counter_.IsGenerated(group_rep)
        || new_g_val - 0.001 <= g_val_[group_rep]) {
      GenerateNode(succ);
      g_val_[succ] = new_g_val;
      parent_[succ] = curr;
      SetAsGroupRepresentative(succ);
      ppq_.InsertOrDecreaseKey(
          succ, FG_Cost(g_val_[succ] + h_val_[succ], g_val_[succ]));
    }
/*
    if (new_g_val + 0.001 < g_val_[succ]) {
      g_val_[succ] = new_g_val;
      parent_[succ] = curr;

      nodeId group_rep = GetGroupRepresentative(GetGroup(succ));
      if (!search_counter_.IsGenerated(group_rep) ||
          g_val_[succ] - 0.001 <= g_val_[group_rep]) {
        SetAsGroupRepresentative(succ);
        ppq_.InsertOrDecreaseKey(
            succ,
            FG_Cost(g_val_[succ] + h_val_[succ], g_val_[succ]));
      }
    }
*/
    num_relaxed_arcs_++;
  }

  groupId ExpandNextAndReturnGroup() {
    nodeId curr = ppq_.PopMin();
    for (auto a : g_->GetSuccessors(curr))
      RelaxArc(curr, a);
    num_expanded_++;
    return GetGroup(curr);
  }

  template<class RG>
  bool CanStallNext(RG* r_graph) {
    nodeId n = ppq_.GetMin();
    for (auto a : r_graph->GetSuccessors(n)) {
      if (search_counter_.IsGenerated(a.target))
        if (g_val_[a.target] + r_graph->GetWeight(n, a) + 0.001 < g_val_[n])
          return true;
    }
    return false;
  }

  groupId StallNextAndReturnGroup() {
    num_stalled_++;
    stalled_[ppq_.GetMin()] = true;
    return GetGroup(ppq_.PopMin());
  }

  Distance ExtractNodePathToGroup(nodeId goal_group, std::vector<nodeId> & path,
                                  bool append = false, bool reverse = false) {
    if (!append)
      path.clear();

    nodeId goal_node = GetGroupRepresentative(goal_group);

    int initial_size = path.size();
    if (g_val_[goal_node] < kMaxDistance) {
      groupId curr_node = goal_node;
      while (curr_node != start_) {
        path.push_back(curr_node);
        curr_node = parent_[curr_node];
      }
      path.push_back(curr_node);
    }

    if (!reverse) { // Reverse once: get the correct order.
                    // Reverse twice (or don't reverse): get the reverse order.
        std::reverse(std::next(path.begin(), initial_size), path.end());
    }
    return g_val_[goal_node];
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
    s.num_relaxed_arcs += num_relaxed_arcs_;
    s.num_pruned_arcs += num_pruned_arcs_;
    s.num_percolations += ppq_.GetNumPercolations();
    s.num_update_key += ppq_.GetNumUpdateKey();
    s.search_time += search_time_ + path_time_;
    if (backward) {
      s.num_backward_expanded = num_expanded_;
      s.num_backward_stalled = num_stalled_;
      s.num_backward_generated += num_generated_;
      s.num_backward_relaxed_arcs += num_relaxed_arcs_;
      s.num_backward_pruned_arcs += num_pruned_arcs_;
    }
  }
  void GetSearchTree(std::vector<Arc> & f_exp,
                     std::vector<Arc> & f_stall,
                     std::vector<Arc> & f_gen,
                     std::vector<Arc> & b_exp,
                     std::vector<Arc> & b_stall,
                     std::vector<Arc> & b_gen) {
    for (nodeId n = 0; n < num_nodes_; n++) {
      if (search_counter_.IsGenerated(n)) {
        Arc a(n, parent_[n] == kNonNode ? kNonNode : parent_[n]);
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
  H* h_;
  GroupingMapper* gm_;
  AvoidanceTable* avoid_;
  FG_Val_PPQ_Higher_G ppq_;

  unsigned int num_nodes_;
  unsigned int num_groups_;
  nodeId start_;
  nodeId goal_;

  bool dijkstra_;
  SearchIterationCounter search_counter_;

  std::vector<nodeId> representative_;
  std::vector<Distance> g_val_;
  std::vector<Distance> h_val_;
  std::vector<nodeId> parent_;
  std::vector<bool> stalled_; // TODO: type. stalled not pushed etc.

  int num_expanded_;
  int num_stalled_;
  int num_generated_;
  int num_relaxed_arcs_;
  int num_pruned_arcs_;
  double search_time_;
  double path_time_;

  void GenerateNode(nodeId n) {
    if (search_counter_.IsGenerated(n))
      return;

    num_generated_++;
    search_counter_.GenerateNode(n);
    parent_[n] = kNonNode;
    g_val_[n] = kMaxDistance;
    stalled_[n] = false;

    if (dijkstra_ || h_ == NULL)
      h_val_[n] = 0;
    else
      h_val_[n] = h_->GetHeuristicDistance(n, goal_);
  }

  void SetAsGroupRepresentative(nodeId n) {
    representative_[gm_->GetGroup(n)] = n;
  }
  nodeId GetGroupRepresentative(groupId g) const {
    return representative_[g];
  }
  groupId GetGroup(nodeId n) const {
    return gm_->GetGroup(n);
  }
};

#endif
