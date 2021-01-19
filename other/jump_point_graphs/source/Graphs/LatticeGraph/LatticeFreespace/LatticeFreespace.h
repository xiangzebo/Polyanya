#ifndef MOTION_LATTICE_FREESPACE_HEURISTIC_H
#define MOTION_LATTICE_FREESPACE_HEURISTIC_H

#include "PreallocatedPriorityQueue.h"
#include <math.h>
#include <algorithm>

#include "../../Parameters/RParam.h"
#include "../../Utils/CPUTimer.h"
#include "../../Utils/FileReadWrite.h"
#include "Lattice.h"
#include "LatticeGraph.h"
#include "ThetaXYThetaLookupTable.h"
#include "XYThetaLookupTable.h"

// A note about doubles:
// An issue arises when trying to determine the number of symmetric paths.
// The multiple different ways of arriving a node through a shortest path can
// have slight differences in cost when working with doubles.
// Current method of minimizing issues:
// During dijkstra, perform comparisons without

// TODO: Careful. I've separated pred calculations from g-val calculations.
// Didn't check if it is bug-free.
// TODO: LatticeGraph no longer necessary? Move graph related functions to
// LatticeFreespaceHeuristic?

#define INCLUDE_EDGES_IN_R

class LatticeFreespaceHeuristic;

class LatticeFreespace {

 public:
  friend class LatticeFreespaceHeuristic;

  LatticeFreespace(LatticeGraph* g, RParam p);
  LatticeFreespace(LatticeGraph* g, RParam p, FileReadWrite & rw) {
    lattice_graph_ = g;
    SetLattice(g->GetLattice());
    Read(rw);
  }
  ~LatticeFreespace() {}

  bool UsingDistances() {
    return dist_.IsInitialized();
  }

  double GetComputationTime() {
    return computation_time_;
  }
  // Freespace distance between two nodes.
  Distance GetHeuristicDistance(xyThetaPos p1, xyThetaPos p2) {
    return dist_.GetValue(p1.o, p2.x - p1.x, p2.y - p1.y, p2.o);
  }
  // Number of predecessors of pos2 that lie on a shortest freespace path from
  // pos1 to pos2.
  PrimCount GetNumPred(xyThetaPos p1, xyThetaPos p2) {
    return num_pred_.GetValue(p1.o, p2.x - p1.x, p2.y - p1.y, p2.o);
  }
  // Number of successors of pos1 that lie on a shortest freespace path from
  // pos1 to pos2.
  PrimCount GetNumSucc(xyThetaPos p1, xyThetaPos p2) {
    return num_succ_.GetValue(p1.o, p2.x - p1.x, p2.y - p1.y, p2.o);
  }
  // Parent of pos2 on the canonical freespace path from pos1 to pos2.
  PrimId GetCanonicalParentId(xyThetaPos p1, xyThetaPos p2) {
    return canonical_parent_.GetValue(p1.o, p2.x - p1.x, p2.y - p1.y, p2.o);
  }

  // Next move of pos2 on the canonical freespace path from pos2 to pos1.
  PrimId GetCanonicalNextMoveId(xyThetaPos p1, xyThetaPos p2) {
    return canonical_next_move_.GetValue(p1.o, p2.x - p1.x, p2.y - p1.y, p2.o);
   }

  // Primitive flags where each 1 corresponds to a canonical successor of
  // pos2 where the canonical ordering starts at 1.
  ExecutablePrimitiveFlags GetCanonicalSuccessorFlags(xyThetaPos p1,
                                                      xyThetaPos p2) {
    return canonical_succ_.GetValue(p1.o, p2.x - p1.x, p2.y - p1.y,
                                            p2.o);
  }
  // Primitive flags where each 1 corresponds to a reverse canonical successor
  // of pos2 where the reverse canonical ordering starts at 1.
  ExecutablePrimitiveFlags GetReverseCanonicalSuccessorFlags(xyThetaPos p1,
                                                             xyThetaPos p2) {
    return canonical_pred_.GetValue(p1.o, p2.x - p1.x, p2.y - p1.y,
                                            p2.o);
  }

