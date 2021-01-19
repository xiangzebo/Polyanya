#include "EmptyClusterAbstraction.h"

#include "ClusterNode.h"
#include "EmptyCluster.h"
#include "IEdgeFactory.h"
#include "IClusterFactory.h"
#include "INodeFactory.h"
#include "graph.h"
#include "heap.h"
#include "map.h"
#include "MacroEdge.h"
#include "MacroNode.h"

EmptyClusterAbstraction::EmptyClusterAbstraction(Map* m, IClusterFactory* cf, 
	INodeFactory* nf, IEdgeFactory* ef, bool allowDiagonals, bool perimeterReduction_,
	bool bfReduction_) 
	throw(std::invalid_argument)
	: GenericClusterAbstraction(m, cf, nf, ef, allowDiagonals),
	  perimeterReduction(perimeterReduction_), bfReduction(bfReduction_)
{
	AbstractCluster* tmp = cf->createCluster(0, 0, this);
	if(!dynamic_cast<EmptyCluster*>(tmp))
		throw std::invalid_argument("EmptyClusterAbstraction requires"
				"a cluster factory that produces objects derived from"
				" EmptyCluster");
				
	sgEdge = 0;
}

EmptyClusterAbstraction::~EmptyClusterAbstraction()
{
	assert(sgEdge == 0);
}

int EmptyClusterAbstraction::getNumMacro()
{
	int macro = 0;
	cluster_iterator ci = getClusterIter();
	EmptyCluster* cluster = clusterIterNext(ci);
	while(cluster)
	{
		macro += cluster->macro;
		cluster = clusterIterNext(ci);
	}
	return macro;
}

void EmptyClusterAbstraction::buildClusters()
{
	if(getVerbose())
		std::cout << "buildClusters...."<<std::endl;

	Map* m = this->getMap();
	int mapheight = m->getMapHeight();
	int mapwidth = m->getMapWidth();

	// set initial priorities of all potential cluster origins
	// based on # of interior nodes in maximal clearance square of each tile
	heap clusterseeds(30, false); // maxheap
	for(int y=0; y<mapheight; y++)
		for(int x=0; x<mapwidth; x++)
		{
			node* n = this->getNodeFromMap(x, y);
			if(n)
			{
				EmptyCluster* cluster = dynamic_cast<EmptyCluster*>(
						getClusterFactory()->createCluster(x, y, this)); 
				cluster->setPerimeterReduction(perimeterReduction);
				cluster->setBFReduction(bfReduction);
				cluster->buildCluster();
				cluster->setVerbose(getVerbose());

				double priority = cluster->getNumNodes();
//				n->print(std::cout);
//				std::cout << "cluster size: "<<cluster->getNumNodes() << 
//					" priority: "<<priority;
//				std::cout << std::endl;

				n->setLabelF(kTemporaryLabel, priority);
				n->setKeyLabel(kTemporaryLabel);
				clusterseeds.add(n);
				delete cluster;
			}
		}

	// start making clusters; prefer clusters with more interior nodes to others
	// with less
	while(!clusterseeds.empty())
	{
		ClusterNode* cur = dynamic_cast<ClusterNode*>(clusterseeds.peek());
		assert(cur);
		if(getVerbose())
		{
			std::cout << "cluster seed ";
			cur->print(std::cout);
			std::cout << " priority: "<<cur->getLabelF(kTemporaryLabel);
			std::cout << std::endl;
		}

		if(cur->getParentClusterId() == -1)
		{
			EmptyCluster* cluster = dynamic_cast<EmptyCluster*>(
					getClusterFactory()->createCluster(
						cur->getLabelL(kFirstData), 
						cur->getLabelL(kFirstData+1), 
						this)); 
			cluster->setPerimeterReduction(perimeterReduction);
			cluster->setBFReduction(bfReduction);
			cluster->buildCluster();
			cluster->setVerbose(getVerbose());

			double priority = cluster->getNumNodes();
			if(priority >= cur->getLabelF(kTemporaryLabel))
			{
				addCluster(cluster);
				if(this->getVerbose())
				{
					std::cout << "new cluster w/ priority "<<priority<<":\n";
				    cluster->print(std::cout);	
				}
				clusterseeds.remove();
				cluster = 0;
			}
			else
			{
				cur->setLabelF(kTemporaryLabel, priority); 
				clusterseeds.decreaseKey(cur);
				if(this->getVerbose())
					std::cout << " updating; new priority "<<priority<<"\n";
				delete cluster;
			}
		}
		else
		{
			if(this->getVerbose())
				std::cout << " seed already assigned; removing\n";
			clusterseeds.remove();
		}
		
	}

	if(this->getVerbose())
	{
		std::cout << "EmptyClusterAbstaction::buildClusters created"
			<< getNumClusters() << " clusters\n";
	}
}

EmptyCluster* EmptyClusterAbstraction::clusterIterNext(cluster_iterator& it) const
{
       return static_cast<EmptyCluster*>(
		GenericClusterAbstraction::clusterIterNext(it));
}

EmptyCluster* EmptyClusterAbstraction::getCluster(int cid)
{
       return static_cast<EmptyCluster*>(
		GenericClusterAbstraction::getCluster(cid));
}

double EmptyClusterAbstraction::getAverageClusterSize()
{
	double total = 0;
	cluster_iterator iter = this->getClusterIter();
	EmptyCluster* cluster = 0;
	while((cluster = this->clusterIterNext(iter)))
	{
		total += cluster->getNumNodes();
	}

	return total/this->getNumClusters();
}

double EmptyClusterAbstraction::getAverageNodesPruned()
{
	double total = 0;
	cluster_iterator iter = this->getClusterIter();
	EmptyCluster* cluster = 0;
	while((cluster = this->clusterIterNext(iter)))
	{
		total += cluster->getNumNodes() - cluster->getNumParents();
	}

	return total/this->getNumClusters();
	
}

int EmptyClusterAbstraction::getNumAbsEdges()
{
	graph* g = this->getAbstractGraph(1);
	int numEdges = g->getNumEdges();

	cluster_iterator iter = this->getClusterIter();
	EmptyCluster* cluster = 0;
	while((cluster = this->clusterIterNext(iter)))
	{
		numEdges += cluster->getNumSecondaryEdges();
	}
	
	return numEdges;
}
