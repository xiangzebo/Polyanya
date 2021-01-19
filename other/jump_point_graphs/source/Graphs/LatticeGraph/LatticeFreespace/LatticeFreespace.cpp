/*
 * LatticeFreespace.cpp
 *
 *  Created on: Mar 3, 2018
 *      Author: idm-lab
 */

#include <Linearize3D.h>
#include "LatticeFreespace.h"

LatticeFreespace::LatticeFreespace(LatticeGraph* g, RParam p) {
  p_ = p;
  bound_ = p.bound;
  //out_of_bound_ = p.bound + 2 * kEpsDistance;
  out_of_bound_ = kMaxDistance;

  lattice_graph_ = g;
  SetLattice(g->GetLattice());

  CPUTimer t;
  t.StartTimer();
  if (p.type != kBoundedDistanceReachability) {
    CalculateHeuristic(bound_);
  }
  if (p.type == kSafeFreespaceReachability) {
    CalculateNumPredAndNumSucc(bound_);
  }
  if (p.type == kCanonicalFreespaceReachability) {
    CalculateCanonicalParents(bound_);
    CalculateCanonicalSuccessors(bound_);
    CalculateCanonicalPredecessors(bound_);
    //CalculateCanonicalNextMoves(bound_);
    dist_.DeleteTable();
  }
  if (p.type == kFreespaceReachability && p.conn == kRConnectSucc) {
    CalculateFreespaceSuccessorsAndPredecessors(bound_);
    CalculateFreespaceParents(bound_);
  }

  //*
  dist_.ShrinkToFit();
  num_pred_.ShrinkToFit();
  num_succ_.ShrinkToFit();
  canonical_succ_.ShrinkToFit();
  canonical_pred_.ShrinkToFit();
  canonical_parent_.ShrinkToFit();
  freespace_parent_.ShrinkToFit();
  //canonical_next_move_.ShrinkToFit();
  //*/
  computation_time_ = t.EndTimer();

  std::cout << "Memory required for freespace information with bound "
            << bound_ << " : " << EstimateStorageMB(true, true) << " Mb"
            << std::endl;
}
void LatticeFreespace::SetLattice(Lattice* lattice) {
  lattice_ = lattice;
  num_angles_ = lattice_->GetNumAngles();
  width_ = lattice_->GetGrid()->GetOriginalWidth();
  height_ = lattice_->GetGrid()->GetOriginalHeight();
  max_prim_reach_ = lattice_->GetMaxPrimitiveReach();

  /*
  // TODO: BUG on arena.map which is too small?
  // TODO: round->ceil?
  // TODO: Assumes bound_ is enough to contain all the bounded entries
  width_ = std::min(lattice_->GetGrid()->GetOriginalWidth(),
                    (int) round(bound_));
  height_ = std::min(lattice_->GetGrid()->GetOriginalHeight(),
                     (int) round(bound_));
  /*/
//  width_ = (int) round(bound_);
//  height_= (int) round(bound_);

  //*/
}

double LatticeFreespace::EstimateStorageMB(bool connect, bool refine,
                                           bool reduce_num_tables,
                                           bool reduce_table_size) {
  long long memory = 0;
  int distance_bits = 32;

  // Calculate the maximum number of incoming / outgoing primitives for
  // an orientation, over all orientations.
  int prim_count = 0;
  for (int a = 0; a < num_angles_; a++) {
    prim_count = max(prim_count, lattice_->GetAllForwardPrimitives(a)->size());
    prim_count = max(prim_count, lattice_->GetAllReversePrimitives(a)->size());
  }

  // Canonical predecssors / successors are stored as bitfields
  int flag_bits = 1;
  while (flag_bits < prim_count)
    flag_bits *= 2;

  // Canonical parents are stored as an integer that points to the 'id' of the
  // primitive applicable at that orientation.
  int count_or_id_bits = 1;
  int ids = 2;
  while (ids < prim_count) {
    count_or_id_bits *= 2;
    ids = ids*ids;
  }
  /*
  int count_or_id_bits = 4;
  int flag_bits = 16;
  if (prim_count > 16) {
    count_or_id_bits = 8;
    flag_bits = 32;
  }
  if (prim_count > 32)
    flag_bits = 64;
  if (prim_count > 64)
    flag_bits = 128;
  */

  // Exclude for freespace reachability implementation with primitive flags.
  if (p_.type != kFreespaceReachability || p_.conn != kRConnectSucc)
    memory += dist_.GetTableSize(reduce_num_tables, reduce_table_size)
        * distance_bits;

  if (connect) {
    memory += num_pred_.GetTableSize(reduce_num_tables, reduce_table_size)
        * count_or_id_bits;
    memory += num_succ_.GetTableSize(reduce_num_tables, reduce_table_size)
        * count_or_id_bits;
    memory += canonical_succ_.GetTableSize(reduce_num_tables, reduce_table_size)
        * flag_bits;
    memory += canonical_pred_.GetTableSize(reduce_num_tables,
                                                   reduce_table_size)
        * flag_bits;
  }
  if (refine) {
    memory += canonical_parent_.GetTableSize(reduce_num_tables,
                                             reduce_table_size)
        * count_or_id_bits;
    memory += freespace_parent_.GetTableSize(reduce_num_tables,
                                             reduce_table_size)
        * flag_bits;
  }
  return memory / (double)(8 * 1024 * 1024);
}

