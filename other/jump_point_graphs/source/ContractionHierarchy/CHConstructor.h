/*
 * CHConstructor.h
 *
 *  Created on: Oct 27, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_CONTRACTIONHIERARCHY_CHCONSTRUCTOR_H_
#define APPS_SUBGOALGRAPH_CONTRACTIONHIERARCHY_CHCONSTRUCTOR_H_

#include <iostream>
#include <limits>
#include <vector>

#include "../Graphs/DynamicGraph.h"
#include "../Graphs/GraphDefinitions.h"
#include "../Parameters/CHParam.h"
#include "../SubgoalGraph/SubgoalIdMapper.h"
#include "../Utils/CPUTimer.h"
#include "../Utils/PreallocatedPriorityQueue.h"
#include "../Utils/Statistics/QueryStatistic.h"
#include "CHSearchGraph.h"
#include "ContractionCore.h"
#include "ContractionSimulator.h"
#include "DoOnlyLongerPathsExistCheck.h"

// Inputs:
// - Number of nodes
// - Set of edges
// - Valid nodes? (a boolean vector specifying whether each node is valid or not)

// Parameters:
// - Max step count (a witness search terminates after this many iterations)

// Outputs:
// - A CH (just the edges, no need for level)
//   - Can be a single graph (undirected) or a forward and a backward graph

// Important components:
// - ContractionCore: A (bidirectional) graph that allows contracting of nodes.
//   That is, removing their incoming and outgoing edges, and adding shortcuts.
//   - Arcs: ContractionArcs (source, target, weight, mid, hops).
//   - Useful in: CH, R-CH; require a different version for R-SPC construction.
//
// - Witness search (bidirectional dijkstra, or single dijkstra?)
//   - Deserves a special class because it should allow a single
//     dijkstra search or bidirectional dijkstra?
// - SimulateContraction (get extra edges and determine whether the node
//   can be contracted.
//
// Heavy-R-Contract (TODO):
// - Add query points to contraction core
// - Witness search is unchanged
//   - Should work fine after adding query points
//   - Should connect start to goal (if local-local edge exists)
//
// - Simulate contraction should also consider local predecessors and successors
//   - Can again be done by changing the core only

// Constructs a contraction hierarcy using online contraction order?
// Importance: arcs_added/arcs_removed + hops_removed/hops_added + level
// See 'ContractionSimulator.h' to see how a lower bound is used to implement
// R-contractions where R = h-reachability.

// TODO: Make CHConstructor take as input a graph.
// FIXME: R-contractions are not fully implemented yet since the non-contracted
// nodes stay in the contraction graph and not moved to the contracted graph.

struct CHConstructionStatistics {
  CHConstructionStatistics() {
    num_initial_arcs = 0;
    num_initial_nodes = 0;

    num_contracted = 0;
    num_shortcuts_added = 0;
    num_shortcuts_removed = 0;
    num_original_arcs_removed = 0;

    num_original_arcs_in_hierarchy = 0;
    num_shortcuts_in_hierarchy = 0;

    num_estimate_importance_calls = 0;
    num_witness_searches_importance = 0;
    num_witness_searches_shortcuts = 0;
    stat_importance = QueryStatistic();
    stat_shortcuts = QueryStatistic();
  }
  int num_initial_nodes, num_initial_arcs, num_contracted, num_shortcuts_added,
      num_shortcuts_removed, num_original_arcs_removed,
      num_original_arcs_in_hierarchy, num_shortcuts_in_hierarchy;

  int num_estimate_importance_calls, num_witness_searches_importance,
      num_witness_searches_shortcuts;
  QueryStatistic stat_importance, stat_shortcuts;
};

class CHConstructor {
 public:
  CHConstructor(CHParam p, int num_nodes,
                std::vector<WeightedArc> & arcs, bool is_undirected)
      : p_(p),
        core_(p_.contraction_type, num_nodes, arcs, is_undirected),
        longer_path_check_(p.contraction_type, &core_,
                           p_.witness_search_expansion_bound),
        contraction_simulator_(p.contraction_type, &core_, &longer_path_check_) {
    assert(p.contraction_type != kNoContraction);
    st_.num_initial_arcs = arcs.size();
    st_.num_initial_nodes = 0;
    for (auto b : core_.is_contracted_)
      if (!b)
        st_.num_initial_nodes++;
  }
  ~CHConstructor() {};

  template <class R = DummyR>
  void ConstructCH(R* r = NULL) {
    ppq_.SetupQueue(core_.GetNumNodes());

    if (p_.contraction_type != kHeavyRContraction)
      ContractUsingOnlineOrder(r);
    else {
      // Keep creating new levels until either no node can be contracted,
      // or every node has been contracted.
      while(!HeavyRContractLevel(r));
    }
  }
  template<class GM, class SM = IdentityMapper, class R = DummyR>
  void ConstructCHAvoidRedundantEdges(int num_r_nodes,
                                      std::vector<WeightedArc> & arcs, GM* gm,
                                      SM* sm = NULL, R* r = NULL) {
    // Initial implementation:
    // - Don't care about estimations
    // - Simply don't add edges during contraction

    assert(p_.online_order_update_type != kUpdateTop);

    for (auto a: arcs) {
      assert(a.source < num_r_nodes);
      assert(a.target < num_r_nodes);
      assert(a.weight < kMaxDistance);
    }

    // Forward search graph.
    ArcListQueryGraph<WeightedArcHead> fg(num_r_nodes, arcs);

    // Backward search graph.
    std::vector<WeightedArc> reverse_arcs(arcs);
    for (auto &a: reverse_arcs)
      std::swap(a.source, a.target);
    ArcListQueryGraph<WeightedArcHead> bg(num_r_nodes, reverse_arcs);
    reverse_arcs.clear();
    // Bidirectional search.
    BidirectionalDijkstra<ArcListQueryGraph<WeightedArcHead>,
        ArcListQueryGraph<WeightedArcHead>> bidij(&fg, &bg);

    auto IsRedundantArc = [&](const ContractionArc & a) -> bool {
      nodeId s = a.source;
      nodeId g = a.target;
      if (sm != NULL) {
        s = sm->ToNodeId(s);
        g = sm->ToNodeId(g);
      }

      bidij.ResetSearchInfo();
      bidij.GetForward()->Reset();
      bidij.GetBackward()->Reset();
      auto s_nodes = gm->GetGroupSiblingNodes(s);
      auto g_nodes = gm->GetGroupSiblingNodes(g);
      for (auto n: *s_nodes) {
        bidij.GetForward()->AddStart(n);
      }
      for (auto n: *g_nodes) {
        bidij.GetBackward()->AddStart(n);
      }
      while (!bidij.CanStop() && bidij.GetSumOfRadii() <= a.weight + 0.001) {
        bidij.ExpandNext();
      }
      return bidij.GetBestDistance() + kEpsDistance < a.weight;
    };

    std::vector<double> importance(core_.GetNumNodes(), 0);

    auto EstimateImportanceAndInsertOrUpdateKey = [&](nodeId n) {
      // Estimate importance.
#ifdef JP_CH_AVOID_REDUNDANT_WHEN_ESTIMATING_IMPORTANCE
      importance[n] = EstimateImportance(n,r,IsRedundantArc);
#else
      importance[n] = EstimateImportance(n,r);
#endif

      // Insert/update key
#ifdef JP_CH_GROUP_CONTRACT
  #ifdef JP_CH_AVOID_REDUNDANT_BY_GRID_SEARCH
      auto group_nodes = gm->GetGroupSiblingNodes(sm->ToNodeId(n));
      double group_importance = 0;
      int num_group_nodes = 0;
      for (auto m : *group_nodes) {
        if (sm->IsSubgoal(m)) {
          group_importance += importance[sm->ToSubgoalId(m)];
          num_group_nodes++;
        }
      }
      ppq_.InsertOrUpdateKey(gm->GetGroup(sm->ToNodeId(n)), group_importance/(double)num_group_nodes);
  #else
      assert(sm == NULL); // Not implemented yet.
      auto group_nodes = gm->GetGroupSiblingNodes(n);
      double group_importance = 0;
      for (auto m : *group_nodes) {
        group_importance += importance[m];
        assert(!core_.WasContracted(m));
      }
      ppq_.InsertOrUpdateKey(gm->GetGroup(n), group_importance/(double)group_nodes->size());
  #endif
#else
      ppq_.InsertOrUpdateKey(n, importance[n]);
#endif
      // Importance for contracted nodes = -1, not sure if necessary.
    };

    // Initialize the contraction order.
#ifdef JP_CH_GROUP_CONTRACT
    ppq_.SetupQueue(gm->GetNumGroups());
#else
    ppq_.SetupQueue(core_.GetNumNodes());
#endif
    ppq_.Reset();

    CPUTimer t;
    t.StartTimer();
#ifndef SG_QUIET
    std::cout<<"Contracting by eliminating redundant edges..."<<std::endl;
    std::cout<<"Calculating initial contraction order..."<<std::endl;
#endif

    int num_nodes = core_.GetNumNodes();
    for (nodeId n = 0; n < num_nodes; n++) {
      if (!core_.WasContracted(n))
        EstimateImportanceAndInsertOrUpdateKey(n);
      else
        importance[n] = -1;
    }

    t.EndTimer();
#ifndef SG_QUIET
    std::cout << "Initial contraction order computed in " << t.GetElapsedTime()
              << " seconds." << std::endl;
#ifdef JP_CH_GROUP_CONTRACT
    std::cout << "Queue contains "<<ppq_.GetSize()<<" groups." <<std::endl;
#endif
#endif
    t.StartTimer();
    int elapsed = kReportIncrementInSeconds;
    int num_contracted = 0;
    std::vector<nodeId> affected_nodes;
    while (!ppq_.IsEmpty()) {

      // Give updates on the contraction process
      t.EndTimer();
#ifndef SG_QUIET
      if (t.GetElapsedTime() >= elapsed) {
        std::cout << "Contracted " << num_contracted << " out of " << num_nodes
            << " nodes."
            << "\tTime: " << elapsed << " seconds." << std::endl;
        elapsed += kReportIncrementInSeconds;
      }
#endif


#ifdef JP_CH_GROUP_CONTRACT
      nodeId m = ppq_.PopMin(); // groupId
#ifdef JP_CH_AVOID_REDUNDANT_BY_GRID_SEARCH
      auto group_nodes = gm->GetGroupNodes(m);
      std::vector<nodeId> nodes_to_contract;
      for (auto n : *group_nodes)
        if (sm->IsSubgoal(n))
          nodes_to_contract.push_back(sm->ToSubgoalId(n));
      for (auto n : nodes_to_contract)
#else
      auto nodes_to_contract = gm->GetGroupNodes(m);
      for (auto n : *nodes_to_contract)
#endif

#else
      nodeId n = ppq_.PopMin();
#endif
      {
        std::vector<ContractionArc> shortcuts;
        bool can_contract = contraction_simulator_.SimulateContraction(
            n, shortcuts, r);
        contraction_simulator_.AddStatistics(st_.num_witness_searches_shortcuts,
                                             st_.stat_shortcuts);
        if (can_contract) {
          for (int i = 0; i < shortcuts.size(); i++) {
            if (IsRedundantArc(shortcuts[i])) {
              shortcuts[i] = shortcuts.back();
              shortcuts.pop_back();
              i--;
            }
          }

          // Update affected nodes
          st_.num_contracted++;
          core_.ContractNode(n, shortcuts, affected_nodes,
                             st_.num_original_arcs_removed,
                             st_.num_shortcuts_added,
                             st_.num_shortcuts_removed);
          if (p_.online_order_update_type == kUpdateShortcuts)
            for (auto a : affected_nodes)
              EstimateImportanceAndInsertOrUpdateKey(a);
          num_contracted++;
        }
      }
    }
    // Move all remaining uncontracted nodes to the search graph.
    core_.FinalizeContraction();

    for (nodeId n = 0; n < num_nodes; n++)
      assert(core_.WasContracted(n));

  }

  // FIXME: Quick, hacky implementation atm.
  template <class SM, class R = DummyR>
  void ConstructCHContractSubgoalsLast(SM* sm,
                                       R* r = NULL) {
    ppq_.SetupQueue(core_.GetNumNodes());

    // Initialize the contraction order without subgoals.
    ppq_.Reset();
    int num_nodes = core_.GetNumNodes();
    for (nodeId n = 0; n < num_nodes; n++)
      if (!core_.WasContracted(n) && !sm->IsSubgoal(n))
        ppq_.InsertOrUpdateKey(n, EstimateImportance(n,r));

    std::vector<nodeId> affected_nodes;
    while (!ppq_.IsEmpty()) {
      nodeId n = ppq_.PopMin();
      if (ContractNode(n, affected_nodes, r)) {
        for (auto a:affected_nodes)
          if (!sm->IsSubgoal(a))
            ppq_.InsertOrUpdateKey(a, EstimateImportance(a,r));
      }
    }

    // Start again with subgoals
    core_.MoveUncontractedNodesToCore();
    core_.ResetHops();

    ppq_.Reset();
    for (nodeId n = 0; n < sm->GetNumSubgoals(); n++)
      ppq_.InsertOrUpdateKey(sm->ToNodeId(n), EstimateImportance(sm->ToNodeId(n), r));

//    for (nodeId n = 0; n < num_nodes; n++)
//      if (!core_.WasContracted(n) && sm->IsSubgoal(n))
//        ppq_.InsertOrUpdateKey(n, EstimateImportance(n,h));

    while (!ppq_.IsEmpty()) {
      nodeId n = ppq_.PopMin();
      if (ContractNode(n, affected_nodes, r)) {
        for (auto a:affected_nodes)
          ppq_.InsertOrUpdateKey(a, EstimateImportance(a,r));
      }
    }

    core_.FinalizeContraction();
  }

  template<class GType>
  CHSearchGraph<GType>* CreateSearchGraph() {
    return core_.CreateSearchGraph<GType>();
  }

  ContractionCore* GetCore() {
    return &core_;
  }
  CHConstructionStatistics GetStatistics() {
    st_.num_original_arcs_in_hierarchy = st_.num_initial_arcs
        - st_.num_original_arcs_removed;
    st_.num_shortcuts_in_hierarchy = st_.num_shortcuts_added
        - st_.num_shortcuts_removed;
    return st_;
  }


 private:
  CHParam p_;
  ContractionCore core_;
  DoOnlyLongerPathsExistCheck longer_path_check_;
  ContractionSimulator contraction_simulator_;
  F_Val_PPQ ppq_;

  CHConstructionStatistics st_;

  template <class R = DummyR>
  bool HeavyRContractLevel(R* r) {
    core_.StartHeavyRContractingLevel();
    ppq_.Reset();

    int num_nodes = core_.GetNumNodes();
    int num_contractable = num_nodes;

    for (nodeId n = 0; n < num_nodes; n++)
      if (!core_.WasContracted(n))
        ppq_.InsertOrUpdateKey(n, EstimateImportance(n,r));
      else
        num_contractable--;

    int num_contracted = 0;

    std::vector<nodeId> affected_nodes;
    while (!ppq_.IsEmpty()) {
      nodeId n = ppq_.PopMin();
      if (ContractNode(n, affected_nodes, r)) {
        for (auto a:affected_nodes)
          ppq_.InsertOrUpdateKey(a, EstimateImportance(a,r));
        num_contracted++;
      }
    }
    core_.FinishHeavyRContractingLevel();

    if (num_contracted == num_contractable) {
      core_.FinalizeContraction();
      return true;
    }
    else if (num_contracted == 0) {
      core_.FinalizeContraction();
      core_.GetLevelManager()->DecrementLevelsOfAllHighestLevelNodes();
      return true;
    }
    return false;

//    return (num_contracted == num_contractable || num_contracted == 0);
  }

  template <class R = DummyR>
  void ContractUsingOnlineOrder(R* r = NULL) {
    // Initialize the contraction order.
    ppq_.Reset();

    CPUTimer t;
    t.StartTimer();
#ifndef SG_QUIET
    std::cout<<"Calculating initial contraction order..."<<std::endl;
#endif

    int num_nodes = core_.GetNumNodes();
    for (nodeId n = 0; n < num_nodes; n++)
      if (!core_.WasContracted(n))
        ppq_.InsertOrUpdateKey(n, EstimateImportance(n,r));

    t.EndTimer();
#ifndef SG_QUIET
    std::cout << "Initial contraction order computed in " << t.GetElapsedTime()
              << " seconds." << std::endl;
#endif
    t.StartTimer();
    int elapsed = kReportIncrementInSeconds;
    int num_contracted = 0;
    std::vector<nodeId> affected_nodes;
    while (!ppq_.IsEmpty()) {

      // Give updates on the contraction process
      t.EndTimer();
#ifndef SG_QUIET
      if (t.GetElapsedTime() >= elapsed) {
        std::cout << "Contracted " << num_contracted << " out of " << num_nodes
            << " nodes."
            << "\tTime: " << elapsed << " seconds." << std::endl;
        elapsed += kReportIncrementInSeconds;
      }
#endif
      nodeId n = ppq_.GetMin();
      Distance old_importance = ppq_.GetMinCost();

      // If its importance has increased, don't contract it and
      // update its importance.
      if (p_.online_order_update_type == kUpdateTop) {
        double curr_importance = EstimateImportance(n,r);
        if (curr_importance > old_importance + 0.01) {
          ppq_.InsertOrUpdateKey(n, curr_importance);
          continue;
        }
      }
      ppq_.PopMin();
      if (ContractNode(n, affected_nodes, r)) {
        if (p_.online_order_update_type == kUpdateShortcuts)
          for (auto a:affected_nodes)
            ppq_.InsertOrUpdateKey(a, EstimateImportance(a,r));
        num_contracted++;
      }
    }
    // Move all remaining uncontracted nodes to the search graph.
    core_.FinalizeContraction();
  }

  // Returns false if a node cannot be contracted (for R-contractions)
  template <class R = DummyR>
  bool ContractNode(nodeId n, std::vector<nodeId> & affected_nodes,
                    R* r) {
    std::vector<ContractionArc> shortcuts;

    bool can_contract = contraction_simulator_.SimulateContraction(n, shortcuts,
                                                                   r);
    contraction_simulator_.AddStatistics(st_.num_witness_searches_shortcuts,
                                         st_.stat_shortcuts);
    if(!can_contract)
      return false;

    // Stats.
    st_.num_contracted++;
    core_.ContractNode(n, shortcuts, affected_nodes,
                       st_.num_original_arcs_removed, st_.num_shortcuts_added,
                       st_.num_shortcuts_removed);
    return true;
  }


  template<class R = DummyR>
  double EstimateImportance(nodeId n, R* r) {
    auto IsRedundantEdge = [&](const ContractionArc & a) -> bool {return false;};
    return EstimateImportance(n, r, IsRedundantEdge);
  }

  template<class RA, class R = DummyR>
  double EstimateImportance(nodeId n, R* r, RA IsRedundantArc) {
    // TODO: Ben's GPPC code also has some special cases:
    // If in_degree <= 1 or out_degree <=1, return -2
    // If in_degree == out_degree == 2, and the two in-arcs are the same as the
    // the two out-arcs, return -1.

    st_.num_estimate_importance_calls++;

    // Determine arcs/hops removed.
    int arcs_removed = 0;
    int hops_removed = 0;

    std::vector<ContractionArcHead> successors;
    core_.GetSuccessors(n, successors);
    for (auto s:successors) {
      arcs_removed++;
      hops_removed += s.hops;
    }

    // Avoid counting the removed arcs twice if the graph is undirected.
    if (!core_.IsUndirected()) {
      std::vector<ContractionArcHead> predecessors;
      core_.GetPredecessors(n, predecessors);
      for (auto p:predecessors) {
            arcs_removed++;
            hops_removed += p.hops;
      }
    }

    // Simulate contraction. If the node cannot be contracted, return a high
    // cost.
    std::vector<ContractionArc> shortcuts;
    bool can_contract = contraction_simulator_.SimulateContraction(n, shortcuts,
                                                                   r);

    contraction_simulator_.AddStatistics(st_.num_witness_searches_importance,
                                         st_.stat_importance);

    if (!can_contract)
      return std::numeric_limits<double>::max()/2;

    // Determine arcs/hops added.
    int arcs_added = 0;
    int hops_added = 0;

    for (auto s:shortcuts) {
      if (!IsRedundantArc(s)) {
        arcs_added++;
        hops_added += s.hops;
      }
    }

    // Not sure if this is necessary.
    if (arcs_removed == 0 || hops_removed == 0)
      return 0;
    //return std::numeric_limits<double>::max();

    return (double) arcs_added / (double) arcs_removed
         + (double) hops_added / (double) hops_removed
         + core_.GetLevelManager()->GetLevel(n);
  }
};

#endif /* APPS_SUBGOALGRAPH_CONTRACTIONHIERARCHY_CHCONSTRUCTOR_H_ */
