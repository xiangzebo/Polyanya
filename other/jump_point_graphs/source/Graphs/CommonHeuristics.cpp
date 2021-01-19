/*
 * CommonHeuristics.cpp
 *
 *  Created on: Oct 25, 2017
 *      Author: idm-lab
 */


#include "CommonHeuristics.h"

double OctileDistance(xyLoc l1, xyLoc l2) {
  int dx = abs(l1.x - l2.x);
  int dy = abs(l1.y - l2.y);
  return
      (dx > dy) ?
          (dx * CARD_COST + dy * DIAG_DIFF) : (dy * CARD_COST + dx * DIAG_DIFF);

  // Branchless?
  return (dx == dy) * dx * DIAG_COST
      + (dx > dy) * (dx * CARD_COST + dy * DIAG_DIFF)
      + (dx < dy) * (dy * CARD_COST + dx * DIAG_DIFF);
}

double EuclideanDistance(xyLoc l1, xyLoc l2) {
  int dx = abs(l1.x - l2.x);
  int dy = abs(l1.y - l2.y);
  return sqrt(dx*dx + dy*dy);
}

double EuclideanDistance(xyThetaPos l1, xyThetaPos l2) {
  int dx = abs(l1.x - l2.x);
  int dy = abs(l1.y - l2.y);
  return sqrt(dx*dx + dy*dy);
}


