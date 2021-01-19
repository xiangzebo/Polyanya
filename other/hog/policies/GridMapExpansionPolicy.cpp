#include "GridMapExpansionPolicy.h"
#include "Heuristic.h"
#include "ProblemInstance.h"

GridMapExpansionPolicy::GridMapExpansionPolicy(unsigned int max)
{
	this->max = max;	
	which = 0;
}

GridMapExpansionPolicy::~GridMapExpansionPolicy()
{
}

// returns the first neighbour of the target node, resetting at the same time
// the value of the "current" neighbour.
//
// @return: first neighbour of the target node, or 0 if no neighbours exist
node* GridMapExpansionPolicy::first()
{
	which = 0;
	node* retVal = n();
	if(retVal == 0)
	{
		retVal = next();
	}

	return retVal;
}

// When all neighbours have been iterated over, 0 is returned.
node* GridMapExpansionPolicy::next()
{
	node* retVal = 0;
	while(hasNext() && retVal == 0)
	{
		which++;
		retVal = n();
	}
	return retVal;
}

bool GridMapExpansionPolicy::hasNext()
{
	if(0 <= which  && which < max)
		return true;
	return false;
}

