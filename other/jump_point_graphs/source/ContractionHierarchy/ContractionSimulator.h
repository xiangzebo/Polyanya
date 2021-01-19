/*
 * SimulateContraction.h
 *
 *  Created on: Oct 29, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_CONTRACTIONHIERARCHY_CONTRACTIONSIMULATOR_H_
#define APPS_SUBGOALGRAPH_CONTRACTIONHIERARCHY_CONTRACTIONSIMULATOR_H_
#include "ContractionCore.h"
#include "DoOnlyLongerPathsExistCheck.h"

// Determines the extra edges that need to be added to the graph if a node
// were to be contracted.
// Two variants (for the moment): Contractions and R-Contractions
// - R-Contractions are not properly implemented. They only work for
//   h-reachability (which includes freespace and Octile reachability).
// - H-reachability is implemented as a function that provides a lower bound
//   on the distance, such that two nodes are h-reachable iff their distance
//   is equal to the h-distance.
// - In this class we say two nodes are h-reachable iff their distance is
//   *smaller than or* equal to the h-distance, which is equivalent if
//   h-distances are admissible (as they should be). This allows us to only
//   implement R-contractions and use a infinite-heuristic to simulate regular
//   contractions.

// TODO: Move somewhere appropriate
class DummyR {
 public:
  DummyR() {}
  bool IsReachable(nodeId start, nodeId goal, Distance d = kMaxDistance) {
    return true;
  }
};

template<class M, class R>
class MappedR {
 public:
  MappedR(M* sm, R* r)
      : sm_(sm),
        r_(r) {
    assert(r_ != NULL);assert(sm_ != NULL);
  }

  bool IsReachable(nodeId n1, nodeId n2, Distance d = kMaxDistance) {
    return r_->IsReachable(sm_->ToNodeId(n1), sm_->ToNodeId(n2), d);
  }

 private:
  M* sm_;
  R* r_;
};

class ContractionSimulator {
 public:
  ContractionSimulator(ContractionType c_type, ContractionCore* ch_core,
                       DoOnlyLongerPathsExistCheck* longer_path_check)
      : c_type_(c_type),
        core_(ch_core),
        longer_path_check_(longer_path_check) {
    num_witness_searches_ = 0;
  }

  // Simulates a regular contraction if h = NULL.
  // Otherwise, calls h->GetHeuristicDistance to determine if an shortcut arc
  // (s,t) is allowed. (s,t) is allowed if and only if d(s,t) <= h(s,t).
  template <class R = DummyR>
  bool SimulateContraction(nodeId n, std::vector<ContractionArc> & shortcuts,
                           R* r) {
    assert(c_type_ != kRegularContraction || r == NULL);
    assert(c_type_ != kRContraction || r != NULL);
    assert(c_type_ != kHeavyRContraction || r != NULL);

    stat_ = QueryStatistic();
    num_witness_searches_ = 0;

    std::vector<ContractionArcHead> predecessors, successors;
    core_->GetPredecessors(n, predecessors);
    core_->GetSuccessors(n, successors);

    longer_path_check_->SetBypassNode(n);
    shortcuts.clear();

    // If one EvaluatePair check identifies that the node can't be contracted,
    // (due to a failed h-reachability check), 'contract' is set to false,
    // which terminates the loops.
    bool can_contract = true;

    // For an undirected graph, we only need half the witness checks.
    if (core_->IsUndirected())
      for (unsigned int p = 0; p < predecessors.size() && can_contract; p++)
        for (unsigned int s = p + 1; s < successors.size() && can_contract; s++)
          can_contract = EvaluatePair(predecessors[p], successors[s], n,
                                      shortcuts, r);
    else
      for (unsigned int p = 0; p < predecessors.size() && can_contract; p++)
        for (unsigned int s = 0; s < successors.size() && can_contract; s++)
          can_contract = EvaluatePair(predecessors[p], successors[s], n,
                                      shortcuts, r);

    longer_path_check_->RestoreBypassNode();
    return can_contract;
  }

  void AddStatistics(int & num_witness_searches, QueryStatistic & stat) {
    num_witness_searches += num_witness_searches_;
    stat = stat + stat_;
  }

 private:
  // Performs the witness check and adds a shortcut arc to 'shortcuts' if
  // necessary.
  // If the shortuct fails the h-reachability check, returns false.
  template <class R = DummyR>
  bool EvaluatePair(ContractionArcHead p, ContractionArcHead s, nodeId n,
                    std::vector<ContractionArc> & shortcuts, R* r) {
    Distance distance_through_n = p.weight + s.weight;
    num_witness_searches_++;

    if (p.target != s.target
        && longer_path_check_->DoOnlyLongerPathsExist(p.target, s.target,
                                                      distance_through_n)) {
      // At this point, we know that distance_through_n = d(p,s).
      // Therefore, we can use distance_through_n for the h-reachability test.
      bool shortcut_allowed =
          (r == NULL)
          || r->IsReachable(p.target, s.target, distance_through_n);

      longer_path_check_->AddStatistics(stat_);

      if (shortcut_allowed) {
        shortcuts.push_back(
          ContractionArc(p.target,
                         s.target,
                         p.weight + s.weight,
                         n,
                         p.hops + s.hops));
        return true;
      }
      else {
        return false;
      }
    }
    else { // No shortcut needed, so the shortcut-check does not fail.
      longer_path_check_->AddStatistics(stat_);
      return true;
    }
  }

  ContractionType c_type_;
  ContractionCore* core_;
  DoOnlyLongerPathsExistCheck* longer_path_check_;

  // For the last call to
  int num_witness_searches_;
  QueryStatistic stat_;
};



#endif /* APPS_SUBGOALGRAPH_CONTRACTIONHIERARCHY_CONTRACTIONSIMULATOR_H_ */
