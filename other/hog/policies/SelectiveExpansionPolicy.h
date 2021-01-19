#ifndef SELECTIVEEXPANSIONPOLICY_H
#define SELECTIVEEXPANSIONPOLICY_H

// SelectiveExpansionPolicy.h
//
// Extends the basic ExpansionPolicy class by adding support
// for an ::filter method which determines if a neighbour should be considered
// or not. Such functionality is useful in search algorithms that need to be
// scope limited in some way (e.g to a corridor of nodes or single cluster).
//
// Neighbours which are not successfully evaluated are skipped during iteration
// and can never be returned by ::first ::next or ::n
//
// @author: dharabor
// @created: 24/09/2010

#include "ExpansionPolicy.h"
#include <vector>

class node;
class NodeFilter;
class SelectiveExpansionPolicy : public ExpansionPolicy
{
	public:
		SelectiveExpansionPolicy();
		virtual ~SelectiveExpansionPolicy();

		virtual node* first();
		virtual node* next();
		virtual node* n();

		void addFilter(NodeFilter* nf);

	protected:
		virtual node* n_impl() = 0;
		virtual node* first_impl() = 0;
		virtual node* next_impl() = 0;

		// returns true if the given node is matched by any available filter
		bool filter(node*);

	private:
		std::vector<NodeFilter*> filters;

};

#endif

