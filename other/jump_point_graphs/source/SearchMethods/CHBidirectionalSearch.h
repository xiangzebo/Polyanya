/*
 * CHBidirectionalSearch.h
 *
 *  Created on: Nov 8, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_SEARCHMETHODS_CHBIDIRECTIONALSEARCH_H_
#define APPS_SUBGOALGRAPH_SEARCHMETHODS_CHBIDIRECTIONALSEARCH_H_
#include "AStar.h"

// Bidirectional A* implementation that is only used for CH search at the moment.

// TODO: rename dont_push_backward_highest_level_ because it simply stalls
// highest level nodes.

template<class FG, class FH = NullHeuristic, class BG = FG, class BH = FH>
class CHBidirectionalSearch {
 public:
  CHBidirectionalSearch(FG* fg, BG* bg, FH* fh, BH* bh, bool use_stall_on_demand)
      : fg_(fg),
        bg_(bg),
        forward_(fg, fh),
        backward_(bg, bh),
        can_stall_(use_stall_on_demand) {
    dont_stall_highest_level_ = false;
    dont_push_backward_highest_level_ = false;
    use_heavy_r_contraction_search_ = false;

    level_ = NULL;
    ResetSearchInfo();
  }

  CHBidirectionalSearch(FG* fg, FH* fh, bool use_stall_on_demand)
      : CHBidirectionalSearch(fg, fg, fh, fh, use_stall_on_demand) {
    ResetSearchInfo();
  }

  ~CHBidirectionalSearch() {
  }

  void SetAStar(bool use_astar) {
    if (use_astar) {
      forward_.UseAstar();
      backward_.UseAstar();
    }
    else {
      forward_.UseDijkstra();
      backward_.UseDijkstra();
    }
  }

  void AddLevelInformation(LevelManager* level) {
    level_ = level;
    level_->AddQueryNodeLevels();
  }

  void SetDontStallHighestLevel(bool dont_stall_highest_level = true) {
    assert(level_);
    dont_stall_highest_level_ = dont_stall_highest_level;
  }

  void SetBackwardDontPushHighestLevelOrSameLevel(
      bool dont_push_backward_highest_level = true) {
    assert(level_);
    dont_push_backward_highest_level_ = dont_push_backward_highest_level;
  }

  void SetHeavyRContractionSearch(bool use_heavy_r_contraction_search = true) {
    assert(level_);
    use_heavy_r_contraction_search_ = use_heavy_r_contraction_search;
  }

  void SetRCHSearch(LevelManager* level) {
    AddLevelInformation(level);
    SetDontStallHighestLevel();
    SetBackwardDontPushHighestLevelOrSameLevel();
  }

  void SetNLevelSearch(LevelManager* level) {
    AddLevelInformation(level);
    SetDontStallHighestLevel();
    SetBackwardDontPushHighestLevelOrSameLevel();
    SetHeavyRContractionSearch();
  }

  Distance FindPath(nodeId start, nodeId goal, std::vector<nodeId> & path) {
    CPUTimer t;
    t.StartTimer();
    StartNewSearch(start, goal);
    if (use_heavy_r_contraction_search_) {
      while (best_distance_ > forward_.GetRadius() + 0.001
          || best_distance_ > backward_.GetRadius() + 0.001) {
        NLevelStallOrExpandNext();
      }
    }
    else {
      while (best_distance_ > forward_.GetRadius() + 0.001
          || best_distance_ > backward_.GetRadius() + 0.001) {
        StallOrExpandNext();
      }
    }
    Distance d = ExtractPath(start, goal, path);
    search_time_ = t.EndTimer();
    return d;
  }

  void AddStatistics(QueryStatistic & s) const {
    bool backward = true;
    forward_.AddStatistics(s);
    backward_.AddStatistics(s, backward);
    s.search_time += search_time_;
  }

  void GetSearchTree(std::vector<Arc> & f_exp, std::vector<Arc> & f_stall,
                     std::vector<Arc> & f_gen, std::vector<Arc> & b_exp,
                     std::vector<Arc> & b_stall, std::vector<Arc> & b_gen) {
    forward_.GetSearchTree(f_exp, f_stall, f_gen, b_exp, b_stall, b_gen);
    backward_.GetSearchTree(b_exp, b_stall, b_gen, f_exp, f_stall, f_gen);
  }

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

 private:

  FG* fg_;
  BG* bg_;
  AStar<FG, FH> forward_;
  AStar<BG, BH> backward_;

  bool can_stall_;
  double search_time_;

  bool dont_stall_highest_level_;
  bool dont_push_backward_highest_level_;
  bool use_heavy_r_contraction_search_;

  LevelManager* level_;

  nodeId best_middle_node_;
  Distance best_distance_;
  bool forward_next_;

  void ResetSearchInfo() {
    best_middle_node_ = kNonNode;
    best_distance_ = kMaxDistance;
    forward_next_ = false;
    search_time_ = 0;
  }

  void ChooseFrontierToExpand() {
    forward_next_ = forward_.GetRadius() < backward_.GetRadius();
    if (forward_.IsFinished())
      forward_next_ = false;
    else if (backward_.IsFinished())
      forward_next_ = true;
  }

  void StartNewSearch(nodeId start, nodeId goal) {
    forward_.StartNewSearch(start, goal);
    backward_.StartNewSearch(goal, start);
    ResetSearchInfo();
    search_time_ = 0;
  }

  // Takes as input a search frontier and the graph in the reverse direction
  // for stall checks.
  template<class S, class RG>
  bool CanStall(nodeId n, S* f_search, RG* r_graph) {
    if (dont_stall_highest_level_ && level_->IsHighestLevel(n))
      return false;
    for (auto a : r_graph->GetSuccessors(n))
      if (f_search->PathToExists(a.target))
        if (f_search->GetTentativeDistance(a.target) + r_graph->GetWeight(n, a)
            - 0.001 < f_search->GetTentativeDistance(n))
          return true;
    return false;
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
//      else if (dont_push_backward_highest_level_
//          && level_->IsHighestLevel(backward_.GetNext()))
//      expanded = backward_.StallNext();
      else if (dont_push_backward_highest_level_)
        expanded = backward_.ExpandNextDontPushHighestLevel(level_, this);
      else
        expanded = backward_.ExpandNext();
    }
    UpdateMid(expanded);
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
      if (can_stall_ && CanStall(forward_.GetNext(), &forward_, bg_))
        expanded = forward_.StallNext();
      else
        expanded = forward_.ExpandNextDontPushSameLocalLevel(level_, this);
    }
    else {
      if (can_stall_ && CanStall(backward_.GetNext(), &backward_, fg_))
        expanded = backward_.StallNext();
      else
        expanded = backward_.ExpandNextGenerateHigherLevelDontPushHighestLevel(
            level_, this);
    }
    UpdateMid(expanded);
  }

  Distance ExtractPath(nodeId start, nodeId goal, std::vector<nodeId> & path) {
    path.clear();
    if (best_distance_ < kMaxDistance) {
      // Extract the first half.
      forward_.ExtractPathToNode(best_middle_node_, path);
      path.pop_back();
      // Extract the second half.
      backward_.ExtractPathToNode(best_middle_node_, path, true, true);
      /*
      std::vector<nodeId> rest;
      backward_.ExtractPathToNode(best_middle_node_, rest);
      // Append.
      std::reverse(rest.begin(), rest.end());
      path.insert(path.end(), rest.begin(), rest.end());
      */
    }
    return best_distance_;
  }
};

#endif /* APPS_SUBGOALGRAPH_SEARCHMETHODS_CHBIDIRECTIONALSEARCH_H_ */
