/*
 *  NodeFactory.cpp
 *  hog
 *
 *  Created by dharabor on 8/11/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "NodeFactory.h"
#include "graph.h"

node* NodeFactory::newNode(const char* name) throw(std::invalid_argument)
{
	node* n = new node(name);
	return n;
}

node* NodeFactory::newNode(const node* _n) throw(std::invalid_argument)
{
	node* n = new node(_n);
	return n;
}

