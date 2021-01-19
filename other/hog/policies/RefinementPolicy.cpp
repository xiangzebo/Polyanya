#include "RefinementPolicy.h"

#include "mapAbstraction.h"

RefinementPolicy::RefinementPolicy()
{
	verbose = false;
}

RefinementPolicy::~RefinementPolicy()
{
}

void
RefinementPolicy::resetMetrics()
{
	searchTime = 0;
	nodesExpanded = nodesGenerated = nodesTouched = 0;
}
