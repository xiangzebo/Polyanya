/*
 *  EdgeFactory.h
 *  hog
 *
 *  Created by dharabor on 8/11/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef EDGEFACTORY_H
#define EDGEFACTORY_H

#include "IEdgeFactory.h"

class edge;
class EdgeFactory : public IEdgeFactory
{
	public:
		virtual edge* newEdge(unsigned int fromId, unsigned int toId, double weight);
		virtual EdgeFactory* clone() { return new EdgeFactory(); }
};

#endif
