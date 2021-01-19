/*
 * SubgoalGenerator.h
 *
 *  Created on: Oct 22, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_SUBGOALGRAPH_SUBGOALGENERATOR_H_
#define APPS_SUBGOALGRAPH_SUBGOALGRAPH_SUBGOALGENERATOR_H_

#include <vector>

#include "../Utils/GraphNodeOrderGenerator.h"
#include "ConnectedSubgoalGenerator.h"
#include "GraphDefinitions.h"
#include "SubgoalSelector.h"
#include "CPUTimer.h"

// Iteratively constructs an R-Reachability Shortest Path Cover by
// identifying fringe vertices for each vertex and adding subgoals to cover
// paths to them.

// SM: Subgoal mapper (need to add subgoals as we generate them)
// E: Reachability relation area explorer.
template<class SM, class G, class E>
class SubgoalGenerator {
 public:
  SubgoalGenerator(SM* sm, G* g, E* e, SGConstructionParam s)
      : sm_(sm),
        g_(g),
        explorer_(e),
        s_(s),
        subgoal_selector_(g) {
    num_connecting_subgoals_ = 0;
  }


  int GetNumConnectingSubgoals() const {
    if (s_.construction_type == kConnectedRSPC)
      return num_connecting_subgoals_;
    else
      return sm_->GetNumSubgoals();
  }

  double IdentifySubgoals() {
    if (s_.construction_type == kGrowRSPC)
      return IdentifySubgoalsGrowRSPC();
    else if (s_.construction_type == kPruneRSPC)
      return IdentifySubgoalsPruneRSPC();
    else if (s_.construction_type == kRandomRSPC)
      return IdentifySubgoalsRandomRSPC();
    else if (s_.construction_type == kConnectedRSPC)
      return IdentifySubgoalsConnectedRSPC();
  }

  double IdentifySubgoalsConnectedRSPC() {
    srand(1337);
    CPUTimer t;
    t.StartTimer();
    ConnectedSubgoalGenerator<G, SM, E> csg(g_, sm_, explorer_, s_);
    csg.IdentifySubgoals();
    num_connecting_subgoals_ = csg.GetNumConnectingSubgoals();
    t.EndTimer();
    return t.GetElapsedTime();
  }

  double IdentifySubgoalsGrowRSPC() {
    // FIXME: Exploring subgoal graph as DFSVisit at the moment.
    srand(1337);
    std::vector<nodeId> order;
    GetNodeOrder(g_, order, s_.node_ordering);

    CPUTimer t;
    t.StartTimer();
    int num_seconds = 0;

    std::vector<nodeId> identified_subgoals;
    std::vector<nodeId> subgoal_stack;
    std::vector<bool> processed(g_->GetNumAllNodes(), false);
    int num_processed = 0;
    int i = 0;  // Iterate over the order.
    while (true) {
      nodeId n;

      // If there are unprocessed subgoals, process them first.
      while (!subgoal_stack.empty() && processed[subgoal_stack.back()])
        subgoal_stack.pop_back();

      if (!subgoal_stack.empty()) {
        n = subgoal_stack.back();
        subgoal_stack.pop_back();
      }
      // Otherwise, process the first unprocessed node in the order.
      else {
        while (i < order.size() && processed[order[i]])
          i++;

        if (i == order.size())
          break;

        n = order[i];
      }

      t.EndTimer();
      if (t.GetElapsedTime() >= num_seconds) {
#ifndef SG_QUIET
        std::cout << "Processed " << num_processed << " out of " << order.size()
            << " nodes." << "\tIdentified subgoals: " << sm_->GetNumSubgoals()
            << "\tTime: " << num_seconds << " seconds." << std::endl;
#endif
        num_seconds += kReportIncrementInSeconds;
      }

      if (s_.prioritize_exploring_from_subgoals) {
        identified_subgoals.clear();
        IdentifySubgoalsForSourceNode(n, identified_subgoals);
        for (unsigned int j = 0; j < identified_subgoals.size(); j++)
          subgoal_stack.push_back(identified_subgoals[j]);
        AddSubgoals(identified_subgoals);
      }
      else {
        GenerateSubgoalsForSourceNode(n);
      }
      processed[n] = true;
      num_processed++;
    }

    t.EndTimer();
    return t.GetElapsedTime();
  }

  double IdentifySubgoalsPruneRSPC() {
    srand(1337);
    std::vector<nodeId> order;
    GetNodeOrder(g_, order, s_.node_ordering);

    CPUTimer t;
    t.StartTimer();
    int num_processed = 0;
    int num_seconds = 0;

    sm_->MakeAllNodesSubgoals(g_);

    for (auto n : order) {
      t.EndTimer();
      if (t.GetElapsedTime() >= num_seconds) {
#ifndef SG_QUIET
        std::cout << "Processed " << num_processed << " out of " << order.size()
            << " nodes." << "\tIdentified subgoals: " << sm_->GetNumSubgoals()
            << "\tTime: " << num_seconds << " seconds." << std::endl;
#endif
        num_seconds += kReportIncrementInSeconds;
      }

      // Explore backwards to identify which forward searches to perform.
      explorer_->SetExploreBackward();
      explorer_->RConnect(n);
      std::vector<nodeId> relevant_nodes = *explorer_->GetExpansionOrder();

      // Temporarily remove n from the R-SPC.
      sm_->PruneSubgoal(n);

      // Do forward searches from all the relevant nodes to see if a fringe
      // vertex exists.
      explorer_->SetExploreForward();
      bool fringe_exists = false;
      for (auto r:relevant_nodes) {
        if (explorer_->DoesFringeNodeExist(r)) {
          fringe_exists = true;
          break;
        }
      }

      // If n cannot be removed from the R-SPC, add it back.
      if (fringe_exists)
        sm_->UnPruneSubgoal(n);

      num_processed++;
    }

    sm_->FinalizePruning();

    t.EndTimer();
    return t.GetElapsedTime();
  }

  double IdentifySubgoalsRandomRSPC() {
    srand(1337);
    std::vector<nodeId> order;
    GetNodeOrder(g_, order, kRandomNodeOrder);

    CPUTimer t;
    t.StartTimer();

    int n = order.size()*s_.percentage_subgoals/100.0;
    for (int i = 0; i < n && i < order.size(); i++)
      sm_->AddSubgoal(order[i]);

    /*
    std::uniform_real_distribution<double> dist(0,100);
    std::default_random_engine rng;

    for (auto n : order) {
      if (dist(rng) < s_.percentage_subgoals)
        sm_->AddSubgoal(n);
    }
    */
    t.EndTimer();
    return t.GetElapsedTime();
  }

  // Processes nodes in a random order.
  double IdentifySubgoals1() {
    srand(1337);
    std::vector<nodeId> order;
    GenerateRandomOrder(g_, order);

    CPUTimer t;
    t.StartTimer();

    int num_seconds = 0;
    for (unsigned int i = 0; i < order.size(); i++) {  //nodeId n = 0; n < g_->GetNumNodes(); n++) {
      nodeId n = order[i];
      t.EndTimer();
      if (t.GetElapsedTime() >= num_seconds) {
#ifndef SG_QUIET
        std::cout << "Processed " << i << " out of " << order.size()
            << " nodes." << "\tIdentified subgoals: " << sm_->GetNumSubgoals()
            << "\tTime: " << num_seconds << " seconds." << std::endl;
#endif
        num_seconds += kReportIncrementInSeconds;
      }

      GenerateSubgoalsForSourceNode(n);
    }

    t.EndTimer();
    return t.GetElapsedTime();
  }

  // Processes nodes in a random order.
  // Whenever a node is selected as a subgoal, prioritizes exploring from that
  // node. (explores known subgoals before other nodes.)
  double IdentifySubgoals2() {
    srand(1337);
    std::vector<nodeId> order;
    GenerateRandomOrder(g_, order);

    CPUTimer t;
    t.StartTimer();
    int num_seconds = 0;
    int num_processed = 0;
    int num_total = order.size();

    std::vector<nodeId> identified_subgoals;
    std::vector<nodeId> subgoal_stack;
    while (num_processed < num_total) {
      nodeId n;
      if (!subgoal_stack.empty()) {
        n = subgoal_stack.back();
        subgoal_stack.pop_back();
      } else {
        while (!order.empty() && sm_->IsSubgoal(order.back()))
          order.pop_back();
        n = order.back();
        order.pop_back();
      }

      identified_subgoals.clear();
      IdentifySubgoalsForSourceNode(n, identified_subgoals);
      for (unsigned int j = 0; j < identified_subgoals.size(); j++)
        subgoal_stack.push_back(identified_subgoals[j]);
      AddSubgoals(identified_subgoals);
      num_processed++;

      t.EndTimer();
      if (t.GetElapsedTime() >= num_seconds) {

#ifndef SG_QUIET
        std::cout << "Processed " << num_processed << " out of " << num_total
            << " nodes." << "\tIdentified subgoals: " << sm_->GetNumSubgoals()
            << "\tTime: " << num_seconds << " seconds." << std::endl;
#endif
        num_seconds += kReportIncrementInSeconds;
      }
    }

    t.EndTimer();
    return t.GetElapsedTime();
  }

  // Only considers nodes with missing predecessors as subgoals.
  // Proceeds in random order, prioritizes identified subgoals.
  // SEEMS TO BE SUBOPTIMAL
  double IdentifySubgoals3() {
    srand(1337);
    std::vector<nodeId> order;
    GenerateRandomOrder(g_, order);

    CPUTimer t;
    t.StartTimer();

    int num_seconds = 0;
    int num_processed = 0;
    int num_total = order.size();

    std::vector<nodeId> identified_subgoals;
    std::vector<nodeId> subgoal_stack;
    while (true) {
      nodeId n;
      if (!subgoal_stack.empty()) {
        n = subgoal_stack.back();
        subgoal_stack.pop_back();
      } else {
        while (!order.empty() && sm_->IsSubgoal(order.back()))
          order.pop_back();

        if (order.empty())
          break;
        n = order.back();
        order.pop_back();
      }

      identified_subgoals.clear();
      IdentifySubgoalsForSourceNode(n, identified_subgoals);
      for (unsigned int j = 0; j < identified_subgoals.size(); j++)
          subgoal_stack.push_back(identified_subgoals[j]);

      AddSubgoals(identified_subgoals);
      num_processed++;

      t.EndTimer();
      if (t.GetElapsedTime() >= num_seconds) {
#ifndef SG_QUIET
        std::cout << "Processed " << num_processed << " out of " << num_total
            << " nodes." << "\tIdentified subgoals: " << sm_->GetNumSubgoals()
            << "\tTime: " << num_seconds << " seconds." << std::endl;
#endif
        num_seconds += kReportIncrementInSeconds;
      }
    }

    // Calling a proper method of identifying subgoals tends to identify more
    // subgoals to guarantee optimality.
    IdentifySubgoals();

    t.EndTimer();
    return t.GetElapsedTime();
  }

  double IdentifySubgoals4() {
    bool only_do_bound_10 = false;

    srand(1337);
    std::vector<nodeId> order;
    GenerateRandomOrder(g_, order);

    Distance bound = 1;
    if (only_do_bound_10)
      bound = 10;

    CPUTimer t;
    t.StartTimer();
    int num_seconds = 0;

    while(!order.empty()) {
      int w = 0;
      for (unsigned int r = 0; r < order.size(); r++) {
        nodeId n = order[r];
        GenerateSubgoalsForSourceNode(n, bound);
        if (!explorer_->IsExplorationComplete()
            && !only_do_bound_10) {
          order[w] = n;
          w++;
        }
      }
      t.EndTimer();
#ifndef SG_QUIET
      std::cout << "Processed " << order.size() << " nodes for bound " << bound
          << "\tIdentified subgoals: " << sm_->GetNumSubgoals() << "\tTime: "
          << t.GetElapsedTime() << " seconds." << std::endl;
#endif
      order.resize(w);
      bound *= 2;
    }
    return t.GetElapsedTime();
  }

  ~SubgoalGenerator() {
  }
 private:
  SM* sm_;
  G* g_;
  E* explorer_;
  SGConstructionParam s_;

  SubgoalSelector<G> subgoal_selector_;

  int num_connecting_subgoals_;

  int GenerateSubgoalsForSourceNode(nodeId n, Distance exploration_bound =
                                        kMaxDistance,
                                    bool forward = true) {
    std::vector<nodeId> subgoals;
    IdentifySubgoalsForSourceNode(n, subgoals, exploration_bound, forward);
    AddSubgoals(subgoals);
    return subgoals.size();
  }

  int IdentifySubgoalsForSourceNode(nodeId n, std::vector<nodeId> & subgoals,
                                    Distance exploration_bound = kMaxDistance,
                                    bool forward = true) {
    subgoals.clear();

    if (forward)
      explorer_->SetExploreForward();
    else
      explorer_->SetExploreBackward();

    explorer_->RExploreFringe(n, exploration_bound);

    if (explorer_->GetFringeNodes()->empty())
      return 0;

    //kRandomCandidate, kParentOfLowestGValueFringeNode, kParentOfHighestGValueFringeNode, kSecretSauce
    //subgoal_selector_.SetSelectionStrategy(kRandomCandidate);
    //subgoal_selector_.SetSelectionStrategy(kParentOfLowestGValueFringeNode);
    //subgoal_selector_.SetSelectionStrategy(kParentOfHighestGValueFringeNode);
    subgoal_selector_.SetSelectionStrategy(kSecretSauce);

    bool select_only_one_subgoal = false;

    subgoal_selector_.SelectSubgoals(explorer_->GetData(),
                                     explorer_->GetFringeNodes(),
                                     explorer_->GetExpansionOrder(),
                                     subgoals, select_only_one_subgoal);
    return subgoals.size();
  }

  void AddSubgoals(std::vector<nodeId> & subgoals) {
    for (unsigned int i = 0; i < subgoals.size(); i++)
      sm_->AddSubgoal(subgoals[i]);
  }
};

#endif /* APPS_SUBGOALGRAPH_SUBGOALGRAPH_SUBGOALGENERATOR_H_ */
