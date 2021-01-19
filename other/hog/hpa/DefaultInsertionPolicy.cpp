#include "DefaultInsertionPolicy.h"

#include "AbstractCluster.h"
#include "ClusterNode.h"
#include "GenericClusterAbstraction.h"
#include "graph.h"
#include "timer.h"

DefaultInsertionPolicy::DefaultInsertionPolicy(GenericClusterAbstraction* _map)
	: InsertionPolicy()
{
	map = _map;
}

DefaultInsertionPolicy::~DefaultInsertionPolicy()
{

}

// insert start and goal nodes into the abstract graph before running
// any search.
node*
DefaultInsertionPolicy::insert(node* n)
	throw(std::invalid_argument)
{

	node* retVal = 0;

	if(n->getLabelL(kParent) == -1)
	{
		ClusterNode* target = dynamic_cast<ClusterNode*>(n);
		AbstractCluster* cluster = map->getCluster(
			target->getParentClusterId());

		if(this->getVerbose())
		{
			const int x = target->getLabelL(kFirstData);
			const int y = target->getLabelL(kFirstData+1);
			std::cout << "inserting node ("<<x<<", "<<y<<") "
				"into abstract graph"<<std::endl;
		}

		cluster->addParent(target);
		graph* absg = map->getAbstractGraph(1);
		retVal = absg->getNode(target->getLabelL(kParent));
		addNode(retVal);
		assert(retVal);
		
		searchTime += cluster->getSearchTime(); 
		nodesExpanded += cluster->getNodesExpanded();
		nodesGenerated += cluster->getNodesGenerated();
		nodesTouched += cluster->getNodesTouched();
	}
	else
	{
		retVal = map->getAbstractGraph(1)->getNode(
				n->getLabelL(kParent));
	}

	return retVal;
}

void 
DefaultInsertionPolicy::remove(node* _n) 
	throw(std::runtime_error)
{
	if(removeNode(_n))
	{
		Timer t;
		t.startTimer();

		// then remove it from the abstract graph and its parent cluster
		graph* g = map->getAbstractGraph(1);
		ClusterNode* n = dynamic_cast<ClusterNode*>(g->getNode(_n->getNum()));

		if(n)
		{               
			edge_iterator ei = n->getEdgeIter();
			edge* e = n->edgeIterNext(ei);
			while(e)
			{
					g->removeEdge(e);
					delete e;
					ei = n->getEdgeIter();
					e = n->edgeIterNext(ei);
			}

			AbstractCluster* parentCluster = dynamic_cast<GenericClusterAbstraction*>(
					map)->getCluster(n->getParentClusterId());
			parentCluster->removeParent(n);
			g->removeNode(n->getNum()); 

			node* originalN = map->getNodeFromMap(n->getLabelL(kFirstData), 
							n->getLabelL(kFirstData+1));
			originalN->setLabelL(kParent, -1);
			delete n;
			n = 0;
		}
		searchTime += t.endTimer();

	}
}

