#ifndef XYTHETA_MOTION_PRIMITIVE_H
#define XYTHETA_MOTION_PRIMITIVE_H

#include <stdio.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

#include "../GraphDefinitions.h"
#include "LatticeDefinitions.h"
#include "LatticeGeometry.h"
#include "DefaultParams.h"

// A primitive can take the agent from anywhere on the grid to anywhere else
// (not necessarily to an adjacent cell).
// Each primitive is applicable at a specific heading angle, and the resulting
// heading angle can be different from the starting one.

// Each primitive is specified by a sequence of intermediate poses. These
// intermediate poses are used for visualization and, more importantly,
// to determine the cells that the agent passes through while executing the
// primitive. We assume that the primitive is executable iff all the cells that
// the agent passes through are unblocked (and the initial heading angle of the
// agent matches the primitive's requirement).
class Lattice;
class LatticeGraph;

class MotionPrimitive {
 public:
  friend class Lattice;
  friend class LatticeGraph;
  MotionPrimitive() {
  }
  MotionPrimitive(int sa, int ex, int ey, int ea,
                         std::vector<xyThetaPosCont> & intermediate_poses,
                         double cost_multiplier,
                         double radius);
  ~MotionPrimitive() {
  }

  void CalculateSweptCells();
  void CalculateSweptCellsGivenRadius(double radius);

  // Override the current cost.
  void SetCostMultiplier(double c) {
    cost_multiplier_ = c;
    CalculateCost();
  }

  double GetCost() const {
    return cost_;
  }
  int GetDiscreteStartAngle() const {
    return disc_start_orientation_;
  }
  int GetDiscreteEndAngle() const {
    return disc_end_orientation_;
  }

  // Returns the highest (absolute) x or y coordinate among all swept cells.
  int GetReach() const {
    int reach = 0;
    for (unsigned int i = 0; i < swept_cells_.size(); i++) {
      if (abs(swept_cells_[i].x) > reach)
        reach = abs(swept_cells_[i].x);
      if (abs(swept_cells_[i].y) > reach)
        reach = abs(swept_cells_[i].y);
    }
    return reach;
  }

  const std::vector<xyPos>* GetSweptCells() const {
    return &swept_cells_;
  }
  const std::vector<int>* GetSweptCellLinearizedPaddedCoordinateOffsets() const {
    return &swept_cell_linearized_padded_coordinate_offsets_;
  }
  void SetSweptCellLinearizedPaddedCoordinateOffsets(
      const std::vector<int> & offsets) {
    swept_cell_linearized_padded_coordinate_offsets_ = offsets;
  }

  void SetDeltaNodeId(nodeId delta) {
    delta_node_id_ = delta;
  }

  nodeId GetResultingNodeId(nodeId from) {
    return from + delta_node_id_;
  }

  // Given the position of the agent, return the end position after applying
  // the primitive. (Assumes the starting orientation is correct.)
  xyThetaPos GetResultingPos(xyThetaPos p) const {
    return xyThetaPos(p.x + delta_x_, p.y + delta_y_, disc_end_orientation_);
  }

  MotionPrimitive GetReversePrimitive();

  void Print(std::ostream & out = std::cout, bool print_intermediate_poses =
                 true,
             bool print_swept_cells = true,
             bool print_swept_cell_offsets = true);

#ifndef NO_HOG
  void DrawPrimitive(const MapEnvironment *env, int x, int y) const;
#endif

 private:
  // Primitive takes the agent from (0, 0, discrete start orientation) to
  // (delta x, delta y, discrete goal orientation).
  int disc_start_orientation_, delta_x_, delta_y_, disc_end_orientation_;

  // Real world coordinates of points that the agent passes through
  // (and the angle in radians at each point)
  std::vector<xyThetaPosCont> intermediate_poses_;

  // Length: The distance traveled.
  // Calculated as the sum of straight lines connecting intermediate poses.
  double length_;

  // Cost of primitive = length of primitive * cost_multiplier_
  double cost_, cost_multiplier_;

  // Location of the cells traversed by the agent, relative to the starting
  // location of the agent.
  double agent_radius_;
  std::vector<xyPos> swept_cells_;


  // Use coordinate offsets rather than cell offsets for faster collision
  // checks.
  std::vector<int> swept_cell_linearized_padded_coordinate_offsets_;

  // Use delta id for faster successor generation.
  nodeId delta_node_id_;

  void CalculateCost() {
    cost_ = length_ * cost_multiplier_;
    if (cost_ < 1)
      cost_ = 1;

    if (kRoundLatticePrimCosts512) {
      std::cout<<"Old cost: "<<cost_<<"\t";
      cost_ = std::round(cost_ * 1024) / 1024;
      std::cout<<"New cost: "<<cost_<<std::endl;
    }

  }
};

#endif
