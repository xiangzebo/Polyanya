/*
 * GridReachabilityRelation.h
 *
 *  Created on: Nov 9, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_GRAPHS_GRID2D_GRIDREACHABILITYRELATION_H_
#define APPS_SUBGOALGRAPH_GRAPHS_GRID2D_GRIDREACHABILITYRELATION_H_


#include <cassert>
#include <vector>

#include "../../Graphs/GraphDefinitions.h"
#include "../../Graphs/Grid2D/Direction2D.h"
#include "../../Graphs/Grid2D/Grid2D.h"
#include "../ReachabilityRelation.h"
#include "Grid2DClearanceManager.h"


class SubgoalIdMapper;

// G: Grid (assumes nodeIds = linearized coordinates)
// S: Subgoal ids.

class GridReachabilityRelation : public ReachabilityRelation {
 public:
  GridReachabilityRelation()
      : ReachabilityRelation() {}

  virtual double InitializeGivenSubgoals() = 0;
  virtual double EstimateStorageMB() = 0;
  virtual void PrintNodeInfo(nodeId n) = 0;

  // FIXME: not being used at the moment (required for connection statistics).
  virtual std::vector<nodeId>* GetExpansionOrder() {
    return &expansion_order_;
  }
//  virtual int GetDirectReachableAreaSize(nodeId source) {
//    return 0;
//  }
//  virtual RConnectStatistic GetRConnectStatistic() {
//    return RConnectStatistic();
//  }


 protected:
  // Not used, always empty.
  std::vector<nodeId> expansion_order_;
};

template<class Grid = GPPCGrid, class S = SubgoalIdMapper>
class GridClearanceReachabilityRelation : public GridReachabilityRelation {
 public:
  GridClearanceReachabilityRelation(Grid* g, S* s, bool use_double_clearance);

  bool CanDetectReachabilityOfQueryPoints() {
    return false;
  }

  // Returns the time for the calculations.
  virtual double IdentifySubgoals() = 0;

  // Returns the time for the calculations.
  virtual double InitializeGivenSubgoals() {
    double time = 0;

    time += card_cl_.CalculateCardinalClearancesToCorners();

    if (use_double_clearance_)
      time += diag_cl_.CalculateDiagonalClearances([&](xyLin n, Direction2D d) {
        return s_->IsSubgoal(n) ||
        card_cl_.GetClearance(n, CW(d)) != 0 ||
        card_cl_.GetClearance(n, CCW(d)) != 0;
      });
    return time;
  }
  virtual double EstimateStorageMB() {
    return card_cl_.EstimateStorageMB() + r_diag_cl_.EstimateStorageMB()
        + diag_cl_.EstimateStorageMB() + r_card_cl_.EstimateStorageMB();
  }
  virtual void PrintNodeInfo(nodeId n) {
    if (card_cl_.IsInitialized()) {
      std::cout << "Cardinal clearances:\t";
      std::cout << "N: " << card_cl_.GetClearance(n, dN) << "\t";
      std::cout << "E: " << card_cl_.GetClearance(n, dE) << "\t";
      std::cout << "S: " << card_cl_.GetClearance(n, dS) << "\t";
      std::cout << "W: " << card_cl_.GetClearance(n, dW) << std::endl;
    }
    if (diag_cl_.IsInitialized()) {
      std::cout << "Diagonal clearances:\t";
      std::cout << "NE: " << diag_cl_.GetClearance(n, dNE) << "\t";
      std::cout << "SE: " << diag_cl_.GetClearance(n, dSE) << "\t";
      std::cout << "SW: " << diag_cl_.GetClearance(n, dSW) << "\t";
      std::cout << "NW: " << diag_cl_.GetClearance(n, dNW) << std::endl;
    }
    if (r_diag_cl_.IsInitialized()) {
      std::cout << "Reverse diagonal clearances:\t";
      std::cout << "NE: " << r_diag_cl_.GetClearance(n, dNE) << "\t";
      std::cout << "SE: " << r_diag_cl_.GetClearance(n, dSE) << "\t";
      std::cout << "SW: " << r_diag_cl_.GetClearance(n, dSW) << "\t";
      std::cout << "NW: " << r_diag_cl_.GetClearance(n, dNW) << std::endl;
    }
    if (r_card_cl_.IsInitialized()) {
      std::cout << "Reverse cardinal clearances:\t";
      std::cout << "N: " << r_card_cl_.GetClearance(n, dN) << "\t";
      std::cout << "E: " << r_card_cl_.GetClearance(n, dE) << "\t";
      std::cout << "S: " << r_card_cl_.GetClearance(n, dS) << "\t";
      std::cout << "W: " << r_card_cl_.GetClearance(n, dW) << std::endl;
    }
  }

  void Reset() {
    existing_subgoals_.clear();
    existing_subgoal_distances_.clear();
    ResetClearanceLines();
  }

#ifndef NO_HOG
#ifdef DRAW_CLEARANCE_LINES
  virtual void Visualize(const MapEnvironment *env, int display_type =
                             kDisplayClearanceLines);
#endif
#endif

 protected:
  Grid* g_;
  S* s_;
  bool use_double_clearance_;
  Grid2DClearanceManager<Grid,S> card_cl_;
  Grid2DClearanceManager<Grid,S> diag_cl_;
  Grid2DClearanceManager<Grid,S> r_card_cl_;
  Grid2DClearanceManager<Grid,S> r_diag_cl_;

#ifdef DRAW_CLEARANCE_LINES
  std::vector<xyLin> line_begin_, line_end_, subgoal_line_begin_,
      subgoal_line_end_;
#endif


  void SafeFreespaceReachabilityConnect(nodeId source);
  bool GetDiagonalFirstPathIfExists(xyLin from, xyLin to,
                                    Distance & dist,
                                    std::vector<xyLin> & path);

  // Direct-safe-freespace connect
  int ScanCardinalForSubgoal(xyLin n, Direction2D c,
                             std::vector<xyLin> & subgoals,
                             int ext = std::numeric_limits<int>::max() / 2);
  void ScanDiagonalFirstForSafeSubgoals(
      xyLin n, Direction2D d,
      std::vector<xyLin> & subgoals);
  void ScanDiagonalFirstForSafeCorners(
      xyLin n, Direction2D d,
      std::vector<xyLin> & subgoals);

  void AddClearanceLine(xyLin begin, xyLin end) {
#ifdef DRAW_CLEARANCE_LINES
    line_begin_.push_back(begin);
    line_end_.push_back(end);
#endif
  }
  void AddSubgoalLine(xyLin begin, xyLin end) {
#ifdef DRAW_CLEARANCE_LINES
    subgoal_line_begin_.push_back(begin);
    subgoal_line_end_.push_back(end);
#endif
  }
  void ResetClearanceLines() {
#ifdef DRAW_CLEARANCE_LINES
    line_begin_.clear();
    line_end_.clear();
    subgoal_line_begin_.clear();
    subgoal_line_end_.clear();
#endif
  }

  // From the initial location s, repeatedly move in direction d.
  // If use_double_clearance_ is set, use the clearance table cl to jump over to
  // the next interesting location.
  // Call the function ProcessCellAndSignalStop at each visited cell.
  // Terminate when movement is no longer possible or the function signalled
  // stop.
  template<typename F>
  xyLin Traverse(xyLin s, Direction2D d, Grid2DClearanceManager<Grid, S> & cl,
                F ProcessCellAndSignalStop) {
    xyLin l = s;
    if (use_double_clearance_) {
      int d_cl = cl.GetClearance(l, d);
      while (d_cl > 0) {
        l = g_->Move(l, d, d_cl);
        d_cl = cl.GetClearance(l, d);
        if (ProcessCellAndSignalStop(l))
          break;
      }
    }
    else {
      while (g_->CanMove(l, d)) {
        l = g_->Move(l,d);
        if (ProcessCellAndSignalStop(l))
          break;
      }
    }
    return l;
  }
};

#include "GridReachabilityRelation.inc"

#endif /* APPS_SUBGOALGRAPH_GRAPHS_GRID2D_GRIDREACHABILITYRELATION_H_ */
