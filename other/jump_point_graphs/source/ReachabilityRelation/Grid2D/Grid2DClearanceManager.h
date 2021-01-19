/*
 * Grid3DCardinalClearanceManager.h
 *
 *  Created on: Oct 9, 2016
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH3DGRID_GRID3DCLEARANCEMANAGER_H_
#define APPS_SUBGOALGRAPH3DGRID_GRID3DCLEARANCEMANAGER_H_

#include "Grid2DGraph.h"
#include "CPUTimer.h"

#include <iostream>

// For each cell, stores the distance to the closest subgoal or obstacle
// towards a cardinal direction.

// Assumes that grid node ids are obtained by linearizing the grid (padding).

// Allows queries in 4 directions (N-0, E-2, S-4, W-6), but can also be
// queried with directions up to 23, where it returns the result for
// direction % 8

// 32 bits are used to represent clearances in 4 directions.
// Max clearance is 255.
const int kGrid2DMaxClearance = 254;
const int kGrid2DClearanceJumpSignal = 255;

// If the clearance of a cell is greater than 255, we simply jump ahead 255
// cells and look-up the clearance of that cell (and add 255).
// If the source cell is a subgoal, its clearance in all directions are 0.
// This is so that we don't have an ambiguity when we jump 255 cells (it could
// either be a blank cell or a subgoal).
// TODO: We could circumvent this by storing the subgoal id mapper, but
// not sure if its worth it at the moment.

// ClearanceMask and ClearanceShift are used to efficiently implement storing
// and retrieving clearances.
const uint32_t kGrid2DClearanceMask = 0x000000FF;
const int kGrid2DCardinalClearanceShift[8] = { 0, 0, 8, 8, 16, 16, 24, 24 };
                           //	Cardinal only: {0,0,8,0,16,0,24,0};

#define CALCULATE_CLEARANCES_FOR_SUBGOALS

// Grid: Grid (Check for obstacles)
// S: Subgoal ids (Check for subgoals)
template<class Grid, class S>
class Grid2DClearanceManager {
public:
	Grid2DClearanceManager(Grid* g, S* s)
      : g_(g), s_(s) {
    initialized_ = false;
	}
	~Grid2DClearanceManager() {}

	bool IsInitialized() const {return initialized_;}

  double CalculateCardinalClearancesToCorners() {
    return CalculateCardinalClearances(
        [&](xyLin n, Direction2D d) {return g_->IsCorner(n);});
	}

  double CalculateDiagonalClearancesToCorners() {
    return CalculateDiagonalClearances(
        [&](xyLin n, Direction2D d) {return g_->IsCorner(n);});
  }

  // Calculates all the clearances and returns the time.
	template<typename F>
	double CalculateCardinalClearances(F should_stop);

  // Calculates all the clearances and returns the time.
  template<typename F>
  double CalculateDiagonalClearances(F should_stop);

	int GetClearance(xyLin n, Direction2D d) const {
	  int c = 0;
		while (ReadClearance(n,d) == kGrid2DClearanceJumpSignal) {
		  c += kGrid2DMaxClearance;
		  n = g_->Move(n, d, kGrid2DMaxClearance);
		}
		return c + ReadClearance(n,d);
	}

	double EstimateStorageMB() const {
	  if (initialized_)
	    return c_.size()*sizeof(uint32_t)/(1024.0*1024.0);
	  else
	    return 0;
	}

	void PrintClearances(xyLin n) {
	  if (!initialized_)
	    return;
	  std::cout<<"Clearances:\t";
    std::cout<<"N: "<<GetClearance(n, dN)<<"\t";
    std::cout<<"E: "<<GetClearance(n, dE)<<"\t";
    std::cout<<"S: "<<GetClearance(n, dS)<<"\t";
    std::cout<<"W: "<<GetClearance(n, dW)<<std::endl;
	}

private:
	std::vector<uint32_t> c_;
	Grid* g_;
	S* s_;
	bool initialized_;

  void Initialize() {
    c_.resize(g_->GetNumPaddedCells(),0);
    initialized_ = true;
  }

	// Read/write the 8-bit clearance.
	int ReadClearance(xyLin n, Direction2D d) const {
		return (c_[n] >> kGrid2DCardinalClearanceShift[d]) & kGrid2DClearanceMask;
	}
	void WriteClearance(xyLin n, Direction2D d, int c) {
		c_[n] =
			// Clear existing clearance.
			(c_[n] & ~ (kGrid2DClearanceMask << kGrid2DCardinalClearanceShift[d]))
			// Write new clearance (up to max clearance)
			| (((c <= kGrid2DMaxClearance)?c:kGrid2DClearanceJumpSignal) << kGrid2DCardinalClearanceShift[d]);
	}

	// Generates clearances for each cell in all four directions.
	template<typename F>
  void CalculateClearancesSubroutine(int x, int y, Direction2D d,
                                     int & clearance, bool & point_of_interest,
                                     F IsPointOfInterest) {
	  xyLin loc = g_->ToXYLin(x, y);

    if (!g_->IsTraversable(loc)) {
      clearance = 0;
      point_of_interest = false;
    }
    else {
      if (point_of_interest)
        clearance++;

      if (g_->CanMove(loc, d))
        WriteClearance(loc, d, clearance);
      else {
        clearance = 0;
        point_of_interest = false;
      }

      if (IsPointOfInterest(loc, d)) {
        clearance = 0;
        point_of_interest = true;
      }
    }
	}
};

template<class Grid, class S>
template<typename F>
double Grid2DClearanceManager<Grid, S>::CalculateCardinalClearances(
    F should_stop) {
  CPUTimer t;
  t.StartTimer();
  Initialize();
  int width = g_->GetOriginalWidth();
  int height = g_->GetOriginalHeight();

  // North clearances
  Direction2D d = dN;
  for (int x = 0; x < (int) width; x++) {
    int clearance = 0;
    bool point_of_interest = false;
    for (int y = 0; y < (int) height; y++) {
      CalculateClearancesSubroutine(x, y, d, clearance, point_of_interest,
                                    should_stop);
    }
  }

  // South clearances
  d = dS;
  for (int x = 0; x < (int) width; x++) {
    int clearance = 0;
    bool point_of_interest = false;
    for (int y = (int) height - 1; y >= 0; y--) {
      CalculateClearancesSubroutine(x, y, d, clearance, point_of_interest,
                                    should_stop);
    }
  }

  // West clearances
  d = dW;
  for (int y = 0; y < (int) height; y++) {
    int clearance = 0;
    bool point_of_interest = false;
    for (int x = 0; x < (int) width; x++) {
      CalculateClearancesSubroutine(x, y, d, clearance, point_of_interest,
                                    should_stop);
    }
  }

  // East clearances
  d = dE;
  for (int y = 0; y < (int) height; y++) {
    int clearance = 0;
    bool point_of_interest = false;
    for (int x = (int) width - 1; x >= 0; x--) {
      CalculateClearancesSubroutine(x, y, d, clearance, point_of_interest,
                                    should_stop);
    }
  }

  t.EndTimer();
  return t.GetElapsedTime();
}


template<class Grid, class S>
template<typename F>
double Grid2DClearanceManager<Grid, S>::CalculateDiagonalClearances(
    F should_stop) {
  CPUTimer t;
  t.StartTimer();
  Initialize();
  int w = g_->GetOriginalWidth();
  int h = g_->GetOriginalHeight();

  // FIXME: Potentially scanning twice the grid size, but meh.
  Direction2D d = dSW;
  for (int i = 0; i < w + h - 1; i++) {
    int clearance = 0;
    bool point_of_interest = false;
    for (int x = 0; x < w; x++) {
      int y = i - x;
      if (0 <= y && y < h) {
        CalculateClearancesSubroutine(x, y, d, clearance, point_of_interest,
                                      should_stop);
      }
    }
  }

  d = dNE;
  for (int i = 0; i < w + h - 1; i++) {
    int clearance = 0;
    bool point_of_interest = false;
    for (int x = w - 1; x >= 0; x--) {
      int y = i - x;
      if (0 <= y && y < h) {
        CalculateClearancesSubroutine(x, y, d, clearance, point_of_interest,
                                      should_stop);
      }
    }
  }

  d = dNW;
  for (int i = -w + 1; i < h; i++) {
    int clearance = 0;
    bool point_of_interest = false;
    for (int x = 0; x < w; x++) {
      int y = i + x;
      if (0 <= y && y < h) {
        CalculateClearancesSubroutine(x, y, d, clearance, point_of_interest,
                                      should_stop);
      }
    }
  }

  d = dSE;
  for (int i = -w + 1; i < h; i++) {
    int clearance = 0;
    bool point_of_interest = false;
    for (int x = w - 1; x >= 0; x--) {
      int y = i + x;
      if (0 <= y && y < h) {
        CalculateClearancesSubroutine(x, y, d, clearance, point_of_interest,
                                      should_stop);
      }
    }
  }

  t.EndTimer();
  return t.GetElapsedTime();
}

#endif /* APPS_SUBGOALGRAPH3DGRID_GRID3DCLEARANCEMANAGER_H_ */
