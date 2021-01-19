/*
 * SGEdgeConstructor.h
 *
 *  Created on: Oct 24, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_SUBGOALGRAPH_SGARCCONSTRUCTOR_H_
#define APPS_SUBGOALGRAPH_SUBGOALGRAPH_SGARCCONSTRUCTOR_H_

#include "GraphDefinitions.h"
#include "CPUTimer.h"
#include <vector>
#include <iostream>

#include "../../../../utils/FPUtil.h"
#include "../Graphs/DynamicGraph.h"
#include "../SearchMethods/Dijkstra.h"
class SGArcConstructor {
 public:
  SGArcConstructor() {
    edge_time_ = 0;
    redundancy_elimination_time_ = 0;
    construction_time_ = 0;
  }
  ~SGArcConstructor() {
  }

  // SM: Subgoal ids
  // SG: Subgoal graph
  // RAE: Reachable area explorer
  template<class SM, class SG, class R>
  void ConstructWeightedArcGraph_DirectRReachable(SM* sm, SG* sg, R* rr,
                                                    double w = 1.0, bool eliminate_redundant = false) {
    bool direct = true;

    std::vector<WeightedArc> arcs;
    if (fabs(w - 1.0) < 0.001) {
      ConstructArcs(sm, rr, direct, arcs);
      if (eliminate_redundant)
        EliminateRedundantArcs(sm, arcs);
    }
    else {
      ConstructSpannerArcs(sm, rr, direct, w, arcs);
    }

    CPUTimer t;
    t.StartTimer();
    sg->CreateGraph(sm->GetNumSubgoals(), arcs);
    t.EndTimer();
    construction_time_ = t.GetElapsedTime();

    assert(sg->GetNumArcs() == arcs.size());
  }

  template<class SM, class SG, class R>
  void ConstructWeightedArcGraph_RReachableThenPrune(SM* sm, SG* sg, R* rr,
                                                       double w = 1.0) {
    std::vector<WeightedArc> arcs;
    bool direct = true;

    if (w <= 1.01) {
      ConstructArcs(sm, rr, !direct, arcs);
      EliminateRedundantArcs(sm, arcs);
    }
    else {
      ConstructSpannerArcs(sm, rr, !direct, w, arcs);
    }

    CPUTimer t;
    t.StartTimer();
    sg->CreateGraph(sm->GetNumSubgoals(), arcs);
    t.EndTimer();
    construction_time_ = t.GetElapsedTime();

    assert(sg->GetNumArcs() == arcs.size());
  }

  double GetEdgeTime() const {return edge_time_;}
  double GetRedundancyEliminationTime() const {
    return redundancy_elimination_time_;
  }
  double GetConstructionTime() const {return construction_time_;}

 private:
  double edge_time_, redundancy_elimination_time_, construction_time_;


  template<class SM, class R>
  void ConstructArcs(SM* sm, R* rr, bool direct,
                       std::vector<WeightedArc> & arcs) {
    CPUTimer t;
    t.StartTimer();

    std::vector<nodeId> subgoals;
    sm->GetSubgoals(subgoals);
    arcs.clear();

    rr->SetExploreForward();

    int num_seconds = kReportIncrementInSeconds;
    for (unsigned int i = 0; i < subgoals.size(); i++) {
      t.EndTimer();
#ifndef SG_QUIET
      if (t.GetElapsedTime() >= num_seconds) {
        std::cout << "Processed " << i << " out of " << subgoals.size()
                  << " subgoals." << "\tIdentified directed edges: "
                  << arcs.size() << "\tTime: " << num_seconds << " seconds."
                  << std::endl;
        num_seconds += kReportIncrementInSeconds;
      }
#endif

      nodeId n = subgoals[i];
      bool can_identify_superset = true;
      if (direct)
        rr->RConnect(n, !can_identify_superset);
      else
        //rr->FindAllRReachableSubgoals(n);
        rr->RConnect(n, can_identify_superset);

      const std::vector<nodeId>* existing_subgoals = rr->GetExistingSubgoals();
      const std::vector<double>* existing_subgoal_distances = rr
          ->GetExistingSubgoalDistances();

      for (unsigned int j = 0; j < existing_subgoals->size(); j++) {
        nodeId neighbor = existing_subgoals->at(j);
        arcs.push_back(
            WeightedArc(sm->ToSubgoalId(n), sm->ToSubgoalId(neighbor),
                        existing_subgoal_distances->at(j)));
        assert(existing_subgoal_distances->at(j) < kMaxDistance / 2);
      }
    }
    t.EndTimer();
    edge_time_ = t.GetElapsedTime();
  }

  template<class SM, class R>
  void ConstructSpannerArcs(SM* sm, R* rr, bool direct, double w,
                              std::vector<WeightedArc> & spanner_arcs) {
    // Create all arcs and sort them in order of increasing weight.
    std::vector<WeightedArc> all_arcs;
    ConstructArcs(sm, rr, direct, all_arcs);

    CPUTimer t;
    t.StartTimer();
    std::sort(all_arcs.begin(), all_arcs.end(),
              [](const WeightedArc & a, const WeightedArc & b) -> bool {
                return a.weight < b.weight;
              });

    // Create an empty graph and a Dijkstra search on the graph (maybe A*?).
    spanner_arcs.clear();
    DynamicGraph<WeightedArcHead> sg;
    sg.CreateGraph(sm->GetNumSubgoals(), spanner_arcs);
    Dijkstra<DynamicGraph<WeightedArcHead> > dij(&sg);

    // Iterate over the sorted arcs and add the arc if the current distance
    // between the vertices it connects is more than w-suboptimal.
    for (auto a : all_arcs) {
      if (!dij.DoesPathUpToLengthExist(a.source, a.target, w * a.weight)) {
        sg.AddArc(a.source, WeightedArcHead(a.target, a.weight));
        spanner_arcs.push_back(a);
      }
    }
    t.EndTimer();
    redundancy_elimination_time_ = t.GetElapsedTime();
  }


  template<class SM>
  void EliminateRedundantArcs(SM* sm, std::vector<WeightedArc> & arcs) {
    CPUTimer t;
    t.StartTimer();

    int num_all_arcs = arcs.size();
    int num_redundant_arcs = 0;

    DynamicGraph<WeightedArcHead> sg;
    sg.CreateGraph(sm->GetNumSubgoals(), arcs);
    Dijkstra<DynamicGraph<WeightedArcHead> > dij(&sg);

    for (nodeId n = 0; n < sm->GetNumSubgoals(); n++) {
      Distance max_dist = 0;
      for (auto a : sg.GetSuccessors(n))
        max_dist = max(max_dist, a.weight);

      dij.Reset();
      dij.AddStart(n);
      while(!dij.IsFinished()) // && dij.GetRadius() < max_dist)
        dij.ExpandNextPreferMostRecentParent(max_dist + kEpsDistance);

      std::vector<nodeId> arcs_to_remove;
      for (auto a: sg.GetSuccessors(n))
        if (dij.GetParent(a.target) != n)
          arcs_to_remove.push_back(a.target);

      for (auto m: arcs_to_remove) {
        sg.RemoveArc(n,m);
        num_redundant_arcs++;
      }
    }

    t.EndTimer();
    redundancy_elimination_time_ = t.GetElapsedTime();

    arcs.clear();
    ExplicitGraphExtractor extractor;
    int num_nodes;
    extractor.ExtractGraph(&sg, num_nodes, arcs);

#ifndef SG_QUIET
    std::cout << "Eliminated " << num_redundant_arcs
              << " redundant arcs out of " << num_all_arcs << " arcs."
              << std::endl;
#endif
  }
};

#endif /* APPS_SUBGOALGRAPH_SUBGOALGRAPH_SGARCCONSTRUCTOR_H_ */
