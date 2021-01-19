/*
 * LatticeParam.h
 *
 *  Created on: Feb 18, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_PARAMETERS_LATTICEPARAM_H_
#define APPS_SUBGOALGRAPH_PARAMETERS_LATTICEPARAM_H_
#include <iostream>
#include <string>
#include <sstream>

#include "DefaultParams.h"

struct LatticeParam {
  LatticeParam() {
    agent_radius = kLatticeParamAgentRadius;
    is_undirected = kLatticeParamIsUndirected;

    prim_type = kLatticeParamPrimType;
//    prim_type = kGridPrim;

    // Circular primitives
    turning_radius = kLatticeParamTurningRadius;
    num_intermediate_poses = kLatticeParamIntermediatePoses;
    reverse_straight_move = kLatticeParamReverseStraightMove;
    reverse_circular_move = kLatticeParamReverseCircularMove;

    forward_turn_cost_multiplier = kLatticeParamForwardTurnCostMultiplier;
    reverse_straight_move_cost_multiplier =
        kLatticeParamReverseStraightMoveCostMultiplier;
    reverse_turn_cost_multiplier = kLatticeParamReverseTurnCostMultiplier;

    if (is_undirected) {
      reverse_straight_move = false;
      reverse_circular_move = false;
    }

    // Input primitive
    input_prim_name = "";
    prim_file = "";
  }

  //
  std::string GetName() {
    if (prim_type == kGridPrim) {
      return "Grid";
    }
    else if (prim_type == kCircularPrim) {
      std::stringstream ss;
      ss
//      << "r" << agent_radius << "-"
      << turning_radius << "C" << forward_turn_cost_multiplier;

      if (reverse_straight_move)
        ss << "RS"<<reverse_straight_move_cost_multiplier;

      if (reverse_circular_move)
        ss << "RC"<<reverse_turn_cost_multiplier;

      return ss.str();
    }
    else if (prim_type == kInputPrim) {
      return input_prim_name;
    }
    else if (prim_type == kInputPrim4D) {
      return input_prim_name;
    }
    return "NoPrim";
  }

  // Primitives
  LatticePrimType prim_type;
  double agent_radius;
  bool is_undirected;

  // Circular primitives
  int turning_radius;
  int num_intermediate_poses;

  bool reverse_straight_move;
  bool reverse_circular_move;

  double forward_turn_cost_multiplier;
  double reverse_straight_move_cost_multiplier;
  double reverse_turn_cost_multiplier;

  // prim file
  std::string prim_file;
  std::string input_prim_name;
};
inline
LatticeParam MakeLatticeParamGrid2D() {
  LatticeParam p;
  p.prim_type = kGridPrim;
  p.input_prim_name = "Grid";
  return p;
}
inline
LatticeParam MakeLatticeParamCircular() {
  LatticeParam p;
  p.prim_type = kCircularPrim;
  return p;
}
inline
LatticeParam MakeLatticeParamInput3D(std::string filename, std::string primname) {
  LatticeParam p;
  p.prim_type = kInputPrim;
  p.prim_file = filename;
  p.input_prim_name = primname;
  return p;
}
inline
LatticeParam MakeLatticeParamInput4D(std::string filename, std::string primname) {
  LatticeParam p;
  p.prim_type = kInputPrim4D;
  p.prim_file = filename;
  p.input_prim_name = primname;
  return p;
}

#endif /* APPS_SUBGOALGRAPH_PARAMETERS_LATTICEPARAM_H_ */