void LatticeFreespace::Write(FileReadWrite & rw) {
  rw.Write(bound_);
  rw.Write(out_of_bound_);
  rw.Write(num_angles_);
  rw.Write(width_);
  rw.Write(height_);
  rw.Write(max_prim_reach_);
  rw.Write(computation_time_);

  dist_.Write(rw);
  num_pred_.Write(rw);
  num_succ_.Write(rw);
  canonical_parent_.Write(rw);
  canonical_succ_.Write(rw);
  canonical_pred_.Write(rw);
}
void LatticeFreespace::Read(FileReadWrite & rw) {
  rw.Read(bound_);
  rw.Read(out_of_bound_);
  rw.Read(num_angles_);
  rw.Read(width_);
  rw.Read(height_);
  rw.Read(max_prim_reach_);
  rw.Read(computation_time_);

  dist_.Read(rw);
  num_pred_.Read(rw);
  num_succ_.Read(rw);
  canonical_parent_.Read(rw);
  canonical_succ_.Read(rw);
  canonical_pred_.Read(rw);
}


void LatticeFreespace::CalculateHeuristic(Distance bound) {
  int x_stretch, y_stretch;
  GenerateTable(dist_, out_of_bound_, bound, x_stretch, y_stretch);

  Linearize3D l(-x_stretch, x_stretch, -y_stretch, y_stretch, 0, num_angles_-1);
  auto ToNodeId = [&](xyThetaPos & p) -> nodeId {
    return l.ToLinear(p.x, p.y, p.o);
  };
  auto ToXYThetaPos = [&](nodeId & n) -> xyThetaPos {
    xyThetaPos p;
    l.ExtractXYZ(n, p.x, p.y, p.o);
    return p;
  };

  F_Val_PPQ ppq_((x_stretch*2 + 1) * (y_stretch*2 + 1) * num_angles_);

  // For each starting angle:
  for (int start_a = 0; start_a < num_angles_; start_a++) {
    //for (int start_a = 0; start_a < 1; start_a++) {
    std::cout << "Computing heuristic for discrete orientation: " << start_a
              << std::endl;

    xyThetaPos start(0, 0, start_a);

    //Dijkstra
    ppq_.Reset();
    nodeId start_id = ToNodeId(start);
    ppq_.InsertOrDecreaseKey(start_id, 0);
    dist_[start_a][start] = 0;

    //int num_expansions = 0;
    while (!ppq_.IsEmpty()) {
      nodeId curr_id = ppq_.PopMin();
      xyThetaPos curr_pos = ToXYThetaPos(curr_id);
      Distance curr_g = dist_[start_a][curr_pos];

      const std::vector<MotionPrimitive>* prims = lattice_
          ->GetAllForwardPrimitives(curr_pos.o);

      for (int i = 0; i < prims->size(); i++) {
        const MotionPrimitive* prim = &prims->at(i);
        xyThetaPos succ_pos = prim->GetResultingPos(curr_pos);
        double new_g = curr_g + prim->GetCost();

        // We want all the outgoing arcs from the start to be R-reachable,
        // even if their cost is greater than the bound.
        if (new_g > bound_ + kEpsDistance)
#ifdef INCLUDE_EDGES_IN_R
          if (curr_id != start_id )
#endif
          continue;

        // Check if the generated state is out of bounds.
        if (succ_pos.x < -x_stretch || succ_pos.x > x_stretch
            || succ_pos.y < -y_stretch || succ_pos.y > y_stretch)
          continue;

        if (new_g  < dist_.GetValue(start_a, succ_pos)) {
          dist_[start_a][succ_pos] =  new_g;
          nodeId succ_id = ToNodeId(succ_pos);
          ppq_.InsertOrDecreaseKey(succ_id, new_g);
        }
      }
    }
  }
}

