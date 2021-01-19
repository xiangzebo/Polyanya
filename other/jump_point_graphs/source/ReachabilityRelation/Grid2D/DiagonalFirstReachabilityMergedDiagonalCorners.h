/*
 * DiagonalFirstReachabilityDiagonalCorners.h
 *
 *  Created on: Nov 11, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_REACHABILITYRELATION_GRID2D_DIAGONALFIRSTREACHABILITYMERGEDDIAGONALCORNERS_H_
#define APPS_SUBGOALGRAPH_REACHABILITYRELATION_GRID2D_DIAGONALFIRSTREACHABILITYMERGEDDIAGONALCORNERS_H_
#include "DiagonalFirstReachability.h"


template<class Grid = GPPCGrid, class S = SubgoalIdMapper>
class DiagonalFirstReachabilityMergedDiagonalCorners : public DiagonalFirstReachability<Grid,S> {
public:
  DiagonalFirstReachabilityMergedDiagonalCorners(Grid* g, S* s,
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
    return sg_gen.AddSubgoals(
        g_,
        s_,
        [&](xyLin n) {
          if (g_->IsCorner(n)) {
            std::vector<bool> should_add(8, false);
            for (Direction2D d_corner = 1; d_corner < 8; d_corner +=2) {
              if (g_->ConvexCornerAtDirection(n, d_corner)) {
                should_add[CW(d_corner,3)] = true;
                should_add[CW(d_corner,5)] = true;
                should_add[CW(d_corner,2)] = true;
                should_add[CW(d_corner,6)] = true;
              }
            }

            // If the incoming edge is blocked, not a vertex on G*
            // (and, therefore, not a jump point).
            for (Direction2D d = 0; d < 8; d++)
              if (!g_->CanMove(n, Reverse(d)))
                should_add[d] = false;

            // If the next diagonal move is blocked, we do not need the
            // diagonal jump point to stop diagonal scans.
            // Removed after Peter's request. Seems to be better after merging.
            //for (Direction2D d = 1; d < 8; d+=2)
            //  if (!g_->CanMove(n,d))
            //    should_add[d] = false;

            for (Direction2D d = 1; d < 8; d+=2) {
              if (should_add[d]) {
                s_->AddSubgoalWithMapping(g_->ToXYDLin(n,d), n);
                auto d_succ_flags = g_->GetTautCanonicalSuccessorDirectionFlags(n, d);
                if (should_add[CW(d)] && d_succ_flags == g_->GetTautCanonicalSuccessorDirectionFlags(n, CW(d))) {
                  should_add[CW(d)] = false;
                  s_->AddAlias(g_->ToXYDLin(n,d), g_->ToXYDLin(n,CW(d)));
                }
                if (should_add[CCW(d)] && d_succ_flags == g_->GetTautCanonicalSuccessorDirectionFlags(n, CCW(d))) {
                  should_add[CCW(d)] = false;
                  s_->AddAlias(g_->ToXYDLin(n,d), g_->ToXYDLin(n,CCW(d)));
                }
              }
            }

            for (Direction2D d = 0; d < 8; d+=2) {
              if (should_add[d]) {
                s_->AddSubgoal(g_->ToXYDLin(n,d));
              }
            }
          }
        });
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
    this->AddSubgoalLine(s, l);

    Direction2D rd = Reverse(d);

    xydLin ld = g_->ToXYDLin(l, rd);
    if (s_->IsSubgoal(ld)) {
      subgoals.push_back(ld);
      return;
    }

    xydLin lcw = g_->ToXYDLin(l, CW(rd));
    xydLin lccw = g_->ToXYDLin(l, CCW(rd));

    bool slcw = g_->IsForcedCanonicalTaut(l, CW(rd), rd);
    bool slccw = g_->IsForcedCanonicalTaut(l, CCW(rd), rd);

    if (slcw) {
      subgoals.push_back(lcw);
    }
    if (slccw) {
      subgoals.push_back(lccw);
    }
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

    nodeId last_interesting_cell = s; // visualization
    xyLin end = this->Traverse(s, d, r_card_cl_,
                               [&](xyLin l) {
      xydLin ld = g_->ToXYDLin(l, rd);
      xydLin lcw = g_->ToXYDLin(l, cw);
      xydLin lccw = g_->ToXYDLin(l, ccw);

      bool slcw = s_->IsSubgoal(lcw);
      bool slccw = s_->IsSubgoal(lccw);

      if (slcw) {
        subgoals.push_back(lcw);
        last_interesting_cell = l; //visualization
      }
      else
        ScanDiagonalForReverseJumpPoint(l, CW(d), subgoals);

      if (slccw) {
        subgoals.push_back(lccw);
        last_interesting_cell = l; //visualization
      }
      else
        ScanDiagonalForReverseJumpPoint(l, CCW(d), subgoals);

      if (!slcw && g_->IsForcedCanonicalTaut(l, p1, rd)) {
        subgoals.push_back(g_->ToXYDLin(l, p1));
        last_interesting_cell = l; //visualization
      }
      if (!slccw && g_->IsForcedCanonicalTaut(l, p2, rd)) {
        subgoals.push_back(g_->ToXYDLin(l, p2));
        last_interesting_cell = l; //visualization
      }

      if (s_->IsSubgoal(ld)) {
        if (!slcw && !slccw) {
          subgoals.push_back(ld);
          last_interesting_cell = l;
        }
        return true;
      }
      return false;
    });
    this->AddSubgoalLine(s,last_interesting_cell);
    this->AddClearanceLine(last_interesting_cell,end);
  }
};


#endif /* APPS_SUBGOALGRAPH_REACHABILITYRELATION_GRID2D_DIAGONALFIRSTREACHABILITYMERGEDDIAGONALCORNERS_H_ */
