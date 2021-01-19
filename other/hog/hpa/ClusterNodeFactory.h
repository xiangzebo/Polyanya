/*
 *  ClusterNodeFactory.h
 *  hog
 *
 *  Created by dharabor on 13/11/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef CLUSTERNODEFACTORY_H
#define CLUSTERNODEFACTORY_H

#include "INodeFactory.h"

class ClusterNode;
class ClusterNodeFactory : public INodeFactory 
{
	public:
		virtual ~ClusterNodeFactory() {}
		virtual ClusterNodeFactory* clone() { return new ClusterNodeFactory(); }
		virtual node* newNode(const char* name) throw(std::invalid_argument);
		virtual node* newNode(const node* n) throw(std::invalid_argument);
};

#endif