void LatticeFreespace::CalculateNumPredAndNumSucc(Distance bound) {
  assert(dist_.IsInitialized());

  int x_stretch, y_stretch;
  GenerateTable(num_pred_, kInvalidPrimCount, bound, x_stretch, y_stretch);
  GenerateTable(num_succ_, kInvalidPrimCount, bound, x_stretch, y_stretch);\

  for (int sa = 0; sa < num_angles_; sa++) {
    xyThetaPos origin(0, 0, sa);

    for (int x = -x_stretch; x <= x_stretch; x++) {
      for (int y = -y_stretch; y <= y_stretch; y++) {
        for (int ea = 0; ea < num_angles_; ea++) {
          // For all starting angles, and all nodes in that table,
          // if there is a shortest (bounded) freespace path to the node,
          // find the number of successors

          xyThetaPos curr(x,y,ea);
          if (fabs(dist_[sa][curr] - out_of_bound_) > kEpsDistance) {// FIXME: check!
          //if (dist_[sa][curr] <= bound_ + kEpsDistance) {
            num_pred_[sa][curr] = 0;
            num_succ_[sa][curr] = 0;

            // Calculate num_pred
            const std::vector<MotionPrimitive>* prims = lattice_
                ->GetAllReversePrimitives(ea);

            for (int i = 0; i < prims->size(); i++) {
              const MotionPrimitive* prim = &prims->at(i);
              xyThetaPos pred = prim->GetResultingPos(curr);
              // origin -> ... -> pred -> curr;
              Distance origin_curr = GetHeuristicDistance(origin, curr);
              Distance origin_pred = GetHeuristicDistance(origin, pred);
              Distance pred_curr = prim->GetCost();
              if (fabs(origin_curr - origin_pred - pred_curr) < kEpsDistance)
                num_pred_[sa][curr]++;
            }
            // Calculate num_succ
            prims = lattice_->GetAllForwardPrimitives(sa);

            for (int i = 0; i < prims->size(); i++) {
              const MotionPrimitive* prim = &prims->at(i);
              xyThetaPos succ = prim->GetResultingPos(origin);

              // origin -> succ -> ... -> curr;
              Distance origin_curr = GetHeuristicDistance(origin, curr);
              Distance succ_curr = GetHeuristicDistance(succ, curr);
              if (fabs(origin_curr - succ_curr - prim->GetCost()) < kEpsDistance)
                num_succ_[sa][curr]++;
            }
          }
        }
      }
    }
  }
}

