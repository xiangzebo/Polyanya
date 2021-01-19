/*
 * ReachabilityRelationExploreCommon.h
 *
 *  Created on: Mar 4, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_REACHABILITYRELATION_REACHABILITYRELATIONEXPLORECOMMON_H_
#define APPS_SUBGOALGRAPH_REACHABILITYRELATION_REACHABILITYRELATIONEXPLORECOMMON_H_
#include "ReachabilityRelation.h"
#include <queue>

// TODO: CRTP rather than inheriting from reachabilityrelationexplore?
// Or, simply, just templated functions declared as friends?
// Implements the following methods:

// RExplore, requires:
// - Can determine reachability from area data.
// - Propagate additional data.

// RRefine:
// Provides distance?


// Extends the reachability relation interface for generic construction methods.
class ReachabilityRelationExplore : public ReachabilityRelation {
 public:
  ReachabilityRelationExplore(int num_nodes = 0)
      : num_nodes_(num_nodes) {
    data_.SetupData(num_nodes);
  }

  // TODO: Remove exploration bound?
  virtual void RExploreFringe(nodeId start,
                      Distance exploration_bound = kMaxDistance) = 0;
  virtual bool DoesFringeNodeExist(nodeId start, Distance exploration_bound =
                               kMaxDistance) = 0;
  virtual int FindLastReachableIndex(std::vector<nodeId> & path, int start,
                                     int goal) = 0;

  const std::vector<nodeId>* GetFringeNodes() const {
    return &fringe_nodes_;
  }
  const AreaExplorationData* GetData() const {
    return &data_;
  }

/*
  // If the following function returns true, then, for any set of subgoals
  // that is not an R-SPC, we need to be careful. Suppose the following holds:
  // - Two nodes A and B are R-reachable from a node S.
  // - A covers a shortest S-B path.
  // Then:
  // - B is not direct-R-reachable from S.
  // - If B is not R-reachable from S, then only using direct-R-reachable
  // arcs might result in a subgoal graph with no path from S to B.
  // This is not the case when:
  // - Any suffix of an R-reachable path is also R-reachable (since that would
  //   imply B is R-reachable from S).
  // - When the set of subgoals is an R-SPC (then we know that either B is
  //   R-reachable from A, or there is a subgoal that covers a shortest path
  //   from A to B).
  virtual bool IsRReachablePathSuffixAlsoRReachable() {
    return true;
  }

  // Assumption: The following function is only called when the above function
  // returns false.
  virtual void FindAllRReachableSubgoals(nodeId source) {
    assert(false && "FindAllRReachableSubgoals not implemented");
  }
*/

 protected:
  int num_nodes_;
  std::vector<nodeId> fringe_nodes_;
  AreaExplorationData data_;
};

template<class G, class S>
class ReachabilityRelationExploreCommon : public ReachabilityRelationExplore {
 public:
  ReachabilityRelationExploreCommon(G* g, S* sm)
      : ReachabilityRelationExplore(g->GetNumAllNodes()),
        g_(g),
        sm_(sm) {
    assert(g_ != NULL);
    assert(sm_ != NULL);
    ppq_.SetupQueue(g->GetNumAllNodes());
    st_processed_ = false;
  }

  virtual int GetDirectReachableAreaSize(nodeId source);
  virtual RConnectStatistic GetRConnectStatistic();

#ifndef NO_HOG
  virtual void Visualize(const MapEnvironment *env, int display_type =
                             kRReachableAreaDisplayParent);
#endif

  virtual void Reset() {
    data_.Reset();
    ppq_.Reset();
    stack_.clear();

    existing_subgoals_.clear();
    existing_subgoal_distances_.clear();
    fringe_nodes_.clear();
    expansion_order_.clear();

    st_ = RConnectStatistic();
    st_processed_ = false;
  }

 protected:
  G* g_;
  S* sm_;
  F_Val_PPQ ppq_;
  std::vector<nodeId> stack_;

  RConnectStatistic st_;  // Time and relaxed is filled during search.
                          // Rest are filled by analyzing the data.
  bool st_processed_;

