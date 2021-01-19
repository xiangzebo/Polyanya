#include "DefaultRefinementPolicy.h"

#include "ReverseClusterFilter.h"
#include "ClusterNode.h"
#include "DebugUtility.h"
#include "FlexibleAStar.h"
#include "IncidentEdgesExpansionPolicy.h"
#include "OctileHeuristic.h"
#include "path.h"
#include "timer.h"

DefaultRefinementPolicy::DefaultRefinementPolicy(mapAbstraction* map)
		: RefinementPolicy(), map_(map), verbose(false)
{
}

DefaultRefinementPolicy::~DefaultRefinementPolicy()
{
}

// NB: there is bug when trying to visualise all nodes expanded during
// each refinement search:
// If a node is expanded during one search and only generated during the
// next, the node generated draw color clobbers the expanded draw
// color. Could fix this in FlexibleAStar but the visualisation is still
// not correct until we can somehow represent nodes that have been 
// expanded multiple times (once per refinement).
path*
DefaultRefinementPolicy::refine(path* abspath)
{
	ReverseClusterFilter *cf = new ReverseClusterFilter();
	IncidentEdgesExpansionPolicy* policy = new IncidentEdgesExpansionPolicy(map_);
	policy->addFilter(cf);
	FlexibleAStar *astar = new FlexibleAStar(policy, new OctileHeuristic());
	astar->verbose = false; 
	astar->markForVis = false;

	Timer t;
	t.startTimer();
	//std::cout << "refining path: \n";
	//DebugUtility debuug(map_, astar->getHeuristic());
	//debuug.printPath(abspath); 
	//std::cout << std::endl;
	if(abspath == 0)
		return 0;

	path* thepath = 0;
	path* tail = 0;
	for(path* current = abspath; current->next != 0; current = current->next)
	{
		node* start = map_->getNodeFromMap(
				current->n->getLabelL(kFirstData), 
				current->n->getLabelL(kFirstData+1));
		node* goal =  map_->getNodeFromMap(
				current->next->n->getLabelL(kFirstData), 
				current->next->n->getLabelL(kFirstData+1));

		// limit search to the two clusters the start and goal are located in
		if(dynamic_cast<ClusterNode*>(start))
		{
			int parentClusterId = dynamic_cast<ClusterNode*>(start)->getParentClusterId();
			if(parentClusterId != -1)
				cf->addTargetCluster(parentClusterId);
		}
		if(dynamic_cast<ClusterNode*>(goal))
		{
			int parentClusterId = dynamic_cast<ClusterNode*>(goal)->getParentClusterId();
			if(parentClusterId != -1)
				cf->addTargetCluster(parentClusterId);
		}

		path* segment = astar->getPath(map_, start, goal); 

		nodesExpanded += astar->getNodesExpanded();
		nodesTouched += astar->getNodesTouched();
		nodesGenerated += astar->getNodesGenerated();

		if(verbose) 
		{
			std::cout << "refined segment: "<<std::endl; 
			DebugUtility debug(map_, astar->getHeuristic());
			debug.printPath(segment); 
			std::cout << " distance: "<<map_->distance(segment)<<std::endl; 
		}

		// append segment to refined path
		if(thepath == 0)
		{
			thepath = segment;										
			tail = segment->tail();
		}

		//avoid overlap between successive segments 
		//(i.e one segment ends with the same node as the next begins)
		if(tail->n->getNum() == segment->n->getNum()) 
		{
			tail->next = segment->next;
			tail = segment->tail();
			segment->next = 0;
			delete segment;
		}
	}

	searchTime = t.endTimer();
	delete astar; // also deletes policy and filter

	return thepath;
}
