/*
 * LatticeGeometry.h
 *
 *  Created on: Oct 26, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_GRAPHS_LATTICE_LATTICEGEOMETRY_H_
#define APPS_SUBGOALGRAPH_GRAPHS_LATTICE_LATTICEGEOMETRY_H_

#include <algorithm>
#include "LatticeDefinitions.h"

class LatticeGeometry {
 public:
  LatticeGeometry() {
  }
  ~LatticeGeometry() {
  }

  // Inputs: start-end denote a line that the agent traverses.
  // Radius is the vehicle's radius
  // Finds all the cells swept by the agent of radius 'radius' moving from
  // 'start' to 'end'.
  std::vector<xyPos> GetLineSweepIntersectedCells(xyPosCont start, xyPosCont end,
                                                double radius) {
    // Bounding box containing relevant cells
    int top = floor(fmin(start.y, end.y) - radius - 0.01);
    int bottom = ceil(fmax(start.y, end.y) + radius + 0.01);
    int left = floor(fmin(start.x, end.x) - radius - 0.01);
    int right = ceil(fmax(start.x, end.x) + radius + 0.01);

    std::vector<xyPos> swept_cells;

    for (int x = left; x <= right; x++)
      for (int y = top; y <= bottom; y++)
        if (LineSweepIntersectsCell(start, end, radius, xyPosCont(x, y)))
          swept_cells.push_back(xyPos(x, y));
    return swept_cells;
  }

  std::vector<xyPos> GetPositionIntersectedCells(xyPosCont pos, double radius) {
    // Bounding box containing relevant cells
    int top = floor(pos.y - radius - 0.01);
    int bottom = ceil(pos.y + radius + 0.01);
    int left = floor (pos.x - radius - 0.01);
    int right = ceil (pos.x + radius + 0.01);

    std::vector<xyPos> cells;

    for (int x = left; x <= right; x++)
      for (int y = top; y <= bottom; y++)
        if (PositionIntersectsCell(pos, xyPosCont(x,y), radius))
          cells.push_back(xyPos(x, y));
    return cells;
  }

 private:
  bool PositionIntersectsCell(xyPosCont pos, xyPosCont cell_center, double r) {
    xyPosCont nw(cell_center.x - 0.5, cell_center.y - 0.5);
    xyPosCont ne(cell_center.x + 0.5, cell_center.y - 0.5);
    xyPosCont sw(cell_center.x - 0.5, cell_center.y + 0.5);
    xyPosCont se(cell_center.x + 0.5, cell_center.y + 0.5);

    double d_n = DistanceToLineSegment(nw, ne, pos);
    double d_e = DistanceToLineSegment(ne, se, pos);
    double d_s = DistanceToLineSegment(se, sw, pos);
    double d_w = DistanceToLineSegment(sw, nw, pos);

/*
    std::cout<<"Agent at: "<<pos<<", cell: "<<cell_center<<std::endl;
    std::cout<<"Northwest corner: "<<nw<<std::endl;
    std::cout<<"Northeast corner: "<<ne<<std::endl;
    std::cout<<"Southwest corner: "<<sw<<std::endl;
    std::cout<<"Southeast corner: "<<se<<std::endl;
    std::cout<<"Distance to north border: "<<d_n<<std::endl;
    std::cout<<"Distance to east border: "<<d_e<<std::endl;
    std::cout<<"Distance to south border: "<<d_s<<std::endl;
    std::cout<<"Distance to west border: "<<d_w<<std::endl;
*/

    double min_dist = fmin(fmin(d_n, d_e), fmin(d_s, d_w));

    if (min_dist < r)
      return true;

    if (nw.x < pos.x && pos.x < se.x &&
        nw.y < pos.y && pos.y < se.y)
      return true;

    return false;
  }

  bool LineSweepIntersectsCell(xyPosCont start, xyPosCont end, double radius,
                               xyPosCont cell_center) {
    xyPosCont nw(cell_center.x - 0.5, cell_center.y - 0.5);
    xyPosCont ne(cell_center.x + 0.5, cell_center.y - 0.5);
    xyPosCont sw(cell_center.x - 0.5, cell_center.y + 0.5);
    xyPosCont se(cell_center.x + 0.5, cell_center.y + 0.5);

    if (DistanceToLineSegment(start, end, nw) < radius - 0.01)
      return true;
    if (DistanceToLineSegment(start, end, ne) < radius - 0.01)
      return true;
    if (DistanceToLineSegment(start, end, sw) < radius - 0.01)
      return true;
    if (DistanceToLineSegment(start, end, se) < radius - 0.01)
      return true;

    if (PositionIntersectsCell(start, cell_center, radius))
      return true;
    if (PositionIntersectsCell(end, cell_center, radius))
      return true;

    return false;
  }

  double DistanceToLine(xyPosCont l1, xyPosCont l2, xyPosCont p) {
    double nominator = fabs(
        (l2.y - l1.y) * p.x - (l2.x - l1.x) * p.y + l2.x * l1.y - l2.y * l1.x);
    double denominator = sqrt(
        (l2.y - l1.y) * (l2.y - l1.y) + (l2.x - l1.x) * (l2.x - l1.x));
    return nominator / denominator;
  }

  double DistanceToLineSegment(xyPosCont v, xyPosCont w, xyPosCont p) {
    if (v == w)
      return Length(v,p);

    double length_squared = LengthSquared(v,w);
    double t = fmax(0, fmin(1, (DotProduct(p - v, w - v))/length_squared));

    xyPosCont dl = w - v;
    xyPosCont projection = v + xyPosCont(dl.x*t, dl.y*t);

    return Length(p, projection);
  }

  double LengthSquared(xyPosCont l1, xyPosCont l2) {
    double dx = l2.x - l1.x;
    double dy = l2.y - l1.y;
    return dx*dx + dy*dy;
  }

  double Length(xyPosCont l1, xyPosCont l2) {
    return sqrt(LengthSquared(l1,l2));
  }

  double DotProduct(xyPosCont l1, xyPosCont l2) {
    return l1.x*l2.x + l1.y*l2.y;
  }
};

#endif /* APPS_SUBGOALGRAPH_GRAPHS_LATTICE_LATTICEGEOMETRY_H_ */
