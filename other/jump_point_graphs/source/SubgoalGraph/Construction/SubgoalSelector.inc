template<class G>
SubgoalSelector<G>::SubgoalSelector(G* graph, SubgoalSelectionStrategy s)
    : g_(graph) {
  num_nodes_ = g_->GetNumAllNodes();
  explored_backward_ = false;

  node_data_.resize(num_nodes_);
  SetSelectionStrategy(s);
  expansion_order_.clear();
  fringe_nodes_.clear();
  candidate_list_.clear();
}

template<class G>
void SubgoalSelector<G>::GetParents(nodeId n, const AreaExplorationData* data,
                                    std::vector<nodeId> & parents) {
  std::vector < WeightedArcHead > neighbors;
  parents.clear();
  g_->GetPredecessors(n, neighbors);
  for (unsigned int i = 0; i < neighbors.size(); i++) {
    nodeId p = neighbors[i].target;
    if (fabs(data->GetGVal(p) + neighbors[i].weight - data->GetGVal(n))
        < 0.0001) {
      //if (data->IsHReachable(p))	// TODO: Is this necessary? Seems to cause issues with actual primitives.
      parents.push_back(p);
      //return;	// TODO: Uncommented, constructs a shortest path tree
    }
  }
}

template<class G>
void SubgoalSelector<G>::GenerateCandidates(
    const AreaExplorationData* data, const std::vector<nodeId>* fringe_nodes,
    const std::vector<nodeId>* expansion_order) {

  // TODO: DFS starting from fringe nodes?

  // Assumption: Fringe nodes and expansion_order are ordered wrt increasing
  // g-value and fringe nodes is a subsequence of expansion_order.
  // Generates all h-reachable nodes on a shortest path to a fringe node.
  // Sets up the parent/children relations.
  // Labels all fringe nodes as leaf nodes (they are still marked as candidate
  // nodes even though they cannot be subgoals).
  // Generates the processing_order_ array, that is a subsequence of expansion
  // order.

  // Start node cannot be made into a subgoal, so we do not generate a candidate for it.
  nodeId start = expansion_order->at(0);

  // Go over the reversed expansion order.
  int j = fringe_nodes->size()-1;
  for (int i = expansion_order->size() - 1; i >= 0; i--) {
    nodeId curr = expansion_order->at(i);

    // If we encounter a fringe node, mark it as such (it is also relevant,
    // since it will be in the DAG).
    // Small optimization: If a fringe node has a parent that is not R-reachable,
    // don't mark it as a fringe node since the non-R-reachable parent has to
    // have a fringe node ancestor, and covering the ancestor fringe node
    // would also cover this node.
    if (j >= 0 && curr == fringe_nodes->at(j)) {
      bool has_non_r_reachable_parent = false;
      GetParents(curr, data, node_data_[curr].parents);
      for (auto n: node_data_[curr].parents) {
        if (!data->IsReachable(n)) {
          has_non_r_reachable_parent = true;
          break;
        }
      }
      node_data_[curr].parents.clear();
      j--;

      if (has_non_r_reachable_parent)
        continue;

      node_data_[curr].is_fringe_node = true;
      node_data_[curr].is_relevant = true;
      fringe_nodes_.push_back(curr);
    }

    // If the current node is marked as relevant, mark all of its parents as
    // relevant (unless it is the start node)
    // Also, mark it as a candidate if it is h-reachable but does not have a
    // shortest path through a subgoal.
    if (node_data_[curr].is_relevant) {
      expansion_order_.push_back(curr);

      if (data->IsReachable(curr) && !data->IsCovered(curr)) {
        node_data_[curr].is_candidate = true;
        candidate_list_.push_back(curr);
      }

      GetParents(curr, data, node_data_[curr].parents);
      unsigned int k = 0;
      while (k < node_data_[curr].parents.size()) {
        // Special treatment for the start node: Remove it from the parents list.
        if (node_data_[curr].parents[k] == start) {
          node_data_[curr].parents[k] = node_data_[curr].parents.back();
          node_data_[curr].parents.pop_back();
        }
        else {
          node_data_[node_data_[curr].parents[k]].is_relevant = true;
          k++;
        }
      }
    }
  }

  for (unsigned int i = 0; i < expansion_order_.size(); i++) {
    nodeId curr = expansion_order_[i];
    for (unsigned int p = 0; p < node_data_[curr].parents.size(); p++) {
      nodeId parent = node_data_[curr].parents[p];
      node_data_[parent].children.push_back(curr);
    }
  }

  // Reverse, so that candidate list and remaining fringe nodes are
  // subsequences of expansion order. (???)
  std::reverse(expansion_order_.begin(), expansion_order_.end());
  std::reverse(fringe_nodes_.begin(), fringe_nodes_.end());
  std::reverse(candidate_list_.begin(), candidate_list_.end());
}

