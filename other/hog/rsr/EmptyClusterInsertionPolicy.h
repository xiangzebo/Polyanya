#ifndef EMPTYCLUSTERINSERTIONPOLICY_H
#define EMPTYCLUSTERINSERTIONPOLICY_H

// EmptyClusterInsertionPolicy.h
//
// An insertion policy for maps of type EmptyClusterAbstraction.
// Each node is connected to a set of neighbours from each of the 4 sides
// of its (rectangular, obstacle-free) parent cluster. 
//
// Two distinct insertion cases are implemented. Which is used depends on
// whether or not the map allows diagonal movement.
//
// For more information, see the following papers:
//		[Harabor and Botea, Breaking Symmetries in 4-connected Grid Maps
// 		AIIDE, 2010]
//
// 		[Harabor, Botea and Kilby, Symmetry-based Search Space Reductions for
// 		Grid Maps, , 2011]
//
// 	@author: dharabor
// 	@created: 05/04/2011
//

#include "InsertionPolicy.h"

class EmptyClusterAbstraction;
class node;
class MacroNode;
class EmptyClusterInsertionPolicy : public InsertionPolicy
{
	public:
		EmptyClusterInsertionPolicy(EmptyClusterAbstraction* _map);
		virtual ~EmptyClusterInsertionPolicy();

		virtual node* insert(node* _n) throw(std::invalid_argument);
		virtual void remove(node* _n) throw(std::runtime_error);
	
	private:
		void connect(MacroNode* absNode);
		void cardinalConnect(MacroNode* absNode);
		void addMacroEdge(MacroNode* absNode, MacroNode* absNeighbour);
		void connectInserted(MacroNode* n);

		EmptyClusterAbstraction* map;
};

#endif

