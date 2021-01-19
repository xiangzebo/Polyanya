/*
 *  NodeFactory.h
 *  hog
 *
	Factory class for creating node objects.
 
 *  Created by dharabor on 8/11/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef NODEFACTORY_H
#define NODEFACTORY_H
 
 #include "INodeFactory.h"
 
 class node;
 
 class NodeFactory : public INodeFactory
 {
	public:
		virtual node* newNode(const char* name) throw(std::invalid_argument);
		virtual node* newNode(const node* n) throw(std::invalid_argument);
		virtual NodeFactory* clone() { return new NodeFactory(); }
 };

#endif
