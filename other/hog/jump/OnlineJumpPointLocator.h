#ifndef ONLINEJUMPPOINTLOCATOR_H
#define ONLINEJUMPPOINTLOCATOR_H

// OnlineJumpPointLocator.h
//
// A class wrapper around some code that finds, online, jump point
// successors of an arbitrary nodes in a uniform-cost grid map.
//
// For theoretical details see:
// [Harabor D. and Grastien A, 2011, 
// Online Graph Pruning Pathfinding on Grid Maps, AAAI]
//
// NB: As of May 2012, this class implements a revised version of JPS
// which does not permit corner cutting. This is a small modification
// but it changes slightly the location of forced neighbours.
// For details see:
//
// [Harabor D. and Grastien A., 2012,
// The JPS Pathfinding System, SoCS]
//
// @author: dharabor
// @created: 04/06/2011
//

#include "JumpPointLocator.h"
#include "Jump.h"

class mapAbstraction;
class node;

class OnlineJumpPointLocator : public JumpPointLocator
{
	public: 
		OnlineJumpPointLocator(mapAbstraction* map);
		virtual ~OnlineJumpPointLocator();

		virtual node* findJumpNode(Jump::Direction d, int x, int y, 
				int goalx, int goaly);
};

#endif

