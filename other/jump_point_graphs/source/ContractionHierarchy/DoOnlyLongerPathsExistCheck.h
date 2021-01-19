/*
 * DoOnlyLongerPathsExistCheck.h
 *
 *  Created on: Oct 28, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_CONTRACTIONHIERARCHY_DOONLYLONGERPATHSEXISTCHECK_H_
#define APPS_SUBGOALGRAPH_CONTRACTIONHIERARCHY_DOONLYLONGERPATHSEXISTCHECK_H_

#include "../Graphs/DynamicGraph.h"
#include "../Graphs/GraphDefinitions.h"
#include "../SearchMethods/BidirectionalDijkstra.h"
#include "../SearchMethods/SearchIterationCounter.h"
#include "ContractionCore.h"

// Given a node n, a predecessor p and a successor s, checks if the only
// shortest p-s path goes through n, as follows:
// - Removes n from the graph (by removing all its in-arcs and out-arcs).
// - Performs a bidirectional dijkstra search between s and p (that bypasses n),
//   which can terminate early as soon as it can be determined that no path
//   exists that is shorter than d(p,n) + d(n,s).
// - Restores n to the graph.
//
// Optimizations:
// - After a node n is fixed, if a new p-s query has the same p as the previous
//   search, the forward search from p is not reset, and reused for the current
//   s.
// - n is removed once (for all its p-s-queries), and inserted back when a new
//   n is specified.
//
// Parameters:
// - Expansion limit: A witness search terminates if no path is found by the
//   time the search has reached the expansion limit.

class DoOnlyLongerPathsExistCheck {
public:
  DoOnlyLongerPathsExistCheck(ContractionType c_type, ContractionCore* ch_core,
                              int expansion_limit = 1000000)
      : c_type_(c_type),
        ch_core_(ch_core),
        bidij_(ch_core->GetForwardWitnessSearchGraph(),
               ch_core->GetBackwardWitnessSearchGraph()),
        last_p_(kNonNode),
        last_s_(kNonNode),
        expansion_limit_(expansion_limit){
    if (c_type == kHeavyRContraction) {
      bidij_.AddLevelInformation(ch_core->GetLevelManager());
      bidij_.SetBackwardDontPushHighestLevelOrSameLevel();
      bidij_.SetDontStallHighestLevel();
    }
  }
  ~DoOnlyLongerPathsExistCheck() {}

  void SetBypassNode(nodeId n) {
    ch_core_->HideNode(n);
  }
  void RestoreBypassNode() {
    ch_core_->RestoreHiddenNode();
    last_p_ = kNonNode;
    last_s_ = kNonNode;
  }

  bool DoOnlyLongerPathsExist(nodeId p, nodeId s, Distance bound) {
    stat_ = QueryStatistic();
    bidij_.ResetSearchInfo();

    // Restart the forward/backward searches as necessary.
    bool always_new_search = false;
    if (always_new_search || last_p_ != p) {
      last_p_ = p;
      bidij_.GetForward()->Reset();
      bidij_.GetForward()->AddStart(p);
    }
    if (always_new_search || last_s_ != s) {
      last_s_ = s;
      bidij_.GetBackward()->Reset();
      bidij_.GetBackward()->AddStart(s);
    }

    // Not sure if these are necessary, but if the p-s distance is already
    // known in one of the searches use it.
    if (bidij_.GetForward()->IsTentativeDistanceCertain(s))
      return bidij_.GetForward()->GetTentativeDistance(s) > bound + 0.001;

    if (bidij_.GetBackward()->IsTentativeDistanceCertain(p))
      return bidij_.GetBackward()->GetTentativeDistance(p) > bound + 0.001;

    // Search until either the distance or the expansion limit is hit.
    int num_remaining_expansions = expansion_limit_;

    if (c_type_ != kHeavyRContraction) {
      while (!bidij_.CanStop() && bidij_.GetSumOfRadii() <= bound + 0.001
          && num_remaining_expansions > 0) {
        bidij_.ExpandNext();
        num_remaining_expansions--;
      }
      bidij_.AddStatistics(stat_);
      return bidij_.GetBestDistance() > bound + 0.001;
    }
    else {
      while ((bidij_.GetForward()->GetRadius() <= bound + 0.001
              || bidij_.GetBackward()->GetRadius() <= bound + 0.001)
          && num_remaining_expansions > 0) {
        bidij_.NLevelStallOrExpandNext();
        num_remaining_expansions--;
      }
      bidij_.AddStatistics(stat_);
      return bidij_.GetBestDistance() > bound + 0.001;
    }
  }
  void AddStatistics(QueryStatistic & stat) {
    stat = stat + stat_;
  }

private:
  ContractionType c_type_;
  ContractionCore* ch_core_;
  BidirectionalDijkstra<ContractionGraph,ContractionGraph> bidij_;
  nodeId last_p_, last_s_;
  int expansion_limit_;
  QueryStatistic stat_;  // Last witness search only.
};


#endif /* APPS_SUBGOALGRAPH_CONTRACTIONHIERARCHY_DOONLYLONGERPATHSEXISTCHECK_H_ */