  // Perform a Dijkstra search from a source vertex until all its fringe nodes
  // are explored.
  // Arguments:
  // - Whether the search is backwards or forwards depends on whether the
  //   class variable 'exploring_backward_' is set.
  // - terminate_if_fringe: Used for constructing SGs by pruning. Terminate
  //   search when the first fringe vertex is found and return true.
  //   Return false if no fringe vertex found.
  // - exploration_bound: Do not generate vertices with g-values higher than the
  //   bound. Typically set to infinite (artifact from a failed experiment).
  // - IsDirectionSensitiveReachable: A function that determines whether
  //   a vertex is R-reachable from the start (should be able to determine
  //   the search direction based on the class variable `exploring_backward_'
  // - OnStrictlyBetterPath: Propagate R-specific information if a shorter path
  //   is found to a successor.
  // - OnSymmetricOrStrictlyBetterPath: Propagate R-specific information if an
  //   alternative path of the same length is found to a successor.
  //
  // Output (fills in member variables):
  // - expansion_order_
  // - fringe_nodes_
  // - data_ (search data including g-values, 'fringe', 'R-reachable',
  //   'covered', 'direct-R-reachable' flags.
  template<class R, class SBP, class SSBP>
  bool ExploreReachableAreaFringeDijkstra(nodeId start,
                                          bool terminate_if_fringe,
                                          Distance exploration_bound,
                                          R IsDirectionSensitiveReachable,
                                          SBP OnStrictlyBetterPath,
                                          SSBP OnSymmetricOrStrictlyBetterPath);
  template<class R>
  bool ExploreReachableAreaFringeDijkstra(nodeId start,
                                          bool terminate_if_fringe,
                                          Distance exploration_bound,
                                          R IsDirectionSensitiveReachable) {
    return ExploreReachableAreaFringeDijkstra(
        start, terminate_if_fringe, exploration_bound,
        IsDirectionSensitiveReachable, [](nodeId curr, nodeId succ)->void {},
        [](nodeId curr, nodeId succ)->void {});
  }

  // Exploration methods for connection.
  // - Conservative: Propagate 'covered'.
  // - Aggressive: Do not expand subgoals.

  // Used by: FR, BD
  template<class R>
  void ExploreDijkstraConservative(nodeId start,
                                          R IsDirectionSensitiveReachable);

  // Used by: BD
  template<class R>
  void ExploreDijkstraAggressive(
      nodeId start, R IsDirectionSensitiveReachable);

  template<class R>
  void ExploreDijkstraStall(
      nodeId start, R IsDirectionSensitiveReachable);

  // Not used at the moment
  template<class R>
  bool ExploreBFSConservative(nodeId start,
                                     R IsDirectionSensitiveReachable);
  template<class R>
  bool ExploreBFSAggressive(nodeId start,
                                     R IsDirectionSensitiveReachable);

  // Used by: FR
  template<class R>
  bool ExploreDFSAggressive(nodeId start,
                                     R IsDirectionSensitiveReachable);

  template<class R>
  int FindLastReachableIndexUsingDistance(std::vector<nodeId> & path, int start,
                                          int goal,
                                          R IsForwardReachable);

  inline void GetDirectionSensitiveSuccessors(
      nodeId & curr, std::vector<WeightedArcHead> & neighbors) {
    exploring_backward_ ?
        g_->GetPredecessors(curr, neighbors) :
        g_->GetSuccessors(curr, neighbors);
  }

  inline void GetDirectionSensitivePredecessors(
      nodeId & curr, std::vector<WeightedArcHead> & neighbors) {
    !exploring_backward_ ?
        g_->GetPredecessors(curr, neighbors) :
        g_->GetSuccessors(curr, neighbors);
  }

  inline void AddStart(nodeId & start, uint8_t flags) {
    data_.GenerateNode(start);
    data_.SetFlag(start, flags);
    data_.SetGVal(start, 0);
    ppq_.InsertOrDecreaseKey(start, 0);
  }

  // Initialize the search with the successors of the start vertex, rather
  // than the start vertex.
  inline int ExpandStart(nodeId & start, uint8_t flags) {
    //AddStart(start, flags);
    //return 1;

    data_.GenerateNode(start);
    data_.SetFlag(start, flags);
    data_.SetGVal(start, 0);
    expansion_order_.push_back(start);

    std::vector < WeightedArcHead > neighbors;
    GetDirectionSensitiveSuccessors(start, neighbors);
    for (unsigned int i = 0; i < neighbors.size(); i++) {
      nodeId succ = neighbors[i].target;
      data_.GenerateNode(succ);
      data_.SetFlag(succ, flags);
      data_.SetGVal(succ, neighbors[i].weight);
      ppq_.InsertOrDecreaseKey(succ, neighbors[i].weight);
    }

    return neighbors.size();
  }

  void AddToExistingSubgoals(nodeId & curr) {
    existing_subgoals_.push_back(curr);
    existing_subgoal_distances_.push_back(data_.GetGVal(curr));
  }

  // If curr is covered or a subgoal, propagate it to succ.
  inline void PropagateIfCovered(nodeId & start, nodeId & curr, nodeId & succ) {
    data_.SetFlagIf(
        succ, kIsCovered,
        (curr != start && (data_.IsCovered(curr) || sm_->IsSubgoal(curr))));
  }

  inline void UpdateHasReachableParent(nodeId & curr, nodeId & succ) {
    data_.SetFlagIf(succ, kHasReachableParent,
                    data_.IsReachable(curr) || data_.HasReachableParent(succ));
  }
#ifndef NO_HOG
  void DetermineAndSetColor(const MapEnvironment *env, nodeId & n) const;
#endif
};

#include "ReachabilityRelationExploreCommon.inc"

#endif /* APPS_SUBGOALGRAPH_REACHABILITYRELATION_REACHABILITYRELATIONEXPLORECOMMON_H_ */
