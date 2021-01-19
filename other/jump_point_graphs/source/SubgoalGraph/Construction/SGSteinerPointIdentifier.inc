/*
 * SGSteinerPointIdentifier.inc
 *
 *  Created on: Sep 18, 2016
 *      Author: idm-lab
 */

template <class G, class H>
double SGSteinerPointIdentifier<G,H>::IdentifySteinerPoints() {
	CPUTimer t;
	t.StartTimer();
	int num_seconds = 0;
	int num_steiner_points = 0;

	std::cout<<"Setting up Steiner point data.."<<std::endl;
	CalculateInitialCandidateData();
	t.EndTimer();
	std::cout<<"Steiner point data set up in "<<t.GetElapsedTime()<<" seconds."<<std::endl;
	num_seconds = t.GetElapsedTime();

	while (true) {
		t.EndTimer();
		if (t.GetElapsedTime() >= num_seconds) {
			std::cout   <<"Identified "<<num_steiner_points<<" new Steiner points."
						<<"\tTime: "<<num_seconds<<" seconds."<<std::endl;
			num_seconds++;
		}

		nodeId st = ChooseNextSteinerPoint();
		if (st != kNonNode)
			AddSteinerPointAndUpdateCandidateData(st);
		else
			break;

		num_steiner_points++;
	}

	t.EndTimer();
	return t.GetElapsedTime();
}

template <class G, class H>
void SGSteinerPointIdentifier<G,H>::CalculateInitialCandidateData() {
	std::vector<nodeId> subgoals;
	sg_->GetSubgoals(subgoals);

	for (unsigned int i = 0; i < subgoals.size(); i++) {
		nodeId curr = subgoals[i];
		ExploreForwardAndUpdateCandidateData(curr);
		ExploreBackwardAndUpdateCandidateData(curr);
	}
}

template <class G, class H>
void SGSteinerPointIdentifier<G,H>::AddSteinerPointAndUpdateCandidateData(nodeId n) {
	// Identify all the subgoals that are h-reachable from n, and those that n is h-reachable from (relevant subgoals).
	explorer_->SetExploreForward();
	explorer_->ExploreDirectReachableArea(n, sg_);
	std::vector<nodeId> subgoals_h_reachable_to = *explorer_->GetExistingSubgoals();

	explorer_->SetExploreBackward();
	explorer_->ExploreDirectReachableArea(n, sg_);
	std::vector<nodeId> subgoals_h_reachable_from = *explorer_->GetExistingSubgoals();

	// Update the existing data by undoing the relevant subgoals' data updates.
	bool decrement_instead = true;
	for (unsigned int i = 0; i < subgoals_h_reachable_to.size(); i++) {
		nodeId s = subgoals_h_reachable_to[i];
		ExploreBackwardAndUpdateCandidateData(s, decrement_instead);
	}
	for (unsigned int i = 0; i < subgoals_h_reachable_from.size(); i++) {
		nodeId s = subgoals_h_reachable_from[i];
		ExploreForwardAndUpdateCandidateData(s, decrement_instead);
	}

	// Make n a subgoal.
	sg_->AddSubgoal(n);

	// Update the data again wrt the relevant subgoals and the new subgoal, n.
	decrement_instead = false;
	ExploreBackwardAndUpdateCandidateData(n, decrement_instead);
	ExploreForwardAndUpdateCandidateData(n, decrement_instead);

	for (unsigned int i = 0; i < subgoals_h_reachable_to.size(); i++) {
		nodeId s = subgoals_h_reachable_to[i];
		ExploreBackwardAndUpdateCandidateData(s, decrement_instead);
	}
	for (unsigned int i = 0; i < subgoals_h_reachable_from.size(); i++) {
		nodeId s = subgoals_h_reachable_from[i];
		ExploreForwardAndUpdateCandidateData(s, decrement_instead);
	}
}

template <class G, class H>
nodeId SGSteinerPointIdentifier<G,H>::ChooseNextSteinerPoint() {
	nodeId best_node = kNonNode;
	int max_score = delta_edge_threshold_;

	for (nodeId n = 0; n < candidates_.size(); n++) {
		if (sg_->IsSubgoal(n))
			continue;

		// Delta number of edges if we make n a new subgoal.
		int score = candidates_[n].num_covered_directed_edges -
				(candidates_[n].num_subgoals_h_reachable_from + candidates_[n].num_subgoals_h_reachable_to);
		if (score > max_score) {
			max_score = score;
			best_node = n;
		}
	}
	return best_node;
}

