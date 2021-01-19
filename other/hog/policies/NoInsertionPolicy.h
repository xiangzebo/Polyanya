#ifndef NOINSERTIONPOLICY_H
#define NOINSERTIONPOLICY_H

// NoInsertionPolicy.h
//
// The insertion policy for when you want to run a 
// hierarchical search that doesn't use insertion.
//
// @author: dharabor
// @created: 08/03/2011
//

#include "InsertionPolicy.h"

class NoInsertionPolicy : public InsertionPolicy
{
	public:
		NoInsertionPolicy();
		virtual ~NoInsertionPolicy();

		virtual node* insert(node* n) throw(std::invalid_argument);
		virtual void remove(node* n) throw(std::runtime_error);
};

#endif

