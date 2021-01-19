/*
 *  HPAClusterAbstraction.cpp
 *  hog
 *
 *  Created by dharabor on 10/11/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "HPAClusterAbstraction.h"
#include "HPACluster.h"
#include "HPAClusterFactory.h"
#include "ClusterNode.h"
#include "ClusterNodeFactory.h"
#include "ClusterAStar.h"

#include "map.h"
#include "NodeFactory.h"
#include "EdgeFactory.h"
#include "path.h"
#include <stdexcept>
#include <sstream>

const unsigned int DEFAULTCLUSTERSIZE = 10;

HPAClusterAbstraction::HPAClusterAbstraction(Map* m, HPAClusterFactory* _cf, 
	INodeFactory* _nf, IEdgeFactory* _ef, bool allowDiagonals_) :
		GenericClusterAbstraction(m, _cf, _nf, _ef, allowDiagonals_),
		clustersize(DEFAULTCLUSTERSIZE)
{	
}

HPAClusterAbstraction::~HPAClusterAbstraction()
{
}


void 
HPAClusterAbstraction::buildClusters()
{
	int mapwidth = this->getMap()->getMapWidth();
	int mapheight= this->getMap()->getMapHeight();

	/* need to split the map into fixed-size cluster areas that will form the basis of our abstract graph building later */
	int csize = getClusterSize();
	for(int x=0; x<mapwidth; x+=csize)
		for(int y=0; y<mapheight; y+= csize)
		{	
			int cwidth=csize;
			if(x+cwidth > mapwidth)
				cwidth = mapwidth - x;
			int cheight=csize;
			if(y+cheight > mapheight)
				cheight = mapheight - y;
				
			HPACluster *cluster = static_cast<HPACluster*>(
					getClusterFactory()->createCluster(x,y,this));
			cluster->setWidth(cwidth);
			cluster->setHeight(cheight);
			addCluster( cluster ); // nb: also assigns a new id to cluster
			cluster->buildCluster();
		}
}

void 
HPAClusterAbstraction::print(std::ostream& out)
{
	GenericClusterAbstraction::print(out);
	out << std::endl << " Cluster Size: "<<getClusterSize()<<std::endl;
}

void 
HPAClusterAbstraction::verifyClusters()
{
//	cluster_iterator iter = getClusterIter();
//	HPACluster* cluster = clusterIterNext(iter);
//	while(cluster)
//	{
//		cluster->verifyCluster();
//		cluster = clusterIterNext(iter);
//	}
}
