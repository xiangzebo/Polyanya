#include "OctileDistanceRefinementPolicy.h"

#include "constants.h"
#include "graph.h"
#include "mapAbstraction.h"
#include "path.h"

OctileDistanceRefinementPolicy::OctileDistanceRefinementPolicy(
		mapAbstraction* _map) : RefinementPolicy(), map(_map)
{
}

OctileDistanceRefinementPolicy::~OctileDistanceRefinementPolicy()
{
}

path* 
OctileDistanceRefinementPolicy::refine(path* abspath)
{
	if(!abspath)
		return 0;

	path* refinedpath = 0;
	path* tail = 0;
	for(path* current = abspath; current->next != 0; 
				current = current->next)
	{
		path* segment = 0;
		path* segtail = 0;
		for(node* first = map->getNodeFromMap(
					current->n->getLabelL(kFirstData),
					current->n->getLabelL(kFirstData+1)); 
				first != 0; 
				first = nextStep(first, current->next->n))
		{
			path* p = new path(first, 0);
			if(segment == 0)
			{
				segment = segtail = p;
			}
			else
			{
				segtail->next = p;
				segtail = p;
			}
		}

		if(refinedpath == 0)
		{
			refinedpath = segment;
			tail = segtail;
		}
		else
		{
			tail->next = segment->next;
			tail = segtail;
			segment->next = 0;
			delete segment;
		}
	}	
	return refinedpath;
}

node* 
OctileDistanceRefinementPolicy::nextStep(node* first, node* last)
{
		int fx = first->getLabelL(kFirstData);
		int fy = first->getLabelL(kFirstData+1);
		int lx = last->getLabelL(kFirstData);
		int ly = last->getLabelL(kFirstData+1);

		int dx = lx - fx; 
		int dy = ly - fy;

		if(dx == 0)
		{
			if(dy < 0)
				return map->getNodeFromMap(fx, --fy);
			if(dy > 0)
				return map->getNodeFromMap(fx, ++fy);

			return 0;
		}

		if(dy == 0)
		{
			if(dx < 0)
				return map->getNodeFromMap(--fx, fy);
			if(dx > 0)
				return map->getNodeFromMap(++fx, fy);

			return 0;
		}

		if(dx < 0)
		{
			if(dy < 0)
				return map->getNodeFromMap(--fx, --fy);
			if(dy > 0)
				return map->getNodeFromMap(--fx, ++fy);
		}

		if(dy < 0)
			return map->getNodeFromMap(++fx, --fy);

		return map->getNodeFromMap(++fx, ++fy);
}

