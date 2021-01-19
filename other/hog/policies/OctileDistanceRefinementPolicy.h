#ifndef OCTILEDISTANCEREFINEMENTPOLICY_H
#define OCTILEDISTANCEREFINEMENTPOLICY_H

// OctileDistanceRefinementPolicy.h
//
// Refines an abstract path by connecting nodes using the most direct path. 
// The length of each refined segment is thus equal to the octile distance  
// between each pair of adjacent nodes appearing on the abstract path.
//
// NB: This approach only works when using a pathfinding technique such as
// RSR or Jump Point Search. 
//
// @author: dharabor
// @created: 09/03/2011
//

#include "RefinementPolicy.h"

class node;
class mapAbstraction;
class path;
class OctileDistanceRefinementPolicy : public RefinementPolicy
{
	public:
		OctileDistanceRefinementPolicy(mapAbstraction* map);
		virtual ~OctileDistanceRefinementPolicy();

		virtual path* refine(path* abspath);

	private:
		mapAbstraction* map;
		node* nextStep(node* first, node* last);
};

#endif

