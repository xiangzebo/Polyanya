#ifndef GRID_2D_H
#define GRID_2D_H

#include "GraphDefinitions.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <stdint.h>
#include "GridDefinitions.h"

#ifndef NO_HOG
#include "Map2DEnvironment.h"
#endif

#include "Direction2D.h"

// Linearized, padded coordinate.
typedef uint32_t xyLin;

// Stores the grid as a linearized vector.
// Can add a border of blocked cells around the map, whose thickness is
// referred to as the 'padding width'.
// Linearization is done after the padding.

class Grid2D {
 public:
  Grid2D() {};

#ifndef NO_HOG
  Grid2D(const MapEnvironment *env, int padding = 1) {
    GenerateGrid(env, padding);
  }
  void GenerateGrid(const MapEnvironment *env, int padding = 1);
#endif

  Grid2D(std::string mapname, int padding = 1) {
    GenerateGrid(mapname, padding);
  }
  void GenerateGrid(std::string mapname, int padding = 1);

  ~Grid2D() {
  }

  bool IsTraversable(int x, int y) const {
    return traversable_[ToXYLin(x, y)];
  }
  bool IsTraversable(const xyLoc & l) const {
    return traversable_[ToXYLin(l)];
  }
  bool IsTraversable(xyLin l) const {
    return traversable_[l];
  }

  // Convert between xyLoc and linearized padded coordinates (nodeId in the
  // context of graphs).
  xyLin ToXYLin(int x, int y) const {
    return y_mult_ * y + x + padding_offset_;
  }
  xyLin ToXYLin(const xyLoc & l) const {
    return y_mult_ * l.y + l.x + padding_offset_;
  }
  xyLoc ToXYLoc(xyLin c) const {
    return xyLoc((c - padding_offset_) % y_mult_,
                 (c - padding_offset_) / y_mult_);
  }
  void ExtractXY(xyLin c, int & x, int & y) const {
    x = (c - padding_offset_) % y_mult_;
    y = (c - padding_offset_) / y_mult_;
  }

  // Returns by how much the linearized padded location changes if x and y,
  // respectively, are increased by 1.
  int GetXMult() const {return 1;}
  int GetYMult() const {return y_mult_;}
  int GetOriginalWidth()  const {return width_;}
  int GetOriginalHeight() const {return height_;}
  int GetPaddedWidth()    const {return padded_width_;}
  int GetPaddedHeight()   const {return padded_height_;}
  int GetNumCells()       const {return width_ * height_;}
  int GetNumPaddedCells() const {return padded_width_ * padded_height_;}

  void PrintGridInfo(std::ostream & out = std::cout) const {
    out << "Grid contains " << num_blocked_ << " blocked cells and "
        << num_unblocked_ << " unblocked cells." << std::endl;
    out << "Original grid dimensions: " << width_ << " x " << height_
        << std::endl;
    out << "Padded grid dimensions: " << padded_width_ << " x "
        << padded_height_ << std::endl;
    out << "Linearization function: x * " << GetXMult() << " + y * "
        << GetYMult() << " + " << padding_offset_ << std::endl;
  }
  void GetNeighbors(xyLoc l, std::vector<xyLoc> & neighbors,
                    bool eight_neighbor = false, bool diagonal_rule = false);

#ifndef NO_HOG
  void DrawNode(const MapEnvironment *env, int x, int y, int priority = 0);
  void DrawDirectedNode(const MapEnvironment *env, int x, int y, double radian,
                        double length);
  void DrawArrowNode(const MapEnvironment *env, int x, int y, double radian,
                     double length, int priority);
#endif


 private:
  // Original dimensions of the grid.
  int width_, height_;

  // Surround the grid with a padding with thickness equal to 'padding_'.
  int padding_;
  int padded_width_, padded_height_;

  // Adjusting the y-coordinate by 1 adjusts the linearized padded coordinates
  // by y_mult. (x_mult_ = 1)
  int y_mult_;

  // To convert original x and y coordinates to linearized padded location:
  // y*y_mult + x + padding_offset_ .
  // Padding offset is calculated as y_mult_*padding_ + padding_
  // = linearized location of 0,0.
  int padding_offset_;

  int num_blocked_, num_unblocked_;

  // Linearized representation of the grid.
  std::vector<bool> traversable_;
};

#endif
