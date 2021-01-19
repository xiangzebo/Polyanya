#ifndef XYTHETA_LATTICE_H
#define XYTHETA_LATTICE_H

#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <cassert>

#include "Grid2D.h"
#include "LatticeDefinitions.h"
#include "MotionPrimitive.h"
#include "LatticeParam.h"

// A Lattice is simply a pairing of a grid with a set of motion primitives that
// are applicable at certain discrete heading angles that move the agent
// to a different position on the grid (relative to its starting position) and
// (possibly) change its heading angle.
// Currently, there are two ways to construct the primitives: Read it from an
// .mprim file, or generate circular primitives (only 4 heading angles, and
// 3 primitives for each heading angle: Move straight, turn right by following
// a quarter circle, or turn left).
// The map should be loaded after the primitives are known, in order to
// determine the padding thickness for storing the grid.

// Given the resolution

// TODO: Vehicle model? Right now we assume the vehicle is a point.

class Lattice {
 public:
  Lattice(std::string mapname, LatticeParam p) {
    agent_radius_ = p.agent_radius;
    if (p.prim_type == kCircularPrim) {
      GenerateCircularPrimitives(p);
    }
    else if (p.prim_type == kGridPrim) {
      GenerateGridPrimitives(p);
    }
    else if (p.prim_type == kInputPrim) {
      ReadPrimitivesFromFile(p.prim_file);
    }
    else if (p.prim_type == kInputPrim4D) {
      Read4DPrimitivesFromFile(p.prim_file);
    }

#ifdef USE_LATTICE_LENGTHS_AS_COSTS
    for (int start_a = 0; start_a < num_discrete_angles_; start_a++) {
      for (auto &p : prims_[start_a]) {
        p.SetCostMultiplier(1);
      }
    }
#endif

    if (kLatticeShrinkPrimitives)
      ShrinkPrimitives();
    RoundPrimitiveCosts(kEpsDistance*8);
    EliminateRedundantPrimitives(true); // TODO: Right now, adjusts costs
    SortPrimitivesInDescendingCostOrder();
    GenerateReversePrimitives();
    LoadMap(mapname);
//    Debug();
  }

  // TODO: Add agent radius
  Lattice(std::string mprim_filename, bool is_undirected = false) {
    ReadPrimitivesFromFile(mprim_filename);
  }

  ~Lattice() {
  }

  void Debug();

  void ReadPrimitivesFromFile(std::string mprim_filename,
                              bool is_undirected = false);

  void Read4DPrimitivesFromFile(std::string mprim_filename, bool is_undirected =
                                    false);

  void RoundPrimitiveCosts(double d);
  void ShrinkPrimitives();  // FIXME: New addition, not used at the moment.
  void EliminateRedundantPrimitives(bool eliminate_if_same_cost);
  void SortPrimitivesInDescendingCostOrder();
  void LoadMap(std::string mapname);

  // Returns true if the agent does not collide with any obstacles in the
  // queried pose.
  bool IsValidPos(xyThetaPos pos) const;

  // Returns true if the agent can execute the specified primitive from the
  // specified pose, without any collisions.
  bool CanExecutePrimitive(xyThetaPos pos,
                           const MotionPrimitive* prim) const;

  // Fills the vector passed as parameter with pointers to executable primitives.
  void GetExecutableForwardPrimitives(
      xyThetaPos pos, std::vector<const MotionPrimitive*> & prims) const;
  void GetExecutableReversePrimitives(
      xyThetaPos pos, std::vector<const MotionPrimitive*> & prims) const;

  // Returns all the primitives that can be applied for a given orientation.
  // Since x and y are not specified, no sweeping checks are done.
  // TODO: Should be const.
  std::vector<MotionPrimitive>* GetAllForwardPrimitives(
      int discrete_orientation) {
    return &prims_[discrete_orientation];
  }
  std::vector<MotionPrimitive>* GetAllReversePrimitives(
      int discrete_orientation) {
    if (is_undirected_)
      return &prims_[discrete_orientation];
    else
      return &reverse_prims_[discrete_orientation];
  }

  double ToRadian(const int orientation) const {
    return
        orientation < orientation_to_radian_.size() ?
            orientation_to_radian_[orientation] : 0;
  }

#ifndef NO_HOG
  // Shows all the primitives applicable from the specified x,y location
  // (for all possible angles).
  void VisualizePrimitives(const MapEnvironment *env, int x, int y) const;
#endif

  int GetNumAngles() const {
    return num_discrete_angles_;
  }
  Grid2D* GetGrid() {
    return &grid_;
  }
  bool IsUndirected() const {
    return is_undirected_;
  }

  // Returns a pointer to a primitive that will take the agent from one
  // position to the other. Returns NULL if no such primitive is found.
  const MotionPrimitive* GetIntermediatePrimitive(
      const xyThetaPos & pos1, const xyThetaPos & pos2) const;
  int GetMaxPrimitiveReach();

 private:
  int num_discrete_angles_;
  Grid2D grid_;

  double agent_radius_;
  std::vector<xyPos> agent_footprint_;

  // prims_[a][i] returns the ith applicable primitive for orientation a.
  // reverse primitives are generated regardless of whether the graph is
  // undirected, since they do not require much memory to store.
  bool is_undirected_;
  std::vector<std::vector<MotionPrimitive> > prims_;
  std::vector<std::vector<MotionPrimitive> > reverse_prims_;


  // TODO: Currently being initialized (correctly I hope), but not being used.
  std::vector<double> orientation_to_radian_;

  void AddPrimitive(MotionPrimitive & prim) {
    prims_[prim.GetDiscreteStartAngle()].push_back(prim);
  }

  // TODO: Penalize turns?
  void GenerateCircularPrimitives(LatticeParam p);
  MotionPrimitive GenerateCircularPrimitive(
      int start_orientation, bool clockwise, int turning_radius_in_cells,
      int num_intermediate_poses, double cost_multiplier_for_turns = 1);
  MotionPrimitive GenerateStraightPrimitive(int start_orientation,
                                                   int length_in_cells = 0);

  void GenerateGridPrimitives(LatticeParam p);

  void GenerateReversePrimitives();
  void CalculateLinearizedPaddedCoordinateOffsets(
      MotionPrimitive & prim);
};

#endif