  ExecutablePrimitiveFlags GetFreespaceParentFlags(xyThetaPos p1,
                                                   xyThetaPos p2) {
    return freespace_parent_.GetValue(p1.o, p2.x - p1.x, p2.y - p1.y, p2.o);
  }

  double EstimateStorageMB(bool connect, bool refine,
                           bool reduce_num_tables = false,
                           bool reduce_table_size = false);

  void Write(FileReadWrite & rw);
  void Read(FileReadWrite & rw);

 private:
  LatticeGraph* lattice_graph_;
  Lattice* lattice_;
  RParam p_;

  Distance bound_, out_of_bound_;

  int num_angles_;
  int width_;
  int height_;
  int max_prim_reach_;

  double computation_time_;


  // Read lattice dimensions and calculate table width / linearization etc.
  void SetLattice(Lattice* lattice);

  // [a1][x][y][a2] = value from (x_stretch_-1,height_-1,a1) -> (x,y,a2).
  ThetaXYThetaLookupTable<Distance> dist_;
  ThetaXYThetaLookupTable<PrimCount> num_pred_;
  ThetaXYThetaLookupTable<PrimCount> num_succ_;
  ThetaXYThetaLookupTable<ExecutablePrimitiveFlags> canonical_succ_;
  ThetaXYThetaLookupTable<ExecutablePrimitiveFlags> canonical_pred_;
  ThetaXYThetaLookupTable<PrimId> canonical_parent_;

  void CalculateHeuristic(Distance bound);
  void CalculateNumPredAndNumSucc(Distance bound);
  void CalculateCanonicalParents(Distance bound);
  void CalculateCanonicalSuccessors(Distance bound);
  void CalculateCanonicalPredecessors(Distance bound);

  // Somewhat-hacky additions
  // F-Connect with flags: uses canonical_succ_ and reverse_canonical_succ_
  // instaed of the following two tables.
  //ThetaXYThetaLookupTable<ExecutablePrimitiveFlags> freespace_succ_;
  //ThetaXYThetaLookupTable<ExecutablePrimitiveFlags> freespace_pred_;
  void CalculateFreespaceSuccessorsAndPredecessors(Distance bound);

  // F-Refine with flags.
  // Cache efficiency: goal_directed_freespace_pred_[sa][x][y][ea] returns
  // the predecessors of (x,y,ea) wrt (0,0,sa)
  // This way, during refinement, we always look at the table for orientation sa.
   ThetaXYThetaLookupTable<ExecutablePrimitiveFlags> freespace_parent_;
  void CalculateFreespaceParents(Distance bound);

  // CF-Refine with next-moves rather than parents.
  // Same idea, but rather than following parents and reversing the path,
  // we simply follow next moves to generate the path in one go.
  // For cache efficiency, canonical_next_move_[ea][x][y][sa]
  // returns the canonical next move from (sa,0,0) to (ea,x,y).
  // This way, during refinement, we always look at the table for orientation ea.
  ThetaXYThetaLookupTable<PrimId> canonical_next_move_;
  void CalculateCanonicalNextMoves(Distance bound);


  template<class T, class V>
  void GenerateTable(T & t, V def_val, Distance bound,
                     int & x_stretch, int & y_stretch) {

/*
    x_stretch = max(
        max_prim_reach_,
        min(lattice_->GetGrid()->GetPaddedWidth(), (int) round(bound_)));
    y_stretch = max(
        max_prim_reach_,
        min(lattice_->GetGrid()->GetPaddedHeight(), (int) round(bound_)));
/*/
    x_stretch = max(max_prim_reach_, (int) round(bound_));
    y_stretch = max(max_prim_reach_, (int) round(bound_));
//*/
    t.InitializeTableByExtendingFromOrigin(x_stretch, y_stretch, num_angles_,
                                           def_val);
  }
};

#endif
