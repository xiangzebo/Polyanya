#include "AbstractCluster.h"
#include "GenericClusterAbstraction.h"
#include "ClusterNode.h"
#include "constants.h"
#include "HPAUtil.h"
#include "IEdgeFactory.h"
#include "INodeFactory.h"
#include "timer.h"

#include <cassert>
unsigned AbstractCluster::uniqueClusterIdCnt = 0;

AbstractCluster::AbstractCluster(GenericClusterAbstraction* map)
{
	this->map = map;
	this->clusterId = ++uniqueClusterIdCnt;

	nodesExpanded = nodesGenerated = nodesTouched = 0;
	searchTime = 0;

}

AbstractCluster::~AbstractCluster()
{
	for(HPAUtil::nodeTable::iterator it = nodes.begin();
			it != nodes.end(); it = nodes.begin())
	{
		ClusterNode* n = dynamic_cast<ClusterNode*>((*it).second);
		n->setParentClusterId(-1); 
		nodes.erase(it);
	}
	assert(nodes.size() == 0);

	for(HPAUtil::nodeTable::iterator it = parents.begin();
			it != parents.end(); it = parents.begin())
	{
		node* n = (*it).second;
		this->removeParent(n);
	}
	assert(parents.size() == 0);
}

// addParent is responsible for adding new abstract nodes to the cluster and
// creating intra-edges that connect the new node to existing abstract nodes. 
void 
AbstractCluster::addParent(node* _p) throw(std::invalid_argument)
{
	Timer t;
	t.startTimer();
	ClusterNode* p = dynamic_cast<ClusterNode*>(_p);

	if(p == 0)
		throw std::invalid_argument(
				"AbstractCluster::addParent: arg not of type AbstractClusterNode");

	if(p->getParentClusterId() != this->getId())
		throw std::invalid_argument(
				"AbstractCluster::addParent: arg not assigned to this cluster");

	if(parents.find(p->getUniqueID()) != parents.end())
		return; // node already in parents collection

	if(p->getLabelL(kAbstractionLevel) == 0)
	{
		ClusterNode* p2 = dynamic_cast<ClusterNode*>(
				map->getNodeFactory()->newNode(p));
		p2->setLabelL(kAbstractionLevel, 1);
		p2->setLabelL(kParent, -1);
		map->getAbstractGraph(1)->addNode(p2);
		p->setLabelL(kParent, p2->getNum());
		p = p2;
	}
	
	if(getVerbose())
	{
		std::cout << "AbstractCluster::addParent ";
		p->print(std::cout);
		std::cout << std::endl;
	}

	p->setParentClusterId(this->getId());
	parents.insert(std::pair<int, node*>(p->getUniqueID(), p));
	connectParent(p);
	searchTime = t.endTimer();
}

void 
AbstractCluster::removeParent(node* n_)
{
	assert(n_->getLabelL(kAbstractionLevel) == 1);

	HPAUtil::nodeTable::iterator p_it = parents.find(n_->getUniqueID());
	if(p_it == parents.end())
		return;
	
	// remove the node from this cluster 
	ClusterNode* n = dynamic_cast<ClusterNode*>(n_);
	n->setParentClusterId(-1);

	unsigned int numParents= parents.size();
	parents.erase(p_it);
	assert(parents.size() == --numParents);
	
	// disconnect parent from the rest of the abstract graph 
	graph* g = getMap()->getAbstractGraph(
			n->getLabelL(kAbstractionLevel));
	edge_iterator it = n->getEdgeIter();
	for(edge* e = n->edgeIterNext(it); 
			e != 0; 
			e = n->edgeIterNext(it = n->getEdgeIter()))
	{
			g->removeEdge(e);
			delete e;
	}
	
	// remove labels pointing to this node as a parent
	map->getNodeFromMap(n->getLabelL(kFirstData),
			n->getLabelL(kFirstData+1))->setLabelL(kParent, -1);

}

