/*
 * GPPCGridUtils.cpp
 *
 *  Created on: Nov 7, 2018
 *      Author: idm-lab
 */

#include "GPPCGrid.h"
#include <cassert>

// INITIALIZATION

#ifndef NO_HOG
GPPCGrid::GPPCGrid(const MapEnvironment *env)
  : g_(env, 1) {
  Initialize();
}
#endif

GPPCGrid::GPPCGrid(std::string mapname)
  : g_(mapname, 1) {
  Initialize();
}

void GPPCGrid::Initialize() {
  CalculateLocationDeltas();
  CalculateCanonicalTautForcingCornerDirections();
  SetCosts(CARD_COST, DIAG_COST);
}


void GPPCGrid::CalculateLocationDeltas() {
  delta_xylin_[dE] = 1;
  delta_xylin_[dW] = -1;
  delta_xylin_[dS] = g_.GetPaddedWidth();
  delta_xylin_[dN] = -g_.GetPaddedWidth();
  delta_xylin_[dNE] = delta_xylin_[dN] + delta_xylin_[dE];
  delta_xylin_[dSE] = delta_xylin_[dS] + delta_xylin_[dE];
  delta_xylin_[dSW] = delta_xylin_[dS] + delta_xylin_[dW];
  delta_xylin_[dNW] = delta_xylin_[dN] + delta_xylin_[dW];

  //*
  // Create the extra copies
  for (Direction2D d = 0; d < 8; d++)
  {
    delta_xylin_[d + 8] = delta_xylin_[d];
    delta_xylin_[d + 16] = delta_xylin_[d];
  }
  //*/

  delta_x_[dN] =  0; delta_y_[dN] = -1;
  delta_x_[dS] =  0; delta_y_[dS] =  1;
  delta_x_[dW] = -1; delta_y_[dW] =  0;
  delta_x_[dE] =  1; delta_y_[dE] =  0;

  delta_x_[dNE] = delta_x_[dN] + delta_x_[dE];
  delta_x_[dSE] = delta_x_[dS] + delta_x_[dE];
  delta_x_[dSW] = delta_x_[dS] + delta_x_[dW];
  delta_x_[dNW] = delta_x_[dN] + delta_x_[dW];

  delta_y_[dNE] = delta_y_[dN] + delta_y_[dE];
  delta_y_[dSE] = delta_y_[dS] + delta_y_[dE];
  delta_y_[dSW] = delta_y_[dS] + delta_y_[dW];
  delta_y_[dNW] = delta_y_[dN] + delta_y_[dW];
}

void GPPCGrid::CalculateCanonicalTautForcingCornerDirections(){
  for (Direction2D d1 = 0; d1 < 8; d1++)
    for (Direction2D d2 = 0; d2 < 8; d2++)
      canonical_taut_forcing_corner_direction_[d1][d2] = dAll;
  
  // Forced neighbors
  for (Direction2D d_corner = 1; d_corner < 8; d_corner+=2) {
    // Diagonal direction di; associated cardinal directions cw and ccw.
    Direction2D cw = CW(d_corner);
    Direction2D ccw = CCW(d_corner);
    canonical_taut_forcing_corner_direction_[Reverse(cw)][ccw] = d_corner;
    canonical_taut_forcing_corner_direction_[Reverse(cw)][CCW(ccw)] = d_corner;
    canonical_taut_forcing_corner_direction_[Reverse(ccw)][cw] = d_corner;
    canonical_taut_forcing_corner_direction_[Reverse(ccw)][CW(cw)] = d_corner;
  }
}

void GPPCGrid::GetNeighbors(nodeId n, std::vector<WeightedArcHead> & neighbors) {
  neighbors.clear();
  if (!IsTraversable(n))
    return;

  static bool move[9];

  for (Direction2D d = 0; d < 8; d++)
    move[d] = IsTraversable(n + delta_xylin_[d]);
  move[8] = move[0];

  for (Direction2D d = 1; d < 8; d+=2) {
    move[d] = move[d-1] & move[d] & move[d+1];
    if (move[d])
      neighbors.push_back(WeightedArcHead(n + delta_xylin_[d], diag_cost_));
  }

  for (Direction2D d = 0; d < 8; d+=2)
    if (move[d])
      neighbors.push_back(WeightedArcHead(n + delta_xylin_[d], card_cost_));
}

// HACKY EXTENSION TO DIRECTED

void GPPCGrid::GetDirectedNeighbors(xydLin n,
                                    std::vector<WeightedArcHead> & neighbors) {
  neighbors.clear();
  xyLin ln = ExtractXYLin(n);
  Direction2D dn = ExtractDirection2D(n);

  if (!IsTraversable(ln) || dn == dAll)
    return;

  for (Direction2D d = 1; d < 8; d+=2)
    if (IsCanonicalTaut(ln, dn, d))
      neighbors.push_back(
          WeightedArcHead(ToXYDLin(ln + delta_xylin_[d], d), diag_cost_));

  for (Direction2D d = 0; d < 8; d+=2)
    if (IsCanonicalTaut(ln, dn, d))
      neighbors.push_back(
          WeightedArcHead(ToXYDLin(ln + delta_xylin_[d], d), card_cost_));

  for (auto a : neighbors) {
    assert(a.target < GetNumPaddedCells()*9);
    assert(a.weight <= diag_cost_);
  }
}

// TODO: Optimize (precompute cases etc).
uint8_t GPPCGrid::GetTautCanonicalSuccessorDirectionFlags(xyLin lc,
                                                          Direction2D d) const {
  assert(IsTraversable(lc));
  assert(d != dAll);
  uint8_t flag = 0;
  uint8_t mask = 1;
  for (Direction2D d2 = 0; d2 < 8; d2++) {
    if (IsCanonicalTaut(lc, d, d2))
      flag = flag | mask;
    mask = mask << 1;
  }
  return flag;
}

void GPPCGrid::GetTautCanonicalSuccessorDirections(
    xyLin lc, Direction2D d, std::vector<Direction2D> & directions) const {
  assert(IsTraversable(lc));
  assert(d != dAll);
  directions.clear();
  for (Direction2D d2 = 0; d2 < 8; d2++)
    if (IsCanonicalTaut(lc, d, d2))
      directions.push_back(d2);
  return;
}

void GPPCGrid::TestDirections(xyLoc l) const {
  for (Direction2D d = 0; d < 8; d++) {
    if (d != dAll && !CanMove(g_.ToXYLin(l),Reverse(d))) {
      std::cout<<"(!)"; // Invalid incoming direction.
    }
    std::cout<<GetDirection2DName(d)<<":";
    std::vector<Direction2D> directions;
    GetTautCanonicalSuccessorDirections(l, d, directions);
    for (int i = 0; i < directions.size(); i++)
      std::cout<<" "<<GetDirection2DName(directions[i]);
    std::cout<<std::endl;
  }
}
