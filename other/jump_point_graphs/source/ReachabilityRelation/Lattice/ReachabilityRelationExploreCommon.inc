#include <vector>

#include "../../../utils/FPUtil.h"
template<class G, class S>
template<class R, class SBP, class SSBP>
bool ReachabilityRelationExploreCommon<G, S>::ExploreReachableAreaFringeDijkstra(
    nodeId start, bool terminate_if_fringe, Distance exploration_bound,
    R IsDirectionSensitiveReachable,
    SBP OnStrictlyBetterPath,
    SSBP OnSymmetricOrStrictlyBetterPath) {

  // We want to perform a dijkstra search from the start node. This search will
  // continue until there are no more nodes that satisfy either criteria:
  // 1- R-Reachable, does not have a shortest path through a subgoal.
  //    (direct-h-reachable).
  // 2- Non-R-reachable, does not have a shortest path through a subgoal,
  //    and has a h-reachable parent (fringe nodes).
  // The search below maintains three flags for each node:
  // R-Reachable, Shortest-Path-Through-Subgoal, and R-Reachable-Parent.

  CPUTimer t;
  t.StartTimer();
  Reset();
  std::vector < WeightedArcHead > neighbors;

  //AddStart(start, kIsReachable | kHasReachableParent);
  //int num_dhr_in_queue = 1;    // Just the start node.
  int num_dhr_in_queue = ExpandStart(start, kIsReachable | kHasReachableParent);

  int num_fringe_in_queue = 0;

  while (!ppq_.IsEmpty() && num_dhr_in_queue + num_fringe_in_queue > 0) {
    nodeId curr = ppq_.PopMin();
    expansion_order_.push_back(curr);

    if (curr != start && sm_->IsSubgoal(curr) && data_.IsDirectReachable(curr))
      AddToExistingSubgoals(curr);

    if (data_.IsDirectReachable(curr))
      num_dhr_in_queue--;

    if (data_.IsFringeNode(curr)) {
      num_fringe_in_queue--;
      fringe_nodes_.push_back(curr);

      if (terminate_if_fringe) {
        t.EndTimer();
        st_.time = t.GetElapsedTime();
        return true;
      }
    }

    // Expand the node.
    double g_curr = data_.GetGVal(curr);
    GetDirectionSensitiveSuccessors(curr, neighbors);

    for (unsigned int i = 0; i < neighbors.size(); i++) {
      st_.num_relaxed++;
      nodeId succ = neighbors[i].target;
      double new_g_val = g_curr + neighbors[i].weight;

      if (new_g_val > exploration_bound)
        continue;

      if (data_.IsGenerated(succ)) {
        if (data_.IsDirectReachable(succ))
          num_dhr_in_queue--;

        if (data_.IsFringeNode(succ))
          num_fringe_in_queue--;
      }

      data_.GenerateNode(succ);

      double old_g_val = data_.GetGVal(succ);

      bool strictly_better_path = new_g_val < old_g_val - kEpsDistance;
      bool symmetric_path = fabs(new_g_val - old_g_val) < kEpsDistance;

      if (strictly_better_path) {
        data_.SetGVal(succ, new_g_val);

        // The new shortest path does not necessarily have go through a subgoal. If it does, it will be determined in the next step.
        data_.UnsetCovered(succ);
        data_.UnsetHasReachableParent(succ);
        OnStrictlyBetterPath(curr, succ);
      }

      // If the current node is a subgoal/has a shortest path through a subgoal, and we have found a symmetric/better path to the successor
      // through the current node, then the current best path to the successor goes through a subgoal.
      if (symmetric_path || strictly_better_path) {
        PropagateIfCovered(start, curr, succ);
        UpdateHasReachableParent(curr, succ);
        OnSymmetricOrStrictlyBetterPath(curr, succ);
      }

      // R-reachability check
      if (!data_.IsReachable(succ)
          && IsDirectionSensitiveReachable(start, curr, succ, new_g_val))
        data_.SetReachable(succ);

      if (data_.IsDirectReachable(succ))
        num_dhr_in_queue++;

      if (data_.IsFringeNode(succ))
        num_fringe_in_queue++;

      // We don't always want to add to open?
      if (strictly_better_path)
        ppq_.InsertOrDecreaseKey(succ, new_g_val);
    }
  }
  t.EndTimer();
  st_.time = t.GetElapsedTime();
  return false;
}

