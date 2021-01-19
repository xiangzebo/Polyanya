/*
 * DiagonalFirstReachabilityAllCorners.h
 *
 *  Created on: Nov 11, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_REACHABILITYRELATION_GRID2D_DIAGONALFIRSTREACHABILITYALLCORNERS_H_
#define APPS_SUBGOALGRAPH_REACHABILITYRELATION_GRID2D_DIAGONALFIRSTREACHABILITYALLCORNERS_H_
#include "DiagonalFirstReachability.h"


template<class Grid = GPPCGrid, class S = SubgoalIdMapper>
class DiagonalFirstReachabilityAllCorners : public DiagonalFirstReachability<Grid,S> {
public:
  DiagonalFirstReachabilityAllCorners(Grid* g, S* s, bool use_double_clearance)
      : DiagonalFirstReachability<Grid,S>(g,s, use_double_clearance) {}

  using GridClearanceReachabilityRelation<Grid,S>::existing_subgoals_;
  using GridClearanceReachabilityRelation<Grid,S>::existing_subgoal_distances_;
  using GridClearanceReachabilityRelation<Grid,S>::g_;
  using GridClearanceReachabilityRelation<Grid,S>::s_;
  using GridClearanceReachabilityRelation<Grid,S>::use_double_clearance_;
  using GridClearanceReachabilityRelation<Grid,S>::card_cl_;
  using GridClearanceReachabilityRelation<Grid,S>::diag_cl_;

  double IdentifySubgoals() {
    Grid2DSubgoalGenerator sg_gen;
    return sg_gen.AddAllDirectionExtendedCorners(g_, s_);
  }
  double InitializeGivenSubgoals() {
    double time = 0;

    time += card_cl_.CalculateCardinalClearancesToCorners();

    if (use_double_clearance_)
      time += diag_cl_.CalculateDiagonalClearances([&](xyLin n, Direction2D d) {
        return g_->IsCorner(n) ||
        card_cl_.GetClearance(n, CW(d)) != 0 ||
        card_cl_.GetClearance(n, CCW(d)) != 0;
      });
    return time;
  }

private:
  void BackwardConnectGoal(xyLin sl) {
    std::vector<xyLin> subgoals;
    for (Direction2D c = 0; c < 8; c += 2)
      this->ScanCardinalForSubgoal(sl, c, subgoals);
    for (Direction2D d = 1; d < 8; d += 2)
      this->ScanDiagonalFirstForSafeCorners(sl, d, subgoals);
    for (auto sg : subgoals) {
      for (Direction2D d = 0; d < 8; d++) {
        xydLin sgld = g_->ToXYDLin(sg, d);
        if (s_->IsSubgoal(sgld)) {
          existing_subgoals_.push_back(sgld);
        }
      }
    }
    this->CalculateExistingSubgoalDistances(sl);
  }
};


#endif /* APPS_SUBGOALGRAPH_REACHABILITYRELATION_GRID2D_DIAGONALFIRSTREACHABILITYALLCORNERS_H_ */
