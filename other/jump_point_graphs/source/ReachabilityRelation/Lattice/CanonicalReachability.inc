/*
 * CanonicalReachability.inc
 *
 *  Created on: Mar 14, 2018
 *      Author: idm-lab
 */

template<class G, class S, class H>
bool CanonicalReachability<G, S, H>::ExploreReachableAreaFringeDijkstra(
    nodeId start, bool terminate_if_fringe, Distance exploration_bound) {

  if (!this->exploring_backward_) {
    auto IsReachable =
        [&](nodeId source, nodeId curr, nodeId succ, Distance g_val) -> bool {
          return this->data_.IsReachable(curr) &&
              h_->GetCanonicalParent(source, succ) == curr;
        };
    return ReachabilityRelationExploreCommon<G, S>::ExploreReachableAreaFringeDijkstra(
        start, terminate_if_fringe, exploration_bound, IsReachable);
  }
  else {
    /*
    assert(!this->exploring_backward_);
    /*/
    auto IsReachable =
        [&](nodeId source, nodeId curr, nodeId succ, Distance g_val) -> bool {
          if (!this->data_.IsReachable(curr))
            return false;
          std::vector<WeightedArcHead> canonical_predecessors;
          h_->GetReverseCanonicalSuccessors(source, curr, canonical_predecessors);
          for (auto a: canonical_predecessors) {
            if (a.target == succ)
              return true;
          }
          return false;
        };
    return ReachabilityRelationExploreCommon<G, S>::ExploreReachableAreaFringeDijkstra(
        start, terminate_if_fringe, exploration_bound, IsReachable);
    //*/
  }
}


template<class G, class S, class H>
void CanonicalReachability<G, S, H>::RConnect(nodeId start, bool can_identify_superset) {
  if (!can_identify_superset) {
    ExploreReachableAreaFringeDijkstra(start, false, kMaxDistance);
    return;
  }

  bool stop_at_subgoals = true;
  assert(!using_distances_ || "Canonical reachability should not have distances");
  if (using_distances_)
    this->ExploreBFSAggressiveNoDuplicateWithFreespaceDistances(start, stop_at_subgoals);
  else
    this->ExploreBFSAggressiveNoDuplicate(start); //, stop_at_subgoals);
}

template<class G, class S, class H>
void CanonicalReachability<G, S, H>::RRefine(
    nodeId from, nodeId to, std::vector<nodeId> & path, bool append) {
  if (!append) {
    path.clear();
  }
  else if (!path.empty()) {
    path.pop_back();
  }
/*  // next-moves
  nodeId curr = from;
  path.push_back(curr);
  while (path.back() != to) {
//  while (curr != to) {
//    curr = h_->GetCanonicalNextMove(to, curr);
    h_->AddCanonicalNextMove(to, path);
//    path.push_back(curr);
  }

/*/ // parents
  int start_index = path.size();
  nodeId curr = to;
  path.push_back(curr);
  while (curr != from) {
    curr = h_->GetCanonicalParent(from, curr);
    path.push_back(curr);
  }
  std::reverse(path.begin()+start_index, path.end());
//*/
}


template<class G, class S, class H>
bool CanonicalReachability<G, S, H>::IsReachable(nodeId start, nodeId goal,
                                                 Distance d) {
  while (goal != start && goal != kNonNode) {
    goal = h_->GetCanonicalParentIfExists(start, goal);
  }
  return goal == start;
}

template<class G, class S, class H>
int CanonicalReachability<G, S, H>::FindLastReachableIndex(
    std::vector<nodeId> & path, int start, int goal) {
//*
  // Go over the path in reverse and find the first reachable index in reverse.
  // When goal = start + 1, return true as it is an edge of G.
  while (goal > start + 1) {
    if (IsReachable(path[start], path[goal]))
      return goal;
    else
      goal--;
  }
  return goal;
/*/
  // Go over the path and find the first non-reachable index
  int curr = start;
  while (curr < goal) {
    // path[curr] is R-reachable path[start].
    // Check if path[curr+1] is also R-reachable from path[start]
    int next = curr + 1;
    if (!(h_->GetCanonicalParent(path[start], path[next]) == path[curr]))
      return curr;
    else
      curr = next;
  }
  return curr;
//*/
}

