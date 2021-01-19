/*
 * ContractionCore.h
 *
 *  Created on: Oct 27, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_CONTRACTIONHIERARCHY_CONTRACTIONCORE_H_
#define APPS_SUBGOALGRAPH_CONTRACTIONHIERARCHY_CONTRACTIONCORE_H_

#include "../Graphs/ArcListQueryGraph.h"
#include "../Parameters/CHParam.h"
#include "../Utils/LevelManager.h"
#include "CHSearchGraph.h"
#include "GraphDefinitions.h"
#include "DynamicGraph.h"

typedef DynamicGraph<ContractionArcHead> ContractionGraph;

// Maintains the graph that is being contracted and the output graph.
//
// Implements mainly low-level operations for contraction hierarchies:
// - Can hide a node (as the bypass node for witness searches) and restore it.
// - Can contract a node by:
//   - Removing all its incident edges in the contraction graph
//   - Adding the relevant edges to the output graph
//   - Adding extra edges to the contraction graph (taken as input)
//   - Reporting all the affected neighbors
//   - Updating the levels of all the affected neighbors
//
// Distinguishes between directed and undirected graphs:
// - Undirected graphs require a single input and output graph.
// - Bypass node and contraction operators are specialized.
//
// Keeps track of contracted nodes and the level of each node in the hierarchy.
// Levels and max_level can be used for search (for R-contractions).
// For a regular contraction hierarchy, max_level is the maximum level
// among all contracted nodes + 1 (+1 is added during finalize).
// For R-contractions, max-level is calculated similar to regular contractions
// before finalize is called. Then, all the remaining nodes in the core get
// assigned max_level (after adding +1).

class CHConstructor;

// The graph being contracted.
class ContractionCore {
 public:
  friend class CHConstructor;
  ContractionCore() {
    c_type_ = kRegularContraction;
    is_undirected_ = false;
    num_nodes_ = 0;
    bypass_node_ = kNonNode;
  }
  ContractionCore(ContractionType c_type, int num_nodes,
                  std::vector<WeightedArc> & arcs, bool is_undirected) {
    c_type_ = c_type;
    BuildContractionCore(num_nodes, arcs, is_undirected);
  }
  ~ContractionCore() {}

  // FIXME: Place somewhere good.
  void MoveUncontractedNodesToCore() {
    level_.IncrementMaxLevel();
    for (nodeId n = 0; n < num_nodes_; n++)
      if (!is_contracted_[n])
        level_.SetToMaxLevel(n);
  }
  void ResetHops() {
    // TODO: Directed case.
    for (nodeId n = 0; n < num_nodes_; n++)
      for (auto& a : this->forward_witness_search_graph_.GetSuccessors(n))
        a.hops = 1;
  }

  void BuildContractionCore(int num_nodes, std::vector<WeightedArc> & arcs,
                            bool is_undirected);

  // Hide node:
  // Removes all in-arcs associated with the node, so that the
  // witness search is forced to find an alternate path.
  void HideNode(nodeId n);
  void RestoreHiddenNode();

  // Removes all in/out-arcs associated with the node and adds the shortcut edges.
  void ContractNode(nodeId n, std::vector<ContractionArc> & shortcuts,
                    std::vector<nodeId> & neighbors,
                    int & num_original_arcs_removed,
                    int & num_shortcuts_added,
                    int & num_shortcuts_removed);

  // First moves all the remaining arcs from the contraction graphs (incident to
  // those nodes that weren't (or couldn't be) contracted into the list of
  // search arcs; then creates a CHSearchGraph object and returns it.
  void FinalizeContraction();

  bool WasContracted(nodeId n) {
    return is_contracted_[n];
  }

  int GetNumNodes() {
    return num_nodes_;
  }

  bool IsUndirected() {
    return is_undirected_;
  }
  ContractionGraph* GetForwardWitnessSearchGraph() {
    return &forward_witness_search_graph_;
  }
  ContractionGraph* GetBackwardWitnessSearchGraph() {
    if (is_undirected_)
      return &forward_witness_search_graph_;
    else
      return &backward_witness_search_graph_;
  }

  ContractionGraph* GetForwardDescendingArcGraph() {
    return &forward_descending_arc_graph_;
  }
  ContractionGraph* GetBackwardDescendingArcGraph() {
    if (is_undirected_)
      return &forward_descending_arc_graph_;
    else
      return &backward_descending_arc_graph_;
  }

  void StartHeavyRContractingLevel();
  void FinishHeavyRContractingLevel();


  // For determining the predecessor-successor pairs between which we want to
  // maintain shortest paths.
  void GetPredecessors(nodeId n, std::vector<ContractionArcHead> & predecessors);
  void GetSuccessors(nodeId n, std::vector<ContractionArcHead> & successors);

  LevelManager* GetLevelManager() {
    return &level_;
  }

  std::vector<ShortcutArc>* GetForwardSearchArcs() {
    return &forward_search_arcs_;
  }
  std::vector<ShortcutArc>* GetReverseSearchArcs()  {
    if (is_undirected_)
      return &forward_search_arcs_;
    else
      return &backward_search_arcs_;
  }

  // TODO: Not used at the moment. Fix it somehow:
  // - Take into account heavy r-contractions.
  template<class GType>
  CHSearchGraph<GType>* CreateSearchGraph();

  template <class Env, class G>
  void Visualize(const Env *env, G* g, int level = 0);

 private:
  ContractionType c_type_;
  int num_nodes_;
  bool is_undirected_;
  nodeId bypass_node_;

  // For regular/R-contractions:
  // - Maintain the core graph (which is sufficient for witness searches).
  // - Keep a list of search arcs (which is required for the final CH).

  // For heavy-R-contractions:
  // - Maintain the search graph (only up or same-level arcs for all the nodes).
  // - Keep a list of down arcs (in both forward/backward directions) for
  //   identifying predecessors/successors to determine the pairs between which
  //   we want to maintain shortest paths.

  // All contraction types (witness search).
  ContractionGraph forward_witness_search_graph_;
  ContractionGraph backward_witness_search_graph_;

  // Resulting search arcs.
  std::vector<ShortcutArc> forward_search_arcs_;
  std::vector<ShortcutArc> backward_search_arcs_;

  // For Heavy-R-Contractions, maintain graphs for descending arcs.
  ContractionGraph forward_descending_arc_graph_;
  ContractionGraph backward_descending_arc_graph_;

  std::vector<bool> is_contracted_;
  LevelManager level_;

  void RemoveIncidentArcs(nodeId n);
};

template <class Env, class G>
void ContractionCore::Visualize(const Env *env, G* g, level level) {
  env->SetColor(0,0,1);
  for (nodeId n = 0; n < num_nodes_; n++) {
    if (level_.GetLevel(n) >= level)
      g->DrawNode(env,n);
  }
}

template<class GType>
CHSearchGraph<GType>* ContractionCore::CreateSearchGraph() {

  typedef typename GType::SuccessorType ArcHeadType;
  typedef typename type_of_arc<ArcHeadType>::type ArcType;

  CHSearchGraph<GType>* search_graph = new CHSearchGraph<GType>();
  search_graph->SetIsUndirected(is_undirected_);

  std::vector<ArcType> converted_arcs;
  for (auto a:forward_search_arcs_) {
    ArcType b;
    convert_arc(a,b);
    converted_arcs.push_back(b);
  }

  search_graph->GetForwardGraph()->CreateGraph(num_nodes_,
                                               converted_arcs);
  if (!is_undirected_) {
    converted_arcs.clear();
    for (auto a:backward_search_arcs_) {
      ArcType b;
      convert_arc(a,b);
      converted_arcs.push_back(b);
    }
    search_graph->GetBackwardGraph()->CreateGraph(num_nodes_,
                                                 converted_arcs);
  }
  return search_graph;
}

#endif /* APPS_SUBGOALGRAPH_CONTRACTIONHIERARCHY_CONTRACTIONCORE_H_ */
