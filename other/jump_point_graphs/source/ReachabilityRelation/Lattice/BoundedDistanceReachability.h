/*
 * BoundedDistanceReachability.h
 *
 *  Created on: Mar 11, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_REACHABILITYRELATION_BOUNDEDDISTANCEREACHABILITY_H_
#define APPS_SUBGOALGRAPH_REACHABILITYRELATION_BOUNDEDDISTANCEREACHABILITY_H_

#include "../SearchMethods/AStar.h"
#include "ReachabilityRelationExploreCommon.h"

// TODO: Better refinement.

// (bounded-)-distance-Reachable iff d(s,t) <= bound
// Heuristic used for A* refinement
template<class G, class S, class H>
class BoundedDistanceReachability : public ReachabilityRelationExploreCommon<G, S> {
public:

  BoundedDistanceReachability(G* graph, S* sg_mapper, H* heuristic,
                              double g_limit =
                                  std::numeric_limits<double>::max(),
                              RConnectType rconn_type =
                                  kRConnectConservative)
    : ReachabilityRelationExploreCommon<G, S>(graph, sg_mapper),
      astar_(graph, heuristic),
      g_limit_(g_limit),
      rconn_type_(rconn_type) {}
  ~BoundedDistanceReachability() {
  }

  bool CanDetectReachabilityOfQueryPoints() {
    return true;
  }

  void RConnect(nodeId start, bool can_identify_superset = false);
  void RRefine(nodeId start, nodeId goal, std::vector<nodeId> & path,
                         bool append = false);

  bool IsReachable(nodeId start, nodeId goal, Distance d = kMaxDistance) {
    assert(d < kMaxDistance);
    return d <= g_limit_ + kEpsDistance;;
  }

  void RExploreFringe(nodeId start, Distance exploration_bound = kMaxDistance) {
    ExploreReachableAreaFringeDijkstra(start, false, exploration_bound);
  }
  bool DoesFringeNodeExist(nodeId start, Distance exploration_bound =
                               kMaxDistance) {
    return ExploreReachableAreaFringeDijkstra(start, true, exploration_bound);
  }
  int FindLastReachableIndex(std::vector<nodeId> & path, int start, int goal);

  RConnectStatistic GetRConnectStatistic() {
    if (!this->st_processed_) {
      ReachabilityRelationExploreCommon<G, S>::GetRConnectStatistic();

      if (rconn_type_ == kRConnectAggressive)
        this->st_.num_expanded = this->st_.num_popped
            - this->st_.num_subgoals;

      if (rconn_type_ == kRConnectStall)
        this->st_.num_expanded = this->st_.num_popped
            - this->st_.num_stalled;
    }
    return this->st_;
  }

  void Reset() {
    ReachabilityRelationExploreCommon<G, S>::Reset();
    astar_.Reset();
  }


protected:
  AStar<G,H> astar_;
  Distance g_limit_;
  RConnectType rconn_type_;

  bool ExploreReachableAreaFringeDijkstra(nodeId start,
                                          bool terminate_if_fringe,
                                          Distance exploration_bound);
};

#include "BoundedDistanceReachability.inc"

#endif /* APPS_SUBGOALGRAPH_REACHABILITYRELATION_BOUNDEDDISTANCEREACHABILITY_H_ */
