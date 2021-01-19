/*
 * GridReachabilityRelation.inc
 *
 *  Created on: Nov 9, 2018
 *      Author: idm-lab
 */
#include "../../SubgoalGraph/Grid2D/Grid2DSubgoalGenerator.h"

template<class Grid, class S>
GridClearanceReachabilityRelation<Grid, S>::GridClearanceReachabilityRelation(
    Grid* g, S* s, bool use_double_clearance)
    : g_(g),
      s_(s),
      use_double_clearance_(use_double_clearance),
      card_cl_(g,s),
      diag_cl_(g,s),
      r_card_cl_(g,s),
      r_diag_cl_(g, s) {
}

template<class Grid, class S>
bool GridClearanceReachabilityRelation<Grid, S>::GetDiagonalFirstPathIfExists(
    xyLin from, xyLin to, Distance & dist,
    std::vector<xyLin> & path) {

  // Direction details.
  xyLoc from_xy = g_->ToXYLoc(from);
  xyLoc to_xy = g_->ToXYLoc(to);
  int num_card_moves, num_diag_moves, num_total_moves;
  Direction2D c, d;
  g_->GetMoveDirectionDetails(from_xy, to_xy, c, d,
                          num_total_moves, num_diag_moves);
  num_card_moves = num_total_moves - num_diag_moves;

  xydLin curr = from;
  path.clear();
  path.push_back(curr);

  while(num_diag_moves > 0) {
    if (g_->CanMove(curr, d)) {
      curr = g_->Move(curr, d);
      path.push_back(curr);
      num_diag_moves--;
    }
    else {
      path.clear();
      return false;
    }
  }

  while(num_card_moves > 0) {
    if (g_->CanMove(curr, c)) {
      curr = g_->Move(curr, c);
      path.push_back(curr);
      num_card_moves--;
    }
    else {
      path.clear();
      return false;
    }
  }

  d = OctileDistance(from_xy, to_xy);
  return true;
}


template<class Grid, class S>
void GridClearanceReachabilityRelation<Grid, S>::SafeFreespaceReachabilityConnect(
    nodeId source) {
  existing_subgoals_.clear();
  existing_subgoal_distances_.clear();
  ResetClearanceLines();

  for (Direction2D c = 0; c < 8; c += 2)
    this->ScanCardinalForSubgoal(
        source, c, existing_subgoals_);

  for (Direction2D d = 1; d < 8; d += 2)
    this->ScanDiagonalFirstForSafeSubgoals(
        source, d, existing_subgoals_);

  // Compute distances to subgoals
  for (unsigned int i = 0; i < existing_subgoals_.size(); i++) {
    existing_subgoal_distances_.push_back(
        OctileDistance(g_->ToXYLoc(source),
                       g_->ToXYLoc(existing_subgoals_[i])));
  }
}

template<class Grid, class S>
int GridClearanceReachabilityRelation<Grid, S>::ScanCardinalForSubgoal(
    xyLin s, Direction2D c,
    std::vector<xyLin> & subgoals, int ext) {

  assert(IsCardinal(c));
  int cl = card_cl_.GetClearance(s, c);
  if (cl != 0 && cl < ext) {
    xyLin l = g_->Move(s, c, cl);
    subgoals.push_back(l);
    AddSubgoalLine(s, l);
    return cl;
  }
  else
    return ext;
}

template<class Grid, class S>
void GridClearanceReachabilityRelation<Grid, S>::ScanDiagonalFirstForSafeSubgoals(
    xyLin s, Direction2D d,
    std::vector<xyLin> & subgoals) {

  Direction2D cw = CW(d);
  Direction2D ccw = CCW(d);
  int cw_max_ext = card_cl_.GetClearance(s, cw);
  int ccw_max_ext = card_cl_.GetClearance(s, ccw);
  if (cw_max_ext == 0)
    cw_max_ext = std::numeric_limits<int>::max()/2;
  if (ccw_max_ext == 0)
    ccw_max_ext = std::numeric_limits<int>::max()/2;

  xyLin end = this->Traverse(s, d, diag_cl_,
                             [&](xyLin l) {
    if (s_->IsSubgoal(l)) {
      subgoals.push_back(l);
      AddSubgoalLine(s, l);
      return true;
    }
    cw_max_ext = ScanCardinalForSubgoal(l, cw, subgoals, cw_max_ext);
    ccw_max_ext = ScanCardinalForSubgoal(l, ccw, subgoals, ccw_max_ext);
    return false;
  });

  AddClearanceLine(s, end);
}

template<class Grid, class S>
void GridClearanceReachabilityRelation<Grid, S>::ScanDiagonalFirstForSafeCorners(
    xyLin s, Direction2D d,
    std::vector<xyLin> & subgoals) {
  nodeId l = s;
  Direction2D cw = CW(d);
  Direction2D ccw = CCW(d);
  int cw_max_ext = card_cl_.GetClearance(s, cw);
  int ccw_max_ext = card_cl_.GetClearance(s, ccw);
  if (cw_max_ext == 0)
    cw_max_ext = std::numeric_limits<int>::max()/2;
  if (ccw_max_ext == 0)
    ccw_max_ext = std::numeric_limits<int>::max()/2;

  while (g_->CanMove(l, d)) {
    // Move along the diagonal
    l = g_->Move(l,d);
    if (g_->IsCorner(l)) {
      subgoals.push_back(l);
      AddSubgoalLine(s, l);
      return;
    }
    cw_max_ext = ScanCardinalForSubgoal(l, cw, subgoals, cw_max_ext);
    ccw_max_ext = ScanCardinalForSubgoal(l, ccw, subgoals, ccw_max_ext);
  }
  AddClearanceLine(s, l);
}



#ifndef NO_HOG
#ifdef DRAW_CLEARANCE_LINES
template<class Grid, class S>
void GridClearanceReachabilityRelation<Grid, S>::Visualize(const MapEnvironment *env, int display_type) {
  env->SetColor(0.5, 0.5, 0.5);
  for (unsigned int i = 0; i < line_begin_.size(); i++) {
    env->GLDrawColoredLine(g_->ToXYLoc(line_begin_[i]), g_->ToXYLoc(line_end_[i]));
  }
  env->SetColor(0,0,1);
  glLineWidth(2);
  for (unsigned int i = 0; i < subgoal_line_begin_.size(); i++) {
    env->GLDrawColoredLine(g_->ToXYLoc(subgoal_line_begin_[i]), g_->ToXYLoc(subgoal_line_end_[i]));
  }
  glLineWidth(1);

  for (auto sg: existing_subgoals_) {
    if (g_->ExtractDirection2D(sg) == dAll) {
      xyLoc l = g_->ExtractXYLoc(sg);
      g_->DrawNode(env, l.x, l.y, 100);
    }
    else
      g_->DrawArrowNode(env, sg);
  }
}
#endif
#endif