template<class G, class S, class H>
void CanonicalReachability<G, S, H>::ExploreBFSAggressiveNoDuplicateWithFreespaceDistances(
    nodeId start, bool stop_at_subgoals) {
  this->Reset();

  // Use expansion order as a BFS queue
  expansion_order_.push_back(start);
  int c = 0;

  while (c < expansion_order_.size()) {
    nodeId curr = expansion_order_[c];
    c++;
    // If we find a non-start subgoal, make a note and don't expand it.
    if (curr != start && this->sm_->IsSubgoal(curr)) {
      existing_subgoals_.push_back(curr);
      existing_subgoal_distances_.push_back(
          GetDirectionSensitiveHeuristicDistance(start, curr));
      if (stop_at_subgoals)
        continue;
    }
    exploring_backward_ ?
        h_->AddReverseCanonicalSuccessors(start, curr,
                                          expansion_order_) :
        h_->AddCanonicalSuccessors(start, curr, expansion_order_);
  }
}

template<class G, class S, class H>
void CanonicalReachability<G, S, H>::ExploreBFSAggressiveNoDuplicate(
    nodeId start) {

  CPUTimer t;
  t.StartTimer();
  Reset();

  // Use expansion order as a BFS queue
  expansion_order_.push_back(start);
  dist_.push_back(0);

  int c = 0;

  while (c < expansion_order_.size()) {
    nodeId curr = expansion_order_[c];
    Distance g_val = dist_[c];
    c++;

    // If we find a non-start subgoal, make a note and don't expand it.
    if (curr != start && sm_->IsSubgoal(curr)) {
      existing_subgoals_.push_back(curr);
      existing_subgoal_distances_.push_back(g_val);
      continue;
    }
    exploring_backward_ ?
        st_.num_relaxed += h_->AddReverseCanonicalSuccessorsAndDistances(
            start, curr, g_val, expansion_order_, dist_) :
        st_.num_relaxed += h_->AddCanonicalSuccessorsAndDistances(
            start, curr, g_val, expansion_order_, dist_);
  }
  t.EndTimer();
  st_.time = t.GetElapsedTime();
}

#ifndef NO_HOG
template<class G, class S, class H>
void CanonicalReachability<G, S, H>::Visualize(const MapEnvironment *env,
                                               int display_type) {
  std::vector < WeightedArcHead > neighbors;

  bool display_node = display_type == kRReachableAreaDisplayNode
      || display_type == kRReachableAreaDisplayNodeAndParent;
  bool display_parent = display_type == kRReachableAreaDisplayParent
      || display_type == kRReachableAreaDisplayNodeAndParent
      || display_type == kRReachableAreaDisplayAllPaths;

  if (this->expansion_order_.empty())
    return;

  nodeId source = this->expansion_order_[0];

  for (unsigned int i = 0; i < this->expansion_order_.size(); i++) {
    nodeId curr = this->expansion_order_[i];
    bool is_subgoal = (this->sm_ != NULL && this->sm_->IsSubgoal(curr));

    // Color: direct-h-reachable (white), shortest path through subgoal
    // (green), subgoal (blue), fringe node (red), rest (black).
    if (is_subgoal)
      env->SetColor(1, 0, 0);
    else
      env->SetColor(0, 0, 0);
    //env->SetColor(1, 1, 1);

    if (display_node) {
      if (is_subgoal) {
        this->g_->DrawArrowNode(env, curr, 0.8);
        env->SetColor(0,0,0);
      }
      else
        this->g_->DrawArrowNode(env, curr, 0.8);
    }

    if (display_parent && (curr == source || !is_subgoal)){
      this->exploring_backward_ ?
          h_->GetReverseCanonicalSuccessors(source, curr, neighbors) :
          h_->GetCanonicalSuccessors(source, curr, neighbors);
      for (unsigned int i = 0; i < neighbors.size(); i++) {
        nodeId succ = neighbors[i].target;
        this->exploring_backward_ ?
            this->g_->DrawEdge(env, succ, curr) : this->g_->DrawEdge(env, curr, succ);
      }
    }
  }
}
#endif
