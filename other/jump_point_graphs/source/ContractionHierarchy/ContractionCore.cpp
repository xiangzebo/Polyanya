/*
 * ContractionCore.cpp
 *
 *  Created on: Oct 29, 2017
 *      Author: idm-lab
 */
#include <CHSearchGraph.h>
#include "ContractionCore.h"
#include <algorithm>

// Heavy-Contract: Requires only small modifications with level initialization.
void ContractionCore::BuildContractionCore(int num_nodes,
                                           std::vector<WeightedArc> & arcs,
                                           bool is_undirected) {
  bypass_node_ = kNonNode;
  is_undirected_ = is_undirected;
  num_nodes_ = num_nodes;

  forward_witness_search_graph_.SetNumNodes(num_nodes);
  for (auto arc: arcs)
    forward_witness_search_graph_.AddArc(
        arc.source, ContractionArcHead(arc.target, arc.weight, kNonNode, 1));

  if (!is_undirected_) {
    backward_witness_search_graph_.SetNumNodes(num_nodes);
    for (auto arc: arcs)
      backward_witness_search_graph_.AddArc(
          arc.target, ContractionArcHead(arc.source, arc.weight, kNonNode, 1));
  }

  // Initialize the was_contracted array (assume any node with no in/out arcs)
  // are already contracted. This assumption seems necessary for grid/lattice
  // graphs that use linearized coordinates as node ids, where blocked cells
  // also get ids but no neighbors.
  is_contracted_.resize(num_nodes_, false);

  /*
  for (nodeId n = 0; n < num_nodes_; n++)
    if (is_undirected_)
      is_contracted_[n] = forward_witness_search_graph_.OutDegree(n) == 0;
    else
      is_contracted_[n] = (forward_witness_search_graph_.OutDegree(n)
          + backward_witness_search_graph_.OutDegree(n)) == 0;
  */

  level_.GenerateInitialLevels(num_nodes_, kBaseLevel);

  for (nodeId n = 0; n < num_nodes_; n++)
    if (is_contracted_[n])
      level_.SetToInvalidLevel(n);

  if (c_type_ == kHeavyRContraction) {
    forward_descending_arc_graph_.SetNumNodes(num_nodes);
    backward_descending_arc_graph_.SetNumNodes(num_nodes);
  }
}

void ContractionCore::GetPredecessors(
    nodeId n, std::vector<ContractionArcHead> & predecessors) {
  GetBackwardWitnessSearchGraph()->GetSuccessors(n, predecessors);
  if (c_type_ == kHeavyRContraction)
    for (auto a: GetBackwardDescendingArcGraph()->GetSuccessors(n))
      predecessors.push_back(a);
}
void ContractionCore::GetSuccessors(
    nodeId n, std::vector<ContractionArcHead> & successors) {
  GetForwardWitnessSearchGraph()->GetSuccessors(n, successors);
  if (c_type_ == kHeavyRContraction)
    for (auto a: GetForwardDescendingArcGraph()->GetSuccessors(n))
      successors.push_back(a);
}

// Heavy-Contract: Can be used as is.. seems to be?
void ContractionCore::HideNode(nodeId n) {
  assert(bypass_node_ == kNonNode);
  bypass_node_ = n;
  if (is_undirected_) {
    std::vector<ContractionArcHead> predecessors;
    GetPredecessors(n, predecessors);
    for (auto p: predecessors)
      forward_witness_search_graph_.HideArc(p.target, n);
  }
  else {
    std::vector<ContractionArcHead> predecessors, successors;
    GetPredecessors(n, predecessors);
    GetSuccessors(n, successors);
    for (auto p: predecessors)
      forward_witness_search_graph_.HideArc(p.target, n);
    for (auto s: successors)
      backward_witness_search_graph_.HideArc(s.target, n);
  }
}

// Heavy-Contract: Can be used as is.
void ContractionCore::RestoreHiddenNode() {
  assert(bypass_node_ != kNonNode);
  bypass_node_ = kNonNode;
  if (is_undirected_) {
    forward_witness_search_graph_.RestoreHiddenArcs();
  }
  else {
    forward_witness_search_graph_.RestoreHiddenArcs();
    backward_witness_search_graph_.RestoreHiddenArcs();
  }
}

