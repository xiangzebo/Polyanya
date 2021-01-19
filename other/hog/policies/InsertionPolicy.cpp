#include "InsertionPolicy.h"

#include "graph.h"

InsertionPolicy::InsertionPolicy()
{
	resetMetrics();
	insertedNodes = new std::vector<node*>();
	verbose = false;
}

InsertionPolicy::~InsertionPolicy()
{

}

void 
InsertionPolicy::resetMetrics()
{
	nodesExpanded = 0;
	nodesTouched = 0;
	nodesGenerated = 0;
	searchTime = 0;
}

void 
InsertionPolicy::addNode(node* n)
{
	insertedNodes->push_back(n);
}

// returns true if an inserted node is successfully deleted.
bool 
InsertionPolicy::removeNode(node* n)
{
	for(std::vector<node*>::iterator it = insertedNodes->begin();
			it != insertedNodes->end();
			it++)	
	{
		node* m = *it; 
		if(n->getUniqueID() == m->getUniqueID())
		{
			insertedNodes->erase(it);
			return true;
		}
	}

	return false;

}