// Only explores reachable nodes. Works for Freespace reachability and
// bounded distance reachability.
template<class G, class S>
template<class R>
void ReachabilityRelationExploreCommon<G, S>::ExploreDijkstraConservative(
    nodeId start, R IsDirectionSensitiveReachable) {

  CPUTimer t;
  t.StartTimer();
  Reset();

  std::vector < WeightedArcHead > neighbors;

  // We want to perform a dijkstra search from the start node. This search will
  // continue until there are no more nodes that are direct-R-reachable
  // from the source. Only expands R-reachable nodes from the source.

  //AddStart(start, kIsReachable);
  //int num_dhr_in_queue = 1;    // Just the start node.
  int num_dhr_in_queue = ExpandStart(start, kIsReachable);

  while (!ppq_.IsEmpty() && num_dhr_in_queue > 0) {
    nodeId curr = ppq_.PopMin();
    expansion_order_.push_back(curr);
    num_dhr_in_queue -= data_.IsDirectReachable(curr);

    if (curr != start && sm_->IsSubgoal(curr) && data_.IsDirectReachable(curr))
      AddToExistingSubgoals(curr);

    // Expand the node.
    double g_curr = data_.GetGVal(curr);
    GetDirectionSensitiveSuccessors(curr, neighbors);

    for (unsigned int i = 0; i < neighbors.size(); i++) {
      st_.num_relaxed++;
      nodeId succ = neighbors[i].target;

      double new_g_val = g_curr + neighbors[i].weight;
      if ((!data_.IsGenerated(succ)
          || new_g_val < data_.GetGVal(succ) + kEpsDistance)
          && IsDirectionSensitiveReachable(start, curr, succ, new_g_val)) {

        num_dhr_in_queue -= data_.IsGenerated(succ)
            && data_.IsDirectReachable(succ);
        data_.GenerateNode(succ);

        if (new_g_val + kEpsDistance < data_.GetGVal(succ))
          data_.UnsetCovered(succ);

        data_.SetGVal(succ, new_g_val);
        data_.SetReachable(succ);
        PropagateIfCovered(start, curr, succ);
        ppq_.InsertOrDecreaseKey(succ, new_g_val);
        num_dhr_in_queue += data_.IsDirectReachable(succ);
      }
    }
  }
  t.EndTimer();
  st_.time = t.GetElapsedTime();
}

// Dijkstra, identify superset
template<class G, class S>
template<class R>
void ReachabilityRelationExploreCommon<G, S>::ExploreDijkstraAggressive(
    nodeId start, R IsDirectionSensitiveReachable) {

  CPUTimer t;
  t.StartTimer();
  Reset();
  //AddStart(start, kIsReachable);
  ExpandStart(start, kIsReachable);
  std::vector < WeightedArcHead > neighbors;

  while (!ppq_.IsEmpty()) {
    nodeId curr = ppq_.PopMin();
    expansion_order_.push_back(curr);

    if (curr != start && sm_->IsSubgoal(curr)) {
      AddToExistingSubgoals(curr);
      continue;
    }

    // Expand the node.
    double g_curr = data_.GetGVal(curr);
    GetDirectionSensitiveSuccessors(curr, neighbors);

    for (unsigned int i = 0; i < neighbors.size(); i++) {
      st_.num_relaxed++;
      nodeId succ = neighbors[i].target;

      double new_g_val = g_curr + neighbors[i].weight;
      if ((!data_.IsGenerated(succ)
          || new_g_val < data_.GetGVal(succ) + kEpsDistance)
          && IsDirectionSensitiveReachable(start, curr, succ, new_g_val)) {

        data_.GenerateNode(succ);
        data_.SetGVal(succ, new_g_val);
        data_.SetReachable(succ);
        ppq_.InsertOrDecreaseKey(succ, new_g_val);
      }
    }
  }
  t.EndTimer();
  st_.time = t.GetElapsedTime();
}