void LatticeFreespace::CalculateCanonicalParents(Distance bound) {
  assert(dist_.IsInitialized());

  int x_stretch, y_stretch;
  GenerateTable(canonical_parent_, kInvalidPrimId, bound, x_stretch, y_stretch);

  for (int sa = 0; sa < num_angles_; sa++) {
    xyThetaPos origin(0, 0, sa);
    std::vector<std::pair<xyThetaPos, xyThetaPos>> stack;
    stack.push_back(std::make_pair(origin, origin));

    while (!stack.empty()) {
      xyThetaPos curr = stack.back().first;
      xyThetaPos parent_pos = stack.back().second;
      stack.pop_back();

      // If this is the first time this node is generated, calculate its
      // L-canonical parent.
      if (canonical_parent_[sa][curr] == kInvalidPrimId && !(curr == origin)) {
        const std::vector<MotionPrimitive>* r_prims = lattice_
            ->GetAllReversePrimitives(curr.o);
        PrimId parent_prim_id = kInvalidPrimId;
        for (int i = 0; i < r_prims->size(); i++) {
          const MotionPrimitive* prim = &r_prims->at(i);
          xyThetaPos pred_of_curr = prim->GetResultingPos(curr);
          if (pred_of_curr == parent_pos) {
            assert(parent_prim_id == kInvalidPrimId);
            parent_prim_id = i;
          }
        }
        assert(parent_prim_id != kInvalidPrimId);
        canonical_parent_[sa][curr] = parent_prim_id;
      }
      else if (!(curr == origin))
        continue;

      // Process neighbors in reverse order so that they are added to the
      // stack in the reverse order, which means that they will be popped from
      // the stack in the lexicographic order.
      const std::vector<MotionPrimitive>* prims = lattice_
          ->GetAllForwardPrimitives(curr.o);
      for (int i = prims->size()-1; i >= 0; i--) {
        const MotionPrimitive* prim = &prims->at(i);
        xyThetaPos succ = prim->GetResultingPos(curr);

        // If successor is out of bounds, skip it.
        if (succ.x < -x_stretch || succ.x > x_stretch
            || succ.y < -y_stretch || succ.y > y_stretch)
          continue;

        // If parent is set, then the successor has already been reached
        // through an L-canonical path, skip it.
        if (canonical_parent_[sa][succ] != kInvalidPrimCount)
          continue;

        // If the successor is reached through a suboptimal path, skip it.
        Distance origin_curr = GetHeuristicDistance(origin, curr);
        Distance curr_succ = prim->GetCost();
        Distance origin_succ = GetHeuristicDistance(origin, succ);
        if (fabs(origin_succ - origin_curr - curr_succ) > kEpsDistance)
          continue;

        assert(!(succ == origin));

        // We are now processing a successor reached with an L-canonical
        // shortest path.
        stack.push_back(std::make_pair(succ, curr));
      }
    }
  }

  for (int sa = 0; sa < num_angles_; sa++) {
    xyThetaPos p(0,0,sa);
    assert(canonical_parent_[sa][p] == kInvalidPrimId);
  }
}

void LatticeFreespace::CalculateCanonicalSuccessors(Distance bound) {
  assert(canonical_parent_.IsInitialized());

  int x_stretch, y_stretch;
  GenerateTable(canonical_succ_, 0, bound, x_stretch, y_stretch);

  for (int sa = 0; sa < num_angles_; sa++) {
    xyThetaPos origin(0, 0, sa);
    for (int x = -x_stretch; x <= x_stretch; x++) {
      for (int y = -y_stretch; y <= y_stretch; y++) {
        for (int ea = 0; ea < num_angles_; ea++) {
          xyThetaPos curr(x,y,ea);
          PrimId parent_prim_id = canonical_parent_[sa][curr];
          if (parent_prim_id != kInvalidPrimId) {
            assert(!(curr == origin));

            // Find the parent pointed by the parent_prim_id.
            xyThetaPos parent = lattice_->GetAllReversePrimitives(
                curr.o)->at(parent_prim_id).GetResultingPos(
                curr);

            // Find the corresponding forward primitive.
            const std::vector<MotionPrimitive>* prims = lattice_
                ->GetAllForwardPrimitives(parent.o);

            PrimId succ_of_parent_prim_id = kInvalidPrimId;
            for (int i = 0; i < prims->size(); i++) {
              const MotionPrimitive* prim = &prims->at(i);
              xyThetaPos succ_of_parent = prim->GetResultingPos(parent);
              if (succ_of_parent == curr) {
                assert(succ_of_parent_prim_id == kInvalidPrimId);
                succ_of_parent_prim_id = i;
              }
            }
            assert(succ_of_parent_prim_id != kInvalidPrimId);

            // Add the flag for the forward primitive to the parent.
            ExecutablePrimitiveFlags flags = canonical_succ_[sa][parent];
            flags = flags | (((ExecutablePrimitiveFlags) 1) << succ_of_parent_prim_id);
            canonical_succ_[sa][parent] = flags;
          }
        }
      }
    }
  }
}


