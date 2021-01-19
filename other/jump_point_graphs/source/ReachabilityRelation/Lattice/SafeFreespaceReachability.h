/*
 * SafeFreespaceReachability.h
 *
 *  Created on: Mar 11, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_REACHABILITYRELATION_SAFEFREESPACEREACHABILITY_H_
#define APPS_SUBGOALGRAPH_REACHABILITYRELATION_SAFEFREESPACEREACHABILITY_H_


#include "ReachabilityRelationExploreCommon.h"

// (bounded-)HReachable iff d(s,t) == h(s,t) <= bound

template<class G, class S, class H>
class SafeFreespaceReachability : public ReachabilityRelationExploreCommon<G, S> {
public:

  SafeFreespaceReachability(G* graph, S* sg_mapper, H* heuristic, double g_limit =
                    std::numeric_limits<double>::max())
    : ReachabilityRelationExploreCommon<G, S>(graph, sg_mapper),
      h_(heuristic), g_limit_(g_limit) {
    assert(h_ != NULL);
  }
  ~SafeFreespaceReachability() {
  }

  bool CanDetectReachabilityOfQueryPoints() {
    return true;
  }
  void RConnect(nodeId start, bool can_identify_superset = false);
  void RRefine(nodeId start, nodeId goal, std::vector<nodeId> & path,
                         bool append = false);
  bool IsReachable(nodeId start, nodeId goal, Distance d = kMaxDistance);

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

  int GetDirectReachableAreaSize(nodeId source) {
    RConnect(source, false);  // Don't identify superset.
    return this->expansion_order_.size();
  }

  RConnectStatistic GetRConnectStatistic() {
    if (!this->st_processed_) {
      ReachabilityRelationExploreCommon<G, S>::GetRConnectStatistic();
      this->st_.num_expanded = this->st_.num_popped - this->st_.num_subgoals;
    }
    return this->st_;
  }

protected:
  H* h_;
  Distance g_limit_;

  double GetHeuristicDistance(nodeId n1, nodeId n2) {
    return h_->GetHeuristicDistance(n1, n2);
//    return std::min(g_limit_, h_->GetHeuristicDistance(n1, n2));
  }
  double GetDirectionSensitiveHeuristicDistance(nodeId & from,
                                                       nodeId & to) {
    return
        this->exploring_backward_ ?
            GetHeuristicDistance(to, from) : GetHeuristicDistance(from, to);
  }
  int GetDirectionSensitiveNumPred(nodeId & from, nodeId & to){
    return
        this->exploring_backward_ ?
            h_->GetNumSucc(to, from) : h_->GetNumPred(from, to);
  }
  bool ExploreReachableAreaFringeDijkstra(nodeId start,
                                          bool terminate_if_fringe,
                                          Distance exploration_bound);
};

#include "SafeFreespaceReachability.inc"



#endif /* APPS_SUBGOALGRAPH_REACHABILITYRELATION_SAFEFREESPACEREACHABILITY_H_ */