// Dijkstra, identify superset
template<class G, class S>
template<class R>
void ReachabilityRelationExploreCommon<G, S>::ExploreDijkstraStall(
    nodeId start, R IsDirectionSensitiveReachable) {

  CPUTimer t;
  t.StartTimer();
  Reset();
  //AddStart(start, kIsReachable);
  ExpandStart(start, kIsReachable);
  std::vector < WeightedArcHead > neighbors;

  while (!ppq_.IsEmpty()) {
    nodeId curr = ppq_.PopMin();
    expansion_order_.push_back(curr);

    if (curr != start && sm_->IsSubgoal(curr)) {
      AddToExistingSubgoals(curr);
      data_.SetStalled(curr); // Stalled.
      continue;
    }

    // Check if the node can be stalled.
    double g_curr = data_.GetGVal(curr);
    GetDirectionSensitivePredecessors(curr, neighbors);
    for (unsigned int i = 0; i < neighbors.size(); i++) {
      st_.num_stall_relaxed++;
      nodeId pred = neighbors[i].target;
      if (data_.IsGenerated(pred) && data_.IsStalled(pred) &&
          g_curr > data_.GetGVal(pred) + neighbors[i].weight - kEpsDistance ) {
        data_.SetStalled(curr);
        data_.SetGVal(curr, data_.GetGVal(pred) + neighbors[i].weight);
        break;
      }
    }
    if (data_.IsStalled(curr))
      continue;

    // Expand the node.
    GetDirectionSensitiveSuccessors(curr, neighbors);

    for (unsigned int i = 0; i < neighbors.size(); i++) {
      st_.num_relaxed++;
      nodeId succ = neighbors[i].target;

      double new_g_val = g_curr + neighbors[i].weight;
      if ((!data_.IsGenerated(succ)
          || new_g_val < data_.GetGVal(succ) + kEpsDistance)
          && IsDirectionSensitiveReachable(start, curr, succ, new_g_val)) {

        data_.GenerateNode(succ);
        data_.SetGVal(succ, new_g_val);
        data_.SetReachable(succ);
        ppq_.InsertOrDecreaseKey(succ, new_g_val);
      }
    }
  }
  t.EndTimer();
  st_.time = t.GetElapsedTime();
}


template<class G, class S>
template<class R>
bool ReachabilityRelationExploreCommon<G, S>::ExploreBFSConservative(
    nodeId start, R IsDirectionSensitiveReachable) {

  // This function never generates a node that is not h-reachable from the start.
  // It explores all the nodes that are direct-h-reachable from the start, and tries to minimize
  // the number of explored nodes that are h-reachable but not direct-h-reachable from the start.

  CPUTimer t;
  t.StartTimer();
  Reset();

  std::vector <WeightedArcHead> neighbors;

  // Add start to the dhr_queue.
  data_.GenerateNode(start);
  data_.SetGVal(start, 0);
  expansion_order_.push_back(start);

  int num_dhr_in_queue = 1;    // Just the start node. // Delta

  int c = 0;
  while (c < expansion_order_.size() && num_dhr_in_queue > 0) {
    nodeId curr = expansion_order_[c];
    data_.SetFlag(curr, kIsClosed);
    c++;

    num_dhr_in_queue -= !data_.IsCovered(curr); // Delta
    if (curr != start && sm_->IsSubgoal(curr) && !data_.IsCovered(curr)) {
      AddToExistingSubgoals(curr);
    }

    // Expand the node.
    double g_curr = data_.GetGVal(curr);
    GetDirectionSensitiveSuccessors(curr, neighbors);
    for (unsigned int i = 0; i < neighbors.size(); i++) {
      st_.num_relaxed++;
      nodeId succ = neighbors[i].target;
      double new_g_val = g_curr + neighbors[i].weight;

      // If 'succ' has not been generated before, generate it only if it is
      // h-reachable from the start, and a shortest path from the start to
      // 'succ' goes through 'curr'.
      if (!data_.IsGenerated(succ)) {
        if (!IsDirectionSensitiveReachable(start, curr, succ, new_g_val))
          continue;

        data_.GenerateNode(succ);
        data_.SetGVal(succ, new_g_val);
        data_.SetReachable(succ);
        PropagateIfCovered(start, curr, succ);
        num_dhr_in_queue += !data_.IsCovered(succ);
        expansion_order_.push_back(succ);
      }

      // If 'succ' has been generated before, we are only interested in updating
      // its IsCovered flag from false to true.
      else if (!data_.GetFlag(succ, kIsClosed) && !data_.IsCovered(succ)
          && (data_.IsCovered(curr) || sm_->IsSubgoal(curr))
          && new_g_val < data_.GetGVal(succ) + kEpsDistance) {
        data_.SetCovered(succ);
        num_dhr_in_queue--;
      }
    }
  }
  expansion_order_.resize(c);

  t.EndTimer();
  st_.time = t.GetElapsedTime();
}



