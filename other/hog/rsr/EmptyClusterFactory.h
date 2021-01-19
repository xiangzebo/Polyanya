#ifndef EMPTYCLUSTERFACTORY_H 
#define EMPTYCLUSTERFACTORY_H

#include "IClusterFactory.h"
#include "EmptyCluster.h"

class EmptyClusterFactory : public IClusterFactory 
{
	public:
		EmptyClusterFactory();
		virtual ~EmptyClusterFactory();
		virtual AbstractCluster* createCluster(int xpos, int ypos,
				GenericClusterAbstraction* map);
		EmptyCluster* createCluster(int x, int y,
				EmptyClusterAbstraction* map, bool pr, bool bfr);
};

#endif

