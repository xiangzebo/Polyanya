#ifndef TILEEXPANSIONPOLICY_H
#define TILEEXPANSIONPOLICY_H

// TileExpansionPolicy.h
//
// Expands each target node as though it were a 4-connected tile.
// Iterates over neighbours reachable by taking a single step from
// the target node in one of the four cardinal directions: up, down, left right
//
// @author: dharabor
// @created: 28/10/2010

#include "GridMapExpansionPolicy.h"

class mapAbstraction;
class TileExpansionPolicy : public GridMapExpansionPolicy
{
	public:
		TileExpansionPolicy();
		virtual ~TileExpansionPolicy();

		// Each 4-connected target node has four neighbours: {above, below, left,
		// right}. 
		// @return: the current neighbour -- or 0 if the neighbour does not exist.
		virtual node* n();
		virtual double cost_to_n();
		virtual void label_n();
};

#endif

