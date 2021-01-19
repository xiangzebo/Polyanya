/*
 * DiagonalFirstReachabilityStraightJP.h
 *
 *  Created on: Nov 11, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_REACHABILITYRELATION_GRID2D_DIAGONALFIRSTREACHABILITYSTRAIGHTJP_H_
#define APPS_SUBGOALGRAPH_REACHABILITYRELATION_GRID2D_DIAGONALFIRSTREACHABILITYSTRAIGHTJP_H_
#include "DiagonalFirstReachability.h"


template<class Grid = GPPCGrid, class S = SubgoalIdMapper>
class DiagonalFirstReachabilityStraightJP : public DiagonalFirstReachability<Grid,S> {
public:
  DiagonalFirstReachabilityStraightJP(Grid* g, S* s, bool use_double_clearance)
      : DiagonalFirstReachability<Grid,S>(g,s, use_double_clearance) {}

  using GridClearanceReachabilityRelation<Grid,S>::existing_subgoals_;
  using GridClearanceReachabilityRelation<Grid,S>::existing_subgoal_distances_;
  using GridClearanceReachabilityRelation<Grid,S>::g_;
  using GridClearanceReachabilityRelation<Grid,S>::s_;
  using GridClearanceReachabilityRelation<Grid,S>::use_double_clearance_;
  using GridClearanceReachabilityRelation<Grid,S>::r_diag_cl_;
  using GridClearanceReachabilityRelation<Grid,S>::r_card_cl_;

  double IdentifySubgoals() {
    Grid2DSubgoalGenerator sg_gen;
    return sg_gen.AddStraightJumpPoints(g_, s_);
  }

private:
  void ScanDiagonalForReverseJumpPoint(xyLin s, Direction2D d,
                                       std::vector<xydLin> & subgoals) {
    assert(IsDiagonal(d));
    xyLin l = s;
    int cl = r_diag_cl_.GetClearance(l, d);

    while (cl > 0) {
      l = g_->Move(l, d, cl);
      Direction2D rd = Reverse(d);

      xydLin lcw = g_->ToXYDLin(l, CW(rd));
      xydLin lccw = g_->ToXYDLin(l, CCW(rd));

      bool slcw = g_->IsForcedCanonicalTaut(l, CW(rd), rd);
      bool slccw = g_->IsForcedCanonicalTaut(l, CCW(rd), rd);

      this->AddSubgoalLine(s, l);
      if (slcw) {
        subgoals.push_back(lcw);
      }
      if (slccw) {
        subgoals.push_back(lccw);
      }

      cl = r_diag_cl_.GetClearance(l, d);
    }
  }

  void ScanCardinalFirstForReverseJumpPoints(xyLin s, Direction2D d,
                                             std::vector<xydLin> & subgoals) {
    assert(IsCardinal(d));
    nodeId last_interesting_cell = s;

    Direction2D rd = Reverse(d);
    Direction2D p1 = CW(d,2);  // Perpendicular directions.
    Direction2D p2 = CW(d,6);

    xyLin end = this->Traverse(s, d, r_card_cl_,
                               [&](xyLin l) {
      ScanDiagonalForReverseJumpPoint(l, CW(d), subgoals);
      ScanDiagonalForReverseJumpPoint(l, CCW(d), subgoals);

      xydLin ld = g_->ToXYDLin(l, rd);
      xydLin lp1 = g_->ToXYDLin(l, p1);
      xydLin lp2 = g_->ToXYDLin(l, p2);

      bool sld = s_->IsSubgoal(ld);
      bool slp1 = g_->IsForcedCanonicalTaut(l, p1, rd);
      bool slp2 = g_->IsForcedCanonicalTaut(l, p2, rd);

      if (slp1) {
        subgoals.push_back(lp1);
        last_interesting_cell = l;  //visualization
      }
      if (slp2) {
        subgoals.push_back(lp2);
        last_interesting_cell = l;  //visualization
      }
      if (sld) {
        subgoals.push_back(ld);
        last_interesting_cell = l;  //visualization
        return true;
      }
      return false;
    });
    this->AddSubgoalLine(s, last_interesting_cell);
    this->AddClearanceLine(last_interesting_cell, end);
  }
};


#endif /* APPS_SUBGOALGRAPH_REACHABILITYRELATION_GRID2D_DIAGONALFIRSTREACHABILITYSTRAIGHTJP_H_ */
