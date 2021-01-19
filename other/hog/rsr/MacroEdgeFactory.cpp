#include "MacroEdgeFactory.h"

MacroEdgeFactory::MacroEdgeFactory()
{
}

MacroEdgeFactory::~MacroEdgeFactory()
{
}

MacroEdge* MacroEdgeFactory::newEdge(unsigned int fromId, unsigned int toId, 
		double weight)
{
	MacroEdge* e = new MacroEdge(fromId, toId, weight);
	return e;
}

