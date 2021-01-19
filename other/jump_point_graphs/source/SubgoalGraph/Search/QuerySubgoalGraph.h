/*
 * QuerySubgoalGraph.h
 *
 *  Created on: Oct 17, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_SUBGOALGRAPH_QUERYSUBGOALGRAPH_H_
#define APPS_SUBGOALGRAPH_SUBGOALGRAPH_QUERYSUBGOALGRAPH_H_
#include "../../Utils/AvoidanceTable.h"
#include "../Utils/Statistics/ConnectionStatistics.h"
#include "SubgoalGraphDefinitions.h"

// Adds the query points to a subgoal graph that can be searched in the
// forward direction (the arcs lead to the goal, not from the goal.
// If it is detected that start and goal are R-reachable, returns the path
// between them.

// Can be specialized to different type of graphs:
// - Graph type is a template argument. Arc type is determined from the graph
//   and only relevant type of arcs are added to the graph.
// - The graph can be directed or undirected (this is not checked, see below).
// - The search can be a forward search or a bidirectional search (constructor
//   requires the parameter 'bidirectional'.

// How:
// - Backward graph is only specified when the search is bidirectional on a
//   directed graph. Otherwise, bg = fg (which is by default if bg is omitted).
// - In any situation (forward/bidirectional search, directed/undirected graphs)
//   the start is always added to the forward graph, and the arcs are added as
//   out-arcs of the start.
// - The goal situation is a bit more different:
//   - For any forward search, goal is added to the forward graph, and the arcs
//     are added as in-arcs of the goal.
//   - For any bidirectional search, goal is added to the backward graph as
//     out-arcs of the goal.

// FIXME: Current implementation assumes the type of BG = FG (not just with the
// default template argument for BG, but also in the implementation.

// TODO: Different arc types.
template<class SM, class E, class FG, class BG = FG>
class QuerySubgoalGraph {
 public:

  QuerySubgoalGraph(SM* sm, E* explorer, bool bidirectional, FG* fg,
                    BG* bg)
      : sm_(sm),
        explorer_(explorer),
        bidirectional_(bidirectional),
        fg_(fg),
        bg_(bg) {
  }

  QuerySubgoalGraph(SM* sm, E* explorer, bool bidirectional, FG* fg)
      :QuerySubgoalGraph(sm, explorer, bidirectional, fg, fg) {}

  ~QuerySubgoalGraph() {
  }

  // TODO: Maybe first make start and goal a subgoal and then
  // find the connecting edges.
  // Right now, we first add start and connect it to subgoals, meaning
  // the start->goal edge is not found (if exists).
  // Then, we reverse connect goal so the start->goal edge is added.
  // For forward search queries, this is not a problem.
  // For bidirectional queries, this might be a problem.

  double AddQueryPoints(nodeId n_start, nodeId n_goal, subgoalId & s_start,
                      subgoalId & s_goal, std::vector<nodeId> & path,
                      AvoidanceTable* forward_avoidance_table = NULL,
                      AvoidanceTable* backward_avoidance_table = NULL) {
    st.num_queries++;
    Distance d;
    if (!explorer_->CanDetectReachabilityOfQueryPoints()
        && explorer_->GetQueryPathIfReachable(n_start, n_goal, d, path)) {
      st.num_reachable_path_found++;
      return d;
    }

    bool is_goal = true;
    // When connecting the start, update the backward avoidance table.
    s_start = ConnectQueryPoint(n_start, !is_goal, backward_avoidance_table);

    // When connecting the goal, update the forward avoidance table and use the
    // (updated) backward avoidance table to avoid connecting to some of the nodes.
    s_goal = ConnectQueryPoint(n_goal, is_goal, forward_avoidance_table,
                               backward_avoidance_table);
    return kMaxDistance;
  }

  void ClearQuery() {
    sm_->ClearQuery();
    fg_->ClearQuery();
    bg_->ClearQuery();
  }

  AggregateConnectionStatistics GetAggregateConnectionStatistics() {
    return st;
  }

 private:
  SM* sm_;
  E* explorer_;
  bool bidirectional_;
  FG* fg_;
  BG* bg_;

  typedef typename FG::SuccessorType FGSuccessorType;
  std::vector<FGSuccessorType> arcs;

  AggregateConnectionStatistics st;

  subgoalId ConnectQueryPoint(nodeId n, bool is_goal,
                              AvoidanceTable* table_to_update = NULL,
                              AvoidanceTable* table_for_pruning = NULL) {
    subgoalId s;
    // If it is already a subgoal, no need to connect it.
    if (sm_->IsSubgoal(n)) {
      return sm_->ToSubgoalId(n);
    }
    s = sm_->AddQuerySubgoal(n);

    if (is_goal) {
      explorer_->SetExploreBackward();
      st.num_backward_connect++;
    }
    else {
      explorer_->SetExploreForward();
      st.num_forward_connect++;
    }
    bool can_identify_superset = true;

    CPUTimer t;
    t.StartTimer();
    explorer_->RConnect(n, can_identify_superset);
    const std::vector<nodeId>* existing_subgoals = explorer_
        ->GetExistingSubgoals();
    const std::vector<double>* existing_subgoal_distances = explorer_
        ->GetExistingSubgoalDistances();


    arcs.clear();

    if (table_for_pruning != NULL) {
      for (unsigned int i = 0; i < existing_subgoals->size(); i++) {
        subgoalId e_s = sm_->ToSubgoalId(existing_subgoals->at(i));
        if (!table_for_pruning->CanAvoid(e_s)) {
          FGSuccessorType arc;
          arc.target = e_s;
          arc.weight = existing_subgoal_distances->at(i);
          arcs.push_back(arc);
        }
      }
    }
    else {
      for (unsigned int i = 0; i < existing_subgoals->size(); i++) {
        subgoalId e_s = sm_->ToSubgoalId(existing_subgoals->at(i));
        FGSuccessorType arc;
        arc.target = e_s;
        arc.weight = existing_subgoal_distances->at(i);
        arcs.push_back(arc);
      }
    }

    if (table_to_update != NULL) {
      table_to_update->AddException(s);
      for (auto a: arcs)
        table_to_update->AddException(a.target);
    }

    // Start is always added to the forward graph with non-reversed arcs.
    if (!is_goal)
      fg_->AddQueryNode(s, arcs, false);

    // Goal is always added to the backward graph (where we assume that bg = fg
    // if the search is a forward search or the graph is undirected).
    // The arcs are only reversed (node to goal) if the search is a forward
    // search.
    else
      bg_->AddQueryNode(s, arcs, !bidirectional_);


    if (is_goal) {
      st.num_backward_expansions += explorer_->GetExpansionOrder()->size();
      //st.num_backward_subgoals += existing_subgoals->size();
      st.num_backward_subgoals += arcs.size();
      st.num_backward_pruned_subgoals += (existing_subgoals->size()
          - arcs.size());
    }
    else {
      st.num_forward_expansions += explorer_->GetExpansionOrder()->size();
      // st.num_forward_subgoals += existing_subgoals->size();
      st.num_forward_subgoals += arcs.size();
    }
    if (existing_subgoals->size() == 0) {
      std::cout<<"Could not connect node "<<n<<" to subgoal graph as a "<<
          (std::string)(!is_goal?"start":"goal") << " node."<<std::endl;

    }

    t.EndTimer();
    st.forward_time += t.GetElapsedTime();
    // FIXME? Resetting data structures for RConnect right after connection.
    // This reset will be included in the RConnect time. Otherwise,
    // this reset might be included in the RRefine time instead.
    explorer_->Reset();

    return s;
  }
};

class DummyQuerySubgoalGraph {
public:
  DummyQuerySubgoalGraph() {}
  double AddQueryPoints(nodeId n_start, nodeId n_goal, subgoalId & s_start,
                        subgoalId & s_goal, std::vector<nodeId> & path) {
    s_start = n_start;
    s_goal = n_goal;
    path.clear();
    return kMaxDistance;
  }
  void ClearQuery() {}
};

#endif /* APPS_SUBGOALGRAPH_SUBGOALGRAPH_QUERYSUBGOALGRAPH_H_ */
