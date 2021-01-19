/*
 * Direction2D.h
 *
 *  Created on: Nov 7, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_GRAPHS_GRID2D_DIRECTION2D_H_
#define APPS_SUBGOALGRAPH_GRAPHS_GRID2D_DIRECTION2D_H_
#include <cassert>

// TODO: Move to grid graph?
#define EIGHT_CONNECTED
#define DIAGONAL_MOVEMENT_RULE
#define CARD_COST 1.000000000
#define DIAG_COST 1.414121356
#define DIAG_DIFF 0.414121356

// 0   1   2   3   4   5   6   7
// N  NE   E  SE   S  SW   W  NW
typedef int32_t Direction2D;

const Direction2D dAll = -1;
const Direction2D dN = 0;
const Direction2D dNE = 1;
const Direction2D dE = 2;
const Direction2D dSE = 3;
const Direction2D dS = 4;
const Direction2D dSW = 5;
const Direction2D dW = 6;
const Direction2D dNW = 7;

inline bool IsSpecificDirection2D(Direction2D d) {
  return 0 <= d && d <= 7;
}

inline std::string GetDirection2DName(Direction2D d) {
  assert(IsSpecificDirection2D(d) || d == dAll);

  const std::string names[9] = { "N", "NE", "E", "SE", "S", "SW", "W", "NW" };

  if (d == dAll)
    return "All";
  else
    return names[d];
}

inline Direction2D Reverse(Direction2D d) {
//  assert(IsSpecificDirection2D(d));
  return (d + 4) & 7; //% 8;
}

inline Direction2D CW(Direction2D d) {
//  assert(IsSpecificDirection2D(d));
  return (d + 1) & 7; //% 8;
}

inline Direction2D CW(Direction2D d, int n) {
//  assert(IsSpecificDirection2D(d));
  return (d + n) & 7; //% 8;
}

inline Direction2D CCW(Direction2D d) {
//  assert(IsSpecificDirection2D(d));
  return (d + 7) & 7; //% 8;
}


inline bool IsCardinal(Direction2D d) {
  return d == dN || d == dS || d == dE || d == dW;
}

inline bool IsDiagonal(Direction2D d) {
  return d == dNW || d == dNE || d == dSW || d == dSE;
}
inline bool IsFreespaceCanonicalTaut(Direction2D d1, Direction2D d2) {
  static bool is_canonical[8][8] = {
  //   N NE  E SE  S SW  W NW
      {1, 0, 0, 0, 0, 0, 0, 0}, // N
      {1, 1, 1, 0, 0, 0, 0, 0}, // NE
      {0, 0, 1, 0, 0, 0, 0, 0}, // E
      {0, 0, 1, 1, 1, 0, 0, 0}, // SE
      {0, 0, 0, 0, 1, 0, 0, 0}, // S
      {0, 0, 0, 0, 1, 1, 1, 0}, // SW
      {0, 0, 0, 0, 0, 0, 1, 0}, // W
      {1, 0, 0, 0, 0, 0, 1, 1}  // NW
      };

  return is_canonical[d1][d2];
}


inline bool IsFreespaceTaut(Direction2D d1, Direction2D d2) {
  static bool is_taut[8][8] = {
  //   N NE  E SE  S SW  W NW
      {1, 1, 0, 0, 0, 0, 0, 1}, // N
      {1, 1, 1, 0, 0, 0, 0, 0}, // NE
      {0, 1, 1, 1, 0, 0, 0, 0}, // E
      {0, 0, 1, 1, 1, 0, 0, 0}, // SE
      {0, 0, 0, 1, 1, 1, 0, 0}, // S
      {0, 0, 0, 0, 1, 1, 1, 0}, // SW
      {0, 0, 0, 0, 0, 1, 1, 1}, // W
      {1, 0, 0, 0, 0, 0, 1, 1}  // NW
      };

  return is_taut[d1][d2];
}

#endif /* APPS_SUBGOALGRAPH_GRAPHS_GRID2D_DIRECTION2D_H_ */
