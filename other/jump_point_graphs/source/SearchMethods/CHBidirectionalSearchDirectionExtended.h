/*
 * CHBidirectionalSearch.h
 *
 *  Created on: Nov 8, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_SEARCHMETHODS_CHBIDIRECTIONALSEARCHDIRECTIONEXTENDED_H_
#define APPS_SUBGOALGRAPH_SEARCHMETHODS_CHBIDIRECTIONALSEARCHDIRECTIONEXTENDED_H_
#include "AStar.h"
#include "AStarDirectionExtended.h"
#include "AStarDirectionExtendedBackward.h"

// Bidirectional A* implementation that is only used for CH search at the moment.

// TODO: rename dont_push_backward_highest_level_ because it simply stalls
// highest level nodes.

// FIXME: The midpoint is a group of nodes. The representative nodes
// in the forward and backward directions fo the midpoint are not necessarily
// the same.
// Current, hacky fix: for the midpoint, have both representatives.
// Only works for pointer unpacking.

template<class FG, class FH = NullHeuristic, class BG = FG, class BH = FH>
class CHBidirectionalSearchDirectionExtended {
 public:
  CHBidirectionalSearchDirectionExtended(FG* fg, BG* bg, FH* fh, BH* bh,
                                         GroupingMapper* gm,
                                         AvoidanceTable* forward_avoid,
                                         AvoidanceTable* backward_avoid,
                                         bool use_stall_on_demand)
      : fg_(fg),
        bg_(bg),
        forward_(fg, fh, gm, forward_avoid),
//*
        backward_(bg, bh, gm, backward_avoid),
/*/
        backward_gm_(*gm),
        backward_(bg, bh, &backward_gm_, backward_avoid),
//*/
        can_stall_(use_stall_on_demand) {

    level_ = NULL;
    ResetSearchInfo();
  }

  CHBidirectionalSearchDirectionExtended(FG* fg, FH* fh, GroupingMapper* gm, bool use_stall_on_demand)
      : CHBidirectionalSearchDirectionExtended(fg, fg, fh, fh, gm, use_stall_on_demand) {
    ResetSearchInfo();
  }

  ~CHBidirectionalSearchDirectionExtended() {
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

  Distance FindPath(nodeId start, nodeId goal, std::vector<nodeId> & path) {
    CPUTimer t;
    t.StartTimer();
    StartNewSearch(start, goal);
    while (best_distance_ > forward_.GetRadius() + 0.001
        || best_distance_ > backward_.GetRadius() + 0.001) {
      StallOrExpandNext();
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

  void UpdateMid(groupId mid) {
    if (forward_.PathToGroupExists(mid) && backward_.PathToGroupExists(mid)) {
      Distance distance = forward_.GetTentativeGroupDistance(mid)
          + backward_.GetTentativeGroupDistance(mid);
      if (distance < best_distance_) {
        best_distance_ = distance;
        best_middle_group_ = mid;
      }
    }
  }

 private:

  FG* fg_;
  BG* bg_;


  AStarDirectionExtended<FG, FH> forward_;
//  GroupingMapper backward_gm_;
//*
  AStarDirectionExtendedBackward<BG, BH> backward_;
/*/
  AStarDirectionExtended<BG, BH> backward_;
//*/

  bool can_stall_;
  double search_time_;

  LevelManager* level_;

  groupId best_middle_group_;
  Distance best_distance_;
  bool forward_next_;

  void ResetSearchInfo() {
    best_middle_group_ = kNonNode;
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

  void StallOrExpandNext() {
    ChooseFrontierToExpand();
    groupId expanded_group;
    if (forward_next_) {
      if (can_stall_ && forward_.CanStallNext(bg_))
        expanded_group = forward_.StallNextAndReturnGroup();
      else
        expanded_group = forward_.ExpandNextAndReturnGroup();
    }
    else {
      if (can_stall_ && backward_.CanStallNext(fg_))
        expanded_group = backward_.StallNextAndReturnGroup();
      else
        expanded_group = backward_.ExpandNextAndReturnGroup();
    }
    UpdateMid(expanded_group);
  }

  Distance ExtractPath(nodeId start, nodeId goal, std::vector<nodeId> & path) {
    path.clear();
    if (best_distance_ < kMaxDistance) {
      // Extract the first half.
      forward_.ExtractNodePathToGroup(best_middle_group_, path);
      //path.pop_back();  // Possible duplication.
      // Extract the second half.
      backward_.ExtractNodePathToGroup(best_middle_group_, path, true, true);
    }
    return best_distance_;
  }
};

#endif /* APPS_SUBGOALGRAPH_SEARCHMETHODS_CHBIDIRECTIONALSEARCH_H_ */