template<class G, class S>
template<class R>
bool ReachabilityRelationExploreCommon<G, S>::ExploreBFSAggressive(
    nodeId start, R IsDirectionSensitiveReachable) {

  // This function never generates a node that is not h-reachable from the start.
  // It explores all the nodes that are direct-h-reachable from the start, and tries to minimize
  // the number of explored nodes that are h-reachable but not direct-h-reachable from the start.

  CPUTimer t;
  t.StartTimer();
  Reset();

  std::vector <WeightedArcHead> neighbors;

  // Add start to the dhr_queue.
  data_.GenerateNode(start);
  data_.SetGVal(start, 0);
  expansion_order_.push_back(start);

  int c = 0;
  while (c < expansion_order_.size()) {
    nodeId curr = expansion_order_[c];
    c++;

    if (curr != start && sm_->IsSubgoal(curr)) {
      AddToExistingSubgoals(curr);
      continue;
    }

    // Expand the node.
    double g_curr = data_.GetGVal(curr);
    GetDirectionSensitiveSuccessors(curr, neighbors);
    for (unsigned int i = 0; i < neighbors.size(); i++) {
      st_.num_relaxed++;
      nodeId succ = neighbors[i].target;
      double new_g_val = g_curr + neighbors[i].weight;

      if (!data_.IsGenerated(succ)
          && IsDirectionSensitiveReachable(start, curr, succ, new_g_val)) {
        data_.GenerateNode(succ);
        data_.SetGVal(succ, new_g_val);
        data_.SetReachable(succ);
        expansion_order_.push_back(succ);
      }
    }
  }

  t.EndTimer();
  st_.time = t.GetElapsedTime();
}

template<class G, class S>
template<class R>
bool ReachabilityRelationExploreCommon<G, S>::ExploreDFSAggressive(
    nodeId start, R IsDirectionSensitiveReachable) {

  // This function never generates a node that is not h-reachable from the start.
  // It explores all the nodes that are direct-h-reachable from the start,
  // and tries to minimize the number of explored nodes that are h-reachable
  // but not direct-h-reachable from the start.
  // It does not expand subgoals.


  // Add start to the dhr_queue.

  CPUTimer t;
  t.StartTimer();
  Reset();

  stack_.clear();
  std::vector<WeightedArcHead> neighbors;

  data_.GenerateNode(start);
  data_.SetGVal(start, 0);
  data_.SetReachable(start);
  stack_.push_back(start);

  while (!stack_.empty()) {
    nodeId curr = stack_.back();
    stack_.pop_back();
    expansion_order_.push_back(curr);

    if (curr != start && sm_->IsSubgoal(curr)) {
      AddToExistingSubgoals(curr);
      continue;
    }

    // Expand the node.
    double g_curr = data_.GetGVal(curr);
    GetDirectionSensitiveSuccessors(curr, neighbors);
    for (unsigned int i = 0; i < neighbors.size(); i++) {
      st_.num_relaxed++;
      nodeId succ = neighbors[i].target;

      data_.GenerateNode(succ);
      Distance new_g_val = g_curr + neighbors[i].weight;

      if (!data_.GetFlag(succ, kIsReachable)
          && IsDirectionSensitiveReachable(start, curr, succ, new_g_val)) {
        // We are at a node that is R-reachable and that hasn't been processed.
        data_.SetGVal(succ, new_g_val);
        data_.SetReachable(succ);
        stack_.push_back(succ);
      }
    }
  }

  t.EndTimer();
  st_.time = t.GetElapsedTime();
}

template<class G, class S>
int ReachabilityRelationExploreCommon<G, S>::GetDirectReachableAreaSize(
    nodeId source) {
//  RExploreFringe(source);
  RConnect(source, false);  // Don't identify superset.

  int direct_r_reachable = 0;
  for (auto n: expansion_order_)
    if (data_.IsDirectReachable(n))
      direct_r_reachable++;
  return direct_r_reachable;
}

