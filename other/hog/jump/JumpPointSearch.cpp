#include "JumpPointSearch.h"

#include "FlexibleAStar.h"
#include "Heuristic.h"
#include "JumpPointAbstraction.h"
#include "JumpPointExpansionPolicy.h"
#include "OfflineJumpPointLocator.h"
#include "OnlineJumpPointLocator.h"
#include "RecursiveJumpPointExpansionPolicy.h"

#include "timer.h"

#include <cstdlib>
#include <iostream>
#include <sstream>

JumpPointSearch::JumpPointSearch(Heuristic* _heuristic, mapAbstraction* _map,
	   	bool _online, unsigned int _maxdepth, unsigned int jumplimit)
	: searchAlgorithm(), online(_online), maxdepth(_maxdepth), 
	heuristic(_heuristic), map(_map)
{
	JumpPointLocator* jpl = 0;
	ExpansionPolicy* expander = 0;
	if(online)
	{
		jpl = new OnlineJumpPointLocator(map);
		name.append("JPS");
	}	
	else
	{
		jpl = new OfflineJumpPointLocator(
				dynamic_cast<JumpPointAbstraction*>(map));
		name.append("JPA");
	}

	jpl->setLimit(jumplimit);
	std::stringstream ss;
	ss << "R" << maxdepth;

	if(jumplimit != INT_MAX)
	{
		ss << "J" << jumplimit;
	}
	name.append(ss.str());

	if(maxdepth == 0)
	{
		expander = new JumpPointExpansionPolicy(jpl);
	}
	else
	{
		expander = new RecursiveJumpPointExpansionPolicy(jpl, maxdepth);
	}
	expander->verbose = verbose;
	astar = new FlexibleAStar(expander, heuristic);
}

JumpPointSearch::~JumpPointSearch()
{
	delete astar;
}

path* 
JumpPointSearch::getPath(graphAbstraction *aMap, node *start,
		node *goal, reservationProvider *rp)
{
	resetMetrics();
	astar->verbose = verbose;

	path* thepath = astar->getPath(aMap, start, goal, rp);
	if(thepath && maxdepth > 0)
	{
		addIntermediateNodes(thepath);
	}

	searchTime += astar->getSearchTime();
	nodesExpanded = astar->getNodesExpanded();
	nodesTouched = astar->getNodesTouched();
	nodesGenerated = astar->getNodesGenerated();

	return thepath;
}

void
JumpPointSearch::addIntermediateNodes(path* thepath)
{
	if(verbose)
		std::cout << "JumpPointSearch::addIntermediateNodes\n";

	RecursiveJumpPointExpansionPolicy* expander;
	expander = dynamic_cast<RecursiveJumpPointExpansionPolicy*>(
			astar->getExpander());
	assert(expander);

	Timer t;
	t.startTimer();
	while(thepath && thepath->next)
	{
		// re-expand thepath->n
		expander->expand(thepath->n);
		for(node* n = expander->first(); n != 0; n = expander->next())
		{
			// find thepath->next->n among its neighbours
			if(&*n == &*(thepath->next->n))
				break;
		}
		assert(expander->n());
		nodesExpanded++;

		// get the intermediate nodes that were skipped on the way to
		// thepath->next->n because their branching factor equaled 1.
		JumpInfo* info = expander->getJumpInfo();
		assert(info);
		if(verbose)
		{
			std::cout << "expanding ("<<thepath->n->getLabelL(kFirstData)
				<<", "<<thepath->n->getLabelL(kFirstData+1)<<") ";
			info->print(std::cout);
		}

		// insert each intermediate jump point into the path
		// (the last node == thepath->n, so skip it)
		for(unsigned int i=0; i < info->nodecount() - 1; i++)
		{
			path* newnext = new path(info->getNode(i), thepath->next);
			thepath->next = newnext;
			thepath = newnext;
		}
		thepath = thepath->next;
	}
	searchTime += t.endTimer();
}

void 
JumpPointSearch::resetMetrics()
{
	nodesExpanded = 0;
	nodesTouched = 0;
	nodesGenerated = 0;
	searchTime = 0;
}
