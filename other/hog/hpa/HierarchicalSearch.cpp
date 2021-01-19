#include "HierarchicalSearch.h"

#include "DebugUtility.h"
#include "InsertionPolicy.h"
#include "OctileHeuristic.h"
#include "path.h"
#include "RefinementPolicy.h"
#include "statCollection.h"

#include "timer.h"

HierarchicalSearch::HierarchicalSearch(InsertionPolicy* _inspol,
		searchAlgorithm* _alg, RefinementPolicy* _refpol) : searchAlgorithm()
{
	insertPolicy = _inspol;
	alg = _alg;
	name = alg->getName();
	refinePolicy = _refpol;

}

HierarchicalSearch::~HierarchicalSearch()
{
	delete insertPolicy;
	delete alg;
	delete refinePolicy;
}


void
HierarchicalSearch::setName(const char* _name)
{
	name = _name;
}


path* 
HierarchicalSearch::getPath(graphAbstraction *aMap, node *from, 
		node *to, reservationProvider *rp)
{
	resetMetrics();
	alg->verbose = verbose;

	node* start = insertPolicy->insert(from);
	node* goal = insertPolicy->insert(to);

	path* abspath = alg->getPath(aMap, start, goal, rp);
	path* refinedPath = refinePolicy->refine(abspath);
	delete abspath;

	insertPolicy->remove(start);
	insertPolicy->remove(goal);

	nodesExpanded = alg->getNodesExpanded() + 
		insertPolicy->getNodesExpanded() + refinePolicy->getNodesExpanded();
	nodesGenerated = alg->getNodesGenerated() + 
		insertPolicy->getNodesGenerated() + refinePolicy->getNodesGenerated();
	nodesTouched = alg->getNodesTouched() + 
		insertPolicy->getNodesTouched() + refinePolicy->getNodesTouched();
	searchTime = alg->getSearchTime() + 
		insertPolicy->getSearchTime() + refinePolicy->getSearchTime();

	if(verbose)
	{
		if(refinedPath)
		{
			std::cout << "refined path: \n";
			OctileHeuristic heuristic;
			DebugUtility debug(aMap, &heuristic);
			debug.printPath(refinedPath);
		}
	}

	return refinedPath;
}

void 
HierarchicalSearch::resetMetrics()
{
	if(insertPolicy)
		insertPolicy->resetMetrics();
	if(refinePolicy)
		refinePolicy->resetMetrics();

	nodesExpanded = 0;
	nodesTouched = 0;
	nodesGenerated = 0;
	searchTime = 0;
}

long 
HierarchicalSearch::getInsertNodesExpanded() 
{ 
	return insertPolicy->getNodesExpanded(); 
}

long 
HierarchicalSearch::getInsertNodesTouched() 
{
   	return insertPolicy->getNodesTouched(); 
}

long 
HierarchicalSearch::getInsertNodesGenerated() 
{ 
	return insertPolicy->getNodesGenerated(); 
}

double 
HierarchicalSearch::getInsertSearchTime() 
{ 
	return insertPolicy->getSearchTime(); 
}

void 
HierarchicalSearch::logFinalStats(statCollection* stats)
{
	stats->addStat("nodesExpanded",getName(),alg->getNodesExpanded());
	stats->addStat("nodesTouched",getName(),alg->getNodesTouched());
	stats->addStat("nodesGenerated",getName(),alg->getNodesGenerated());
	stats->addStat("searchTime",getName(),alg->getSearchTime());

	stats->addStat("insNodesExpanded",getName(),getInsertNodesExpanded());
	stats->addStat("insNodesTouched",getName(),getInsertNodesTouched());
	stats->addStat("insNodesGenerated",getName(),getInsertNodesGenerated());
	stats->addStat("insSearchTime",getName(),getInsertSearchTime());
}
