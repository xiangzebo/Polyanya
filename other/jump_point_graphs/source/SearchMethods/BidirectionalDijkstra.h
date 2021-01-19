/*
 * BidirectionalDijkstra.h
 *
 *  Created on: Oct 28, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_UTILS_BIDIRECTIONALDIJKSTRA_H_
#define APPS_SUBGOALGRAPH_UTILS_BIDIRECTIONALDIJKSTRA_H_
#include "BidirectionalDijkstra.h"

#include "../Utils/CPUTimer.h"
#include "Dijkstra.h"

template<class FG, class BG = FG>
class BidirectionalDijkstra {
 public:
  BidirectionalDijkstra(FG* f, BG* b, bool ch_search = false,
                        bool stall_on_demand = false)
      : fg_(f),
        bg_(b),
        forward_(f),
        backward_(b),
        ch_search_(ch_search),
        can_stall_(stall_on_demand){
    dont_stall_highest_level_ = false;
    dont_push_backward_highest_level_ = false;

    level_ = NULL;
    ResetSearchInfo();
  }
  ~BidirectionalDijkstra() {
  }

  void AddLevelInformation(LevelManager* level) {
    level_ = level;
    level_->AddQueryNodeLevels();
  }
  void SetDontStallHighestLevel() {
    assert(level_);
    dont_stall_highest_level_ = true;
  }
  void SetBackwardDontPushHighestLevelOrSameLevel() {
    assert(level_);
    dont_push_backward_highest_level_ = true;
  }
  void SetStallOnDemand(bool can_stall) {
    can_stall_ = can_stall;
  }

  void SetRCHSearch(LevelManager* level) {
    assert(
        ch_search_
            || "Bidirectional Dijkstra cannot run RCH search if CH search is not set");
    AddLevelInformation(level);
    SetDontStallHighestLevel();
    SetBackwardDontPushHighestLevelOrSameLevel();
  }

  template <class S, class  RG>
  bool CanStall(nodeId n, S* f_search, RG* r_graph) {
    if (dont_stall_highest_level_ && level_->IsHighestLevel(n))
      return false;
    for (auto a: r_graph->GetSuccessors(n))
      if (f_search->PathToExists(a.target))
        if (f_search->GetTentativeDistance(a.target) + r_graph->GetWeight(n, a)
            + 0.001 < f_search->GetTentativeDistance(n))
          return true;
    return false;
  }

  bool LocalSameLevelParentInForwardDirection() {
    nodeId n = forward_.GetNext();
    nodeId p = forward_.GetParent(n);
    return (p != kNonNode) && level_->SameLevel(n, p)
        && !level_->IsHighestLevel(n);
  }

  void NLevelStallOrExpandNext() {
    ChooseFrontierToExpand();
    nodeId expanded;
    if (forward_next_) {
      if (LocalSameLevelParentInForwardDirection())
        expanded = forward_.StallNext();
//      else
//        if (can_stall_ && CanStall(forward_.GetNext(), &forward_, bg_))
//        expanded = forward_.StallNext();
      else
        expanded = forward_.ExpandNext();
    }
    else {
//      if (can_stall_ && CanStall(backward_.GetNext(), &backward_, fg_))
//        expanded = backward_.StallNext();
//      else
        if (
            //dont_push_backward_highest_level_ &&
            level_->IsHighestLevel(backward_.GetNext()))
        expanded = backward_.StallNext();
      else
        expanded = backward_.ExpandNextGenerateHigherLevel(level_);
    }
    UpdateMid(expanded);
  }

  void StallOrExpandNext() {
    ChooseFrontierToExpand();
    nodeId expanded;
    if (forward_next_) {
      if (can_stall_ && CanStall(forward_.GetNext(), &forward_, bg_))
        expanded = forward_.StallNext();
      else
        expanded = forward_.ExpandNext();
    }
    else {
      if (can_stall_ && CanStall(backward_.GetNext(), &backward_, fg_))
        expanded = backward_.StallNext();
      else if (dont_push_backward_highest_level_
          && level_->IsHighestLevel(backward_.GetNext()))
        expanded = backward_.StallNext();
      else
        expanded = backward_.ExpandNext();
    }
    UpdateMid(expanded);
  }

  void ExpandNext() {
    ChooseFrontierToExpand();
    nodeId mid_node;
    bool path_found = false;

    if (forward_next_) {
      mid_node = forward_.ExpandNext();
      path_found = backward_.PathToExists(mid_node);
    } else {
      mid_node = backward_.ExpandNext();
      path_found = forward_.PathToExists(mid_node);
    }

    if (path_found) {
      Distance distance = forward_.GetTentativeDistance(mid_node)
          + backward_.GetTentativeDistance(mid_node);
      if (distance < best_distance_) {
        best_distance_ = distance;
        best_middle_node_ = mid_node;
      }
    }
  }

  // BOTH BIDIRECTIONAL DIJKSTRA AND BIDIRECTIONAL CH SEARCH

  void ResetSearchInfo() {
    best_middle_node_ = kNonNode;
    best_distance_ = kMaxDistance;
    forward_next_ = false;
    search_time_ = 0;
  }

  void ChooseFrontierToExpand() {
    // Decide on which frontier to expand.
    //forward_next_ = !forward_next_; // Alternate.
    forward_next_ = forward_.GetRadius() < backward_.GetRadius();

    if (forward_.IsFinished())
      forward_next_ = false;
    else if (backward_.IsFinished())
      forward_next_ = true;
  }

  void StartNewSearch(nodeId start, nodeId goal) {
    forward_.Reset();
    forward_.AddStart(start);
    backward_.Reset();
    backward_.AddStart(goal);
    ResetSearchInfo();
    search_time_ = 0;
  }

  void ResetStatistics() {
    forward_.ResetStatistics();
    backward_.ResetStatistics();
  }

  Distance ExtractPath(nodeId start, nodeId goal, std::vector<nodeId> & path) {
    path.clear();
    if (best_distance_ < kMaxDistance) {
      // Extract the first half.
      forward_.ExtractPathToNode(best_middle_node_, path);
      path.pop_back();
      // Extract the second half.
      std::vector<nodeId> rest;
      backward_.ExtractPathToNode(best_middle_node_, rest);
      // Append.
      std::reverse(rest.begin(), rest.end());
      path.insert(path.end(), rest.begin(), rest.end());
    }
    return best_distance_;
  }

  void AddStatistics(QueryStatistic & s) const {
    bool backward = true;
    forward_.AddStatistics(s);
    backward_.AddStatistics(s, backward);
    s.search_time += search_time_;
  }

  // BIDIRECTIONAL DIJKSTRA ONLY

  Distance FindPath(nodeId start, nodeId goal, std::vector<nodeId> & path) {
    CPUTimer t;
    t.StartTimer();
    Distance d;
    if (ch_search_)
      d = FindCHPath(start, goal, path);
    else
      d = FindRegularPath(start, goal, path);
    search_time_ = t.EndTimer();
    return d;
  }

  bool CanStop() {
    return (forward_.IsFinished() && backward_.IsFinished())
        || (forward_.GetRadius() + backward_.GetRadius() >= best_distance_);
  }

  Distance FindRegularPath(nodeId start, nodeId goal,
                           std::vector<nodeId> & path) {
    StartNewSearch(start, goal);
    while (!CanStop())
      ExpandNext();
    return ExtractPath(start, goal, path);
  }
  Distance FindCHPath(nodeId start, nodeId goal, std::vector<nodeId> & path) {
    StartNewSearch(start, goal);
    while (GetBestDistance() > GetForward()->GetRadius() + 0.001
        || GetBestDistance() > GetBackward()->GetRadius() + 0.001) {
      StallOrExpandNext();
    }
    return ExtractPath(start, goal, path);
  }

  // Longer path check
  Dijkstra<FG>* GetForward() {
    return &forward_;
  }
  Dijkstra<BG>* GetBackward() {
    return &backward_;
  }

  Distance GetBestDistance() {
    return best_distance_;
  }
  Distance GetSumOfRadii() {
    return forward_.GetRadius() + backward_.GetRadius();
  }

  void GetSearchTree(std::vector<Arc> & f_exp,
                     std::vector<Arc> & f_stall,
                     std::vector<Arc> & f_gen,
                     std::vector<Arc> & b_exp,
                     std::vector<Arc> & b_stall,
                     std::vector<Arc> & b_gen) {
    forward_.GetSearchTree(f_exp, f_stall, f_gen, b_exp, b_stall, b_gen);
    backward_.GetSearchTree(b_exp, b_stall, b_gen, f_exp, f_stall, f_gen);
  }

 private:
  FG* fg_;
  BG* bg_;
  Dijkstra<FG> forward_;
  Dijkstra<BG> backward_;

  double search_time_;
  bool ch_search_;
  bool can_stall_;

  bool dont_stall_highest_level_;
  bool dont_push_backward_highest_level_;

  LevelManager* level_;

  nodeId best_middle_node_;
  Distance best_distance_;
  bool forward_next_;


  void UpdateMid(nodeId mid) {
    if (forward_.PathToExists(mid) && backward_.PathToExists(mid)) {
      Distance distance = forward_.GetTentativeDistance(mid)
                        + backward_.GetTentativeDistance(mid);
      if (distance < best_distance_) {
        best_distance_ = distance;
        best_middle_node_ = mid;
      }
    }
  }
};

#endif /* APPS_SUBGOALGRAPH_UTILS_BIDIRECTIONALDIJKSTRA_H_ */
