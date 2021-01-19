/*
 * SGSteinerPointIdentifier.h
 *
 *  Created on: Sep 18, 2016
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_SUBGOALGRAPH_SGSTEINERPOINTIDENTIFIER_H_
#define APPS_SUBGOALGRAPH_SUBGOALGRAPH_SGSTEINERPOINTIDENTIFIER_H_

#include "SubgoalGraph.h"
#include "CPUTimer.h"
#include "FreespaceReachableAreaExplorer.h"
#include <vector>
#include <cassert>

struct SteinerPointCandidate {
	SteinerPointCandidate() {
		num_subgoals_h_reachable_from = 0;
		num_subgoals_h_reachable_to = 0;
		num_covered_directed_edges = 0;
	}

	unsigned int num_subgoals_h_reachable_from, num_subgoals_h_reachable_to, num_covered_directed_edges;
};


// TODO: Add S as a template argument (subgoal ID mapper).
template <class G, class H>
class SGSteinerPointIdentifier {
public:
	SGSteinerPointIdentifier(G* g, SubgoalGraph* sg, FreespaceReachableAreaExplorer<G,SubgoalGraph,H>* explorer)
	:g_(g), sg_(sg), explorer_(explorer)
	{
		assert(g_ != NULL);
		assert(sg_ != NULL);
		assert(explorer_ != NULL);

		candidates_.resize(g_->GetNumNodes());

		delta_edge_threshold_ = 1;
	}

	// If the addition of a Steiner point p reduces the total number of edges by t,
	// only make it a Steiner point if t >= delta_edge_threshold_.
	void SetDeltaEdgeThresholdForSteinerPoint(int threshold) {delta_edge_threshold_ = threshold;}
	int GetDeltaEdgeThresholdForSteinerPoint() const {return delta_edge_threshold_;}

	// Returns the time to identify all Steiner points.
	double IdentifySteinerPoints();

private:
	G* g_;
	SubgoalGraph* sg_;
	FreespaceReachableAreaExplorer<G,SubgoalGraph,H>* explorer_;
	std::vector<SteinerPointCandidate> candidates_;

	int delta_edge_threshold_;

	void CalculateInitialCandidateData();
	void AddSteinerPointAndUpdateCandidateData(nodeId n);
	nodeId ChooseNextSteinerPoint();

	void ExtractNodesOnShortestPath(nodeId from, nodeId to, std::vector<nodeId> & nodes);

	// Explores the direct-h-reachable area.
	// For each non-subgoal that is direct-h-reachable from n, increments num_subgoals_h_reachable_from counter for that node.
	// For each subgoals s that is direct-h-reachable from n, identifies all the nodes on a shortest path from n to s,
	// and increments num_covered_directed_edges counters for those nodes.
	// If decrement_instead is true, all the counters are decremented instead.
	void ExploreForwardAndUpdateCandidateData(nodeId n, bool decrement_instead = false);

	// Explores the direct-h-reachable area backwards (that is, explores all nodes x such that n is h-reachable from x.
	// For each non-subgoal x such that n is h-reachable from x, increments the num_subgoals_h_reachable_to counter for x.
	// If decrement_instead is true, the counter is decremented instead.
	void ExploreBackwardAndUpdateCandidateData(nodeId n, bool decrement_instead = false);
};

#include "SGSteinerPointIdentifier.inc"

#endif /* APPS_SUBGOALGRAPH_SUBGOALGRAPH_SGSTEINERPOINTIDENTIFIER_H_ */
