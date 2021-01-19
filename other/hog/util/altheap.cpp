#include "altheap.h"
#include "constants.h"
#include "fpUtil.h"
#include "HPAUtil.h"
#include "Heuristic.h"
#include "graph.h"

altheap::altheap(Heuristic* heuristic, node* goal, int s) : heap(s)
{
	this->heuristic = heuristic;
	this->goal = goal;
}

altheap::~altheap()
{

}

// Returns true if key(first) < key(second).
// In case of a tie, the method returns true if g(first) > g(second)
// and false at all other times. 
bool 
altheap::lessThan(graph_object* first, graph_object* second)
{
	if(fless(first->getKey(), second->getKey()))
		return true;
	else if(fequal(first->getKey(), second->getKey()))
	{
		double gcost_first = first->getKey() - heuristic->h(
					dynamic_cast<node*>(first), dynamic_cast<node*>(goal));	
		double gcost_second = second->getKey() - heuristic->h(
					dynamic_cast<node*>(second), dynamic_cast<node*>(goal));

		if(fgreater(gcost_first, gcost_second))
			return true;
	}	
	return false;
}

// Returns true if key(first) > key(second).
// In case of a tie, the method returns true if g(first) < g(second)
// and false at all other times. 
bool 
altheap::greaterThan(graph_object* first, graph_object* second)
{
	if(fgreater(first->getKey(), second->getKey()))
		return true;
	else if(fequal(first->getKey(), second->getKey()))
	{
		double gcost_first = first->getKey() - heuristic->h(
					dynamic_cast<node*>(first), dynamic_cast<node*>(goal));	
		double gcost_second = second->getKey() - heuristic->h(
					dynamic_cast<node*>(second), dynamic_cast<node*>(goal));

		if(fless(gcost_first, gcost_second))
			return true;
	}	
	return false;
}
