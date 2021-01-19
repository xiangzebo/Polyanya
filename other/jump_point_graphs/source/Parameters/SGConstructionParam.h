/*
 * SGParam.h
 *
 *  Created on: Feb 18, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_PARAMETERS_SGCONSTRUCTIONPARAM_H_
#define APPS_SUBGOALGRAPH_PARAMETERS_SGCONSTRUCTIONPARAM_H_

#include "DefaultParams.h"

// TODO: For SGParam:
// Construction type: Iterative bounds / default
// Selection strategy: Random etc
// bool Expand fringe vertices

// GrowRSPC
// Order
// Initial list
// Subgoals first
// Selection strategy

// PruneRSPC

const RSPCConstructionType kDefaultRSPCConstructionType = kGrowRSPC;
const NodeOrdering kDefaultNodeOrdering =
    //kRandomNodeOrder;
    //kReverseDFSVisitNodeOrder;
    kReverseDFSCompletionNodeOrder;
const SubgoalSelectionStrategy kDefaultSubgoalSelectionStrategy = kSecretSauce;
const bool kDefaultPrioritizeExploringFromSubgoals = true;

const double kDefaultSpannerSuboptimality = 1.0;

// TODO: Selection strategy not reflected in name.

struct SGConstructionParam {
  SGConstructionParam() {
    construction_type = kDefaultRSPCConstructionType;

    // GrowRSPC
    node_ordering = kDefaultNodeOrdering;
    selection_strategy = kDefaultSubgoalSelectionStrategy;
    prioritize_exploring_from_subgoals =
        kDefaultPrioritizeExploringFromSubgoals;

    // PruneRSPC
    // node_ordering
    verify_undirected = kLatticeParamDefaultVerifyUndirected;
    explore_from_only_leaf_nodes = true;

    percentage_subgoals = 100;
    extra_subgoal_percentage = 0;

    spanner_suboptimality = kDefaultSpannerSuboptimality;

  }

  SGConstructionParam(RSPCConstructionType t)
    : SGConstructionParam() {
    construction_type = t;
    extra_subgoal_percentage = 0;
  }

  std::string GetRSPCConstructionName() {
    std::stringstream ss;
    if (construction_type == kRandomRSPC) {
      ss << "R" << percentage_subgoals;
      return ss.str();
    }

    if (construction_type == kGrowRSPC)
      ss << "G";
    if (construction_type == kPruneRSPC)
      ss << "P";
    if (construction_type == kConnectedRSPC) {
      ss << "C";
      if (extra_subgoal_percentage > kEpsDistance) {
        ss<<extra_subgoal_percentage;
      }
    }

    if (node_ordering == kIdNodeOrder)
      ss << "-Id";
    if (node_ordering == kRandomNodeOrder)
      ss << "-Rand";
    if (node_ordering == kDFSVisitNodeOrder)
      ss << "-DFS";
    if (node_ordering == kReverseDFSVisitNodeOrder)
      ss << "-rDFS";
    if (node_ordering == kDFSCompletionNodeOrder)
      ss << "-DFSc";
    if (node_ordering == kReverseDFSCompletionNodeOrder)
      ss << "-rDFSc";

    if (construction_type == kGrowRSPC && prioritize_exploring_from_subgoals) {
      ss << "-S";
    }

    return ss.str();

    return "Def";
  }

  std::string GetSubConstructionName() {
    // Add arc construction method to RSPC name.
    // Default = direct-R-reachable = no extension.
    std::stringstream ss;
    ss << GetRSPCConstructionName();

    if (fabs(spanner_suboptimality - 1.0) > 0.0001) {
     ss << "_w" << spanner_suboptimality;
    }

    return ss.str();
  }

  RSPCConstructionType construction_type;

  // GrowRSPC
  SubgoalSelectionStrategy selection_strategy;
  NodeOrdering node_ordering;
  bool prioritize_exploring_from_subgoals;

  // PruneRSPC
  bool explore_from_only_leaf_nodes;

  // RandomRSPC
  double percentage_subgoals;

  // Spanner arcs
  double spanner_suboptimality;

  double extra_subgoal_percentage;

  bool verify_undirected;
};

inline
SGConstructionParam MakeSGGrowConstructionParam(
    SubgoalSelectionStrategy st, NodeOrdering node_order =
        kDefaultNodeOrdering,
    bool prioritize_subgoals = kDefaultPrioritizeExploringFromSubgoals) {
  SGConstructionParam p;
  p.construction_type = kGrowRSPC;
  p.selection_strategy = st;
  p.node_ordering = node_order;
  p.prioritize_exploring_from_subgoals = prioritize_subgoals;
  return p;
}

inline
SGConstructionParam MakeSGPruneConstructionParam(NodeOrdering node_order) {
  SGConstructionParam p;
  p.construction_type = kPruneRSPC;
  p.node_ordering = node_order;
  return p;
}

inline
SGConstructionParam MakeSGRandomConstructionParam(double percentage) {
  SGConstructionParam p;
  p.construction_type = kRandomRSPC;
  p.percentage_subgoals = percentage;
  return p;
}

inline
SGConstructionParam MakeSGConnectedConstructionParam(
    NodeOrdering no, double random_extra_subgoals = 0) {
  SGConstructionParam p;
  p.construction_type = kConnectedRSPC;
  p.node_ordering = no;
  p.extra_subgoal_percentage = random_extra_subgoals;
  return p;
}

inline
SGConstructionParam MakeSGSpannerConstructionParam(
    SGConstructionParam p, double suboptimality) {
  p.spanner_suboptimality = suboptimality;
  return p;
}

#endif /* APPS_SUBGOALGRAPH_PARAMETERS_SGCONSTRUCTIONPARAM_H_ */