// argNode is responsible for assigning nodes to the cluster
void 
AbstractCluster::addNode(node* n_) throw(std::invalid_argument)
{
	ClusterNode* n = dynamic_cast<ClusterNode*>(n_);
	if(n == 0)
		throw std::invalid_argument("AbstractCluster::addNode arg is null");

	if(nodes[n->getUniqueID()] != 0) // already added
		return;

	n->setParentClusterId(this->getId());
	if(n->getLabelL(kParent) != -1)
	{
		ClusterNode* parent = dynamic_cast<ClusterNode*>(
				map->getAbstractGraph(1)->getNode(n->getLabelL(kParent)));
		parent->setParentClusterId(this->getId());
	}
	nodes[n->getUniqueID()] = n;
}

// a transition point connects two nodes on the map. usually the nodes are on the
// border of two adjacent clusters (in which case the transition results in an
// inter-edge being created) but this is not necessary.
void 
AbstractCluster::addTransition(node* from_, node* to_, double edgeweight)
	throw(std::invalid_argument)
{
	if(edgeweight < 0)
		throw std::invalid_argument(
			"AbstractCluster::addTransition edge weight cannot be < 0");

	ClusterNode* from = dynamic_cast<ClusterNode*>(from_);
	ClusterNode* to = dynamic_cast<ClusterNode*>(to_);
	assert(from && to);

	graph *g = map->getAbstractGraph(1);
	ClusterNode* absfrom = 
		dynamic_cast<ClusterNode*>(g->getNode(from->getLabelL(kParent)));
	ClusterNode* absto = 
		dynamic_cast<ClusterNode*>(g->getNode(to->getLabelL(kParent)));
	
	// Try to reuse existing nodes from the abstract graph, else create new.
	if(absfrom == 0)
	{
		if(from->getLabelL(kFirstData) == 20 && from->getLabelL(kFirstData+1) == 0)
			std::cout << "master\n";
		AbstractCluster* parentCluster =
			map->getCluster(from->getParentClusterId());
		parentCluster->addParent(from); // also creates intra-edges
		absfrom = dynamic_cast<ClusterNode*>(g->getNode(from->getLabelL(kParent)));
		assert(absfrom);
	}
	if(absto == 0)
	{	
		AbstractCluster* parentCluster = 
			map->getCluster(to->getParentClusterId());
		parentCluster->addParent(to); // also creates intra-edges
		absto = dynamic_cast<ClusterNode*>(g->getNode(to->getLabelL(kParent)));
		assert(absto);
	}

	edge* e = g->findEdge(absfrom->getNum(), absto->getNum());
	if(!e)
	{
		edge* e = map->getEdgeFactory()->newEdge(absfrom->getNum(), 
				absto->getNum(), edgeweight);
		g->addEdge(e);

		if(getVerbose())
		{
			std::cout << "AbstractCluster::addTransition between ";
			from->print(std::cout);
			to->print(std::cout);
			std::cout << " cost: "<<edgeweight<<std::endl;
		}
	}
	else
	{
		if(getVerbose())
		{
			std::cout << "AbstractCluster::addTransition edge already exists: ";
			from->print(std::cout);
			to->print(std::cout);
			std::cout << " cost: "<<e->getWeight()<<std::endl;
		}
	}
}

void
AbstractCluster::print(std::ostream& out)
{
	out << "Cluster "<<getId();
	out << " #nodes: "<<nodes.size();
	out << " #parents: "<<parents.size();
}

void 
AbstractCluster::printParents()
{
	HPAUtil::nodeTable::iterator it;
	it = parents.begin();
	while(it != parents.end())
	{
		node* n = (*it).second;
		std::cout << "parent node: " <<"addr: "<<&(*n);
		std::cout <<" num: "<<n->getUniqueID() <<" ("<<n->getLabelL(kFirstData);
		std::cout <<","<<n->getLabelL(kFirstData+1)<<")"<<std::endl;
		it++;
	}
}

