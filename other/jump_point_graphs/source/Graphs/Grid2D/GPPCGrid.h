/*
 * Grid2D8NeighborUtils.h
 *
 *  Created on: Nov 7, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_GRAPHS_GRID2D_GPPCGRID_H_
#define APPS_SUBGOALGRAPH_GRAPHS_GRID2D_GPPCGRID_H_

#include "Grid2D.h"

typedef uint32_t xydLin;

class GPPCGrid {
 public:

#ifndef NO_HOG
  GPPCGrid(const MapEnvironment *env);
#endif
  GPPCGrid(std::string mapname);
  GPPCGrid(Grid2D* g) {
    g_ = *g;
    Initialize();
  }
  ~GPPCGrid(){}

  void SetCosts(Distance c, Distance d) {card_cost_ = c; diag_cost_ = d;}

  int GetNumPaddedCells() const {return g_.GetNumPaddedCells();}
  int GetOriginalWidth() const {return g_.GetOriginalWidth();}
  int GetOriginalHeight() const {return g_.GetOriginalHeight();}

  bool IsTraversable(int x, int y) const {return g_.IsTraversable(x,y);}
  bool IsTraversable(xyLoc l) const {return g_.IsTraversable(l);}
  bool IsTraversable(xyLin l) const {return g_.IsTraversable(l);}

  void GetNeighbors(nodeId n, std::vector<WeightedArcHead> & neighbors);

  // CONVERSIONS (between xyLin, xydLin, xyLoc, direction2D)
  // To xyLin
  xyLin ToXYLin(int x, int y) const {return g_.ToXYLin(x,y);}
  xyLin ToXYLin(const xyLoc & l) const { return g_.ToXYLin(l);}
  // From xyLin
  xyLoc ToXYLoc(xyLin c) const {return g_.ToXYLoc(c);}
  // To xydLin
  xydLin ToXYDLin(xyLin c, Direction2D d) const
    {return (d + 1) * GetNumPaddedCells() + c;}
  xydLin ToXYDLin(xyLoc l, Direction2D d) const
    {return ToXYDLin(ToXYLin(l), d);}
  // From xydLin
  xyLin ExtractXYLin(xydLin c) const {return c % GetNumPaddedCells();}
  xyLoc ExtractXYLoc(xydLin c) const {return ToXYLoc(ExtractXYLin(c));}
  Direction2D ExtractDirection2D(xydLin c) {return c / GetNumPaddedCells() - 1;}

  // MOVEMENT
  inline xyLin Move(xyLin from, Direction2D d, int num_steps = 1) const;
  inline xyLoc Move(xyLoc from, Direction2D d, int num_steps = 1) const;
  inline bool CanMove(xyLin from, Direction2D d) const;
  inline bool CanMove(xyLoc from, Direction2D d) const;
  inline void GetMoveDirectionDetails(xyLoc from, xyLoc to, Direction2D & c,
                               Direction2D & d, int & nTotal, int & nTotalDiag);

  // MISC

  inline bool ConvexCornerAtDirection(xyLin l, Direction2D d) const;
  inline bool IsCorner(xyLin l) const;

  // HACKY EXTENSION TO DIRECTED
  void GetDirectedNeighbors(xydLin n, std::vector<WeightedArcHead> & neighbors);
  bool IsForcedCanonicalTaut(xyLin l, Direction2D in, Direction2D out) const {
    Direction2D d = canonical_taut_forcing_corner_direction_[in][out];
    if (d == dAll)
      return false;
    else
      return ConvexCornerAtDirection(l,d);
  }
  bool IsCanonicalTaut(xyLin l, Direction2D in, Direction2D out) const {
    return CanMove(l, out)
        && (IsFreespaceCanonicalTaut(in, out)
            || IsForcedCanonicalTaut(l, in, out));
  }

  void GetTautCanonicalSuccessorDirections(
      xyLin l, Direction2D d, std::vector<Direction2D> & directions) const;
  void GetTautCanonicalSuccessorDirections(
      xyLoc l, Direction2D d, std::vector<Direction2D> & directions) const {
    GetTautCanonicalSuccessorDirections(ToXYLin(l), d, directions);
  }

  uint8_t GetTautCanonicalSuccessorDirectionFlags(
      xyLin l, Direction2D d) const;

  void TestDirections(xyLoc l) const;

#ifndef NO_HOG
  // FIXME: Move to .cpp
  // VISUALIZATION
  void DrawNode(const MapEnvironment *env, int x, int y, int priority = 0) {
    g_.DrawNode(env, x, y, priority);
  }

  void DrawDirectedNode(const MapEnvironment *env, int x, int y, Direction2D d,
                        double length) {
    g_.DrawDirectedNode(env, x, y, (d * 45 - 90) * 0.0174533, length);
  }
  void DrawArrowNode(const MapEnvironment *env, int x, int y, Direction2D d,
                     double length, int priority = 0) {
    g_.DrawArrowNode(env, x, y, (d * 45 - 90) * 0.0174533, length, priority);
  }
  void DrawArrowNode(const MapEnvironment *env, xydLin l,
                     double length = 1, int priority = 0) {
    auto xy = ToXYLoc(ExtractXYLin(l));
    auto d = ExtractDirection2D(l);
    g_.DrawArrowNode(env, xy.x, xy.y, (d * 45 - 90) * 0.0174533, length, priority);
  }

  void DrawDiagonalFirstEdge(const MapEnvironment *env, int x1, int y1, int x2, int y2) {
    Direction2D c, d;
    int md, mt;
    this->GetMoveDirectionDetails(xyLoc(x1,y1), xyLoc(x2,y2), c, d, mt, md);

    xyLoc m = Move(xyLoc(x1,y1), d, md);

    env->GLDrawColoredLine(x1, y1, m.x, m.y);
    env->GLDrawColoredLine(m.x, m.y, x2, y2);
  }

  void DrawDiagonalFirstEdge(const MapEnvironment *env, int x1, int y1, int x2,
                             int y2, Direction2D out_dir) {
    Direction2D c, d;
    int md, mt;
    this->GetMoveDirectionDetails(xyLoc(x1,y1), xyLoc(x2,y2), c, d, mt, md);

    xyLoc m = Move(xyLoc(x1,y1), d, md);

    double x1_offset = 0.4 * cos((out_dir * 45 - 90) * 0.0174533);
    double y1_offset = 0.4 * sin((out_dir * 45 - 90) * 0.0174533);

    //env->GLDrawColoredLine(x1 + x1_offset, y1 + y1_offset, m.x, m.y);
    //env->GLDrawColoredLine(m.x, m.y, x2, y2);
    env->GLDrawColoredLine(x1 + x1_offset, y1 + y1_offset, x2, y2);
  }

  void DrawDiagonalFirstGoingRightEdge(const MapEnvironment *env, int x1,
                                       int y1, int x2, int y2) {
    Direction2D c, d;
    int md, mt;
    this->GetMoveDirectionDetails(xyLoc(x1,y1), xyLoc(x2,y2), c, d, mt, md);

    xyLoc m = Move(xyLoc(x1,y1), d, md);

    if (x1 > x2)
      m = Move(xyLoc(x1,y1), c, mt-md);

    env->GLDrawColoredLine(x1, y1, m.x, m.y);
    env->GLDrawColoredLine(m.x, m.y, x2, y2);
  }
#endif

 private:
  Grid2D g_;
  Distance card_cost_, diag_cost_;

  int delta_xylin_[24]; // FIXME: size 8
  int delta_x_[8];
  int delta_y_[8];

  // canonical_taut_forcing_corner_direction_[di][do] returns the
  // diagonal direction of a convex corner that would make (di,do) forced taut.
  // Returns dAll if no such corner exists.
  Direction2D canonical_taut_forcing_corner_direction_[8][8];

  void Initialize();
  void CalculateLocationDeltas();
  void CalculateCanonicalTautForcingCornerDirections();
};

// MOVEMENT
inline xyLin GPPCGrid::Move(xyLin from, Direction2D d, int num_steps) const {
  return from + delta_xylin_[d]*num_steps;
}
inline xyLoc GPPCGrid::Move(xyLoc from, Direction2D d, int num_steps) const {
  return xyLoc(from.x + delta_x_[d]*num_steps, from.y + delta_y_[d]*num_steps);
}
inline bool GPPCGrid::CanMove(xyLin from, Direction2D d) const {
  // If it is a cardinal move
  if ((d & 1) == 0)
    return IsTraversable(Move(from, d));
  else
    return IsTraversable(Move(from, d)) && IsTraversable(Move(from, d + 1))
        && IsTraversable(Move(from, d - 1));
}
inline bool GPPCGrid::CanMove(xyLoc from, Direction2D d) const {
  // If it is a cardinal move
  if ((d & 1) == 0 && IsTraversable(Move(from, d)))
    return true;
  else
    return IsTraversable(Move(from, d)) && IsTraversable(Move(from, d + 1))
        && IsTraversable(Move(from, d - 1));
}

inline void GPPCGrid::GetMoveDirectionDetails(xyLoc from, xyLoc to, Direction2D & c,
                                       Direction2D & d, int & total_moves,
                                       int & diag_moves) {

  int dx = (from.x > to.x) ? (from.x - to.x) : (to.x - from.x);
  int dy = (from.y > to.y) ? (from.y - to.y) : (to.y - from.y);
  // Total number of diagonal and cardinal moves that must be made to reach the target
  total_moves = (dx > dy) ? (dx) : (dy);
  // Total number of diagonal moves that must be made to reach the target
  diag_moves = (dx < dy) ? (dx) : (dy);

  // Only make this kind of cardinal moves
  c = (dx > dy) ? ((from.x < to.x) ? dE : dW) : ((from.y < to.y) ? dS : dN);
  // Only make this kind of diagonal moves
  d = (from.x < to.x) ?
      ((from.y < to.y) ? dSE : dNE) : ((from.y < to.y) ? dSW : dNW);

  // FIXME: Lookup directions rather than calculating them.
  /*
  // dx > dy, from.x < to. x, from.y < to.y
  static int c_move_direction[2][2][2] = {
      {{}, {}}, {{}, {}},
      {{}, {}}, {{}, {}}
  };

  // from.x < to.x, from.y < to.y
  static int d_move_direction[2][2] = {

  };
  */

}

// MISC
// FIXME: Use Move and IsTraversable
inline bool GPPCGrid::ConvexCornerAtDirection(xyLin l, Direction2D d) const {
  return !CanMove(l,d) && CanMove(l,CW(d)) && CanMove(l,CCW(d));
}
inline bool GPPCGrid::IsCorner(xyLin l) const {
  if (!IsTraversable(l))
    return false;
  for (Direction2D d = 1; d <= 7; d += 2)
    if (ConvexCornerAtDirection(l, d))
      return true;
  return false;
}

#endif /* APPS_SUBGOALGRAPH_GRAPHS_GRID2D_GPPCGRID_H_ */
