#ifndef JUMPPOINTREFINEMENTPOLICY_H
#define JUMPPOINTREFINEMENTPOLICY_H

// JumpPointRefinementPolicy.h
//
// A class for refining the paths returned by Jump Point Search.
// The approach is simple: once JPS finds a path, we re-expand
// each node on the path, beginning with the start node. 
// During each such expansion we mark the path from the current
// node to the optimal successor. When the goal is reached, the conjunction
// of all optimal path segments is returned.
//
// @author: dharabor
// @created: 01/07/2011
//

#include "RefinementPolicy.h"
#include "Jump.h"

class mapAbstraction;
class node;
class OnlineJumpPointLocator;
class path;
class JumpPointRefinementPolicy : public RefinementPolicy
{
	public:
		JumpPointRefinementPolicy(mapAbstraction* map, int maxdepth);
		virtual ~JumpPointRefinementPolicy();
		virtual path* refine(path* abspath);

	private:
		path* recurse(Jump::Direction d, node* current, node* next, int depth);
		int computeSuccessors(Jump::Direction d, int x, int y);

		mapAbstraction* map;
		OnlineJumpPointLocator* jpl;
		int maxdepth;

};

#endif

