/*
 * ConnectedSubgoalGenerator.h
 *
 *  Created on: Feb 24, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_SUBGOALGRAPH_CONNECTEDSUBGOALGENERATOR_H_
#define APPS_SUBGOALGRAPH_SUBGOALGRAPH_CONNECTEDSUBGOALGENERATOR_H_
#include <cstdlib>

#include "../Graphs/GraphDefinitions.h"
#include "../Parameters/SGConstructionParam.h"
#include "../Utils/SearchUtils.h"

template <class G, class SM, class R>
class ConnectedSubgoalGenerator{
 public:
  ConnectedSubgoalGenerator(G* g, SM* sm, R* r, SGConstructionParam s)
    : g_(g), sm_(sm), r_(r), p_(s){
    num_connecting_subgoals_ = 0;
  }

  int GetNumConnectingSubgoals() const {
    return num_connecting_subgoals_;
  }

  void IdentifySubgoals() {
    IdentifyConnectingSubgoals();
    ConnectSubgoals();
    AddRandomSubgoals();
  }

  void AddRandomSubgoals() {
    int num_all_nodes = g_->GetNumValidNodes();
    int num_curr_subgoals = sm_->GetNumSubgoals();

    std::uniform_real_distribution<double> dist(0,100);
    std::default_random_engine rng;

    for (nodeId n = 0; n < g_->GetNumAllNodes(); n++) {
      if (g_->IsValidNode(n) && !sm_->IsSubgoal(n)) {
        if (dist(rng) < p_.extra_subgoal_percentage)
          sm_->AddSubgoal(n);
      }
    }
  }

  void IdentifyConnectingSubgoals() {
    srand(1337);
    std::vector<nodeId> order;
    GetNodeOrder(g_, order, p_.node_ordering);

    bool allow_superset = true;

    // Make sure all nodes are connected to at least one subgoal.
    std::vector<bool> can_forward_connect(g_->GetNumAllNodes(), false);
    std::vector<bool> can_reverse_connect(g_->GetNumAllNodes(), false);

    CPUTimer t;
    t.StartTimer();
    int num_seconds = 0;
    for (unsigned int i = 0; i < order.size(); i++) {
      nodeId n = order[i];

#ifndef SG_QUIET
        t.EndTimer();
        if (t.GetElapsedTime() >= num_seconds) {
          std::cout << "Processed " << i << " out of " << order.size()
              << " nodes." << "\tIdentified subgoals: " << sm_->GetNumSubgoals()
              << "\tTime: " << num_seconds << " seconds." << std::endl;
          num_seconds += kReportIncrementInSeconds;
        }
#endif

      if (!can_forward_connect[n] || !can_reverse_connect[n]) {
        sm_->AddSubgoal(n);

        r_->SetExploreForward();
        r_->RConnect(n, allow_superset);
        for (auto m: *r_->GetExpansionOrder())
          can_reverse_connect[m] = true;

        r_->SetExploreBackward();
        r_->RConnect(n, allow_superset);
        for (auto m: *r_->GetExpansionOrder())
          can_forward_connect[m] = true;
      }
    }
#ifndef SG_QUIET
    std::cout << "Identified " << sm_->GetNumSubgoals()
        << " to make sure every node has a subgoal to connect to in either direction."
        << std::endl;
#endif
    num_connecting_subgoals_ = sm_->GetNumSubgoals();
  }

  void ConnectSubgoals() {
    // Pick a subgoal and make sure all the subgoals are forward and backward
    // reachable from it.
    if (sm_->GetSubgoals()->empty())
      return;

    nodeId source_sub = sm_->GetSubgoals()->at(0);

    for (int b = 0; b <= 1; b++) {
      bool backward = (bool) b;
#ifndef SG_QUIET
      std::cout << "Connecting subgoal " << source_sub
          << " to all other subgoals in the "
          << (std::string) (backward ? "backward" : "forward") << " direction."
          << std::endl;
#endif

      F_Val_PPQ ppq(g_->GetNumAllNodes());
      std::vector<Distance> g_val(g_->GetNumAllNodes(), kMaxDistance);
      std::vector<nodeId> parent(g_->GetNumAllNodes(), kNonNode);
      std::vector<nodeId> parent_subgoal(g_->GetNumAllNodes(), kNonNode);

      int num_initial_subgoals = sm_->GetNumSubgoals();
      int num_subgoals_to_reach = sm_->GetNumSubgoals();
//    std::vector<WeightedArc> connections;
      std::vector<nodeId> subgoals_to_add;

      ppq.InsertOrDecreaseKey(source_sub, 0);
      g_val[source_sub] = 0;
      parent_subgoal[source_sub] = source_sub;
      num_subgoals_to_reach--;

      CPUTimer t;
      t.StartTimer();
      int num_seconds = 0;
      while (!ppq.IsEmpty()) {

#ifndef SG_QUIET
        t.EndTimer();
        if (t.GetElapsedTime() >= num_seconds) {
          std::cout << "Reached "
              << num_initial_subgoals - num_subgoals_to_reach << " out of "
              << num_initial_subgoals << " subgoals." << "\tIdentified "
              << subgoals_to_add.size() << " additional subgoals.\tTime: "
              << num_seconds << " seconds." << std::endl;
          num_seconds += kReportIncrementInSeconds;
        }
#endif

        nodeId curr = ppq.PopMin();
        if (sm_->IsSubgoal(curr)) {
          if (parent_subgoal[curr] != curr) {
            num_subgoals_to_reach--;

            // If the current subgoal is not R-reachable from its parent,
            // introduce new subgoals to break down a shortest path between
            // into R-reachable segments

            // TODO? Defer this to later?
            // connections.push_back(WeightedArc(last_subgoal[curr], curr, g_val[curr]));
            std::vector<nodeId> subgoals;
            if (!backward)
              r_->SetExploreForward();
            else
              r_->SetExploreBackward();
            BreakdownPathBySubgoals(parent_subgoal[curr], curr, g_val,
                                        parent, subgoals, backward);

            // For the current subgoal and all the identified intermediate
            // subgoals, set their g-value to 0. The current subgoal is already
            // selected for expansion, so we don't update it in the priority
            // queue. For the intermediate subgoals, update their priority in
            // the queue.
            // Set their parent subgoals to themselves, since any nodes that
            // are reached from these nodes will need to be connected from them.
            // The motivation for this step is as follows:
            // - If we do not reset the g-values to 0, then this algorithm
            //   is simply Dijkstra from the source node, which will identify
            //   a shortest path tree rooted at the source node.
            //   This could result in worse solution quality for the graph (TODO).
            //   Essentially, we want some kind of a minimum spanning tree
            //   rather than a shortest path tree rooted at the source.
            // - By immediately identifying connecting subgoals and resetting
            //   their g-values as well, we encourage shorter edges in the graph
            //   TODO: Why is this important?
            parent_subgoal[curr] = curr;
            g_val[curr] = 0;
            for (auto s : subgoals) {
              subgoals_to_add.push_back(s);
              g_val[s] = 0;
              parent_subgoal[s] = s;
              ppq.InsertOrDecreaseKey(s, 0);
            }
          }
        }

        std::vector<WeightedArcHead> neighbors;
        if (!backward)
          g_->GetSuccessors(curr, neighbors);
        else
          g_->GetPredecessors(curr, neighbors);

        // Expand the node.
        for (auto a : neighbors) {
          nodeId succ = a.target;
          Distance new_g_val = g_val[curr] + g_->GetWeight(curr, a);
          if (new_g_val + 0.001 < g_val[succ]) {
            g_val[succ] = new_g_val;
            parent[succ] = curr;
            parent_subgoal[succ] = parent_subgoal[curr];
            ppq.InsertOrDecreaseKey(succ, new_g_val);
          }
        }
      }
      for (auto s : subgoals_to_add)
        sm_->AddSubgoal(s);

#ifndef SG_QUIET
      std::cout<<"Num subgoals: "<<sm_->GetNumSubgoals()<<std::endl;
      std::cout<<"Num unreached subgoals: "<<num_subgoals_to_reach<<std::endl;
#endif
    }

    /*
    Simple algorithm:
    Pick a subgoal. Do a forward search and construct a tree of subgoals.
    Do a backward search and construct a tree of subgoals.
    The combination of both trees form a strongly connected graph.
    (In a strongly connected component, it is bound to find all the subgoals.)
    In both directions, break down the tree of subgoals into R-reachable segments.
    Add all the extra subgoals.

    Complex algorithm:
    Initialize bidirectional dijkstra such that:
    In the forward direction, the queue contains all the successors of
    all subgoals with appropriate costs. (or, simply, just the subgoals?)
    In the backward direction, the queue contains all the predecessors of all
    subgoals with appropriate costs. (or, simply, just the subgoals?)
    Annotate each node with the ancestor subgoal with the least cost.

    Also initialize a DAG that contains all subgoals but no edges.

    Run bidirectional dijkstra. When two frontiers meet with different subgoal
    ancestors (DEFINE THIS):
      Add an edge (u,v) to DAG.
      If the edge creates a cycle:
        contract the cycle into a single node in the DAG.
      Delete u in the forward direction.
      Delete v in the backward direction.
      Or, simply merge them so we don't need more computation?
      Repeat until DAG contains a single node.

    Invariant: The forward search (voronoi diagram) only contains nodes in the
    DAG with no successors. The backward search (voronoi diagram) only contains
    nodes in the DAG with no predecessors.
    */
  }


 private:
  G* g_;
  SM* sm_;
  R* r_;
  SGConstructionParam p_;

  int num_connecting_subgoals_;


  void BreakdownPathBySubgoals(nodeId start, nodeId goal,
                                 std::vector<Distance> & g_val,
                                 std::vector<nodeId> & parent,
                                 std::vector<nodeId> & subgoals,
                                 bool backward) {
      assert(g_val.size() == g_->GetNumAllNodes());

      std::vector<nodeId> p;  // Path
      ExtractPath(start, goal, parent, p);

      if (backward)
        std::reverse(p.begin(), p.end());
      int curr = 0;
      int end = p.size()-1;
      while (curr < end) {
        int last_r = r_->FindLastReachableIndex(p, curr, end);
        assert (last_r != curr);
        subgoals.push_back(p[last_r]);
        curr = last_r;
      }
      return;
  }
};


#endif /* APPS_SUBGOALGRAPH_SUBGOALGRAPH_CONNECTEDSUBGOALGENERATOR_H_ */
