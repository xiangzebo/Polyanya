#include "JumpPointRefinementPolicy.h"

#include "graph.h"
#include "JumpPointExpansionPolicy.h"
#include "mapAbstraction.h"
#include "OnlineJumpPointLocator.h"
#include "OctileDistanceRefinementPolicy.h"
#include "path.h"

#include <cstdlib>

JumpPointRefinementPolicy::JumpPointRefinementPolicy(mapAbstraction* _map,
		int _maxdepth) : RefinementPolicy(), map(_map), maxdepth(_maxdepth)
{
	this->jpl = new OnlineJumpPointLocator(_map);
	this->maxdepth = _maxdepth;
}

JumpPointRefinementPolicy::~JumpPointRefinementPolicy()
{
	delete jpl;
}

path*
JumpPointRefinementPolicy::refine(path* abspath)
{
	if(!abspath || !abspath->next)
		return 0;

	if(verbose)
	{
		std::cout << "refining abstract jump point path. maxdepth: "<<
			maxdepth<<std::endl;
	}

	Jump::Direction which = Jump::NONE;
	path* retVal = recurse(which, abspath->n, abspath->next->n, 0);
	if(retVal == 0)
	{
		std::cerr << "couldn't refine segment. cannot continue.\n";
		exit(1);
	}
	path* tail = retVal->tail();

	abspath = abspath->next;
	while(abspath->next)
	{
		if(verbose)
		{
			std::cout << "refining segment ("<<
				abspath->n->getLabelL(kFirstData)<<", "<<
				abspath->n->getLabelL(kFirstData+1)<<") (";
			std::cout << abspath->next->n->getLabelL(kFirstData)<<", "<<
				abspath->next->n->getLabelL(kFirstData+1)<<")" <<std::endl;
		}

		path *segment = recurse(which, abspath->n, abspath->next->n, 1);
		if(segment == 0)
		{
			std::cerr << "couldn't refine segment. cannot continue.\n";
			exit(1);
		}

		// append the new segment to the refined path and delete the overlapping
		// node that appears at the end of one and beginning of the other.
		tail->next = segment->next;
		segment->next = 0;
		delete segment;

		tail = tail->tail();
		abspath = abspath->next;
	}

	// Now that we have a path of jump points that are reachanble by travelling
	// straight from one to the next, refine that path
	path* tmp = retVal;
	OctileDistanceRefinementPolicy rpol(map);
	retVal = rpol.refine(retVal);
	delete tmp;
	return retVal;
}


path*
JumpPointRefinementPolicy::recurse(Jump::Direction d, node* current, 
		node* target, int depth)
{
	if(verbose)
	{
		std::cout << "current: ("<<current->getLabelL(kFirstData)<<", "<<
			current->getLabelL(kFirstData+1)<<") direction: "<<d 
			<< " depth: "<<depth<<std::endl;
	}

	if(depth == maxdepth)
	{
		if(verbose)
			std::cout << "not expanding; max depth reached.\n";
		return 0;
	}

	int x = current->getLabelL(kFirstData);
	int y = current->getLabelL(kFirstData+1);
	int nx = target->getLabelL(kFirstData);
	int ny = target->getLabelL(kFirstData+1);

	// generate the set of immediately adjacent jump points
	// (we branch on these)
	int branching = 0;
	std::vector<node*> jplist;
	int successors = jpl->computeSuccessors(d, x, y);
	for(int i=1; i <= 128; i*=2)
	{
		if(i & successors)
		{
			node* jp_next = jpl->findJumpNode((Jump::Direction)i, x, y, nx, ny);
			
			if(jp_next)
			{
				jplist.push_back(jp_next);
				branching |= i;
			}
		}
	}

	if(jplist.size() > 1)
		depth++;

	// recurse over each adjacent jump point until the target node
	// (or maximum depth) is reached.
	int j = 0;
	for(int i=1; i <= 128; i*=2)
	{
		if(!(i & branching))
			continue;

		node* jp_next = jplist.at(j);
		if(jp_next->getNum() == target->getNum())
		{
			if(verbose)
				std::cout << "target node found!\n"<<std::endl;
			return new path(current, new path(jp_next, 0));
		}

		path* p = recurse((Jump::Direction)i, jp_next, target, depth);
		if(p)
		{
			p = new path(current, p);
			return p;
		}
		j++;
	}

	if(verbose)
	{
		std::cout << "node ("<<current->getLabelL(kFirstData)<<", "<<
			current->getLabelL(kFirstData+1)<<") is a dead end."<<std::endl;
	}
	return 0;
}


