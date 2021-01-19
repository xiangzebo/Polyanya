#include "CorridorFilter.h"

#include <cassert>

CorridorFilter::CorridorFilter()
{
	corridorNodes = new std::map<int, node*>();
}

CorridorFilter::~CorridorFilter()
{
}

bool CorridorFilter::filter(node* n)
{
	if(corridorNodes->size() == 0) 
		return false; // no corridor set; filter nothing.
	
	if(corridorNodes->find(n->getUniqueID()) == corridorNodes->end()) 
		return true; // node not in corridor; filter it out

	return false;
}

void CorridorFilter::setCorridorNodes(std::map<int, node*>* _nodes) 
{ 
	corridorNodes = _nodes; 
	assert(corridorNodes == _nodes); 
}
