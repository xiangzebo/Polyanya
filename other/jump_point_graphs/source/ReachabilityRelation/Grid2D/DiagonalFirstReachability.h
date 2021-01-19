/*
 * CanonicalFreespaceReachabilityGrid2D.h
 *
 *  Created on: Nov 9, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_REACHABILITYRELATION_GRID2D_DIAGONALFIRST_H_
#define APPS_SUBGOALGRAPH_REACHABILITYRELATION_GRID2D_DIAGONALFIRST_H_

#include "CommonHeuristics.h"
#include "GridReachabilityRelation.h"

#include <vector>
#include <cmath>

template<class Grid = GPPCGrid, class S = SubgoalIdMapper>
class DiagonalFirstReachability : public GridClearanceReachabilityRelation<Grid,S> {
 public:
  using GridClearanceReachabilityRelation<Grid,S>::existing_subgoals_;
  using GridClearanceReachabilityRelation<Grid,S>::existing_subgoal_distances_;
  using GridClearanceReachabilityRelation<Grid,S>::g_;
  using GridClearanceReachabilityRelation<Grid,S>::s_;
  using GridClearanceReachabilityRelation<Grid,S>::use_double_clearance_;
  using GridClearanceReachabilityRelation<Grid,S>::card_cl_;
  using GridClearanceReachabilityRelation<Grid,S>::r_diag_cl_;
  using GridClearanceReachabilityRelation<Grid,S>::diag_cl_;
  using GridClearanceReachabilityRelation<Grid,S>::r_card_cl_;

  DiagonalFirstReachability(Grid* g, S* s, bool use_double_clearance)
      : GridClearanceReachabilityRelation<Grid,S>(g,s, use_double_clearance) {}
  virtual ~DiagonalFirstReachability() {}

  virtual double IdentifySubgoals() = 0;
  virtual double InitializeGivenSubgoals() {
    double time = 0;
    time += card_cl_.CalculateCardinalClearances(
        [&](xyLin n, Direction2D d) {
      return s_->IsSubgoal(g_->ToXYDLin(n,d));
    });

    time += r_diag_cl_.CalculateDiagonalClearances(
        [&](xyLin n, Direction2D d) {
      Direction2D rd = Reverse(d);
      return s_->IsSubgoal(g_->ToXYDLin(n,rd)) ||
          g_->IsForcedCanonicalTaut(n, CW(rd), rd) ||
          g_->IsForcedCanonicalTaut(n, CCW(rd), rd);
    });

    if (use_double_clearance_) {
      time += diag_cl_.CalculateDiagonalClearances([&](xyLin n, Direction2D d) {
        return s_->IsSubgoal(g_->ToXYDLin(n,d)) ||
        card_cl_.GetClearance(n, CW(d)) != 0 ||
        card_cl_.GetClearance(n, CCW(d)) != 0;
      });

      time += r_card_cl_.CalculateCardinalClearances(
          [&](xyLin n, Direction2D d) {
            Direction2D rd = Reverse(d);
            return s_->IsSubgoal(g_->ToXYDLin(n, rd)) ||
            s_->IsSubgoal(g_->ToXYDLin(n, CW(rd))) ||
            s_->IsSubgoal(g_->ToXYDLin(n, CCW(rd))) ||
            g_->IsForcedCanonicalTaut(n, CW(d,2), rd) ||
            g_->IsForcedCanonicalTaut(n, CW(d,6), rd) ||
            r_diag_cl_.GetClearance(n, CW(d)) != 0 ||
            r_diag_cl_.GetClearance(n, CCW(d)) != 0;
          });
    }
    return time;
  }

  void RConnect(nodeId start, bool can_identify_superset = false) {
    this->ResetClearanceLines();
    existing_subgoals_.clear();
    existing_subgoal_distances_.clear();

    auto sl = g_->ExtractXYLin(start);
    auto sd = g_->ExtractDirection2D(start);

    if (this->exploring_backward_) {
      assert(sd == dAll);
      BackwardConnectGoal(sl);
    }
    else {
      if (sd == dAll)
        ForwardConnectStart(sl);
      else
        ForwardConnectJumpPoint(sl, sd);
    }
  }

  void RRefine(nodeId start, nodeId goal, std::vector<nodeId> & path,
               bool append = false) {

    GetDiagonalFirstPath(g_->ExtractXYLin(start),
                         g_->ExtractXYLin(goal), path, append);
  }

  bool IsReachable(nodeId start, nodeId goal, Distance d = kMaxDistance) {
    return CheckDiagonalFirstReachability(start, goal);
  }

  bool GetQueryPathIfReachable(nodeId start, nodeId goal, Distance & d,
                          std::vector<nodeId> & path) {
    return this->GetDiagonalFirstPathIfExists(
        g_->ExtractXYLin(start),
        g_->ExtractXYLin(goal), d, path);
  }


  bool CanConstructEdgesExactly() const {return false;}

 protected:
  virtual void ScanCardinalForJumpPoint(xyLin n, Direction2D c,
                                std::vector<xydLin> & subgoals);
  virtual void ScanDiagonalFirstForJumpPoints(xyLin n, Direction2D d,
                                      std::vector<xydLin> & subgoals);

  virtual void ScanDiagonalForReverseJumpPoint(xyLin n, Direction2D c,
                                std::vector<xydLin> & subgoals) {}
  virtual void ScanCardinalFirstForReverseJumpPoints(xyLin n, Direction2D d,
                                      std::vector<xydLin> & subgoals) {};

  void CalculateExistingSubgoalDistances(xyLin sl) {
    existing_subgoal_distances_.clear();
    for (unsigned int i = 0; i < existing_subgoals_.size(); i++) {
      existing_subgoal_distances_.push_back(
          OctileDistance(g_->ToXYLoc(sl),
                         g_->ToXYLoc(g_->ExtractXYLin(existing_subgoals_[i]))));
    }
  }

  virtual void ForwardConnectJumpPoint(xyLin sl, Direction2D sd) {
    std::vector<Direction2D> directions;
    g_->GetTautCanonicalSuccessorDirections(sl, sd, directions);
    for (auto d : directions) {
      if (IsCardinal(d)) {
        this->ScanCardinalForJumpPoint(sl, d, existing_subgoals_);
      }
      else {
        this->ScanDiagonalFirstForJumpPoints(sl, d, existing_subgoals_);
      }
    }
    CalculateExistingSubgoalDistances(sl);
  }

  virtual void ForwardConnectStart(xyLin sl) {
    for (Direction2D c = 0; c < 8; c += 2)
      this->ScanCardinalForJumpPoint(sl, c, existing_subgoals_);
    for (Direction2D d = 1; d < 8; d += 2)
      this->ScanDiagonalFirstForJumpPoints(sl, d, existing_subgoals_);
    CalculateExistingSubgoalDistances(sl);
  }

  virtual void BackwardConnectGoal(xyLin sl) {
    for (Direction2D d = 1; d < 8; d += 2)
      this->ScanDiagonalForReverseJumpPoint(sl, d, existing_subgoals_);
    for (Direction2D c = 0; c < 8; c += 2)
      this->ScanCardinalFirstForReverseJumpPoints(sl, c, existing_subgoals_);
    CalculateExistingSubgoalDistances(sl);
  }

  // Returns true if the canonical path between two nodes is unblocked.
  bool CheckDiagonalFirstReachability(nodeId start, nodeId goal);

  // Returns the canonical path between two canonical-reachable nodes.
  void GetDiagonalFirstPath(nodeId start, nodeId goal,
                                 std::vector<nodeId> & path,
                                 bool append = false);
};

// IMPLEMENTATION
template<class Grid, class S>
bool DiagonalFirstReachability<Grid, S>::CheckDiagonalFirstReachability(
    nodeId from, nodeId to) {

  xyLoc from_xy = g_->ToXYLoc(from);
  xyLoc to_xy = g_->ToXYLoc(to);
  int num_card_moves, num_diag_moves, num_total_moves;
  Direction2D c, d;
  g_->GetMoveDirectionDetails(from_xy, to_xy, c, d,
                          num_total_moves, num_diag_moves);

  num_card_moves = num_total_moves - num_diag_moves;
  nodeId curr = from;

  // Diagonal first
  while (num_diag_moves > 0) {
    if (g_->CanMove(curr, d)) {
      curr = g_->Move(curr, d);
      num_diag_moves--;
    }
    else
      return false;
  }

  // Cardinal after diagonal
  while(num_card_moves > 0) {
    if (g_->CanMove(curr, c)) {
      curr = g_->Move(curr, c);
      num_card_moves--;
    }
    else
      return false;
  }
  return true;
}

template<class Grid, class S>
void DiagonalFirstReachability<Grid, S>::GetDiagonalFirstPath(
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

  // Diagonal first
  while (num_diag_moves > 0) {
    loc = g_->Move(loc, d);
    path.push_back(loc);
    num_diag_moves--;
  }

  // Cardinal after diagonal
  while (num_card_moves > 0) {
    loc = g_->Move(loc, c);
    path.push_back(loc);
    num_card_moves--;
  }
}

// FIXME

template<class Grid, class S>
void DiagonalFirstReachability<Grid, S>::ScanCardinalForJumpPoint(
    xyLin s, Direction2D c,
    std::vector<xydLin> & subgoals) {
  assert(IsCardinal(c));

  int cl = card_cl_.GetClearance(s, c);
  if (cl == 0)
    return;

  xydLin l = g_->ToXYDLin(g_->Move(s, c, cl), c);
  subgoals.push_back(l);
  this->AddSubgoalLine(s, g_->ExtractXYLin(l));
}
template<class Grid, class S>
void DiagonalFirstReachability<Grid, S>::ScanDiagonalFirstForJumpPoints(
    xyLin s, Direction2D d,
    std::vector<xydLin> & subgoals) {

  assert(IsDiagonal(d));

  Direction2D cw = CW(d);
  Direction2D ccw = CCW(d);

  xyLin end = this->Traverse(s, d, diag_cl_,
                             [&](xyLin l) {
    xydLin nd = g_->ToXYDLin(l, d);
    if (s_->IsSubgoal(nd)) {
      subgoals.push_back(nd);
      this->AddSubgoalLine(s, l);
      return true;
    }
    ScanCardinalForJumpPoint(l, cw, subgoals);
    ScanCardinalForJumpPoint(l, ccw, subgoals);
    return false;
  }
  );
  this->AddClearanceLine(s,end);
}

#endif /* APPS_SUBGOALGRAPH_REACHABILITYRELATION_GRID2D_DIAGONALFIRST_H_ */
