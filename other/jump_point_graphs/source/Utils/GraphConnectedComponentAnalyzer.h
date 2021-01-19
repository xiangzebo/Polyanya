/*
 * GraphConnectedComponentAnalyzer.h
 *
 *  Created on: Nov 18, 2016
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_UTILS_GRAPHCONNECTEDCOMPONENTANALYZER_H_
#define APPS_SUBGOALGRAPH_UTILS_GRAPHCONNECTEDCOMPONENTANALYZER_H_

#include <cassert>
#include <iostream>
#include <vector>

#include "../Graphs/GraphDefinitions.h"

//TODO: Right now, this only works for undirected graphs.

template<class G>
class GraphConnectedComponentAnalyzer {
 public:
  GraphConnectedComponentAnalyzer(G* g)
      : g_(g) {
    IdentifyFullyConnectedComponents();
    //AnalyzeComponents();
  }
  ~GraphConnectedComponentAnalyzer() {
  }

  void AnalyzeComponents();
  const std::vector<int>* GetConnectedComponentIds() const {
    return &component_id_;
  }
  const std::vector<bool>* GetLargestConnectedComponent() const {
    return &in_largest_connected_component_;
  }

  int GetNumConnectedComponents() {
    return num_components_;
  }

  void ListSmallerConnectedCompmonents(bool list_only_one = true) {
    for (int i = 0; i < num_components_; i++) {
      if (i != largest_component_id_) {
        std::cout<<"Component id: "<<i<<std::endl;
        for (nodeId n = 0; n < g_->GetNumAllNodes(); n++) {
          if (component_id_[n] == i) {
            std::cout<<n<<std::endl;
            if (list_only_one)
              return;
          }
        }
      }
    }
  }

  void IdentifyFullyConnectedComponents() {
    component_id_.resize(g_->GetNumAllNodes(), -1);
    num_components_ = 0;

    nodeId source = 0;
    while (true) {
      // Identify a valid node that is not part of an existing component as a
      // starting point for identifying a new component.
      while (source < g_->GetNumAllNodes()
          && (!g_->IsValidNode(source) || (component_id_[source] != -1)))
        source++;
      if (source >= g_->GetNumAllNodes())
        break;

      // Id of the current component.
      int c_id = num_components_;
      num_components_++;

      std::vector<bool> forward_reachable(g_->GetNumAllNodes(), false);
      // Identify all forward reachable nodes from the source.
      // Don't expand nodes that are already in another component.
      forward_reachable[source] = true;
      std::vector<nodeId> stack;
      stack.push_back(source);
      while (!stack.empty()) {
        nodeId curr = stack.back();
        stack.pop_back();
        std::vector<WeightedArcHead> neighbors;
        for (auto a : g_->GetSuccessors(curr)) {
          if (component_id_[curr] == -1 && !forward_reachable[a.target]) {
            forward_reachable[a.target] = true;
            stack.push_back(a.target);
          }
        }
      }

      // Identify all backward reachable nodes from the source.
      // Don't explore any nodes that are not forward reachable from the source.
      // (In the forward search above, if nodes in another component are not
      // marked as forward reachable, the backward search will not expand them
      // either.
      component_id_[source] = c_id;
      assert(stack.empty());
      stack.push_back(source);
      while (!stack.empty()) {
        nodeId curr = stack.back();
        stack.pop_back();
        std::vector<WeightedArcHead> neighbors;
        for (auto a : g_->GetPredecessors(curr)) {
          if (forward_reachable[a.target] && component_id_[a.target] != c_id) {
            stack.push_back(a.target);
            component_id_[a.target] = c_id;
          }
        }
      }
    }
    IdentifyLargestComponent();
  }

 private:
  G* g_;
  int num_components_;
  std::vector<int> component_id_;
  std::vector<bool> in_largest_connected_component_;
  std::vector<nodeId> num_nodes_in_compenent_;
  int largest_component_id_;

  void IdentifyLargestComponent() {
    int num_nodes = 0;
    std::vector<int> nodes_in_component(num_components_, 0);
    for (nodeId n = 0; n < g_->GetNumAllNodes(); n++) {
      if (g_->IsValidNode(n)) {
        num_nodes++;
        assert(component_id_[n] != -1);
        nodes_in_component[component_id_[n]]++;
      }
    }

    int largest_component_size = 0;
    largest_component_id_ = 0;
    for (int i = 0; i < num_components_; i++) {
      if (nodes_in_component[i] > largest_component_size) {
        largest_component_size = nodes_in_component[i];
        largest_component_id_ = i;
      }
    }

  #ifndef SG_QUIET
    std::cout << "The graph has " << num_components_ << " disjoint components!"
              << std::endl;
    std::cout << "Largest component contains " << largest_component_size
              << " nodes out of a total of " << num_nodes << " nodes."
              << std::endl;
  #endif

    in_largest_connected_component_.resize(g_->GetNumAllNodes(), false);
    for (nodeId n = 0; n < g_->GetNumAllNodes(); n++)
      if (component_id_[n] == largest_component_id_)
        in_largest_connected_component_[n] = true;
  }
};

template<class G>
void GraphConnectedComponentAnalyzer<G>::AnalyzeComponents() {
  component_id_.resize(g_->GetNumAllNodes(), -1);
  num_components_ = 0;

  nodeId start = 0;
  while (start < g_->GetNumAllNodes()) {
    // Identify a valid node that is not part of an existing component as a
    // starting point for identifying a new component.
    while (start < g_->GetNumAllNodes()
        && (!g_->IsValidNode(start) || (component_id_[start] != -1)))
      start++;

    if (start >= g_->GetNumAllNodes())
      break;

    // Do a DFS to find all the nodes that are connected to 'start' (Identify the connected component).
    std::vector<nodeId> stack;
    stack.push_back(start);
    component_id_[start] = num_components_;

    while (!stack.empty()) {
      nodeId curr = stack.back();
      stack.pop_back();

      std::vector<WeightedArcHead> neighbors;
      g_->GetSuccessors(curr, neighbors);
      for (unsigned int i = 0; i < neighbors.size(); i++) {
        nodeId succ = neighbors[i].target;

        if (component_id_[succ] != num_components_) {
          assert(component_id_[succ] == -1);

          component_id_[succ] = num_components_;
          stack.push_back(succ);
        }
      }
      g_->GetPredecessors(curr, neighbors);
      for (unsigned int i = 0; i < neighbors.size(); i++) {
        nodeId pred = neighbors[i].target;

        if (component_id_[pred] != num_components_) {
          assert(component_id_[pred] == -1);

          component_id_[pred] = num_components_;
          stack.push_back(pred);
        }
      }
    }

    num_components_++;
  }
  IdentifyLargestComponent();
}

#endif /* APPS_SUBGOALGRAPH_UTILS_GRAPHCONNECTEDCOMPONENTANALYZER_H_ */
