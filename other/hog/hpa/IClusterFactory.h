// IClusterFactory.h
//
// A common interface for creating objects that derive from AbstractCluster.
//
// @author: dharabor
// @created: 11/11/2008
//

#ifndef ICLUSTERFACTORY_H
#define ICLUSTERFACTORY_H

#include "AbstractCluster.h"

class AbstractClusterAStar;
class GenericClusterAbstraction;
class IClusterFactory
{
	public:
		virtual ~IClusterFactory() {}
		virtual AbstractCluster* createCluster(int xpos, int ypos,
				GenericClusterAbstraction* map) = 0;
};

#endif
