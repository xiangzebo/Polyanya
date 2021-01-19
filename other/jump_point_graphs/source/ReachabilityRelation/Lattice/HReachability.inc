/*
 * HReachability.inc
 *
 *  Created on: Mar 10, 2018
 *      Author: idm-lab
 */

template<class G, class S, class H>
void HReachability<G, S, H>::RConnect(nodeId start, bool can_identify_superset) {
  auto IsReachable =
      [&](nodeId source, nodeId curr, nodeId succ, Distance g_val) -> bool {
    Distance h_val = GetDirectionSensitiveHeuristicDistance(source, succ);
    return fabs(h_val - g_val) < kEpsDistance;
  };
  if (!can_identify_superset)
    ReachabilityRelationExploreCommon<G, S>::ExploreDijkstraConservative(
        start, IsReachable);
  else if (rconn_type_ == kRConnectConservative)
    ReachabilityRelationExploreCommon<G, S>::ExploreBFSConservative(
        start, IsReachable);
  else if (rconn_type_ == kRConnectAggressive)
    ReachabilityRelationExploreCommon<G, S>::ExploreBFSAggressive(
        start, IsReachable);
  else if (rconn_type_ == kRConnectSucc)
    ExploreBFSAggressiveFlags(start);
  else
    assert(false && "Invalid RConnect type for HReachability");
}
template<class G, class S, class H>
void HReachability<G, S, H>::RRefine(
    nodeId from, nodeId to, std::vector<nodeId> & path, bool append) {
  if (rconn_type_ == kRConnectSucc) // FIXME: Shouldn't be rconn_type anymore
    RRefineDFSFlags(from, to, path, append);
  else
    RRefineDFSFreespaceDistances(from, to, path, append);
}

template<class G, class S, class H>
void HReachability<G, S, H>::ExploreBFSAggressiveFlags(nodeId start) {
  CPUTimer t;
  t.StartTimer();
  Reset();

  // Use expansion order as a BFS queue
  expansion_order_.push_back(start);
  dist_.push_back(0);

  data_.GenerateNode(start);
  auto IsDuplicate =
      [&](nodeId n) -> bool {
    return !data_.GenerateNode(n);
  };

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
        st_.num_relaxed += h_->AddReverseFreespaceSuccessorsAndDistances(
            start, curr, g_val, expansion_order_, dist_, IsDuplicate) :
        st_.num_relaxed += h_->AddFreespaceSuccessorsAndDistances(
            start, curr, g_val, expansion_order_, dist_, IsDuplicate);
  }
  t.EndTimer();
  st_.time = t.GetElapsedTime();
}

template<class G, class S, class H>
void HReachability<G, S, H>::RRefineDFSFreespaceDistances(
    nodeId from, nodeId to, std::vector<nodeId> & path, bool append) {
  Reset();

  // Search backwards for two reasons:
  // 1- It is quicker to retrieve the path (no reversing necessary).
  // 2- If goal is h-reachable from the start, then it is h-reachable from any
  // node n on a shortest path from the start to the goal (assuming that the
  // heuristic is consistent).
  // It doesn't necessarily hold from start to n, but it does work for freespace
  // heuristics.

  double path_length = GetHeuristicDistance(from, to);

  // TODO: Remove kIsReachable.

  // Add goal to the open list.
  this->data_.GenerateNode(to);
  this->data_.SetFlag(to, kIsReachable);
  this->data_.SetGVal(to, 0);

  std::vector < WeightedArcHead > neighbors;
  this->stack_.push_back(to);
  while (!this->stack_.empty()) {
    nodeId n = this->stack_.back();

    if (n == from)
      break;

    this->stack_.pop_back();

    double g_val = this->data_.GetGVal(n);
    this->g_->GetPredecessors(n, neighbors);
    for (unsigned int i = 0; i < neighbors.size(); i++) {
      nodeId p = neighbors[i].target;
      if (!this->data_.IsGenerated(p)) {
        double p_g_val = g_val + neighbors[i].weight;
        double p_h_val = GetHeuristicDistance(from, p);
        if (fabs(path_length - p_g_val - p_h_val) < kEpsDistance) {
          this->data_.GenerateNode(p);
          this->data_.SetGVal(p, p_g_val);
          r_parent_[p] = n;
          this->stack_.push_back(p);
        }
      }
    }
  }

  if (this->stack_.empty())
    return;

  // Append mode means that we append the h-reachable path to 'path', except for the 'from' node.
  if (!append) {
    path.clear();
    path.push_back(from);
  }

  nodeId curr = from;
  while (curr != to) {
    curr = r_parent_[curr];
    path.push_back(curr);

/*  // Without parents
    double g_val = this->data_.GetGVal(curr);
    this->g_->GetSuccessors(curr, neighbors);
    for (unsigned int i = 0; i < neighbors.size(); i++) {
      nodeId succ = neighbors[i].target;

      if (this->data_.IsGenerated(succ)) {
        // We subtract the edge cost from the g-val because the search was performed backwards,
        // and we are extracting the path forwards ('from' has the highest g-val).
        double s_g_val = g_val - neighbors[i].weight;
        if (fabs(s_g_val - this->data_.GetGVal(succ)) < kEpsDistance) {
          curr = succ;
          path.push_back(curr);
          break;
        }
      }
    }
*/
  }
}


template<class G, class S, class H>
void HReachability<G, S, H>::RRefineDFSFlags(
    nodeId from, nodeId to, std::vector<nodeId> & path, bool append) {
  Reset();

  // Same as RRefineFreespaceDistances, but use FreespaceParents instead.

  auto IsDuplicate =
      [&](nodeId n) -> bool {
    return !data_.GenerateNode(n);
  };

  this->data_.GenerateNode(to);
  this->stack_.push_back(to);
  while (!this->stack_.empty()) {
    nodeId n = this->stack_.back();
    if (n == from) break;
    this->stack_.pop_back();
    //st_.num_relaxed +=
    h_->AddFreespaceParents(from, n, this->stack_, r_parent_, IsDuplicate);
  }

  if (this->stack_.empty()) {
    std::cout<<"F-Refine-DFS-Flags did not find a path!!!"<<std::endl;
    return;
  }

  // Append mode means that we append the h-reachable path to 'path', except for the 'from' node.
  if (!append) {
    path.clear();
    path.push_back(from);
  }

  nodeId curr = from;
  while (curr != to) {
    curr = r_parent_[curr];
    path.push_back(curr);
  }
}


template<class G, class S, class H>
int HReachability<G, S, H>::FindLastReachableIndex(std::vector<nodeId> & path,
                                                   int start, int goal) {
  auto IsReachable =
      [&](nodeId source, nodeId curr, nodeId succ, Distance g_val) -> bool {
    Distance h_val = GetHeuristicDistance(source, succ);
    return fabs(h_val - g_val) < kEpsDistance;
  };
  return ReachabilityRelationExploreCommon<G, S>::FindLastReachableIndexUsingDistance(
      path, start, goal, IsReachable);
}

template<class G, class S, class H>
bool HReachability<G, S, H>::ExploreReachableAreaFringeDijkstra(
    nodeId start, bool terminate_if_fringe, Distance exploration_bound) {
  auto IsReachable =
      [&](nodeId source, nodeId curr, nodeId succ, Distance g_val) -> bool {
    Distance h_val = GetDirectionSensitiveHeuristicDistance(source, succ);
    return fabs(h_val - g_val) < kEpsDistance;
  };

  return ReachabilityRelationExploreCommon<G, S>::ExploreReachableAreaFringeDijkstra(
      start, terminate_if_fringe, exploration_bound, IsReachable);
}
