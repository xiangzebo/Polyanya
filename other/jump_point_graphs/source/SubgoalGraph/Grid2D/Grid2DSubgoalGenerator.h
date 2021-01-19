/*
 * Grid2DSubgoalIdentifier.h
 *
 *  Created on: Oct 22, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_SUBGOALGRAPH_GRID2D_GRID2DSUBGOALGENERATOR_H_
#define APPS_SUBGOALGRAPH_SUBGOALGRAPH_GRID2D_GRID2DSUBGOALGENERATOR_H_

#include "GraphDefinitions.h"
#include "Grid2D.h"
#include "CPUTimer.h"

class Grid2DSubgoalGenerator {
 public:
  Grid2DSubgoalGenerator() {

  }
  ~Grid2DSubgoalGenerator() {}

  template <class Grid, class S>
  double AddCorners(Grid* g, S* s) {
    return AddSubgoals(g, s,
    [&](xyLin n) {
      if (g->IsCorner(n))
        s->AddSubgoal(n);
    });
  }

  template <class Grid, class S>
  double AddAllDirectionExtendedCorners(Grid* g, S* s) {
    return AddSubgoals(g, s,
    [&](xyLin n) {
      if (g->IsCorner(n)) {
        for (Direction2D d = 0; d < 8; d++) {
          if (g->CanMove(n, Reverse(d))) {
            std::vector<Direction2D> directions;
            g->GetTautCanonicalSuccessorDirections(n, d, directions);
            if (!directions.empty())
              s->AddSubgoalWithMapping(g->ToXYDLin(n,d), n);
          }
        }
      }
    });
  }

  template <class Grid, class S>
  double AddStraightJumpPoints(Grid* g, S* s) {
    return AddSubgoals(g, s,
    [&](xyLin n) {
      if (g->IsCorner(n)) {
        for (Direction2D d = 0; d < 8; d++) {
          std::vector<bool> should_add(8, false);
          for (Direction2D d_corner = 1; d_corner < 8; d_corner +=2) {
            if (g->ConvexCornerAtDirection(n, d_corner)) {
              should_add[CW(d_corner,3)] = true;
              should_add[CW(d_corner,5)] = true;
            }
          }

          for (Direction2D d = 0; d < 8; d++) {
            if (should_add[d] && g->CanMove(n, Reverse(d))) {
              std::vector<Direction2D> directions;
              g->GetTautCanonicalSuccessorDirections(n, d, directions);
              if (!directions.empty())
                s->AddSubgoalWithMapping(g->ToXYDLin(n,d), n);
            }
          }
        }
      }
    });
  }

  template <class Grid, class S>
  double AddStraightAndDiagonalJumpPoints(Grid* g, S* s) {
    return AddSubgoals(g, s,
    [&](xyLin n) {
      if (g->IsCorner(n)) {
        std::vector<bool> should_add(8, false);
        for (Direction2D d_corner = 1; d_corner < 8; d_corner +=2) {
          if (g->ConvexCornerAtDirection(n, d_corner)) {
            should_add[CW(d_corner,3)] = true;
            should_add[CW(d_corner,5)] = true;
            should_add[CW(d_corner,2)] = true;
            should_add[CW(d_corner,6)] = true;
          }
        }

        // If the incoming edge is blocked, not a vertex on G*
        // (and, therefore, not a jump point).
        for (Direction2D d = 0; d < 8; d++)
          if (!g->CanMove(n, Reverse(d)))
            should_add[d] = false;

        // If the next diagonal move is blocked, we do not need the
        // diagonal jump point to stop diagonal scans.
        // Removed after Peter's request. Seems to be better after merging.
        //for (Direction2D d = 1; d < 8; d+=2)
        //  if (!g->CanMove(n,d))
        //    should_add[d] = false;

        for (Direction2D d = 0; d < 8; d++) {
          if (should_add[d]) {
            //std::vector<Direction2D> directions;
            //g->GetTautCanonicalSuccessorDirections(n, d, directions);
            //if (!directions.empty())
              s->AddSubgoalWithMapping(g->ToXYDLin(n,d), n);
          }
        }
      }
    });
  }

  template <class Grid, class S, typename F>
  double AddSubgoals(Grid* g, S* s, F AddSubgoalsForLocation) {
    CPUTimer t;
    t.StartTimer();
    for (int x = 0; x < g->GetOriginalWidth(); x++) {
      for (int y = 0; y < g->GetOriginalHeight(); y++) {
        nodeId n = g->ToXYLin(x,y);
        if (g->IsTraversable(n))
          AddSubgoalsForLocation(n);
      }
    }
    t.EndTimer();
    return t.GetElapsedTime();
  }
 private:
};

#endif /* APPS_SUBGOALGRAPH_SUBGOALGRAPH_GRID2D_GRID2DSUBGOALGENERATOR_H_ */
