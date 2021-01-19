/*
 * OctileReachability.h
 *
 *  Created on: Oct 22, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_SUBGOALGRAPH_GRID2D_OCTILEREACHABILITY_H_
#define APPS_SUBGOALGRAPH_SUBGOALGRAPH_GRID2D_OCTILEREACHABILITY_H_

#include "CommonHeuristics.h"
#include "GridReachabilityRelation.h"

#include <vector>
#include <cmath>

template<class Grid = GPPCGrid, class S = SubgoalIdMapper>
class FreespaceReachabilityGrid2D : public GridClearanceReachabilityRelation<Grid,S> {
 public:
  using GridClearanceReachabilityRelation<Grid,S>::existing_subgoals_;
  using GridClearanceReachabilityRelation<Grid,S>::existing_subgoal_distances_;
  using GridClearanceReachabilityRelation<Grid,S>::g_;
  using GridClearanceReachabilityRelation<Grid,S>::s_;

  FreespaceReachabilityGrid2D(Grid* g, S* s, bool use_double_clearance)
      : GridClearanceReachabilityRelation<Grid,S>(g,s, use_double_clearance) {}
  ~FreespaceReachabilityGrid2D() {}

  double IdentifySubgoals() {
    Grid2DSubgoalGenerator sg_gen;
    return sg_gen.AddCorners(g_, s_);
  }

  void RConnect(nodeId start, bool can_identify_superset = false) {
    this->SafeFreespaceReachabilityConnect(start);
  }
  void RRefine(nodeId start, nodeId goal, std::vector<nodeId> & path,
               bool append = false) {
    CheckFreespaceReachability(start, goal, path, true, append);
  }

  bool IsReachable(nodeId start, nodeId goal, Distance d = kMaxDistance) {
    if (d < kMaxDistance)
      return fabs(OctileDistance(g_->ToXYLoc(start), g_->ToXYLoc(goal)) - d)
          < kEpsDistance;

    std::vector<nodeId> dummy_path;
    return CheckFreespaceReachability(start, goal, dummy_path, false, false);
  }

  bool GetQueryPathIfReachable(nodeId start, nodeId goal, Distance & d,
                          std::vector<nodeId> & path) {
    return this->GetDiagonalFirstPathIfExists((xyLin) start,
                                              (xyLin) goal, d, path);
  }

 private:
  bool CheckFreespaceReachability(nodeId start, nodeId goal,
                                 std::vector<nodeId> & path,
                                 bool extract_path,
                                 bool append);

  // Returns true if the canonical path between two nodes is unblocked.
  bool CheckCanonicalFreespaceReachability(nodeId start, nodeId goal);

  // Returns the canonical path between two canonical-reachable nodes.
  void GetCanonicalFreespacePath(nodeId start, nodeId goal,
                                 std::vector<nodeId> & path,
                                 bool append = false);
};

// IMPLEMENTATION
template<class Grid, class S>
bool FreespaceReachabilityGrid2D<Grid, S>::CheckFreespaceReachability(
    nodeId from, nodeId to, std::vector<nodeId> & path, bool extract_path,
    bool append) {

  int num_card_moves, num_diag_moves, num_total_moves;
  Direction2D c, d;
  g_->GetMoveDirectionDetails(g_->ToXYLoc(from), g_->ToXYLoc(to), c, d,
                          num_total_moves, num_diag_moves);
  num_card_moves = num_total_moves - num_diag_moves;

  int *diag_count = new int[num_total_moves + 1];
  diag_count[0] = 0;
  for (int i = 1; i <= num_total_moves; i++)
    diag_count[i] = -1;

  nodeId loc = from;
  int i = 0;
  while (i < num_total_moves) {
    // Move cardinally first if possible.
    if (i - diag_count[i] < num_card_moves && diag_count[i + 1] < diag_count[i]
        && g_->CanMove(loc, c)) {
      diag_count[i + 1] = diag_count[i];
      i++;
      loc = g_->Move(loc, c);
    }

    //else, if we can move diagonally, do it
    else if (diag_count[i] < num_diag_moves
        && diag_count[i + 1] <= diag_count[i] && g_->CanMove(loc, d)) {
      diag_count[i + 1] = diag_count[i] + 1;
      i++;
      loc = g_->Move(loc, d);
    }

    //else, backtrack
    else {
      if (i == 0) {  // cannot backtrack
        delete[] diag_count;
        return false;
      }

      i--;

      if (diag_count[i] == diag_count[i + 1])
        loc = g_->Move(loc, c + 4);
      else
        loc = g_->Move(loc, d + 4);
    }
  }

  if (extract_path) {
    loc = from;
    if (!append) {
      path.clear();
      path.push_back(loc);
    }
    for (int i = 1; i <= num_total_moves; i++) {
      if (diag_count[i] > diag_count[i - 1])  // Make a diagonal move
        loc = g_->Move(loc, d);
      else
        loc = g_->Move(loc, c);

      path.push_back(loc);
    }
  }

  delete[] diag_count;
  return true;
}

#endif /* APPS_SUBGOALGRAPH_SUBGOALGRAPH_GRID2D_OCTILEREACHABILITY_H_ */