template<class G, class S>
RConnectStatistic ReachabilityRelationExploreCommon<G, S>::GetRConnectStatistic() {
  if (!st_processed_) {
    st_processed_ = true;

    st_.num_popped = this->expansion_order_.size();
    st_.num_expanded = this->expansion_order_.size();
    st_.num_subgoals = this->existing_subgoals_.size();

    for (auto n: expansion_order_) {
      if (data_.IsGenerated(n)) {
        if (data_.IsCovered(n))
          st_.num_covered++;
        if (data_.IsStalled(n))
          st_.num_stalled++;
        if (data_.GetGVal(n) > st_.radius)
          st_.radius = data_.GetGVal(n);
      }
    }
  }
  return st_;
}

template<class G, class S>
template<class R>
int ReachabilityRelationExploreCommon<G, S>::FindLastReachableIndexUsingDistance(
    std::vector<nodeId> & path, int start, int goal,
    R IsForwardReachable) {
  int curr = start;
  Distance g = 0;
  while (curr < goal) {
    // path[curr] is R-reachable path[start].
    // Check if path[curr+1] is also R-reachable from path[start]
    int next = curr + 1;
    Distance w = kMaxDistance;
    for (auto a: this->g_->GetSuccessors(path[curr])) {
      if (a.target == path[next]) {
        w = a.weight;
        break;
      }
    }
    assert(w < kMaxDistance);
    g = g + w;
    if (!IsForwardReachable(path[start], path[curr], path[next], g))
      return max(curr, start+1);
    else
      curr = next;
  }
  return max(curr, start+1);
}

#ifndef NO_HOG
template<class G, class S>
void ReachabilityRelationExploreCommon<G, S>::DetermineAndSetColor(
    const MapEnvironment *env, nodeId & n) const {
  if (sm_ != NULL && sm_->IsSubgoal(n)) {
    env->SetColor(1, 0, 0);
    return;
  }

  env->SetColor(0, 0, 0);
  if (data_.IsGenerated(n)) {
    if (data_.IsCovered(n))
       env->SetColor(1, 0.75, 0);
  }

}

template<class G, class S>
void ReachabilityRelationExploreCommon<G, S>::Visualize(
    const MapEnvironment *env, int display_type) {
  std::vector < WeightedArcHead > neighbors;

  bool display_node = display_type == kRReachableAreaDisplayNode
      || display_type == kRReachableAreaDisplayNodeAndParent;
  bool display_parent = display_type == kRReachableAreaDisplayParent
      || display_type == kRReachableAreaDisplayNodeAndParent
      || display_type == kRReachableAreaDisplayAllPaths;

  for (unsigned int i = 0; i < expansion_order_.size(); i++) {
    nodeId curr = expansion_order_[i];
    if (data_.IsGenerated(curr)) {
      bool is_subgoal = (sm_ != NULL && sm_->IsSubgoal(curr));

      // Color: direct-h-reachable (white), shortest path through subgoal
      // (green), subgoal (blue), fringe node (red), rest (black).
      if (is_subgoal)
        env->SetColor(1, 0, 0);
      else if (data_.IsCovered(curr))
        env->SetColor(1, 0.75, 0);
//      if (data_.IsReachable(curr))

//      else if (data_.IsDirectReachable(curr))
//        env->SetColor(0, 0, 0);
//      else if (data_.IsFringeNode(curr))
//        env->SetColor(1, 0, 0);
      else
        env->SetColor(0, 0, 0);

      if (display_node) {
        if (is_subgoal) {
          g_->DrawArrowNode(env, curr, 0.8);
          env->SetColor(0,0,0);
        }
        else
          g_->DrawArrowNode(env, curr, 0.8);
      }

      if (is_subgoal)
        env->SetColor(0,0,0); // Don't draw edges to subgoals in red.

      if (display_parent) {
        exploring_backward_ ?
            g_->GetSuccessors(curr, neighbors) :
            g_->GetPredecessors(curr, neighbors);
        for (unsigned int i = 0; i < neighbors.size(); i++) {
          nodeId pred = neighbors[i].target;
          if (fabs(
              data_.GetGVal(pred) + neighbors[i].weight - data_.GetGVal(curr))
              < kEpsDistance) {
            exploring_backward_ ?
                g_->DrawEdge(env, curr, pred) :
                g_->DrawEdge(env, pred, curr);
            if (display_type != kRReachableAreaDisplayAllPaths)
              break;
          }
        }
      }
    }
  }
}
#endif

