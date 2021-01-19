/*
 *  INodeFactory.h
 *  hog
 *
	Defines a common interface for all NodeFactory classes.
 
 *  Created by dharabor on 8/11/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef INODEFACTORY_H
#define INODEFACTORY_H

#include <stdexcept>

class node;

class INodeFactory
{
	public:
		virtual ~INodeFactory() {}
		virtual INodeFactory* clone() = 0;
		virtual node* newNode(const char* name) throw(std::invalid_argument) = 0;
		virtual node* newNode(const node* n) throw(std::invalid_argument) = 0;
};


#endif