// Assumes that the explorer has done a forward direct-h-reachable area exploration from node 'from', and 'to' is direct-h-reachable from 'from'.
template <class G, class H>
void SGSteinerPointIdentifier<G,H>::ExtractNodesOnShortestPath(nodeId from, nodeId to, std::vector<nodeId> & nodes_on_shortest_path) {
	const AreaExplorationData* explorer_data = explorer_->GetData();

    static std::vector<bool> visited(g_->GetNumNodes(), false);	// TODO: fix this hacky part.
    std::vector<nodeId> stack;
    nodes_on_shortest_path.clear();

    stack.push_back(to);
    visited[to] = true;

    std::vector<WeightedArcHead> predecessors;
    while(!stack.empty()) {
        nodeId curr = stack.back();
        stack.pop_back();
        nodes_on_shortest_path.push_back(curr);
        g_->GetPredecessors(curr, predecessors);

        for (unsigned int i = 0; i < predecessors.size(); i++) {
            nodeId pred = predecessors[i].target;
            // If the predecessor is on a shortest path to the current node, extract the edge,
            // and add the predecessor to the stack (if not already visited).
            if (!explorer_data->IsGenerated(pred))
            	continue;

            if (fabs(explorer_data->GetGVal(pred) + predecessors[i].weight - explorer_data->GetGVal(curr)) < 0.001) {
                if (!visited[pred]) {
                    visited[pred] = true;
                    stack.push_back(pred);
                }
            }
        }
    }

    // Clean the visited nodes.
    for (unsigned int i = 0; i < nodes_on_shortest_path.size(); i++) {
    	nodeId curr = nodes_on_shortest_path[i];
    	visited[curr] = false;
    }
}

template <class G, class H>
void SGSteinerPointIdentifier<G,H>::ExploreForwardAndUpdateCandidateData(nodeId n, bool decrement_instead) {
	explorer_->SetExploreForward();
	explorer_->ExploreDirectReachableArea(n, sg_);

	// For each non-subgoal that is direct-h-reachable from n, increment num_subgoals_h_reachable_from counter for that node.
	const std::vector<nodeId>* expanded_nodes = explorer_->GetExpansionOrder();
	const AreaExplorationData* explorer_data = explorer_->GetData();

	for(unsigned int i = 0; i < expanded_nodes->size(); i++) {
		nodeId curr = expanded_nodes->at(i);
		if (explorer_data->IsDirectReachable(curr)) {
			if (decrement_instead)
				candidates_[curr].num_subgoals_h_reachable_from--;
			else
				candidates_[curr].num_subgoals_h_reachable_from++;
		}
	}

	// For each subgoals s that is direct-h-reachable from n, identify all the nodes on a shortest path from n to s,
	// and increment num_covered_directed_edges counters for those nodes.
	const std::vector<nodeId>* subgoals = explorer_->GetExistingSubgoals();
	std::vector<nodeId> nodes_on_shortest_path;
	for (unsigned int i = 0; i < subgoals->size(); i++) {
		nodeId s = subgoals->at(i);
		ExtractNodesOnShortestPath(n, s, nodes_on_shortest_path);
		for (unsigned int j = 0; j < nodes_on_shortest_path.size(); j++) {
			nodeId curr = nodes_on_shortest_path[j];
			if (decrement_instead)
				candidates_[curr].num_covered_directed_edges--;
			else
				candidates_[curr].num_covered_directed_edges++;
		}
	}
}

template <class G, class H>
void SGSteinerPointIdentifier<G,H>::ExploreBackwardAndUpdateCandidateData(nodeId n, bool decrement_instead) {
	explorer_->SetExploreBackward();
	explorer_->ExploreDirectReachableArea(n, sg_);

	// For each non-subgoal that is direct-h-reachable from n, increment num_subgoals_h_reachable_from counter for that node.
	const std::vector<nodeId>* expanded_nodes = explorer_->GetExpansionOrder();
	const AreaExplorationData* explorer_data = explorer_->GetData();

	for(unsigned int i = 0; i < expanded_nodes->size(); i++) {
		nodeId curr = expanded_nodes->at(i);
		if (explorer_data->IsDirectReachable(curr)) {
			if (decrement_instead)
				candidates_[curr].num_subgoals_h_reachable_to--;
			else
				candidates_[curr].num_subgoals_h_reachable_to++;
		}
	}
}



