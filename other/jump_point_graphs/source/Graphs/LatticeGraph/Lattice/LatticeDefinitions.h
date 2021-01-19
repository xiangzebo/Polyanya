#ifndef XYTHETA_LATTICE_DEFINITIONS_H
#define XYTHETA_LATTICE_DEFINITIONS_H

#include <vector>
#include <iostream>
#include "GridDefinitions.h"

#ifndef NO_HOG  // Use '-DNO_HOG' in the makefile to compile this outside the HOG platform.
#define MOTION_LATTICE_RUNNING_IN_HOG    // Otherwise, the program assumes that it is running in HOG, which enables a specific constructor and visualization for HOG.
#include "Map2DEnvironment.h"
#include "FPUtil.h"
#endif


// Three different representations:
// xyThetaLoc: The actual location of the agent
// xyThetaPaddedLoc: The corresponding location of the agent after surrounding
// the map with obstacles.
// nodeId: The linearized id of the xyThetaPaddedLoc.

struct xyPos {
  xyPos(int _x = -1, int _y = -1)
      : x(_x),
        y(_y) {
  }
  int x;
  int y;
};

struct xyThetaPos {
  xyThetaPos() {x = 0; y = 0; o = 0;}
  xyThetaPos(int _x, int _y, int _o)
      : x(_x),
        y(_y),
        o(_o) {
  }
  int x;
  int y;
  int o;
};

static std::ostream& operator <<(std::ostream & out, const xyThetaPos &loc) {
  out << "(" << loc.x << ", " << loc.y << ", " << loc.o << ")";
  return out;
}

static bool operator==(const xyThetaPos &l1, const xyThetaPos &l2) {
  return (l1.x == l2.x) && (l1.y == l2.y)
      && (l1.o == l2.o);
}


// If x and y are not required to be integers (for storing intermediate poses).
struct xyThetaPosCont {
  xyThetaPosCont(double _x = -1, double _y = -1, double _o = -1)
      : x(_x),
        y(_y),
        orientation(_o) {
  }
  double x;
  double y;
  double orientation;  // In radians?
};

#endif
