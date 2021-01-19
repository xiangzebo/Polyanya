/*
 * DefaultParams.h
 *
 *  Created on: Feb 15, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_PARAMETERS_DEFAULTPARAMS_H_
#define APPS_SUBGOALGRAPH_PARAMETERS_DEFAULTPARAMS_H_

#include <limits>
#include "ParamEnums.h"

//#define SG_QUIET;
const bool kDefaultEchoStatistics = false;
const int kReportIncrementInSeconds = 10;

//*
const ContractionOrderUpdatePolicy kCHParamDefaultUpdatePolicy = kUpdateShortcuts;
/*/
const ContractionOrderUpdatePolicy kCHParamDefaultUpdatePolicy = kUpdateTop;
//*/
const bool kCHParamDefaultSave = true;
const bool kCHParamDefaultLoad = false;
const bool kCHParamDefaultClear = true;

const bool kExperiment = true;
const bool kInstanceClear = true;

const bool kLatticeParamDefaultVerifyUndirected = false;
const bool kLatticeLargestConnectedComponent = true;
const bool kLatticeEliminateDeadEnds = false;

const int kDefaultNumInstances = 1000;

const bool kUseJPSearch = true;
const bool kUseAvoidanceTable = true;
const bool kUseDoubleClearance = false;

const bool kManagerForceDefaultAlgNames = false;

//#define JP_CH_AVOID_REDUNDANT_BY_GRID_SEARCH  // less edges, more prep time.
//#define JP_CH_AVOID_REDUNDANT_WHEN_ESTIMATING_IMPORTANCE  // Significantly higher prep times, especially with the above flag.
//#define JP_CH_GROUP_CONTRACT  // Leave it unset, it's awful.

//#define VIS_RANDOM_COLOR
//#define LEVEL_VIZUALIZATION

#define INCLUDE_GPPC_CH


#define USE_GRID
//#define USE_LATTICE

//#define USE_LATTICE_LENGTHS_AS_COSTS  // Unset in experiments
//*/

#ifndef NO_HOG
//#define DRAW_CLEARANCE_LINES
#endif

//const LatticePrimType kLatticeParamPrimType = kCircularPrim;
//const LatticePrimType kLatticeParamPrimType = kGridPrim;
const LatticePrimType kLatticeParamPrimType = kInputPrim;

//const ReachabilityType kLatticeParamDefaultType = kFreespaceReachability;
//const ReachabilityType kLatticeParamDefaultType = kCanonicalFreespaceReachability;
//const ReachabilityType kLatticeParamDefaultType = kSafeFreespaceReachability;
const ReachabilityType kLatticeParamDefaultType = kBoundedDistanceReachability;
const double kLatticeParamDefaultBound = 50.0;

const double kLatticeParamAgentRadius = 0.5;
//const double kLatticeParamTurnInPlaceCostPerRadian = 5;

// Bit of a hack. kEps is 1/1024, edge rounding is 1/512.
const bool kRoundLatticePrimCosts512 = false;
const bool kLatticeParamIsUndirected = false;

const bool kLatticeShrinkPrimitives = false; // FIXME: False for the experiments

const double kDrawEdgeWidth = 2;
const double kDrawArrowNodeWidth = 3;


// TODO: Move
//*
typedef uint64_t ExecutablePrimitiveFlags;
typedef uint8_t PrimId;
/*/
typedef uint16_t ExecutablePrimitiveFlags;
typedef uint8_t PrimId;
//*/
typedef PrimId PrimCount;

const PrimId kInvalidPrimId = std::numeric_limits<PrimId>::max();
const PrimId kInvalidPrimCount = std::numeric_limits<PrimCount>::max();

const std::string kLatticeParamInputPrimName = "";
const std::string kLatticeParamInputPrimFile = "";

const int kLatticeParamTurningRadius = 2;
const int kLatticeParamIntermediatePoses = 25;
const bool kLatticeParamReverseStraightMove = true;
const bool kLatticeParamReverseCircularMove = false;
const double kLatticeParamForwardTurnCostMultiplier = 1;
const double kLatticeParamReverseStraightMoveCostMultiplier = 2;
const double kLatticeParamReverseTurnCostMultiplier = 2;

#endif /* APPS_SUBGOALGRAPH_PARAMETERS_DEFAULTPARAMS_H_ */
