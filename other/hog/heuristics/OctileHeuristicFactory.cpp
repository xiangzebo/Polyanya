#include "OctileHeuristicFactory.h"
#include "OctileHeuristic.h"

OctileHeuristicFactory::OctileHeuristicFactory()
{

}

OctileHeuristicFactory::~OctileHeuristicFactory()
{

}

Heuristic* OctileHeuristicFactory::newHeuristic()
{
	return new OctileHeuristic();
}