void LatticeFreespace::CalculateCanonicalPredecessors(Distance bound) {
  assert(canonical_parent_.IsInitialized());

  int x_stretch, y_stretch;
  GenerateTable(canonical_pred_, 0, bound, x_stretch, y_stretch);
//  reverse_canonical_succ_.InitializeTableByExtendingFromOrigin(x_stretch,
//                                                               y_stretch,
//                                                               num_angles_, 0);
  for (int sa = 0; sa < num_angles_; sa++) {
    xyThetaPos origin(0, 0, sa);
    for (int x = -x_stretch; x <= x_stretch; x++) {
      for (int y = -y_stretch; y <= y_stretch; y++) {
        for (int ea = 0; ea < num_angles_; ea++) {
          xyThetaPos curr(x,y,ea);
          PrimId parent_prim_id = canonical_parent_[sa][curr];
          if (parent_prim_id != kInvalidPrimId) {
            // Follow the parent pointers to the origin.
            xyThetaPos p = curr;
            int target_table_orientation = curr.o;
            int delta_x = origin.x - curr.x;
            int delta_y = origin.y - curr.y;

            // TODO: Switch to if? (originally while)
            while (parent_prim_id != kInvalidPrimId) {
              // Add the parent primitive to the reverse canonical successors
              // of the relevant entry.
              xyThetaPos translated_p(p.x + delta_x, p.y + delta_y,
                                      p.o);


              // Check if the generated state is out of bounds.
              if (translated_p.x < -x_stretch || translated_p.x > x_stretch
                  || translated_p.y < -y_stretch || translated_p.y > y_stretch)
                break;

              ExecutablePrimitiveFlags flags =
                  canonical_pred_[target_table_orientation][translated_p];
              flags = flags | (((ExecutablePrimitiveFlags) 1) << parent_prim_id);
              canonical_pred_[target_table_orientation][translated_p] =
                  flags;

              // Update p and parent prim id.
              p = lattice_->GetAllReversePrimitives(
                  p.o)->at(parent_prim_id).GetResultingPos(
                  p);
              parent_prim_id = canonical_parent_[sa][p];
            }
            //assert (p == origin);
          }
        }
      }
    }
  }

  // TODO:
  // Code to check if the reverse DAG is actually a tree
}

void LatticeFreespace::CalculateFreespaceSuccessorsAndPredecessors(Distance bound) {
  assert(dist_.IsInitialized());

  int x_stretch, y_stretch;
  GenerateTable(canonical_succ_, 0, bound, x_stretch, y_stretch);
  GenerateTable(canonical_pred_, 0, bound, x_stretch, y_stretch);

  for (int sa = 0; sa < num_angles_; sa++) {
    xyThetaPos origin(0, 0, sa);
    for (int x = -x_stretch; x <= x_stretch; x++) {
      for (int y = -y_stretch; y <= y_stretch; y++) {
        for (int ca = 0; ca < num_angles_; ca++) {
          xyThetaPos curr(x, y, ca);

          // Calculate successors
          // origin -> ... -> curr -> succ;
          if (fabs(dist_[sa][curr] - out_of_bound_) > kEpsDistance) {
            const std::vector<MotionPrimitive>* prims = lattice_
                ->GetAllForwardPrimitives(ca);
            ExecutablePrimitiveFlags flags = 0;
            for (int i = 0; i < prims->size(); i++) {
              xyThetaPos succ = prims->at(i).GetResultingPos(curr);
              Distance origin_curr = dist_[sa].GetValue(curr);
              Distance origin_succ = dist_[sa].GetValue(succ);
              Distance curr_succ = prims->at(i).GetCost();
              ExecutablePrimitiveFlags flag = 0;
              if (fabs(origin_succ - out_of_bound_) > kEpsDistance &&
                  fabs(origin_succ - origin_curr - curr_succ) < kEpsDistance)
                flag = 1;
              flags = flags | (flag << i);
            }
            canonical_succ_[sa][curr] = flags;
          }


          // Calculate successors
          // origin <- ... <- curr <- pred;
          xyThetaPos origin_rel_curr (-curr.x, -curr.y, sa);
          if (fabs(dist_[ca][origin_rel_curr] - out_of_bound_) > kEpsDistance) {
            const std::vector<MotionPrimitive>* prims = lattice_
                ->GetAllReversePrimitives(ca);
            ExecutablePrimitiveFlags flags = 0;
            for (int i = 0; i < prims->size(); i++) {
              xyThetaPos pred = prims->at(i).GetResultingPos(curr);
              xyThetaPos origin_rel_pred (-pred.x, -pred.y, sa);

              Distance curr_origin = dist_[ca].GetValue(origin_rel_curr);
              Distance pred_origin = dist_[pred.o].GetValue(origin_rel_pred);
              Distance pred_curr = prims->at(i).GetCost();
              ExecutablePrimitiveFlags flag = 0;
              if (fabs(pred_origin - out_of_bound_) > kEpsDistance &&
                  fabs(pred_origin - pred_curr - curr_origin) < kEpsDistance)
                flag = 1;
              flags = flags | (flag << i);
            }
            canonical_pred_[sa][curr] = flags;
          }
        }
      }
    }
  }
}

