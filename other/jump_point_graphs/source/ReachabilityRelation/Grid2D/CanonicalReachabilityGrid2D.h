/*
 * CanonicalFreespaceReachabilityGrid2D.h
 *
 *  Created on: Nov 9, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_REACHABILITYRELATION_GRID2D_CANONICALREACHABILITYGRID2D_H_
#define APPS_SUBGOALGRAPH_REACHABILITYRELATION_GRID2D_CANONICALREACHABILITYGRID2D_H_

#include "CommonHeuristics.h"
#include "GridReachabilityRelation.h"

#include <vector>
#include <cmath>

template<class Grid = GPPCGrid, class S = SubgoalIdMapper>
class CanonicalReachabilityGrid2D : public GridClearanceReachabilityRelation<Grid,S> {
 public:
  using GridClearanceReachabilityRelation<Grid,S>::existing_subgoals_;
  using GridClearanceReachabilityRelation<Grid,S>::existing_subgoal_distances_;
  using GridClearanceReachabilityRelation<Grid,S>::g_;
  using GridClearanceReachabilityRelation<Grid,S>::s_;

  CanonicalReachabilityGrid2D(Grid* g, S* s, bool use_double_clearance)
      : GridClearanceReachabilityRelation<Grid,S>(g,s,use_double_clearance) {
  }
  ~CanonicalReachabilityGrid2D() {}

  double IdentifySubgoals() {
    Grid2DSubgoalGenerator sg_gen;
    return sg_gen.AddCorners(g_, s_);
  }

  void RConnect(nodeId start, bool can_identify_superset = false) {
    this->SafeFreespaceReachabilityConnect(start);
  }

  void RRefine(nodeId start, nodeId goal, std::vector<nodeId> & path,
               bool append = false) {
    GetCanonicalFreespacePath(start, goal, path, append);
  }

  bool IsReachable(nodeId start, nodeId goal, Distance d = kMaxDistance) {
    return CheckCanonicalFreespaceReachability(start, goal);
  }

  bool GetQueryPathIfReachable(nodeId start, nodeId goal, Distance & d,
                          std::vector<nodeId> & path) {
    return this->GetDiagonalFirstPathIfExists((xyLin) start,
                                              (xyLin) goal, d, path);
  }


 private:
  // Returns true if the canonical path between two nodes is unblocked.
  bool CheckCanonicalFreespaceReachability(nodeId start, nodeId goal);

  // Returns the canonical path between two canonical-reachable nodes.
  void GetCanonicalFreespacePath(nodeId start, nodeId goal,
                                 std::vector<nodeId> & path,
                                 bool append = false);
};

// IMPLEMENTATION
template<class Grid, class S>
bool CanonicalReachabilityGrid2D<Grid, S>::CheckCanonicalFreespaceReachability(
    nodeId from, nodeId to) {

  xyLoc from_xy = g_->ToXYLoc(from);
  xyLoc to_xy = g_->ToXYLoc(to);
  int num_card_moves, num_diag_moves, num_total_moves;
  Direction2D c, d;
  g_->GetMoveDirectionDetails(from_xy, to_xy, c, d,
                          num_total_moves, num_diag_moves);

  num_card_moves = num_total_moves - num_diag_moves;
  nodeId curr = from;

  // To make canonical reachability symmetric (that is, if (s,t) \in R then
  // (t,s) \in R), we need to make sure that the canonical path from s to t
  // uses the same edges that the canonical path from t to s uses.
  // Diagonal-first canonical reachability does not have this property.
  // To ensure this property, we modify it as follows:
  // If s.x < t.x, use diagonal first then cardinal.
  // Otherwise, use cardinal first then diagonal.

  // Diagonal first
  if (from_xy.x < to_xy.x) {
    while(num_diag_moves > 0) {
      if (g_->CanMove(curr, d)) {
        curr = g_->Move(curr, d);
        num_diag_moves--;
      }
      else
        return false;
    }
  }

  // Cardinal after diagonal or cardinal first
  while(num_card_moves > 0) {
    if (g_->CanMove(curr, c)) {
      curr = g_->Move(curr, c);
      num_card_moves--;
    }
    else
      return false;
  }

  // Diagonal last
  if (from_xy.x >= to_xy.x) {
    while(num_diag_moves > 0) {
      if (g_->CanMove(curr, d)) {
        curr = g_->Move(curr, d);
        num_diag_moves--;
      }
      else
        return false;
    }
  }

  return true;
}

template<class Grid, class S>
void CanonicalReachabilityGrid2D<Grid, S>::GetCanonicalFreespacePath(
    nodeId from, nodeId to, std::vector<nodeId> & path, bool append) {
  xyLoc from_xy = g_->ToXYLoc(from);
  xyLoc to_xy = g_->ToXYLoc(to);
  int num_card_moves, num_diag_moves, num_total_moves;
  Direction2D c, d;
  g_->GetMoveDirectionDetails(from_xy, to_xy, c, d,
                          num_total_moves, num_diag_moves);
  num_card_moves = num_total_moves - num_diag_moves;

  nodeId loc = from;
  if (!append) {
    path.clear();
    path.push_back(loc);
  }

  // Same principle as CheckCanonicalFreespaceReachability.

  // Diagonal first
  if (from_xy.x < to_xy.x) {
    while (num_diag_moves > 0) {
      loc = g_->Move(loc, d);
      path.push_back(loc);
      num_diag_moves--;
    }
  }

  // Cardinal after diagonal or cardinal first
  while (num_card_moves > 0) {
    loc = g_->Move(loc, c);
    path.push_back(loc);
    num_card_moves--;
  }

  // Diagonal last
  if (from_xy.x >= to_xy.x) {
    while (num_diag_moves > 0) {
      loc = g_->Move(loc, d);
      path.push_back(loc);
      num_diag_moves--;
    }
  }
}

#endif /* APPS_SUBGOALGRAPH_REACHABILITYRELATION_GRID2D_CANONICALREACHABILITYGRID2D_H_ */
