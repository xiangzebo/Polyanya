#ifndef A_STAR_DIRECTION_EXTENDED_H
#define A_STAR_DIRECTION_EXTENDED_H

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
class AStarDirectionExtended {
 public:
  AStarDirectionExtended(G* g, H* h, GroupingMapper* gm, AvoidanceTable* avoid = NULL)
      : g_(g),
        h_(h),
        gm_(gm),
        avoid_(avoid),
//        num_nodes_(g->GetNumAllNodes() + 2),
        num_groups_(gm_->GetNumGroups()),
        search_counter_(num_groups_) {
    assert (h != NULL);

    g_val_.resize(num_groups_);
    h_val_.resize(num_groups_);
    parent_.resize(num_groups_);
    stalled_.resize(num_groups_);
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
    start_group_ = kNonNode;
    goal_group_ = kNonNode;
    ppq_.SetupQueue(num_groups_);
  }
  ~AStarDirectionExtended() {
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
  bool PathToGroupExists(groupId n) {
    return search_counter_.IsGenerated(n);
  }
  Distance GetTentativeGroupDistance(groupId n) {
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
    start_group_ = gm_->GetGroup(start);
    goal_group_ = gm_->GetGroup(goal);

    SetAsGroupRepresentative(start_);
    SetAsGroupRepresentative(goal_);
    GenerateGroup(start_group_);
    GenerateGroup(goal_group_);

    g_val_[start_group_] = 0;
    ppq_.InsertOrDecreaseKey(start_group_, FG_Cost(h_val_[start_group_], 0));
  }

  template<class ArcHead>
  void RelaxArc(nodeId curr, ArcHead & a) {
    nodeId succ = a.target;

    if (avoid_ != NULL && avoid_->CanAvoid(succ)) {
      num_pruned_arcs_++;
      return;
    }

    groupId succ_group = gm_->GetGroup(succ);
    groupId curr_group = gm_->GetGroup(curr);
    GenerateGroup(succ_group);

    Distance new_g_val = g_val_[curr_group] + g_->GetWeight(curr, a);
    if (new_g_val + 0.001 < g_val_[succ_group]) {
      SetAsGroupRepresentative(succ);
      g_val_[succ_group] = new_g_val;
      parent_[succ_group] = curr_group;
      ppq_.InsertOrDecreaseKey(
          succ_group,
          FG_Cost(g_val_[succ_group] + h_val_[succ_group], g_val_[succ_group]));
    }
    num_relaxed_arcs_++;
  }

  groupId ExpandNextAndReturnGroup() {
    groupId curr_group = ppq_.PopMin();
    nodeId curr = GetGroupRepresentative(curr_group);
    for (auto a : g_->GetSuccessors(curr))
      RelaxArc(curr, a);
    num_expanded_++;
    return curr_group;
  }

  template<class RG>
  bool CanStallNext(RG* r_graph) {
    groupId g = ppq_.GetMin();
    nodeId n = GetGroupRepresentative(g);
    for (auto a : r_graph->GetSuccessors(n)) {
      groupId gs = GetGroup(a.target);
      if (search_counter_.IsGenerated(gs))
        if (g_val_[gs] + r_graph->GetWeight(n, a) + 0.001 < g_val_[g])
          return true;
    }
    return false;
  }


  groupId StallNextAndReturnGroup() {
    num_stalled_++;
    stalled_[ppq_.GetMin()] = true;
    return ppq_.PopMin();
  }

  Distance ExtractNodePathToGroup(nodeId goal_group, std::vector<nodeId> & path,
                                  bool append = false, bool reverse = false) {
    if (!append)
      path.clear();

    int initial_size = path.size();
    if (g_val_[goal_group] < kMaxDistance) {
      groupId curr_group = goal_group;
      while (curr_group != start_group_) {
        path.push_back(GetGroupRepresentative(curr_group));
        curr_group = parent_[curr_group];
      }
      path.push_back(GetGroupRepresentative(curr_group));
    }

    if (!reverse) { // Reverse once: get the correct order.
                    // Reverse twice (or don't reverse): get the reverse order.
        std::reverse(std::next(path.begin(), initial_size), path.end());
    }
    return g_val_[goal_group];
  }

  Distance ExtractPathToNode(nodeId goal, std::vector<nodeId> & path) {
    return ExtractNodePathToGroup(gm_->GetGroup(goal), path);
  }

  Distance FindPath(nodeId start, nodeId goal, std::vector<nodeId> & path) {
    CPUTimer t;
    t.StartTimer();

    StartNewSearch(start, goal);
    while (!ppq_.IsEmpty() && g_val_[goal_group_] > ppq_.GetMinCost().f_val + 0.001)
      ExpandNextAndReturnGroup();

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
    for (groupId g = 0; g < num_groups_; g++) {
      if (search_counter_.IsGenerated(g)) {
        Arc a(
            GetGroupRepresentative(g),
            parent_[g] == kNonNode ?
                kNonNode : GetGroupRepresentative(parent_[g]));
        if (ppq_.Contains(g))
          f_gen.push_back(a);
        else if (stalled_[g])
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

//  unsigned int num_nodes_;
  unsigned int num_groups_;
  nodeId start_;
  nodeId goal_;
  nodeId start_group_;
  nodeId goal_group_;

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

  void GenerateGroup(groupId group) {
    if (search_counter_.IsGenerated(group))
      return;

    num_generated_++;
    search_counter_.GenerateNode(group);
    parent_[group] = kNonNode;
    g_val_[group] = kMaxDistance;
    stalled_[group] = false;

    if (dijkstra_ || h_ == NULL)
      h_val_[group] = 0;
    else
      h_val_[group] = h_->GetHeuristicDistance(GetGroupRepresentative(group),
                                               goal_);
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
