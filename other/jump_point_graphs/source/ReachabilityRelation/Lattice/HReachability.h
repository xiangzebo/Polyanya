/*
 * HReachability.h
 *
 *  Created on: Mar 10, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_REACHABILITYRELATION_HREACHABILITY_H_
#define APPS_SUBGOALGRAPH_REACHABILITYRELATION_HREACHABILITY_H_
#include "ReachabilityRelationExploreCommon.h"

// (bounded-)HReachable iff d(s,t) == h(s,t) <= bound

template<class G, class S, class H>
class HReachability : public ReachabilityRelationExploreCommon<G, S> {
public:
  using ReachabilityRelationExploreCommon<G, S>::expansion_order_;
  using ReachabilityRelationExploreCommon<G, S>::existing_subgoals_;
  using ReachabilityRelationExploreCommon<G, S>::existing_subgoal_distances_;
  using ReachabilityRelationExploreCommon<G, S>::exploring_backward_;
  using ReachabilityRelationExploreCommon<G, S>::sm_;
  using ReachabilityRelationExploreCommon<G, S>::st_;
  using ReachabilityRelationExploreCommon<G, S>::Reset;
  using ReachabilityRelationExploreCommon<G, S>::data_;

  HReachability(G* graph, S* sg_mapper, H* heuristic, double g_limit =
                    std::numeric_limits<double>::max(),
                    RConnectType rconn_type =
                        kRConnectAggressive)
    : ReachabilityRelationExploreCommon<G, S>(graph, sg_mapper),
      h_(heuristic),
      rconn_type_(rconn_type) {
    assert(h_ != NULL);
    r_parent_.resize(graph->GetNumAllNodes());
//    g_limit_ = g_limit;
  }
  ~HReachability() {
  }

  bool CanDetectReachabilityOfQueryPoints() {
    return true;
  }

  void RConnect(nodeId start, bool can_identify_superset = false);
  void RRefine(nodeId start, nodeId goal, std::vector<nodeId> & path,
                         bool append = false);

  bool IsReachable(nodeId start, nodeId goal, Distance d = kMaxDistance) {
    assert(d < kMaxDistance);
    Distance h_val = GetHeuristicDistance(start, goal);
    return fabs(h_val - d) < kEpsDistance;
  }

  void RExploreFringe(nodeId start, Distance exploration_bound = kMaxDistance) {
    ExploreReachableAreaFringeDijkstra(start, false, exploration_bound);
  }
  bool DoesFringeNodeExist(nodeId start, Distance exploration_bound =
                               kMaxDistance) {
    return ExploreReachableAreaFringeDijkstra(start, true, exploration_bound);
  }
  int FindLastReachableIndex(std::vector<nodeId> & path, int start, int goal);

//  double GetReachableDistance(nodeId start, nodeId goal) {
//    return h_->GetHeuristicDistance(g_->ToState(start), g_->ToState(goal));
//  }

  RConnectStatistic GetRConnectStatistic() {
    if (!this->st_processed_) {
      ReachabilityRelationExploreCommon<G, S>::GetRConnectStatistic();
      if (rconn_type_ == kRConnectAggressive)
        this->st_.num_expanded = this->st_.num_popped
            - this->st_.num_subgoals;

      if (rconn_type_ == kRConnectSucc) {
        this->st_.radius = 0;
        for (auto d: dist_)
          if (d > this->st_.radius)
            this->st_.radius = d;

        this->st_.num_expanded = this->st_.num_popped - this->st_.num_subgoals;
      }
    }
    return this->st_;
  }

  void Reset() {
    ReachabilityRelationExploreCommon<G, S>::Reset();
    dist_.clear();
  }

protected:
  H* h_;
  RConnectType rconn_type_;
  std::vector<Distance> dist_;  // Only used for BFSAggressiveFlags
  std::vector<nodeId> r_parent_;

  double GetHeuristicDistance(nodeId n1, nodeId n2) {
    //return std::min(g_limit_, h_->GetHeuristicDistance(n1, n2));
    return h_->GetHeuristicDistance(n1, n2);
  }
  double GetDirectionSensitiveHeuristicDistance(nodeId & from,
                                                       nodeId & to) {
    return
        this->exploring_backward_ ?
            GetHeuristicDistance(to, from) : GetHeuristicDistance(from, to);
  }
  bool ExploreReachableAreaFringeDijkstra(nodeId start,
                                          bool terminate_if_fringe,
                                          Distance exploration_bound);

  void ExploreBFSAggressiveFlags(nodeId start);

  void RRefineDFSFreespaceDistances(nodeId start, nodeId goal,
                                    std::vector<nodeId> & path, bool append);
  void RRefineDFSFlags(nodeId start, nodeId goal, std::vector<nodeId> & path,
                       bool append);
};

#include "HReachability.inc"

#endif /* APPS_SUBGOALGRAPH_REACHABILITYRELATION_HREACHABILITY_H_ */
