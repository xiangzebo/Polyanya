/*
 *  ClusterAStar.cpp
 *  hog
 *
	TODO: Move getPath implementation into Abstract class. Then, all anyone needs to do to change the behaviour of this algorithm
	is override evaluate. Could contribute this back to HOG; abstractAStar??
	
	Current setup isn't too bad though; just inherit from here and override as needed.
	
 *  Created by dharabor on 14/11/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "ClusterAStar.h"
#include "ClusterNode.h"
#include "mapAbstraction.h"
#include "MacroNode.h"
#include "timer.h"
#include "altheap.h"
#include "graph.h"

#include <ext/hash_map>
#include <sstream>

using namespace std;


bool AbstractClusterAStar::isInCorridor(node* _n)
{
/*	ClusterNode* n = static_cast<ClusterNode*>(_n);
	if(n->getParentClusterId() != corridorClusterId) 
		return false; 
	return true; */
	
	if(corridorNodes == NULL) // corridor not set. every node should be considered.
		return true;
	
	if(corridorNodes->find(_n->getUniqueID()) != corridorNodes->end()) // already added
		return true;

	return false;
}

// debugging function
void AbstractClusterAStar::printPath(path* p)
{
	if(!p)
	{
		std::cout << "search failed"<<std::endl;
		return;
	}

	graphAbstraction* aMap = this->getGraphAbstraction();
	node* goal = p->tail()->n;
	node* last = 0;
	double g=0;
	while(p)
	{
		node* n = p->n;
		
		double h = aMap->h(n, goal);
		if(last)
			g += aMap->h(n, last);
		std::cout << "0x"<<*&n<< " id: "<<n->getUniqueID()<<" node @ "<<n->getName();
		std::cout << " g: "<<g<<" f: "<<g+h<<std::endl;
		last = n;
		p = p->next;
	}		
}

/*
	1. get the current node on the open list
	2. check if node is the goal; goto 6 if yes.
	3. evaluate each neighbour of the newly opened node
		b. if neighbour is on the closed list, skip it
		a. if neighbour is already on the open list, update weights
		c. else check if node is reachable by the agent
			i. if reachable, push node on the open list
			ii. else, skip node
	4. push node on closed list
	5. if openlist is null return failure, else, goto 1
	6. return path
*/
path* ClusterAStar::getPath(graphAbstraction *aMap, node* from, node* to, reservationProvider *rp)
{
	if(verbose) 
	{
		std::cout << "getPath() mapLevel: ";
		std::cout <<from->getLabelL(kAbstractionLevel)<<std::endl;
	}
	if(!checkParameters(aMap, from, to))
	{
		nodesExpanded=0;
		nodesTouched=0;
		searchTime =0;
		nodesGenerated = 0;
		return NULL;
	}
	this->setGraphAbstraction(aMap);

	graph *g = aMap->getAbstractGraph(from->getLabelL(kAbstractionLevel));
	return search(g, from, to);
}

path* AbstractClusterAStar::search(graph* g, node* from, node* goal)
{
	nodesExpanded=0;
	nodesTouched=0;
	searchTime =0;
	nodesGenerated=0;

	// label start node cost 0 
	from->setLabelF(kTemporaryLabel, h(from, goal));
	//from->setLabelF(kTemporaryLabel, 0);
	from->markEdge(0);
	
	heap* openList = new heap(30);
	__gnu_cxx::hash_map<int, node*> closedList;
	
	openList->add(from);
	path *p = NULL;
	
	Timer t;
	t.startTimer();
	while(1) 
	{
		/* get the current node on the open list and check if it contains the goal */
		node* current = ((node*)openList->remove()); 

		if(current == goal)
		{
			p = extractBestPath(g, current->getNum());
			if(verbose)
			{	
				printNode(string("goal found! "), current);
			}
			break;
		}
		
		this->expand(current, goal, current->getEdgeIter(), current->getNumEdges(), 
				openList, closedList, g);
				
		/* check if there is anything left to search; fail if not */
		if(openList->empty())
		{
			if(verbose) std::cout << "search failed. ";
			break;
		}
	}
	searchTime = t.endTimer();
	delete openList; 
	closedList.clear();

	if(verbose)
	{
		std::cout << "\n";
		printPath(p);
	}

	return p;	
}

