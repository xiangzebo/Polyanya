#include "DebugUtility.h"

#include "constants.h"
#include "graph.h"
#include "graphAbstraction.h"
#include "Heuristic.h"
#include "MacroNode.h"
#include "path.h"

DebugUtility::DebugUtility(graphAbstraction* map, Heuristic* h)
{
	this->map = map;
	this->heuristic = h;
}

DebugUtility::~DebugUtility()
{
}

void DebugUtility::printPath(path* p)
{
	if(!p)
	{
		std::cout << "cannot print null path"<<std::endl;
		return;
	}

	node* goal = p->tail()->n;
	node* last = 0;
	double gcost=0;
	while(p)
	{
		node* n = p->n;
		
		double hcost = heuristic->h(n, goal);
		if(last)
			gcost += heuristic->h(n, last);
		std::cout << "0x"<<*&n<<" id: "<<n->getUniqueID()<<" node @ "<<n->getName();
		std::cout << " g: "<<gcost<<" f: "<<gcost+hcost;
		std::cout << " drawColor: "<<n->drawColor<<std::endl;
		last = n;
		p = p->next;
	}		
}

void DebugUtility::printNode(std::string msg, node* n, node* goal)
{	
	graph* g = map->getAbstractGraph(n->getLabelL(kAbstractionLevel));
	std::cout << msg <<&(*n)<<" ["<<n->getUniqueID() << "]";
	std::cout <<" ("<<n->getLabelL(kFirstData)<<","<<n->getLabelL(kFirstData+1)<<") ";

	if(n->getMarkedEdge())
	{
		edge* e = n->getMarkedEdge();
		int parentId = e->getTo() == n->getNum()?e->getFrom():e->getTo();
		node* parent = g->getNode(parentId);
		std::cout << " p: ("<<parent->getLabelL(kFirstData)<<", "<<
			parent->getLabelL(kFirstData+1)<<") ";
	}

	if(goal)
	{
		double hcost = heuristic->h(n, goal);
		//double hcost = 0; 
		double gcost = n->getLabelF(kTemporaryLabel) - hcost;
		std::cout << " f: "<<gcost+hcost<<" g: "<<gcost<<" h: "<<hcost<<std::endl;
	}
	else 
		std::cout << std::endl;
}

// a node is correctly closed only if its fCost is <= the cost of
// of any path through an alternative parent. Here node c is being expanded
// and it contains the closed neighbour n. We want to make sure 
// f(n) <= g(c) + d(c, n) + h(n, goal)
void DebugUtility::debugClosedNode(node* c, node* n, double c_to_n_cost, node* goal)
{
	double fclosed = n->getLabelF(kTemporaryLabel);
	double gclosed =  fclosed - heuristic->h(n, goal);

	// alternate fcost
	double alth = heuristic->h(n, goal);
	double altg = c->getLabelF(kTemporaryLabel) - heuristic->h(c, goal);

	if((altg + c_to_n_cost + alth) < fclosed)
	{
		std::cout << "node "<<n->getName()<<" expanded out of order! ";
		std::cout << " fClosed = "<<fclosed;
		std::cout << " fActual: "<<altg + c_to_n_cost + alth;
		std::cout << " gClosed = "<<gclosed;
		std::cout << "; alternative: "<<altg+c_to_n_cost;
		printNode("\nfaulty node: ", n, goal); 
		std::cout << std::endl;
		printNode(" alt parent: ", c, goal);
		std::cout << std::endl;
	}
}

void DebugUtility::printGraph(graph* g)
{
	if(g == 0)
	{
		std::cout << "DebugUtility::printGraph: "
			"cannot print graph (non-null argument required)"<<std::endl;
		return;
	}

	for(int i=0; i<g->getNumNodes(); i++)
	{
		node* n = g->getNode(i);
		printNode("", n);
	}
}

