#ifndef OCTILEHEURISTIC_H
#define OCTILEHEURISTIC_H

// OctileHeuristic.h
//
// Calculates octile distance between two nodes on a grid.
//
// @author: dharabor
// @author: Nathan Sturtevant 
//
// @created: 28/10/2010

#include "Heuristic.h"

class node;
class OctileHeuristic : public Heuristic
{
	public:
		OctileHeuristic();
		virtual ~OctileHeuristic();

		virtual double h(node* first, node* second) const;
};

#endif