void
AbstractClusterAStar::expand(node* current, node* goal, edge_iterator iter, unsigned int card, 
				heap* openList, ClosedList& closedList, graph* g)
{
	if(verbose) printNode(string("expanding... "), current, goal);
	nodesExpanded++;

	/* evaluate each neighbour of the newly opened node */
	for(unsigned int i = 0; i < card; i++)
	{
		edge* e = *iter;
		assert(e);

		int neighbourid = e->getFrom()==current->getNum()?e->getTo():e->getFrom();
		ClusterNode* neighbour = static_cast<ClusterNode*>(
				g->getNode(neighbourid));

		if(evaluate(current, neighbour, e)) 
		{
			processNeighbour(current, e, goal, openList, closedList, g);
		}
		iter++;
	}

	closeNode(current, closedList);
}

void AbstractClusterAStar::closeNode(node* current, 
		ClosedList& closedList)
{
	if(markForVis)
		current->drawColor = 2; // visualise expanded

	if(verbose)
	{	
		printNode(string("closing... "), current);
		std::cout << " f: "<<current->getLabelF(kTemporaryLabel) <<std::endl;
	}
	closedList[current->getUniqueID()] = current;	

}

void AbstractClusterAStar::processNeighbour(node* current, edge* e, 
		node* to, heap* openList, ClosedList& closedList, graph* g)
{
	int neighbourid = e->getFrom()==current->getNum()?e->getTo():e->getFrom();
	ClusterNode* neighbour = static_cast<ClusterNode*>(
			g->getNode(neighbourid));
	assert(neighbour->getUniqueID() != current->getUniqueID());
	nodesTouched++;
	
	if(closedList.find(neighbour->getUniqueID()) == closedList.end()) 
	{
		// if a node on the openlist is reachable via this new edge, 
		// relax the edge (see cormen et al)
		if(openList->isIn(neighbour)) 
		{	
			if(evaluate(current, neighbour, e)) 
			{		
				if(verbose) 
				{
					printNode("\t\trelaxing...", neighbour);
					std::cout << " f: "<<neighbour->getLabelF(kTemporaryLabel);
				}

				relaxEdge(openList, g, e, current->getNum(), neighbourid, to); 
			}
			else
			{
				if(verbose)
					std::cout << "\t\tin open list but not evaluating?!?!";
			}
		}
		else
		{
			if(evaluate(current, neighbour, e)) 
			{
				if(verbose) 
				{
					printNode("\t\tgenerating...", neighbour);
				}

				neighbour->setLabelF(kTemporaryLabel, MAXINT); // initial fCost 
				neighbour->setKeyLabel(kTemporaryLabel); // store priority here 
				neighbour->reset();  // reset any marked edges 
				openList->add(neighbour);
				relaxEdge(openList, g, e, current->getNum(), neighbourid, to); 
				nodesGenerated++;

				double gParent = current->getLabelF(kTemporaryLabel) - h(current, to);
				assert(gParent >= 0);
				double gNeighbour = neighbour->getLabelF(kTemporaryLabel) - h(neighbour, to);
				assert((gParent + e->getWeight()) == gNeighbour);
			}
			else
			{
				if(verbose)
					std::cout << "\t\tnot in open list and not evaluating";
			}

		}
		if(markForVis)
			neighbour->drawColor = 1; // visualise touched
	}
	else
	{
		if(verbose) 
		{
			printNode("\t\tclosed! ", neighbour);
		}

		double fclosed = neighbour->getLabelF(kTemporaryLabel);
		double gclosed =  fclosed - h(neighbour, to);

		// alternate fcost
		double alth = h(neighbour, to);
		double altg = current->getLabelF(kTemporaryLabel) - h(current, to);

		if((altg + e->getWeight() + alth) < fclosed)
		{
			std::cout << "node "<<neighbour->getName()<<" expanded out of order! ";
			std::cout << " fClosed = "<<fclosed;
			std::cout << " fActual: "<<altg + e->getWeight() + alth;
			std::cout << " gClosed = "<<gclosed;
			std::cout << "; alternative: "<<altg+e->getWeight();
			printNode("\nfaulty node: ", neighbour, to); 
			std::cout << std::endl;
			printNode(" alt parent: ", current, to);
			std::cout << std::endl;
		}
	}
	if(verbose)
		std::cout << std::endl;
}

