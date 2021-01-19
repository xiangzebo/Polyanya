/*
 *  ClusterAStarFactory.h
 *  hog
 *
 *  Created by dharabor on 13/11/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef CLUSTERASTARFACTORY_H
#define CLUSTERASTARFACTORY_H

#include "IClusterAStarFactory.h"

class ClusterAStar;
class ClusterAStarFactory : public IClusterAStarFactory 
{
	public:
		ClusterAStarFactory() {}
		virtual AbstractClusterAStar* newClusterAStar();
};

#endif
