#ifndef JUMPPOINTLOCATOR_H
#define JUMPPOINTLOCATOR_H

// JumpPointLocator.h
//
// An abstract base class for identifying jump point successors of nodes
// on a grid map.
//
// Known implementations:
// 	- OnlineJumpPointLocator
// 	- OfflineJumpPointLocator
//
// @author: dharabor
// @created: 7/06/2011

#include "Jump.h"
#include <vector>

class node;
class mapAbstraction;
class JumpPointLocator
{
	public:
		JumpPointLocator(mapAbstraction* _map);
		virtual ~JumpPointLocator();

		virtual node* findJumpNode(Jump::Direction d, int x, int y, 
				int goalx, int goaly) = 0;

		int computeSuccessors(Jump::Direction d, int x, int y);
		int computeNatural(Jump::Direction d, int x, int y);
		int computeForced(Jump::Direction d, int x, int y);
		unsigned int numForcedNeighbours();
		node* getForcedNeighbour(unsigned int i);

		inline int getLimit() { return jumplimit; }
		inline void setLimit(int lim) { jumplimit = lim; }

	protected:
		bool canStep(int x, int y, Jump::Direction checkdir);

		mapAbstraction* map;
		int jumplimit; // max # of steps to take before giving up (default=inf)

};

#endif