ClusterAStar::ClusterAStar() : AbstractClusterAStar()
{
}

ClusterAStar::~ClusterAStar()
{
}

/* evaluate()
	check if it is possible to move from the current location to an adjacent target location via some edge.
	things we look for:
		- both nodes are non null
		- both nodes are inside the corridor (if useCorridor is set)
*/
bool ClusterAStar::evaluate(node* current, node* target, edge* e)
{
	if(!current || !target)
	{
		if(verbose)
		{
//			if(!current)
//				std::cout << " ::evaluate: current not set!";
//			if(!target)
//				std::cout << " ::evaluate: current not set!";
		}
		return false;
	}
				
	if(!isInCorridor(target))
	{
		//if(verbose)
		//{
		//	std::cout << " ::evaluate: target not in corridor!";
		//}
		return false;
	}

	return true;
}

void ClusterAStar::logFinalStats(statCollection *stats)
{
	searchAlgorithm::logFinalStats(stats);
}

bool ClusterAStar::checkParameters(graphAbstraction* aMap, node* from, node* to)
{
	if(aMap == NULL)
		return false;
				
	if(!from || !to)
		return false;

	if(from->getUniqueID() == to->getUniqueID())
		return false;
		
	if(from->getLabelL(kFirstData) == to->getLabelL(kFirstData) && from->getLabelL(kFirstData+1) == to->getLabelL(kFirstData+1))
		return false;

	return true;
	
}

// defer to h function in graphAbstraction
// (implementation could differ if map is 4 or 8 connected)
double ClusterAStar::h(node* a, node* b) 
	throw(std::invalid_argument)
{
	graphAbstraction* aMap = this->getGraphAbstraction();
	return aMap->h(a, b);
	
	//return aStarOld::h(a, b);
}

void AbstractClusterAStar::printNode(string msg, node* n, node* goal)
{	
	std::cout << msg <<"addr: "<<&(*n)<<" num: "<<n->getUniqueID();
	std::cout <<" ("<<n->getLabelL(kFirstData)<<","<<n->getLabelL(kFirstData+1)<<") ";

	if(static_cast<MacroNode*>(n))
	{
		MacroNode* mp = static_cast<MacroNode*>(n)->getParent();
		if(mp)
		{
			std::cout << " mp: "<<static_cast<MacroNode*>(n)->getParent()->getName()<<" ";
		}
			if(n->getMarkedEdge())
			{
				graph* g =  getGraphAbstraction()->getAbstractGraph(n->getLabelL(kAbstractionLevel));
				edge* e = n->getMarkedEdge();
				int parentId = e->getTo() == n->getNum()?e->getFrom():e->getTo();
				node* parent = g->getNode(parentId);
				std::cout << " p: ("<<parent->getLabelL(kFirstData)<<", "<<parent->getLabelL(kFirstData+1)<<") ";
			}
	}

	if(goal)
	{
		double hcost = h(n, goal);
		//double hcost = 0; 
		double gcost = n->getLabelF(kTemporaryLabel) - hcost;
		std::cout << " f: "<<gcost+hcost<<" g: "<<gcost<<" h: "<<hcost<<std::endl;
	}
}

void AbstractClusterAStar::printStats()
{
	std::cout << "st: "<<this->searchTime<<" ne: "<<this->nodesExpanded<<std::endl;
}

