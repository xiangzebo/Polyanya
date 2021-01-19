/*
 * CHQuery.h
 *
 *  Created on: Oct 18, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_METHODMANAGERS_CHQUERY_H_
#define APPS_SUBGOALGRAPH_METHODMANAGERS_CHQUERY_H_

#include <stddef.h>
#include <vector>

#include "../ContractionHierarchy/CHConstructor.h"
#include "../ContractionHierarchy/CHSearchGraph.h"
#include "../Graphs/GraphDefinitions.h"
#include "../SearchMethods/CHBidirectionalDijkstra.h"
#include "../Utils/CPUTimer.h"
#include "../Utils/GraphUtils.h"
#include "../Utils/InstanceManager.h"
#include "../Utils/Statistics.h"

template<class S, class G>
class CHQuery {
 public:
  CHQuery(G* g, bool is_undirected, CHParam p)
      : g_(g),
        is_undirected_(is_undirected),
        p_(p){
    ch_constructor_ = NULL;
    ch_search_graph_ = NULL;
    ch_bidij_ = NULL;
    preprocess_time_ = 0;
  }
  ~CHQuery() {
    if (ch_constructor_)
      delete ch_constructor_;
    if (ch_search_graph_)
      delete ch_search_graph_;
    if (ch_bidij_)
      delete ch_bidij_;
  }

  void Preprocess() {
    int num_nodes;
    std::vector<WeightedArc> arcs;
    ExplicitGraphExtractor graph_extractor;
    graph_extractor.ExtractWeightedGraph(g_, num_nodes, arcs);

    CPUTimer t;
    t.StartTimer();
    ch_constructor_ = new CHConstructor(p_, num_nodes, arcs,
                                        is_undirected_);
    ch_constructor_->ConstructCH();
    ch_search_graph_ = ch_constructor_
        ->CreateSearchGraph<ShortcutArcListQueryGraph>();
    preprocess_time_ = t.EndTimer();

    ch_bidij_ = new CHBidirectionalDijkstra<ShortcutArcListQueryGraph>(
        ch_search_graph_->GetForwardGraph(),
        ch_search_graph_->GetBackwardGraph(),
        p_.use_stall_on_demand);
  }
  void ReportPreprocessStatistics(Statistics* s) {
    int num_arcs = ch_search_graph_->GetForwardGraph()->GetNumArcs();
    if (!is_undirected_)
      num_arcs += ch_search_graph_->GetBackwardGraph()->GetNumArcs();
    s->ReportDouble("Preprocessing time (s)", preprocess_time_);
    s->ReportInt("Number of arcs", num_arcs);
    s->ReportDouble(
        "Memory",
        (4+4+4) * num_arcs
            / (1024.0 * 1024.0));
  }
  void ReportAdditionalSearchStatistics(Statistics* s) {
  }

  QueryStatistic Query(S start, S goal, std::vector<S> & state_path) {
    QueryStatistic s;
    std::vector<nodeId> node_path;

    if (ch_bidij_ != NULL) {
      CPUTimer t;
      s.dist = ch_bidij_->FindPath(g_->ToNodeId(start), g_->ToNodeId(goal),
                                  node_path);
      ch_bidij_->AddStatistics(s);
    }

    // Convert to state path.
    state_path.clear();
    for (unsigned int i = 0; i < node_path.size(); i++)
      state_path.push_back(g_->ToState(node_path[i]));
    return s;
  }
  QueryStatistic Query(ProblemInstance<S> ins) {
    std::vector<S> dummy_path;
    return Query(ins.start, ins.goal, dummy_path);
  }
  CHBidirectionalDijkstra<ShortcutArcListQueryGraph>* GetSearchMethod() {
    return ch_bidij_;
  }

 private:
  G* g_;
  bool is_undirected_;
  double preprocess_time_;
  CHParam p_;
  CHConstructor* ch_constructor_;
  CHSearchGraph<ShortcutArcListQueryGraph>* ch_search_graph_;
  CHBidirectionalDijkstra<ShortcutArcListQueryGraph>* ch_bidij_;
};

#endif /* APPS_SUBGOALGRAPH_METHODMANAGERS_CHQUERY_H_ */
