/*
 * DiagonalFirstReachabilityDiagonalCorners.h
 *
 *  Created on: Nov 11, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_REACHABILITYRELATION_GRID2D_DIAGONALFIRSTREACHABILITYDIAGONALCORNERS_H_
#define APPS_SUBGOALGRAPH_REACHABILITYRELATION_GRID2D_DIAGONALFIRSTREACHABILITYDIAGONALCORNERS_H_
#include "DiagonalFirstReachability.h"


template<class Grid = GPPCGrid, class S = SubgoalIdMapper>
class DiagonalFirstReachabilityDiagonalCorners : public DiagonalFirstReachability<Grid,S> {
public:
  DiagonalFirstReachabilityDiagonalCorners(Grid* g, S* s,
                                           bool use_double_clearance)
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
    return sg_gen.AddStraightAndDiagonalJumpPoints(g_, s_);
  }

private:
  void ScanDiagonalForReverseJumpPoint(
      xyLin s, Direction2D d,
      std::vector<xydLin> & subgoals) {
    assert(IsDiagonal(d));

    int cl = r_diag_cl_.GetClearance(s, d);
    if (cl == 0)
      return;

    xyLin l = g_->Move(s, d, cl);
    Direction2D rd = Reverse(d);

    xydLin ld = g_->ToXYDLin(l, rd);
    xydLin lcw = g_->ToXYDLin(l, CW(rd));
    xydLin lccw = g_->ToXYDLin(l, CCW(rd));

    bool sld = s_->IsSubgoal(ld);
    bool slcw = g_->IsForcedCanonicalTaut(l, CW(rd), rd);
    bool slccw = g_->IsForcedCanonicalTaut(l, CCW(rd), rd);

    if (slcw) {
      subgoals.push_back(lcw);
    }
    if (slccw) {
      subgoals.push_back(lccw);
    }
    if (sld) {
      subgoals.push_back(ld);
    }
    this->AddSubgoalLine(s, l);
  }


  void ScanCardinalFirstForReverseJumpPoints(
      xyLin s, Direction2D d,
      std::vector<xydLin> & subgoals) {

    assert(IsCardinal(d));
    Direction2D rd = Reverse(d);
    Direction2D cw = CW(rd);
    Direction2D ccw = CCW(rd);
    Direction2D p1 = CW(rd,2);
    Direction2D p2 = CW(rd,6);

    xyLin last_interesting_cell = s;
    xyLin end = this->Traverse(s, d, r_card_cl_, [&](xyLin l) {
      xydLin ld = g_->ToXYDLin(l, rd);
      xydLin lcw = g_->ToXYDLin(l, cw);
      xydLin lccw = g_->ToXYDLin(l, ccw);

      bool sld = s_->IsSubgoal(ld);
      bool slcw = s_->IsSubgoal(lcw);
      bool slccw = s_->IsSubgoal(lccw);
      bool slp1 = g_->IsForcedCanonicalTaut(l, p1, rd);
      bool slp2 = g_->IsForcedCanonicalTaut(l, p2, rd);

      if (slcw) {
        subgoals.push_back(lcw);
        last_interesting_cell = l; //visualization
      }
      else
        ScanDiagonalForReverseJumpPoint(l, CW(d), subgoals);

      if (slccw) {
        subgoals.push_back(lccw);
        last_interesting_cell = l;  //visualization
      }
      else
        ScanDiagonalForReverseJumpPoint(l, CCW(d), subgoals);

      if (slp1) {
        subgoals.push_back(g_->ToXYDLin(l, p1));
        last_interesting_cell = l; //visualization
      }
      if (slp2) {
        subgoals.push_back(g_->ToXYDLin(l, p2));
        last_interesting_cell = l; //visualization
      }

      if (sld) {
        subgoals.push_back(ld);
        last_interesting_cell = l; //visualization
        return true;
      }
      return false;
    });
    this->AddSubgoalLine(s,last_interesting_cell);
    this->AddClearanceLine(last_interesting_cell,end);
  }
};


#endif /* APPS_SUBGOALGRAPH_REACHABILITYRELATION_GRID2D_DIAGONALFIRSTREACHABILITYDIAGONALCORNERS_H_ */
