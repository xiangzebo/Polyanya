#ifndef DEFAULTINSERTIONPOLICY_H
#define DEFAULTINSERTIONPOLICY_H

// DefaultInsertionPolicy.h
//
// The default insertion method for cluster-based abstractions.
// This class is used in conjunction with HierarchicalSearch
// to temporarily insert start and goal nodes (possibly others)
// into the abstract graph.
//
// If a start or goal node already has a parent in the abstract
// graph, no insertion is performed.
//
// @author: dharabor
// @created: 10/03/2011
//

#include "InsertionPolicy.h"
#include <stdexcept>

class GenericClusterAbstraction;
class DefaultInsertionPolicy : public InsertionPolicy
{
	public:
		DefaultInsertionPolicy(GenericClusterAbstraction* _map);
		virtual ~DefaultInsertionPolicy();

		virtual node* insert(node* n) 
			throw(std::invalid_argument);
		virtual void remove(node* n)
			throw(std::runtime_error);

	private:
		GenericClusterAbstraction* map;
		bool verbose;

};

#endif