template<class G>
void SubgoalSelector<G>::UpdateAfterPlacingSubgoal(nodeId subgoal) {

  // STEP 1:
  // Do a depth-first-search to identify the cleared fringe nodes.
  // The DFS only clears the is_candidate flags.
  std::vector < nodeId > stack;
  stack.push_back(subgoal);
  while (!stack.empty()) {
    nodeId curr = stack.back();
    stack.pop_back();

    for (unsigned int i = 0; i < node_data_[curr].children.size(); i++) {
      nodeId child = node_data_[curr].children[i];
      if (node_data_[child].is_relevant) {
        stack.push_back(child);
        node_data_[child].is_relevant = false;
      }
    }
  }

  // STEP 2:
  // Go over the candidate list in reverse order and eliminate nodes that
  // cannot be on a shortest path to the remaining fringe nodes.
  // Reverse order guarantees that all the children of a node are
  // processed before the node itself.
  // This step removes all unnecessary candidates, and updates the child
  // pointers of the remaining nodes.
  for (int i = expansion_order_.size() - 1; i >= 0; i--) {
    nodeId curr = expansion_order_[i];

    if (node_data_[curr].should_update_children) {
      // Remove the node's unrelevant children.
      unsigned int j = 0;
      while (j < node_data_[curr].children.size()) {
        nodeId child = node_data_[curr].children[j];
        if (!node_data_[child].is_relevant) {
          node_data_[curr].children[j] = node_data_[curr].children.back();
          node_data_[curr].children.pop_back();
        }
        else
          j++;
      }

      // If the node is not a leaf node and it no longer has any
      // candidate children, it is also no longer a candidate.
      if (!node_data_[curr].is_fringe_node && node_data_[curr].children.empty())
        node_data_[curr].is_relevant = false;
    }

    // We are at a node that is no longer a candidate.
    // Signal its parents to update their children.
    if (!node_data_[curr].is_relevant) {
      for (unsigned int j = 0; j < node_data_[curr].parents.size(); j++) {
        nodeId parent = node_data_[curr].parents[j];
        if (node_data_[parent].is_relevant)
          node_data_[parent].should_update_children = true;
      }
    }
    // Clear the should update children flags.
    // No such flags remain at the end of Step 2.
    node_data_[curr].should_update_children = false;
  }

  // STEP 3: Update the fringe node and the candidate lists.
  int w = -1;
  for (unsigned int r = 0; r < fringe_nodes_.size(); r++) {
    nodeId curr = fringe_nodes_[r];
    if (node_data_[curr].is_relevant) {
      w++;
      fringe_nodes_[w] = fringe_nodes_[r];
    }
  }
  fringe_nodes_.resize(w + 1);

  w = -1;
  for (unsigned int r = 0; r < candidate_list_.size(); r++) {
    nodeId curr = candidate_list_[r];
    if (node_data_[curr].is_relevant) {
      w++;
      candidate_list_[w] = candidate_list_[r];
    }
  }
  candidate_list_.resize(w + 1);

  // STEP 4: Update the remaining candidate list.
  // Clean up the data for nodes that are no longer candidates.
  w = -1;
  for (unsigned int r = 0; r < expansion_order_.size(); r++) {
    nodeId curr = expansion_order_[r];
    if (node_data_[curr].is_relevant) {
      w++;
      expansion_order_[w] = expansion_order_[r];
    }
    else {
      // Clean-up
      node_data_[curr].is_fringe_node = false;
      node_data_[curr].is_candidate = false;
      node_data_[curr].children.clear();
      node_data_[curr].parents.clear();
    }

    node_data_[curr].score = 0;    // TODO: Maybe not?
  }
  expansion_order_.resize(w + 1);
}

