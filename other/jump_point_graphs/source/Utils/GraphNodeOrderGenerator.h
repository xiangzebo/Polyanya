/*
 * GraphNodeOrder.h
 *
 *  Created on: Feb 18, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_UTILS_GRAPHNODEORDERGENERATOR_H_
#define APPS_SUBGOALGRAPH_UTILS_GRAPHNODEORDERGENERATOR_H_
#include <vector>

#include "../Graphs/GraphDefinitions.h"

// Generates various orderings for (valid) nodes in a graph.
// Random
// DFSvisit (Prioritize nodes who are visited earlier by DFS).
// DFScomp (Prioritize nodes whose children are processed earlier by DFS).

// kIdNodeOrder: Smallest nodeId first.
template<class G>
void GenerateIdNodeOrder(G* g, std::vector<nodeId> & order) {
  order.clear();
  for (nodeId n = 0; n < g->GetNumAllNodes(); n++) {
    if (g->IsValidNode(n))
      order.push_back(n);
  }
}

// kRandomNodeOrder
template<class G>
void GenerateRandomOrder(G* g, std::vector<nodeId> & order) {
  GenerateIdNodeOrder(g, order);
  std::random_shuffle(order.begin(), order.end());
}

template<class G>
void GenerateDFSOrder(G* g, std::vector<nodeId> & order, bool rev_direction,
                      bool comp_order);

// kDFSVisitOrder: Node visited earlier by DFS first.
template<class G>
void GenerateDFSVisitOrder(G* g, std::vector<nodeId> & order) {
  // Search backwards.
  GenerateDFSOrder(g, order, false, false);
  std::reverse(order.begin(), order.end());
}

// kReverseDFSCompletionOrder: The first node whose subtree is completely
// explored is last in the order.
// Similar to kDFSVisitOrder, but completion order is basically "the order in
// which the recursive calls of a DFS are completed" (Funke 2014).
template<class G>
void GenerateDFSCompletionOrder(G* g, std::vector<nodeId> & order) {
  GenerateDFSOrder(g, order, false, true);
}

template<class G>
void GenerateDFSOrder(G* g, std::vector<nodeId> & order, bool rev_direction,
                      bool comp_order) {
  std::vector<nodeId> dfs_start_order;
  GenerateRandomOrder(g, dfs_start_order);
  std::vector<bool> generated(g->GetNumAllNodes(), false);
  std::vector<bool> expanded(g->GetNumAllNodes(), false);
  std::vector<nodeId> stack;
  order.clear();

  // Go over the dfs start order
  for (unsigned int i = 0; i < dfs_start_order.size(); i++) {
    nodeId n = dfs_start_order[i];

    // If the current node is valid and not visited yet by a dfs search
    // (for some reason such as disconnected neighborhoods), perform a dfs.
    if (!generated[n] && g->IsValidNode(n)) {
      assert(stack.empty());
      stack.push_back(n);
      generated[n] = true;

      while (!stack.empty()) {
        nodeId curr = stack.back();

        // For calculating the (reverse) completion order, leave the node in the
        // stack the first time it is encountered and mark it.
        // When it is encountered for the second time, it will be so after
        // all its children are placed in the order. Only then remove it and
        // place it in the order (without generating its successors again).
        if (comp_order) {
          if (!expanded[curr])
            expanded[curr] = true;
          else {
            stack.pop_back();
            order.push_back(curr);
            continue;
          }
        }
        // For regular dfs visit order, pop the node and add it to the dfs.
        else {
          stack.pop_back();
          order.push_back(curr);
        }
        for (auto a : (
            rev_direction ? g->GetPredecessors(curr) : g->GetSuccessors(curr))) {
          nodeId succ = a.target;
          if (!generated[succ] && g->IsValidNode(n)) {
            generated[succ] = true;
            stack.push_back(succ);
          }
        }
      }
    }
  }
}

template<class G>
void GetNodeOrder(G* g, std::vector<nodeId> & order, NodeOrdering type) {
  if (type == kIdNodeOrder)
    GenerateIdNodeOrder(g, order);
  else if (type == kRandomNodeOrder)
    GenerateRandomOrder(g, order);
  else if (type == kDFSVisitNodeOrder)
    GenerateDFSVisitOrder(g, order);
  else if (type == kReverseDFSVisitNodeOrder) {
    GenerateDFSVisitOrder(g, order);
    std::reverse(order.begin(), order.end());
  }
  else if (type == kDFSCompletionNodeOrder)
    GenerateDFSCompletionOrder(g, order);
  else if (type == kReverseDFSCompletionNodeOrder) {
    GenerateDFSCompletionOrder(g, order);
    std::reverse(order.begin(), order.end());
  }
  else
    assert(false && "Node order not specified!");
}

#endif /* APPS_SUBGOALGRAPH_UTILS_GRAPHNODEORDERGENERATOR_H_ */
