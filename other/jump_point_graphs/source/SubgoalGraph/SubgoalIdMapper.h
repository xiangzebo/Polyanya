/*
 * SubgoalIdMapper.h
 *
 *  Created on: Sep 24, 2016
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_SUBGOALGRAPH_SUBGOALIDMAPPER_H_
#define APPS_SUBGOALGRAPH_SUBGOALGRAPH_SUBGOALIDMAPPER_H_

#include <vector>
#include <cassert>
#include <iostream>

#include "../Graphs/GraphDefinitions.h"
#include "../Utils/FileReadWrite.h"
#include "SubgoalGraphDefinitions.h"

// Maps nodeId's to subgoalId's and vice versa.
// Can add new subgoals, but can't delete existing subgoals.
// After all subgoals are added, allows addition/removal of query subgoals.

class SubgoalIdMapper {
 public:
  SubgoalIdMapper(int num_nodes_in_original_graph) {
    Reset(num_nodes_in_original_graph);
  }
  ~SubgoalIdMapper() {
  }

  void Reset(int num_nodes_in_original_graph) {
    subgoal_ids_.resize(num_nodes_in_original_graph, kNonSubgoal);
    num_query_subgoals_ = 0;
  }

  bool IsSubgoal(nodeId n) const {
    return subgoal_ids_[n] != kNonSubgoal;
  }

  subgoalId ToSubgoalId(nodeId node_id) const {
    return subgoal_ids_[node_id];
  }
  nodeId ToNodeId(subgoalId s_id) const {
    return node_ids_[s_id];
  }

  unsigned int GetNumSubgoals() const {
    return node_ids_.size();
  }

  // Returns the id of the new subgoal.
  subgoalId AddSubgoal(nodeId n) {
    if (IsSubgoal(n))
      return ToSubgoalId(n);

    subgoalId s = node_ids_.size();
    node_ids_.push_back(n);
    subgoal_ids_[n] = s;
    return s;
  }


  subgoalId AddSubgoalWithMapping(nodeId n, nodeId mapped_node) {
    // FIXME: Not being used at the moment.
    // The idea is to map subgoals to locations, rather than location-direction
    // pairs. That is, location-direction pairs are mapped to subgoals,
    // but subgoals are mapped to only locations.
    // Edge connection would need to be fixed for this.
    return AddSubgoal(n);

    if (IsSubgoal(n))
      return ToSubgoalId(n);

    subgoalId s = node_ids_.size();
    node_ids_.push_back(mapped_node);
    subgoal_ids_[n] = s;
    return s;
  }

  // Merges the subgoal int
  subgoalId AddAlias(nodeId original, nodeId merged) {
    assert(IsSubgoal(original));
    assert(!IsSubgoal(merged));
    subgoal_ids_[merged] = subgoal_ids_[original];
    return subgoal_ids_[merged];
  }

  subgoalId AddQuerySubgoal(nodeId n) {
    if (IsSubgoal(n))
      return ToSubgoalId(n);

    num_query_subgoals_++;
    return AddSubgoal(n);
  }

  void ClearQuery() {
    while (num_query_subgoals_ > 0) {
      nodeId n = node_ids_.back();
      ;
      subgoal_ids_[n] = kNonSubgoal;
      node_ids_.pop_back();
      num_query_subgoals_--;
    }
  }

  void GetSubgoals(std::vector<nodeId> & subgoals) {
    subgoals.clear();
    for (subgoalId s = 0; s < node_ids_.size(); s++)
      subgoals.push_back(node_ids_[s]);
  }

  const std::vector<nodeId>* GetSubgoals() {
    return &node_ids_;
  }

  void Write(FileReadWrite & rw) {
    // FIXME: No need to write/read all? Especially num_query_subgoals?
    rw.WriteVector(node_ids_);
    rw.WriteVector(subgoal_ids_);
    rw.Write(num_query_subgoals_);
  }
  void Read(FileReadWrite & rw) {
    rw.ReadVector(node_ids_);
    rw.ReadVector(subgoal_ids_);
    rw.Read(num_query_subgoals_);
  }

  bool Write(std::string filename) {
    FileReadWrite rw;
    if (!rw.StartWrite(filename))
      return false;
    Write(rw);
    rw.CloseWrite();
    return true;
  }
  bool Read(std::string filename) {
    FileReadWrite rw;
    if (!rw.StartRead(filename))
      return false;
    Read(rw);
    rw.CloseRead();
    return true;
  }

  // A different mode of operation for construction by pruning.
  // Does not maintain node_ids_. As a hack, simply maintains the size
  // of the node_ids_.
    // Graph is given to identify valid nodes.
  template<class G>
  void MakeAllNodesSubgoals(G* g) {
    node_ids_.clear();
    assert(g->GetNumAllNodes() == subgoal_ids_.size());
    for (nodeId n = 0; n < g->GetNumAllNodes(); n++) {
      if (g->IsValidNode(n)) {
        subgoal_ids_[n] = n;
        node_ids_.push_back(n);
      }
      else {
        subgoal_ids_[n] = kNonSubgoal;
      }
    }
  }

  void PruneSubgoal(nodeId n) {
    assert(subgoal_ids_[n] != kNonSubgoal);
    subgoal_ids_[n] = kNonSubgoal;
    node_ids_.pop_back();
  }

  void UnPruneSubgoal(nodeId n) {
    assert(subgoal_ids_[n] == kNonSubgoal);
    subgoal_ids_[n] = n;
    node_ids_.push_back(n);
  }

  void FinalizePruning() {
    subgoalId s = 0;
    for (nodeId n = 0; n < subgoal_ids_.size(); n++) {
      if (subgoal_ids_[n] != kNonSubgoal) {
        subgoal_ids_[n] = s;
        node_ids_[s] = n;
        s++;
      }
    }
    assert (s == node_ids_.size());
  }

 private:
  std::vector<nodeId> node_ids_;	// Accessed with subgoalId
  std::vector<subgoalId> subgoal_ids_;	// Accessed with nodeId
  int num_query_subgoals_;
};

// TODO: Move somewhere else? Maybe to manager?
template <class SM, class SGM>
void GetEdges(SM* sm, SGM* sg, std::vector<nodeId> & from,
              std::vector<nodeId> & to) {
  from.clear();
  to.clear();

  for (subgoalId s = 0; s < sm->GetNumSubgoals(); s++) {
    std::vector<WeightedArcHead> neighbors;
    sg->GetSuccessors(s, neighbors);

    for (unsigned int i = 0; i < neighbors.size(); i++) {
      from.push_back(sm->ToNodeId(s));
      to.push_back(sm->ToNodeId(neighbors[i].target));
      //assert(SubgoalIdToNodeId(s) != kNonNode);
      //assert(SubgoalIdToNodeId(subgoals_[s].neighbors[i].id) >= 0);
    }
  }
}


// TODO: Move somewhere else
class IdentityMapper {
 public:
  IdentityMapper() {}
  ~IdentityMapper() {}

  nodeId ToNodeId(nodeId id) {
    return id;
  }

  nodeId ToSubgoalId(nodeId id) {
    return id;
  }
 private:
};

class SubgoalGraphTester {
 public:
  SubgoalGraphTester() {
    SubgoalIdMapper sg(5);

    sg.AddSubgoal(3);
    sg.AddSubgoal(2);
    sg.AddSubgoal(3);

    assert(sg.IsSubgoal(2));
    assert(sg.IsSubgoal(3));
    assert(!sg.IsSubgoal(4));

    std::cout << "SubgoalGraph test completed successfully!" << std::endl;
  }
  ~SubgoalGraphTester() {
  }

 private:
};


#endif /* APPS_SUBGOALGRAPH_SUBGOALGRAPH_SUBGOALIDMAPPER_H_ */
