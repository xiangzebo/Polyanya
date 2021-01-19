/*
 * CanonicalReachability.h
 *
 *  Created on: Mar 14, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_REACHABILITYRELATION_CANONICALREACHABILITY_H_
#define APPS_SUBGOALGRAPH_REACHABILITYRELATION_CANONICALREACHABILITY_H_

#include "ReachabilityRelationExploreCommon.h"

// (bounded-)HReachable iff d(s,t) == h(s,t) <= bound

template<class G, class S, class H>
class CanonicalReachability : public ReachabilityRelationExploreCommon<G, S> {
public:
  using ReachabilityRelationExploreCommon<G, S>::expansion_order_;
  using ReachabilityRelationExploreCommon<G, S>::existing_subgoals_;
  using ReachabilityRelationExploreCommon<G, S>::existing_subgoal_distances_;
  using ReachabilityRelationExploreCommon<G, S>::exploring_backward_;
  using ReachabilityRelationExploreCommon<G, S>::sm_;
  using ReachabilityRelationExploreCommon<G, S>::st_;
//  using ReachabilityRelationExploreCommon<G, S>::Reset;

  CanonicalReachability(G* graph, S* sg_mapper, H* heuristic, double g_limit =
                    std::numeric_limits<double>::max())
    : ReachabilityRelationExploreCommon<G, S>(graph, sg_mapper),
      h_(heuristic),
      g_limit_(g_limit),
      using_distances_(h_->UsingDistances()){ // FIXME: Set to false.
    assert(h_ != NULL);
    dist_.reserve(100000);
  }
  ~CanonicalReachability() {
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

  bool CanConstructSCSGEdgesExactly() const {return false;}

  void Reset() {
    ReachabilityRelationExploreCommon<G, S>::Reset();
    dist_.clear();
  }

  /*
  bool IsRReachablePathSuffixAlsoRReachable() {
    return false;
  }

  void FindAllRReachableSubgoals(nodeId source) {
    // Stopping at subgoals only eliminates R-Reachable subgoals to which
    // the canonical path is covered by a subgoal.
    // Since this function is used to generate an initial set of arcs
    // and then prune the redundant ones, and since arcs to those R-Reachable
    // nodes will always be redundant, this is a safe optimization.
    bool stop_at_subgoals = true;
    ExploreRReachableAreaDFSWithoutDistance(source, stop_at_subgoals);
  }
  */

  RConnectStatistic GetRConnectStatistic() {
    if (!this->st_processed_) {
      ReachabilityRelationExploreCommon<G, S>::GetRConnectStatistic();

      this->st_.radius = 0;
      for (auto d: dist_)
        if (d > this->st_.radius)
          this->st_.radius = d;

      this->st_.num_expanded = this->st_.num_popped - this->st_.num_subgoals;
    }
    return this->st_;
  }


#ifndef NO_HOG
  void Visualize(const MapEnvironment *env, int display_type);
#endif
protected:
  H* h_;
  Distance g_limit_;  // Not used at the moment.
  bool using_distances_;
  std::vector<Distance> dist_;

  // Freespace distances are not required, but can help.
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

  void GetDirectionSensitiveCanonicalSuccessors(
      nodeId source, nodeId curr, std::vector<WeightedArcHead> & neighbors) {
    this->exploring_backward_ ?
        h_->GetReverseCanonicalSuccessors(source, curr, neighbors) :
        h_->GetCanonicalSuccessors(source, curr, neighbors);
  }

  bool ExploreReachableAreaFringeDijkstra(nodeId start,
                                          bool terminate_if_fringe,
                                          Distance exploration_bound);

  void ExploreBFSAggressiveNoDuplicateWithFreespaceDistances(
      nodeId start, bool stop_at_subgoals);
  void ExploreBFSAggressiveNoDuplicate(nodeId start);
  //  void ExploreBFSAggressiveNoDuplicate(nodeId start, bool stop_at_subgoals);
};

#include "CanonicalReachability.inc"




#endif /* APPS_SUBGOALGRAPH_REACHABILITYRELATION_CANONICALREACHABILITY_H_ */
