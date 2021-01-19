#ifndef OFFLINEJUMPPOINTLOCATOR_H
#define OFFLINEJUMPPOINTLOCATOR_H

// OfflineJumpPointLocator.h
//
// A class wrapper around some code that finds, offline, jump point
// successors of arbitrary nodes in a uniform-cost grid map.
//
// This

#include "JumpPointLocator.h"

class JumpPointAbstraction;
class OfflineJumpPointLocator : public JumpPointLocator
{
	public:
		OfflineJumpPointLocator(JumpPointAbstraction* map);
		virtual ~OfflineJumpPointLocator();

		node* findJumpNode(Jump::Direction d, int x, int y, 
				int goalx, int goaly);
	
	private:
		int calculateEdgeIndex(Jump::Direction dir);
		node* findSterileJumpNode(Jump::Direction d, int x, int y);
};

#endif

