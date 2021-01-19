/*
 *  EdgeFactory.cpp
 *  hog
 *
 *  Created by dharabor on 8/11/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "EdgeFactory.h"
#include "graph.h"

edge* EdgeFactory::newEdge(unsigned int fromId, unsigned int toId, double weight)
{
	edge* e = new edge(fromId, toId, weight);
	return e;
}