void LatticeFreespace::CalculateFreespaceParents(Distance bound) {
  assert(dist_.IsInitialized());

  int x_stretch, y_stretch;
  GenerateTable(freespace_parent_, 0, bound, x_stretch, y_stretch);

  for (int sa = 0; sa < num_angles_; sa++) {
    xyThetaPos origin(0, 0, sa);
    for (int x = -x_stretch; x <= x_stretch; x++) {
      for (int y = -y_stretch; y <= y_stretch; y++) {
        for (int ca = 0; ca < num_angles_; ca++) {

          // start -> ... -> parent -> curr ;
          xyThetaPos curr(x, y, ca);
          if (fabs(dist_[sa][curr] - out_of_bound_) > kEpsDistance) {
            const std::vector<MotionPrimitive>* prims = lattice_
                ->GetAllReversePrimitives(ca);
            ExecutablePrimitiveFlags flags = 0;
            for (int i = 0; i < prims->size(); i++) {
              xyThetaPos parent = prims->at(i).GetResultingPos(curr);
              Distance origin_curr = dist_[sa][curr];
              Distance origin_parent = dist_[sa].GetValue(parent);
              Distance parent_curr = prims->at(i).GetCost();
              ExecutablePrimitiveFlags flag = 0;

              if (fabs(origin_parent - out_of_bound_) > kEpsDistance &&
                  fabs(origin_curr - origin_parent - parent_curr) < kEpsDistance)
                flag = 1;
              flags = flags | (flag << i);
            }
            freespace_parent_[sa][curr] = flags;
          }
        }
      }
    }
  }
}


void LatticeFreespace::CalculateCanonicalNextMoves(Distance bound) {
  assert(canonical_pred_.IsInitialized());

  int x_stretch, y_stretch;
  GenerateTable(canonical_next_move_, kInvalidPrimId, bound, x_stretch,
                y_stretch);

  for (int sa = 0; sa < num_angles_; sa++) {
    xyThetaPos origin(0, 0, sa);
    for (int x = -x_stretch; x <= x_stretch; x++) {
      for (int y = -y_stretch; y <= y_stretch; y++) {
        for (int ca = 0; ca < num_angles_; ca++) {

          // goal -> ... -> curr -> canonical_preds
          // Canonical pred: curr -> canonical_pred
          // Canonical next move: canonical_pred -> curr.
          xyThetaPos curr(x, y, ca);
          ExecutablePrimitiveFlags pred_flags = canonical_pred_[sa][curr];
          if (pred_flags == 0)
            continue;

          const std::vector<MotionPrimitive>* pred_prims = lattice_
              ->GetAllReversePrimitives(ca);

          // For each canonical pred...
          ExecutablePrimitiveFlags mask = 1;
          for (unsigned int i = 0; i < pred_prims->size(); i++) {
            if (pred_flags & mask) {
              xyThetaPos pred = pred_prims->at(i).GetResultingPos(curr);

              // ... calculate its corresponding canonical next move, as the
              // primitive id that induces the edge (pred,curr).
              const std::vector<MotionPrimitive>* succ_prims = lattice_
                  ->GetAllForwardPrimitives(pred.o);
              for (int j = 0; j < succ_prims->size(); j++) {
                if (succ_prims->at(j).GetResultingPos(pred) == curr) {
                  canonical_next_move_[sa][pred] = j;
                }
              }
            }
            mask = mask << 1;
          }
        }
      }
    }
  }
}