// TODO: Statistics for directed graphs.
// For undirected graphs, the shortcuts are only given in a single direction.
void ContractionCore::ContractNode(nodeId n,
                                   std::vector<ContractionArc> & shortcuts,
                                   std::vector<nodeId> & neighbors,
                                   int & num_original_arcs_removed,
                                   int & num_shortcuts_added,
                                   int & num_shortcuts_removed) {

  assert(bypass_node_ == kNonNode
         && "Cannot contract a node when a bypass node is set");
  neighbors.clear();

  // Remove the arcs from the contraction graph and add arcs (where n is the
  // source) to the output graph.
  // Keep track of all the nodes whose neighborhood has changed (so their
  // priorities can be updated).
  if (is_undirected_) {
    std::vector<ContractionArcHead> successors;
    GetSuccessors(n, successors);
    for (auto s: successors) {
      if (c_type_ != kHeavyRContraction) {
        forward_witness_search_graph_.RemoveArc(n, s.target);
        forward_witness_search_graph_.RemoveArc(s.target, n);
        forward_search_arcs_.push_back(
          ShortcutArc(n, s.target, s.weight, s.mid));

        neighbors.push_back(s.target);

        // 1 removed, 1 moved to search arcs.
        if (s.mid == kNonNode)
          num_original_arcs_removed ++;
        else
          num_shortcuts_removed ++;
      }
      else {
        if (level_.IsHighestLevel(s.target)) { // n->s.target: same-level arc.
          // n->s.target is now an ascending arc: do nothing
          // s.target->n is now a descending arc: move to descending arc graph.
          forward_witness_search_graph_.RemoveArc(s.target, n);
          forward_descending_arc_graph_.AddArc(n, s, true); // reverse

          neighbors.push_back(s.target); // Only highest-level nodes are affected.

          if (s.mid == kNonNode)
            num_original_arcs_removed ++;
          else
            num_shortcuts_removed ++;
        }
        else { // n->s.target: descending arc
          // n->s.target is now a same-level arc: move to witness search graph.
          // s.target->n is now a same-level arc: do nothing
          forward_witness_search_graph_.AddArc(n, s);
          forward_descending_arc_graph_.RemoveArc(n, s.target);

          // Descending arc becomes same-level arc. Since it is previously
          // counted as removed, we fix it by decrementing num-removed.
          if (s.mid == kNonNode)
            num_original_arcs_removed --;
          else
            num_shortcuts_removed --;
        }
      }
    }
  }
  else {
    // TODO: Heavy-R-Contraction?
    assert(c_type_ != kHeavyRContraction
           || "Heavy-R-Contraction not yet implemented for directed graphs");
    std::vector<ContractionArcHead> successors, predecessors;
    GetSuccessors(n, successors);
    GetPredecessors(n, predecessors);
    for (auto s: successors) {
      forward_witness_search_graph_.RemoveArc(n, s.target);
      backward_witness_search_graph_.RemoveArc(s.target, n);
      forward_search_arcs_.push_back(
          ShortcutArc(n, s.target, s.weight, s.mid));
      neighbors.push_back(s.target);
    }
    for (auto p: predecessors) {
      backward_witness_search_graph_.RemoveArc(n, p.target);
      forward_witness_search_graph_.RemoveArc(p.target, n);
      backward_search_arcs_.push_back(
          ShortcutArc(n, p.target, p.weight, p.mid));
      neighbors.push_back(p.target);
    }

    // Eliminate duplicates in the neighbors.
    std::sort(neighbors.begin(), neighbors.end());
    neighbors.erase(std::unique(neighbors.begin(), neighbors.end()),
                    neighbors.end());
  }

  // Add the shortcut arcs to the graph being contracted.
  // TODO: Check if this is correct for the directed scenario.
  for (auto a: shortcuts) {
    bool reverse = true;
    if (c_type_ != kHeavyRContraction) {
      GetForwardWitnessSearchGraph()->AddArc(a.source, get_head(a), !reverse);
      GetBackwardWitnessSearchGraph()->AddArc(a.source, get_head(a), reverse);
      num_shortcuts_added += 2;
    }
    else {
      if (!level_.IsHighestLevel(a.source) && level_.IsHighestLevel(a.target)) {
        GetForwardWitnessSearchGraph()->AddArc(a.source, get_head(a), !reverse);
        GetBackwardDescendingArcGraph()->AddArc(a.source, get_head(a), reverse);
        num_shortcuts_added ++;
      }
      else if (level_.IsHighestLevel(a.source) && !level_.IsHighestLevel(a.target)) {
        GetForwardDescendingArcGraph()->AddArc(a.source, get_head(a), !reverse);
        GetBackwardWitnessSearchGraph()->AddArc(a.source, get_head(a), reverse);
        num_shortcuts_added ++;
      }
      else { // Same level (either highest or lower level)
        GetForwardWitnessSearchGraph()->AddArc(a.source, get_head(a), !reverse);
        GetBackwardWitnessSearchGraph()->AddArc(a.source, get_head(a), reverse);
        num_shortcuts_added += 2;
      }
    }
  }

  // For non-heavy contractions, update levels of neighbors.
  if (c_type_ != kHeavyRContraction)
    for (auto v: neighbors)
      level_.IncreaseLevel(v, level_.GetLevel(n) + 1);
  else
    level_.DecrementLevel(n);

  // Node n is contracted.
  is_contracted_[n] = true;
}

