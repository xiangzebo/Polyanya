// HPAClusterFactory.h
//
// A factory class for making HPACluster objects.
//
// @author: dharabor
// @created: 11/11/2008
//

#ifndef HPACLUSTERFACTORY_H
#define HPACLUSTERFACTORY_H

#include "IClusterFactory.h"

class AbstractClusterAStar;
class HPAClusterAbstraction;
class HPACluster;
class HPAClusterFactory : public IClusterFactory
{
	public:
		HPAClusterFactory();
		virtual AbstractCluster* createCluster(int xpos, int ypos,
				GenericClusterAbstraction* map);
		virtual HPACluster* createCluster(int xpos, int ypos, 
				int width, int height, AbstractClusterAStar* castar,
				HPAClusterAbstraction* map);	
};

#endif

