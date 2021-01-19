/*
 *  ClusterAStarFactory.cpp
 *  hog
 *
 *  Created by dharabor on 14/11/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "ClusterAStarFactory.h"
#include "ClusterAStar.h"

AbstractClusterAStar* ClusterAStarFactory::newClusterAStar()
{
	ClusterAStar* castar = new ClusterAStar();
	return castar;
}
