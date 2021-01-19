#include "constants.h"
#include "jps.h"

// computes the forced neighbours of a node.
// for a neighbour to be forced we must check that 
// (a) the alt path from the parent is blocked and
// (b) the neighbour is not an obstacle.
// if the test succeeds, we set a bit to indicate 
// the direction from the current node (tiles[4])
// to the forced neighbour.
//
// @return an integer value whose lower 8 bits indicate
// the directions of forced neighbours
//
// NB: the first 3 bits of the first 3 bytes of @param tiles represent
// a 3x3 block of nodes. the current node is at the centre of
// the block.
// its NW neighbour is bit 0
// its N neighbour is bit 1
// its NE neighbour is bit 2
// its W neighbour is bit 8
// ...
// its SE neighbour is bit 18 
// There are optimisations below that use bitmasks in order
// to speed up forced neighbour computation. 
// We use the revised forced neighbour rules described in
// [Harabor and Grastien, The JPS Pathfinding System, SoCS, 2012]
// These rules do not allow diagonal transitions that cut corners.
uint32_t
warthog::jps::compute_forced(warthog::jps::direction d, uint32_t tiles)
{
	// NB: to avoid branching statements, shift operations are
	// used below. The values of the shift constants correspond to 
	// direction values. 
	uint32_t ret = 0;
	switch(d)
	{
		case warthog::jps::NORTH:
			if((tiles & 65792) == 256)
			{
				ret |= (warthog::jps::WEST | warthog::jps::NORTHWEST);
			}
			if((tiles & 263168) == 1024)
			{
				ret |= (warthog::jps::EAST | warthog::jps::NORTHEAST);
			}
			break;
		case warthog::jps::SOUTH:
			if((tiles & 257) == 256) 
			{
				ret |= (warthog::jps::WEST | warthog::jps::SOUTHWEST);
			}
			if((tiles & 1028) == 1024)
			{
				ret |= (warthog::jps::EAST | warthog::jps::SOUTHEAST);
			}
			break;
		case warthog::jps::EAST:
			if((tiles & 3) == 2) 
			{
				ret |= (warthog::jps::NORTH | warthog::jps::NORTHEAST);
			}
			if((tiles & 196608) == 131072)
			{
				ret |= (warthog::jps::SOUTH | warthog::jps::SOUTHEAST);
			}
			break;
		case warthog::jps::WEST:
			if((tiles & 6) == 2)
			{
				ret |= (warthog::jps::NORTH | warthog::jps::NORTHWEST);
			}
			if((tiles & 393216) == 131072)
			{
				ret |= (warthog::jps::SOUTH | warthog::jps::SOUTHWEST);
			}
			break;
		default:
			break;
	}
	return ret;
}

// Computes the natural neighbours of a node. 
//
// NB: the first 3 bits of the first 3 bytes of @param tiles represent
// a 3x3 block of nodes. the current node is at the centre of
// the block.
// its NW neighbour is bit 0
// its N neighbour is bit 1
// its NE neighbour is bit 2
// its W neighbour is bit 8
// ...
// its SE neighbour is bit 18 
// There are optimisations below that use bitmasks in order
// to speed up forced neighbour computation. 
uint32_t 
warthog::jps::compute_natural(warthog::jps::direction d, uint32_t tiles)
{
	// In the shift operations below the constant values
	// correspond to bit offsets for warthog::jps::direction
	uint32_t ret = 0;
	switch(d)
	{
		case warthog::jps::NORTH:
			ret |= ((tiles & 2) == 2) << 0;
			break;
		case warthog::jps::SOUTH:
			ret |= ((tiles & 131072) == 131072) << 1;
			break;
		case warthog::jps::EAST: 
			ret |= ((tiles & 1024) == 1024) << 2;
			break;
		case warthog::jps::WEST:
			ret |= ((tiles & 256) == 256) << 3;
			break;
		case warthog::jps::NORTHWEST:
			ret |= ((tiles & 2) == 2) << 0;
			ret |= ((tiles & 256) == 256) << 3;
			ret |= ((tiles & 259) == 259) << 5;
			break;
		case warthog::jps::NORTHEAST:
			ret |= ((tiles & 2) == 2) << 0;
			ret |= ((tiles & 1024) == 1024) << 2;
			ret |= ((tiles & 1030) == 1030) << 4;
			break;
		case warthog::jps::SOUTHWEST:
			ret |= ((tiles & 131072) == 131072) << 1;
			ret |= ((tiles & 256) == 256) << 3;
			ret |= ((tiles & 196864) == 196864) << 7;
			break;
		case warthog::jps::SOUTHEAST:
			ret |= ((tiles & 131072) == 131072) << 1;
			ret |= ((tiles & 1024) == 1024) << 2;
			ret |= ((tiles & 394240) == 394240) << 6;
			break;
		default:
			ret |= ((tiles & 2) == 2) << 0;
			ret |= ((tiles & 131072) == 131072) << 1;
			ret |= ((tiles & 1024) == 1024) << 2;
			ret |= ((tiles & 256) == 256) << 3;
			ret |= ((tiles & 259) == 259) << 5;
			ret |= ((tiles & 1030) == 1030) << 4;
			ret |= ((tiles & 196864) == 196864) << 7;
			ret |= ((tiles & 394240) == 394240) << 6;
			break;
	}
	return ret;
}

