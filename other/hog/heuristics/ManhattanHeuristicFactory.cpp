#include "ManhattanHeuristicFactory.h"
#include "ManhattanHeuristic.h"

ManhattanHeuristicFactory::ManhattanHeuristicFactory()
{

}

ManhattanHeuristicFactory::~ManhattanHeuristicFactory()
{

}

Heuristic* ManhattanHeuristicFactory::newHeuristic()
{
	return new ManhattanHeuristic();
}
