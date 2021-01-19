/*
 * GridDefinitions.h
 *
 *  Created on: Nov 5, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_GRAPHS_GRID2D_GRIDDEFINITIONS_H_
#define APPS_SUBGOALGRAPH_GRAPHS_GRID2D_GRIDDEFINITIONS_H_


#ifdef NO_HOG
// Redefine xyLoc as in HOG
#include <cstdint>
#include <iostream>
struct xyLoc {
public:
  xyLoc() {}
  xyLoc(uint16_t _x, uint16_t _y) :x(_x), y(_y) {}
  uint16_t x;
  uint16_t y;
};

static bool operator==(const xyLoc &l1, const xyLoc &l2) {
  return (l1.x == l2.x) && (l1.y == l2.y);
}

static std::ostream& operator <<(std::ostream & out, const xyLoc &loc)
{
  out << "(" << loc.x << ", " << loc.y << ")";
  return out;
}
#endif


struct xyPosCont {
  xyPosCont(double _x = -1, double _y = -1)
      : x(_x),
        y(_y) {
  }
  double x;
  double y;
};

static xyPosCont operator+(const xyPosCont &l1, const xyPosCont &l2) {
  return xyPosCont(l1.x+l2.x, l1.y+l2.y);
}

static xyPosCont operator-(const xyPosCont &l1, const xyPosCont &l2) {
  return xyPosCont(l1.x-l2.x, l1.y-l2.y);
}

static bool operator==(const xyPosCont &l1, const xyPosCont &l2) {
  return fabs(l1.x - l2.x) < 0.001 && fabs(l1.y - l2.y) < 0.001;
}

static std::ostream& operator <<(std::ostream & out, const xyPosCont &loc) {
  out << "(" << loc.x << ", " << loc.y << ")";
  return out;
}




#endif /* APPS_SUBGOALGRAPH_GRAPHS_GRID2D_GRIDDEFINITIONS_H_ */