void ContractionCore::RemoveIncidentArcs(nodeId n) {
  if (is_undirected_) {
    std::vector<ContractionArcHead> successors;
    forward_witness_search_graph_.GetSuccessors(n, successors);
    for (auto s: successors) {
      forward_witness_search_graph_.RemoveArc(n, s.target);
      forward_search_arcs_.push_back(
          ShortcutArc(n, s.target, s.weight, s.mid));
    }
  }
  else {
    std::vector<ContractionArcHead> successors, predecessors;
    GetForwardWitnessSearchGraph()->GetSuccessors(n, successors);
    GetBackwardWitnessSearchGraph()->GetSuccessors(n, predecessors);
    for (auto s: successors) {
      forward_witness_search_graph_.RemoveArc(n, s.target);
      forward_search_arcs_.push_back(
          ShortcutArc(n, s.target, s.weight, s.mid));
    }
    for (auto p: predecessors) {
      backward_witness_search_graph_.RemoveArc(n, p.target);
      backward_search_arcs_.push_back(
          ShortcutArc(n, p.target, p.weight, p.mid));
    }
  }
}

void ContractionCore::StartHeavyRContractingLevel() {
  assert(forward_descending_arc_graph_.GetNumArcs() == 0);
  assert(backward_descending_arc_graph_.GetNumArcs() == 0);
  level_.IncrementLevelsOfAllHighestLevelNodes();
}
void ContractionCore::FinishHeavyRContractingLevel() {
  // TODO: Directed graphs
  assert(is_undirected_ || "HeavyRContract not implemented for directed graphs");
  std::vector<ContractionArcHead> successors;
  for(nodeId n = 0; n < num_nodes_; n++) {
    if (!level_.IsHighestLevel(n)) {
      GetForwardWitnessSearchGraph()->GetSuccessors(n, successors);
      for (auto a: successors) {
        GetForwardWitnessSearchGraph()->RemoveArc(n, a.target);
        GetForwardSearchArcs()->push_back(
            ShortcutArc(n, a.target, a.weight, a.mid));
      }
    }
  }
  GetForwardDescendingArcGraph()->RemoveAllArcs();
}

void ContractionCore::FinalizeContraction() {
  // Remove all uncontracted nodes from the graph and add their incident arcs
  // to search arc lists.
//  if (c_type_ != kHeavyRContraction)
//    level_.IncrementMaxLevel();

  for (nodeId n = 0; n < num_nodes_; n++) {
    if (!is_contracted_[n]) {
      RemoveIncidentArcs(n);
      is_contracted_[n] = true;
      level_.SetToMaxLevel(n);
    }
  }
  assert(forward_witness_search_graph_.GetNumArcs() == 0);
  assert(backward_witness_search_graph_.GetNumArcs() == 0);
}


