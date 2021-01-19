#include "RSRSearch.h"

#include "EmptyClusterAbstraction.h"
#include "EmptyClusterInsertionPolicy.h"
#include "ExpansionPolicy.h"
#include "FlexibleAStar.h"
#include "Heuristic.h"
#include "IncidentEdgesExpansionPolicy.h"
#include "RRExpansionPolicy.h"


RSRSearch::RSRSearch(bool _bfReduction, Heuristic* _heuristic) 
	: searchAlgorithm()
{
	bfReduction = _bfReduction;
	heuristic = _heuristic;
	inserter = 0;
	alg = 0;
	name.append("RSR");

	assert(heuristic);
}

RSRSearch::~RSRSearch()
{
	delete inserter;
	delete alg;
}

path*
RSRSearch::getPath(graphAbstraction* aMap, node* from, node* to, 
		reservationProvider *rp)
{
	path* retVal = 0;

	// need an empty rectangle decomposition to proceed
	if(!dynamic_cast<EmptyClusterAbstraction*>(aMap))
	{
		std::cout << "RSRSearch::getPath only works on maps of type "
			"EmptyClusterAbstraction. Aborting.";
		return retVal;
	}
	else 
	{
		if(inserter == 0)
		{
			inserter = new EmptyClusterInsertionPolicy(
					dynamic_cast<EmptyClusterAbstraction*>(aMap));
		}
		if(alg == 0)
		{
			ExpansionPolicy* expander = 0;
			if(bfReduction)
			{
				expander = new RRExpansionPolicy(
						dynamic_cast<EmptyClusterAbstraction*>(aMap));
			}
			else
			{
				expander = new IncidentEdgesExpansionPolicy(aMap);
			}
			alg = new FlexibleAStar(expander, heuristic);
		}
	}


	// reset metrics
	nodesExpanded = nodesTouched = nodesGenerated = 0;
	searchTime = 0;
	inserter->resetMetrics();

	// avoid search if S+G are from the same empty cluster
	retVal = getClusterPath(from, to);
	if(retVal)
		return retVal;

	// insert S+G
	node* start = inserter->insert(from);
	node* goal = inserter->insert(to);
	alg->verbose = verbose;

	// search
	retVal = alg->getPath(aMap, start, goal);
	inserter->remove(start);
	inserter->remove(goal);
		
	// record some stats
	searchTime = inserter->getSearchTime() + alg->getSearchTime();
	nodesTouched = inserter->getNodesTouched() + alg->getNodesTouched();
	nodesGenerated = inserter->getNodesGenerated() + alg->getNodesGenerated();

	return retVal;
}

// constructs a single-edge path between two nodes if they're located in the 
// same empty cluster.
// typically called only when @param _from is the start goal and @param _to
// is the goal node -- to try and avoid any insertion or search.
path*
RSRSearch::getClusterPath(node* _from, node* _to)
{
	ClusterNode* from = dynamic_cast<ClusterNode*>(_from);
	ClusterNode* to = dynamic_cast<ClusterNode*>(_to);

	assert(from);
	assert(to);

	path *p = 0;
	if(from->getParentClusterId() == to->getParentClusterId())
	{
		to->backpointer = from;
		p = new path(from, new path(to, 0));
	}
	return p;
}
