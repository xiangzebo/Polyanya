/*
 * XYThetaLatticeFreespaceHeuristic.h
 *
 *  Created on: Oct 15, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_GRAPHS_XYTHETALATTICE_LATTICEFREESPACEHEURISTIC_H_
#define APPS_SUBGOALGRAPH_GRAPHS_XYTHETALATTICE_LATTICEFREESPACEHEURISTIC_H_

#include "LatticeFreespace.h"
#include "LatticeGraph.h"

class LatticeFreespaceHeuristic {
 public:
  LatticeFreespaceHeuristic(LatticeGraph* g, LatticeFreespace* f)
    :g_(g), f_(f) {
    start_pos_ = xyThetaPos(-1, -1, -1);
  }

  bool UsingDistances() const {
    return f_->UsingDistances();
  }

  Distance GetHeuristicDistance(nodeId n1, nodeId n2) const {
    return f_->GetHeuristicDistance(g_->ToState(n1), g_->ToState(n2));
  }
  PrimCount GetNumPred(nodeId n1, nodeId n2) const {
    return f_->GetNumPred(g_->ToState(n1), g_->ToState(n2));
  }
  PrimCount GetNumSucc(nodeId n1, nodeId n2) const {
    return f_->GetNumSucc(g_->ToState(n1), g_->ToState(n2));
  }
  PrimId GetCanonicalParentId(nodeId n1, nodeId n2) const {
    return f_->GetCanonicalParentId(g_->ToState(n1), g_->ToState(n2));
  }
  ExecutablePrimitiveFlags GetCanonicalSuccessorFlags(nodeId n1,
                                                      nodeId n2) const {
    return f_->GetCanonicalSuccessorFlags(g_->ToState(n1), g_->ToState(n2));
  }
  ExecutablePrimitiveFlags GetReverseCanonicalSuccessorFlags(nodeId n1,
                                                             nodeId n2) const {
    return f_->GetReverseCanonicalSuccessorFlags(g_->ToState(n1),
                                                 g_->ToState(n2));
  }

  nodeId GetCanonicalParent(nodeId n1, nodeId n2) {
    // TODO: Make it more efficient? Double if check at the moment (one during
    // look-up in LatticeFreespace, one here.
    xyThetaPos p1 = g_->ToState(n1);
    xyThetaPos p2 = g_->ToState(n2);
    PrimId parent_prim_id = f_->GetCanonicalParentId(p1, p2);

    if (parent_prim_id == kInvalidPrimId)
      return kNonNode;
    else
      return
          g_->GetLattice()->GetAllReversePrimitives(p2.o)->at(
              parent_prim_id).GetResultingNodeId(n2);
  }
  nodeId GetCanonicalParentIfExists(nodeId n1, nodeId n2) {
    xyThetaPos p1 = g_->ToState(n1);
    xyThetaPos p2 = g_->ToState(n2);
    PrimId parent_prim_id = f_->GetCanonicalParentId(p1, p2);

    if (parent_prim_id == kInvalidPrimId)
      return kNonNode;
    else if (g_->GetReverseExecutablePrimitiveFlags(n2) & 1 << parent_prim_id)
      return
          g_->GetLattice()->GetAllReversePrimitives(p2.o)->at(
              parent_prim_id).GetResultingNodeId(n2);
    else
      return kNonNode;

  }

  // Not using next moves at the moment.
  nodeId GetCanonicalNextMove(nodeId n1, nodeId n2) {
    xyThetaPos p1 = g_->ToState(n1);
    xyThetaPos p2 = g_->ToState(n2);
    PrimId next_move_id = f_->GetCanonicalNextMoveId(p1, p2);

    if (next_move_id == kInvalidPrimId)
      return kNonNode;
    return g_->GetLattice()->GetAllForwardPrimitives(p2.o)->at(next_move_id)
        .GetResultingNodeId(n2);
  }
  void AddCanonicalNextMove(nodeId to, std::vector<nodeId> & path) {
    xyThetaPos p1 = g_->ToState(to);
    xyThetaPos p2 = g_->ToState(path.back());
    PrimId next_move_id = f_->canonical_next_move_[p1.o].GetValueDirectly(
        p2.x - p1.x, p2.y - p1.y, p2.o);
    path.push_back(
        g_->GetLattice()->GetAllForwardPrimitives(p2.o)->at(next_move_id)
            .GetResultingNodeId(path.back()));
  }
  void FillPath(nodeId to, std::vector<nodeId> & path) {
    xyThetaPos target = g_->ToState(to);
    xyThetaPos curr = g_->ToState(path.back());
    int x_offset = curr.x - target.x;
    int y_offset = curr.y - target.y;
//    xyThetaPos curr_offset(, , curr.o);

    PrimId next_move_id = f_->canonical_next_move_[target.o].GetValueDirectly(
        curr.x - target.x, curr.y - target.y, curr.o);
    path.push_back(
        g_->ToNodeId(
            g_->GetLattice()->GetAllForwardPrimitives(curr.o)->at(next_move_id)
                .GetResultingPos(curr)));
  }


  void GetCanonicalSuccessors(nodeId n1, nodeId n2,
                              std::vector<WeightedArcHead> & neighbors) const {
    g_->GetSuccessors(
        n2, neighbors,
        f_->GetCanonicalSuccessorFlags(g_->ToState(n1), g_->ToState(n2)));
  }

  void GetReverseCanonicalSuccessors(
      nodeId n1, nodeId n2, std::vector<WeightedArcHead> & neighbors) const {
    g_->GetPredecessors(
        n2, neighbors,
        f_->GetReverseCanonicalSuccessorFlags(g_->ToState(n1), g_->ToState(n2)));
  }

  void AddCanonicalSuccessors(nodeId n1, nodeId n2,
                              std::vector<nodeId> & list) const {
    xyThetaPos p1 = g_->ToState(n1);
    xyThetaPos p2 = g_->ToState(n2);
    ExecutablePrimitiveFlags valid_successors = g_
        ->GetForwardExecutablePrimitiveFlags(n2);
    ExecutablePrimitiveFlags canonical_successors = f_
        ->GetCanonicalSuccessorFlags(p1, p2);

    canonical_successors = canonical_successors & valid_successors;

    auto prims = g_->GetLattice()->GetAllForwardPrimitives(p2.o);
    ExecutablePrimitiveFlags mask = 1;
    for (unsigned int i = 0; i < prims->size(); i++) {
      if (canonical_successors & mask)
        list.push_back(prims->at(i).GetResultingNodeId(n2));
      mask = mask << 1;
    }
  }

  void AddReverseCanonicalSuccessors(
      nodeId n1, nodeId n2, std::vector<nodeId> & list) const {
    xyThetaPos p1 = g_->ToState(n1);
    xyThetaPos p2 = g_->ToState(n2);
    ExecutablePrimitiveFlags valid_predecessors = g_
        ->GetReverseExecutablePrimitiveFlags(n2);
    ExecutablePrimitiveFlags reverse_canonical_successors = f_
        ->GetReverseCanonicalSuccessorFlags(p1, p2);
    reverse_canonical_successors = reverse_canonical_successors
        & valid_predecessors;

    auto prims = g_->GetLattice()->GetAllReversePrimitives(p2.o);
    ExecutablePrimitiveFlags mask = 1;
    for (unsigned int i = 0; i < prims->size(); i++) {
      if (reverse_canonical_successors & mask)
        list.push_back(prims->at(i).GetResultingNodeId(n2));
      mask = mask << 1;
    }
  }


  int AddCanonicalSuccessorsAndDistances(
      nodeId n1, nodeId n2, Distance g_val, std::vector<nodeId> & list,
      std::vector<Distance> & distance_list) const {
    xyThetaPos p1 = g_->ToState(n1);
    xyThetaPos p2 = g_->ToState(n2);
    ExecutablePrimitiveFlags valid_successors = g_
        ->GetForwardExecutablePrimitiveFlags(n2);
    ExecutablePrimitiveFlags canonical_successors = f_
        ->GetCanonicalSuccessorFlags(p1, p2);

    canonical_successors = canonical_successors & valid_successors;

    auto prims = g_->GetLattice()->GetAllForwardPrimitives(p2.o);
    ExecutablePrimitiveFlags mask = 1;
    int num_succ = 0;
    for (unsigned int i = 0; i < prims->size(); i++) {
      if (canonical_successors & mask) {
        num_succ++;
        list.push_back(prims->at(i).GetResultingNodeId(n2));
        distance_list.push_back(g_val + prims->at(i).GetCost());
      }
      mask = mask << 1;
    }
    return num_succ;
  }

  int AddReverseCanonicalSuccessorsAndDistances(
      nodeId n1, nodeId n2, Distance g_val, std::vector<nodeId> & list,
      std::vector<Distance> & distance_list) const {
    xyThetaPos p1 = g_->ToState(n1);
    xyThetaPos p2 = g_->ToState(n2);
    ExecutablePrimitiveFlags valid_predecessors = g_
        ->GetReverseExecutablePrimitiveFlags(n2);
    ExecutablePrimitiveFlags reverse_canonical_successors = f_
        ->GetReverseCanonicalSuccessorFlags(p1, p2);
    reverse_canonical_successors = reverse_canonical_successors
        & valid_predecessors;

    auto prims = g_->GetLattice()->GetAllReversePrimitives(p2.o);
    int num_succ = 0;
    ExecutablePrimitiveFlags mask = 1;
    for (unsigned int i = 0; i < prims->size(); i++) {
      if (reverse_canonical_successors & mask) {
        num_succ++;
        list.push_back(prims->at(i).GetResultingNodeId(n2));
        distance_list.push_back(g_val + prims->at(i).GetCost());
      }
      mask = mask << 1;
    }
    return num_succ;
  }

  template<class D>
  int AddFreespaceSuccessorsAndDistances(
      nodeId n1, nodeId n2, Distance g_val, std::vector<nodeId> & list,
      std::vector<Distance> & distance_list, D IsDuplicate) const {
    xyThetaPos p1 = g_->ToState(n1);
    xyThetaPos p2 = g_->ToState(n2);
    ExecutablePrimitiveFlags valid_successors = g_
        ->GetForwardExecutablePrimitiveFlags(n2);
    ExecutablePrimitiveFlags canonical_successors = f_
        ->GetCanonicalSuccessorFlags(p1, p2);

    canonical_successors = canonical_successors & valid_successors;

    auto prims = g_->GetLattice()->GetAllForwardPrimitives(p2.o);
    ExecutablePrimitiveFlags mask = 1;
    int num_succ = 0;
    for (unsigned int i = 0; i < prims->size(); i++) {
      if (canonical_successors & mask) {
        nodeId succ = prims->at(i).GetResultingNodeId(n2);
        num_succ++;
        if (!IsDuplicate(succ)) {
          list.push_back(succ);
          distance_list.push_back(g_val + prims->at(i).GetCost());
        }
      }
      mask = mask << 1;
    }
    return num_succ;
  }

  template<class D>
  int AddReverseFreespaceSuccessorsAndDistances(
      nodeId n1, nodeId n2, Distance g_val, std::vector<nodeId> & list,
      std::vector<Distance> & distance_list, D IsDuplicate) const {
    xyThetaPos p1 = g_->ToState(n1);
    xyThetaPos p2 = g_->ToState(n2);
    ExecutablePrimitiveFlags valid_predecessors = g_
        ->GetReverseExecutablePrimitiveFlags(n2);
    ExecutablePrimitiveFlags reverse_canonical_successors = f_
        ->GetReverseCanonicalSuccessorFlags(p1, p2);
    reverse_canonical_successors = reverse_canonical_successors
        & valid_predecessors;

    auto prims = g_->GetLattice()->GetAllReversePrimitives(p2.o);
    int num_succ = 0;
    ExecutablePrimitiveFlags mask = 1;
    for (unsigned int i = 0; i < prims->size(); i++) {
      if (reverse_canonical_successors & mask) {
        nodeId succ = prims->at(i).GetResultingNodeId(n2);
        num_succ++;
        if (!IsDuplicate(succ)) {
          list.push_back(succ);
          distance_list.push_back(g_val + prims->at(i).GetCost());
        }
      }
      mask = mask << 1;
    }
    return num_succ;
  }


  template<class D>
  int AddFreespaceParents(
      nodeId n1, nodeId n2, std::vector<nodeId> & list,
                          std::vector<nodeId> & r_parent_map,
                          D IsDuplicate) const {
    xyThetaPos p1 = g_->ToState(n1);
    xyThetaPos p2 = g_->ToState(n2);
    ExecutablePrimitiveFlags valid_predecessors = g_
        ->GetReverseExecutablePrimitiveFlags(n2);
    ExecutablePrimitiveFlags freespace_parents = f_->GetFreespaceParentFlags(
        p1, p2);
    freespace_parents = freespace_parents & valid_predecessors;

    auto prims = g_->GetLattice()->GetAllReversePrimitives(p2.o);
    int num_succ = 0;
    ExecutablePrimitiveFlags mask = 1;
    for (unsigned int i = 0; i < prims->size(); i++) {
      if (freespace_parents & mask) {
        nodeId succ = prims->at(i).GetResultingNodeId(n2);
        num_succ++;
        if (!IsDuplicate(succ)) {
          list.push_back(succ);
          r_parent_map[succ] = n2;
        }
      }
      mask = mask << 1;
    }
    return num_succ;
  }

  void SetStartNode(nodeId n) {
    start_pos_ = g_->ToState(n);
  }

 private:
   LatticeGraph* g_;
   LatticeFreespace* f_;
   xyThetaPos start_pos_;
};


#endif /* APPS_SUBGOALGRAPH_GRAPHS_XYTHETALATTICE_LATTICEFREESPACEHEURISTIC_H_ */
