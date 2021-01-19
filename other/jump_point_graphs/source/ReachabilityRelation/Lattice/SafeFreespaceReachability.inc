/*
 * SafeFreespaceReachability.inc
 *
 *  Created on: Mar 11, 2018
 *      Author: idm-lab
 */

// TODO: DFS instead?
template<class G, class S, class H>
void SafeFreespaceReachability<G, S, H>::RConnect(nodeId start, bool can_identify_superset) {
  CPUTimer t;
  t.StartTimer();

  this->Reset();

//  std::queue < nodeId > dsfr_queue;
  std::vector < WeightedArcHead > neighbors;

  // Add start to the dhr_stack.
  this->data_.GenerateNode(start);
  this->data_.SetGVal(start, 0);
  this->expansion_order_.push_back(start);

  int c = 0;
  while (c < this->expansion_order_.size()) {
    nodeId curr = this->expansion_order_[c];
    c++;

    // If we find a non-start subgoal, make a note and don't expand it.
    if (curr != start && this->sm_->IsSubgoal(curr)) {
      this->AddToExistingSubgoals(curr);
      continue;
    }

    // Expand the non-subgoal (or start) node.
    double g_curr = this->data_.GetGVal(curr);
    this->GetDirectionSensitiveSuccessors(curr, neighbors);
    for (unsigned int i = 0; i < neighbors.size(); i++) {
      this->st_.num_relaxed ++;
      nodeId succ = neighbors[i].target;
//      if (data_.IsGenerated(succ) && data_.GetFlag(succ, kIsClosed))
//        continue;

      double new_g_val = g_curr + neighbors[i].weight;
      double f_dist = GetDirectionSensitiveHeuristicDistance(start, succ);
      if (fabs(f_dist - new_g_val) > kEpsDistance)
        continue;

      this->data_.GenerateNode(succ);
      this->data_.SetGVal(succ, new_g_val);
      this->data_.IncrementNumPred(succ);

      int num_pred = GetDirectionSensitiveNumPred(start, succ);
      if (num_pred == this->data_.GetNumPred(succ)) {
        this->expansion_order_.push_back(succ);
      }
    }
  }
  t.EndTimer();
  this->st_.time = t.GetElapsedTime();
}
template<class G, class S, class H>
void SafeFreespaceReachability<G, S, H>::RRefine(
    nodeId from, nodeId to, std::vector<nodeId> & path, bool append) {
  if (!append) {
    path.clear();
    path.push_back(from);
  }
  nodeId curr = from;
  Distance remaining_g = GetHeuristicDistance(from, to);
  while (curr != to) {
    bool next_found = false;
    for (auto a: this->g_->GetSuccessors(curr)) {
      Distance g = remaining_g - a.weight;
      if (fabs(GetHeuristicDistance(a.target, to) - g) < kEpsDistance) {
        curr = a.target;
        remaining_g = g;
        path.push_back(curr);
        next_found = true;
        break;
      }
    }
    if (!next_found)
      return;
  }
}

template<class G, class S, class H>
bool SafeFreespaceReachability<G, S, H>::IsReachable(
    nodeId start, nodeId goal, Distance d) {
  // TODO: Not tested

  bool is_reachable = true;
  std::vector < nodeId > stack;
  stack.push_back(start);
  this->data_.GenerateNode(start);

  while (!stack.empty()) {
    nodeId curr = stack.back();
    stack.pop_back();
    for (auto a : this->g_->GetSuccessors(curr)) {
      nodeId succ = a.target;
      if (fabs(
          h_->GetHeuristicDistance(start, succ)
              + h_->GetHeuristicDistance(succ, goal)
              - h_->GetHeuristicDistance(start, goal)) < kEpsDistance) {

        if (!this->data_.IsGenerated(succ)) {
          this->data_.GenerateNode(succ);
          this->data_.SetNumPred(succ, 1);
        }
        else
          this->data_.IncrementNumPred(succ);

        if (h_->GetNumPred(start, succ) == this->data_.GetNumPred(succ)) {
          if (succ == goal)
            return true;
          else
            stack.push_back(succ);
        }
      }
    }
  }
  return false;
}

template<class G, class S, class H>
int SafeFreespaceReachability<G, S, H>::FindLastReachableIndex(std::vector<nodeId> & path,
                                                   int start, int goal) {
  int curr = start+1;

  // We determine the reachabilities of nodes incrementally.
  // We reset the data once, and reachability checks of new nodes reuse
  // information from previous checks.
  this->Reset();
  //data_.SetFlag(path[curr], kIsReachable);
  while (curr < goal) {
    // path[curr] is R-reachable path[start].
    // Check if path[curr+1] is also R-reachable from path[start]
    int next = curr+1;

    bool is_reachable = true;
    std::vector<nodeId> stack;
    stack.push_back(path[next]);
    this->data_.GenerateNode(path[next]);

    while (!stack.empty()) {
      nodeId n = stack.back();
      stack.pop_back();
      int pred_count = 0;
      for (auto a : this->g_->GetPredecessors(n)) {
        if (fabs(
            h_->GetHeuristicDistance(path[start], a.target) + a.weight
                - h_->GetHeuristicDistance(path[start], n)) < kEpsDistance) {
          pred_count++;

          if (!this->data_.IsGenerated(a.target)) {
            this->data_.GenerateNode(a.target);
            stack.push_back(a.target);
          }
        }
      }
      if (pred_count != h_->GetNumPred(path[start], n)) {
        is_reachable = false;
        break;
      }
    }
    if (!is_reachable) {
      // Curr is reachable from start but curr+1 is not, so curr is the last
      // reachable node on the path.
      // TODO: Skip over reachable vertices as well?
      //   Assuming the given path is a shortest path, this is not necessary.
      return curr;
    }
    else {
      curr = next;
    }
  }
  return curr;
}


template<class G, class S, class H>
bool SafeFreespaceReachability<G, S, H>::ExploreReachableAreaFringeDijkstra(
    nodeId start, bool terminate_if_fringe, Distance exploration_bound) {
  auto IsReachable =
      [&](nodeId source, nodeId curr, nodeId succ, Distance g_val) -> bool {
        Distance h_val = GetDirectionSensitiveHeuristicDistance(source, succ);
        int num_pred = GetDirectionSensitiveNumPred(source, succ);
        return fabs(h_val - g_val) < kEpsDistance &&
            this->data_.GetNumPred(succ) == num_pred;
      };
  auto OnStrictlyBetterPath = [&](nodeId curr, nodeId succ)->void {
    this->data_.SetNumPred(succ, 0);
  };
  auto OnSymmetricOrStrictlyBetterPath = [&](nodeId curr, nodeId succ)->void {
    if (this->data_.IsReachable(curr))
      this->data_.IncrementNumPred(succ);
  };

  return ReachabilityRelationExploreCommon<G, S>::ExploreReachableAreaFringeDijkstra(
      start, terminate_if_fringe, exploration_bound, IsReachable,
      OnStrictlyBetterPath, OnSymmetricOrStrictlyBetterPath);
}