template<class G>
nodeId SubgoalSelector<G>::SelectSubgoal(const AreaExplorationData* data) {
  //kRandomCandidate, kParentOfLowestGValueFringeNode, kParentOfHighestGValueFringeNode, kSecretSauce
  switch (selection_strategy_) {
    case kRandomCandidate:
      return candidate_list_[rand() % candidate_list_.size()];
    case kParentOfLowestGValueFringeNode:
      return node_data_[fringe_nodes_[0]].parents[0];
    case kParentOfHighestGValueFringeNode:
      return node_data_[fringe_nodes_.back()].parents[0];
    default:
      break;
  }

  // Secret sauce
  nodeId best_id = 0;
  double max_score = -1;

  // Go over the candidate list in reverse order (since we are constructing
  // the values bottom up).
  for (int i = expansion_order_.size() - 1; i >= 0; i--) {
    nodeId curr = expansion_order_[i];
    if (node_data_[curr].is_fringe_node) {
      node_data_[curr].score = 1.0 / (1.0 + data->GetGVal(curr));    //A
      //node_data_[curr].score = 1.0;//B
    }
    else if (node_data_[curr].is_candidate) {
      double s = node_data_[curr].score * data->GetGVal(curr);            //C
      //double s = node_data_[curr].score + data->GetGVal(curr);//D
      if (s > max_score) {
        max_score = s;
        best_id = curr;
      }
      //std::cout<<"Node "<<g_->GetNodeName(curr)<<" has score "<<s<<"."<<std::endl;
    }

    // Propagate it upwards.
    for (unsigned int j = 0; j < node_data_[curr].parents.size(); j++) {
      nodeId p = node_data_[curr].parents[j];
      node_data_[p].score = node_data_[p].score
          + node_data_[curr].score / (double) node_data_[curr].parents.size();  //E
              //node_data_[p].score = node_data_[p].score + node_data_[curr].score;//F
    }
  }

  //std::cout<<"Best node: "<<g_->GetNodeName(best_id)<<" with score "<<max_score<<std::endl;

  return best_id;
}

template<class G>
void SubgoalSelector<G>::SelectSubgoals(
    const AreaExplorationData* data, const std::vector<nodeId>* fringe_nodes,
    const std::vector<nodeId>* expansion_order, std::vector<nodeId> & subgoals,
    bool select_only_one_subgoal) {

  subgoals.clear();

#ifdef SUBGOAL_SELECTOR_DEBUG
  std::cout<<"Starting selection of subgoals..."<<std::endl;
  Debug(data);
#endif

  GenerateCandidates(data, fringe_nodes, expansion_order);

#ifdef SUBGOAL_SELECTOR_DEBUG
  std::cout<<"Candidates generated."<<std::endl;
  Debug(data);
#endif

  while (!fringe_nodes_.empty()) {
    nodeId subgoal = SelectSubgoal(data);
    subgoals.push_back(subgoal);
    UpdateAfterPlacingSubgoal(subgoal);
#ifdef SUBGOAL_SELECTOR_DEBUG
    std::cout<<"Candidates updated after placing subgoal."<<std::endl;
    Debug(data);
#endif
  }

  if (select_only_one_subgoal)
    subgoals.resize(1);
}

template<class G>
void SubgoalSelector<G>::Debug(const AreaExplorationData* data) {

//    std::cout<<"Number of relevant nodes: "<<expansion_order_.size()<<std::endl;

  int num_relevant_nodes = 0;
  for (nodeId n = 0; n < num_nodes_; n++) {
    if (node_data_[n].is_relevant) {
      num_relevant_nodes++;
    }
    else {
      assert(node_data_[n].is_fringe_node == false);
      assert(node_data_[n].is_candidate == false);
      assert(node_data_[n].parents.empty());
      assert(node_data_[n].children.empty());
    }

    assert(node_data_[n].should_update_children == false);
  }

  assert(num_relevant_nodes == expansion_order_.size());

  for (int i = 1; i < expansion_order_.size(); i++) {
    assert(
        data->GetGVal(expansion_order_[i - 1])
            < data->GetGVal(expansion_order_[i]) + 0.0001);
  }
  std::cout << "Subgoal selector debug completed successfully!" << std::endl;
}
