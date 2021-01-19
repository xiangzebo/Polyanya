#include "ProblemInstance.h"
#include "graph.h"
#include "Heuristic.h"
#include "mapAbstraction.h"

ProblemInstance::ProblemInstance(node* start, node* goal, mapAbstraction* map,
		Heuristic* heuristic)
{
	assert(start && goal);
	assert(map);
	assert(heuristic);

	this->start = start;
	this->goal = goal;
	this->map = map;
	this->heuristic = heuristic;
}

ProblemInstance::~ProblemInstance()
{
}

ProblemInstance*
ProblemInstance::clone()
{
	return new ProblemInstance(start, goal, map, heuristic);
}

node* 
ProblemInstance::getGoalNode()
{
	return goal;
}

node* 
ProblemInstance::getStartNode()
{
	return start;
}

mapAbstraction* 
ProblemInstance::getMap()
{
	return map;
}

Heuristic*
ProblemInstance::getHeuristic()
{
	return heuristic;
}
