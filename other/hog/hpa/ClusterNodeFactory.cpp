/*
 *  ClusterNodeFactory.cpp
 *  hog
 *
 *  Created by dharabor on 13/11/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "ClusterNodeFactory.h"
#include "ClusterNode.h"

node* ClusterNodeFactory::newNode(const char* name) throw(std::invalid_argument)
{ 
	return new ClusterNode(name); 
}

node* ClusterNodeFactory::newNode(const node* _n) throw(std::invalid_argument)
{
	const ClusterNode* n = dynamic_cast<const ClusterNode*>(_n);
	if(!n)
		throw std::invalid_argument("ClusterNodeFactory::newNode requires a node of type 'ClusterNode'");
		
	return new ClusterNode(n);

}

