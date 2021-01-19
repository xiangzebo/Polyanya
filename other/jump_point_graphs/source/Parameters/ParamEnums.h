/*
 * ParamEnums.h
 *
 *  Created on: Feb 18, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_PARAMETERS_PARAMENUMS_H_
#define APPS_SUBGOALGRAPH_PARAMETERS_PARAMENUMS_H_

// SG Construction
enum RSPCConstructionType {
  kGrowRSPC = 0,
  kPruneRSPC,
  kRandomRSPC,
  kConnectedRSPC,
  kRSPCConstructionTypeNoType
};

enum SubgoalSelectionStrategy {
  kRandomCandidate = 0,
  kParentOfLowestGValueFringeNode,
  kParentOfHighestGValueFringeNode,
  kSecretSauce,

  kSubgoalSelectionStrategyNoType
};

enum NodeOrdering {
  kIdNodeOrder = 0,
  kRandomNodeOrder,
  kDFSVisitNodeOrder,
  kReverseDFSVisitNodeOrder,
  kDFSCompletionNodeOrder,
  kReverseDFSCompletionNodeOrder,

  kNodeOrderingNoType
};

// CH
enum ContractionOrderUpdatePolicy {
  kUpdateShortcuts = 0,
  kUpdateTop,

  kContractionOrderUpdatePolicyNoType
};

// Lattice
enum LatticePrimType {
  kCircularPrim = 0, kGridPrim, kInputPrim, kInputPrim4D
};

// R
enum ReachabilityType {
  kFreespaceReachability,
  kSafeFreespaceReachability,
  kBoundedDistanceReachability,
  kCanonicalFreespaceReachability
};

enum RConnectType {
  kRConnectDefault,
  kRConnectConservative,
  kRConnectAggressive,
  kRConnectStall,
  kRConnectSucc
};


#endif /* APPS_SUBGOALGRAPH_PARAMETERS_PARAMENUMS_H_ */
