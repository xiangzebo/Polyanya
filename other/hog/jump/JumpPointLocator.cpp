#include "JumpPointLocator.h"

#include "graph.h"
#include "mapAbstraction.h"

#include <climits>

JumpPointLocator::JumpPointLocator(mapAbstraction* _map)
{
	this->map = _map;
	jumplimit = INT_MAX;
}

JumpPointLocator::~JumpPointLocator()
{
}

// Computes the set of directions (both forced and natural) in which to search 
// for jump points from a given location (x, y). 
//
// @param d: the direction of travel used to reach (x, y) -- from its parent.
// @param x: x-coordinate of the current location
// @param y: y-coordinate of the current location

int
JumpPointLocator::computeSuccessors(Jump::Direction d, int x, int y)
{
	return (computeNatural(d, x, y) | computeForced(d, x, y));
}

// Computes the set of "natural" directions in which to search for jump points
// from a given location (x, y). 
//
// @param d: the direction of travel used to reach (x, y) -- from its parent.
// @param x: x-coordinate of the current location
// @param y: y-coordinate of the current location
//
// @return an integer representing the set of natural directions.
// Each of the first 8 bits of the returned value corresponds to a direction, as
// defined in Jump::Direction
int
JumpPointLocator::computeNatural(Jump::Direction d, int x, int y)
{
	int retVal = 0;
	switch(d)
	{
		case Jump::S:
		{
			retVal |= Jump::S;
			break;
		}

		case Jump::SW:
		{
			retVal |= Jump::SW;
			retVal |= Jump::S;
			retVal |= Jump::W;
			break;
		}

		case Jump::W:
		{
			retVal |= Jump::W;
			break;
		}

		case Jump::NW:
		{
			retVal |= Jump::NW;
			retVal |= Jump::N;
			retVal |= Jump::W;
			break;
		}

		case Jump::N:
		{
			retVal |= Jump::N;
			break;
		}

		case Jump::NE:
		{
			retVal |= Jump::NE;
			retVal |= Jump::E;
			retVal |= Jump::N;
			break;
		}

		case Jump::E:
		{
			retVal |= Jump::E;
			retVal |= Jump::SE;
			break;
		}

		case Jump::SE:
		{
			retVal |= Jump::SE;
			retVal |= Jump::S;
			retVal |= Jump::E;
			break;
		}
		case Jump::NONE:
		{
			// when a node has no parent (usually only the start node)
			// we generate jump point successors in every traversable direction
			retVal = 0xFF;
		}
	}
	return retVal;
}

// Computes the set of "forced" directions in which to search for jump points
// from a given location (x, y). 
//
// @param d: the direction of travel used to reach (x, y) -- from its parent.
// @param x: x-coordinate of the current location
// @param y: y-coordinate of the current location
//
// @return an integer representing the set of forced directions.
// Each of the first 8 bits of the returned value, when set, correspond to a direction, 
// as defined in Jump::Direction
//
int
JumpPointLocator::computeForced(Jump::Direction d, int x, int y)
{
	int retVal = 0;
	switch(d)
	{
		case Jump::S:
		{
			if(!map->getNodeFromMap(x+1, y) && 
					map->getNodeFromMap(x+1, y+1))
				retVal |= Jump::SE;
			if(!map->getNodeFromMap(x-1, y) &&
					map->getNodeFromMap(x-1, y+1))
				retVal |= Jump::SW;
			break;
		}

		case Jump::SW:
		{
			if(!map->getNodeFromMap(x, y-1) &&
					map->getNodeFromMap(x-1, y-1))
				retVal |= Jump::NW;
			if(!map->getNodeFromMap(x+1, y) &&
					map->getNodeFromMap(x+1,y+1))
				retVal |= Jump::SE;
			break;
		}

		case Jump::W:
		{
			if(!map->getNodeFromMap(x, y-1) &&
					map->getNodeFromMap(x-1, y-1))
				retVal |= Jump::NW;
			if(!map->getNodeFromMap(x, y+1) &&
					map->getNodeFromMap(x-1, y+1))
				retVal |= Jump::SW;
			break;
		}

		case Jump::NW:
		{
			if(!map->getNodeFromMap(x, y+1) &&
					map->getNodeFromMap(x-1, y+1))
				retVal |= Jump::SW;
			if(!map->getNodeFromMap(x+1, y) && 
					map->getNodeFromMap(x+1, y-1))
				retVal |= Jump::NE;
			break;
		}

		case Jump::N:
		{
			if(!map->getNodeFromMap(x+1, y) &&
					map->getNodeFromMap(x+1, y-1))
				retVal |= Jump::NE;
			if(!map->getNodeFromMap(x-1, y) &&
					map->getNodeFromMap(x-1, y-1))
				retVal |= Jump::NW;
			break;
		}

		case Jump::NE:
		{
			if(!map->getNodeFromMap(x, y+1) &&
					map->getNodeFromMap(x+1, y+1))
				retVal |= Jump::SE;
			if(!map->getNodeFromMap(x-1, y) &&
					map->getNodeFromMap(x-1, y-1))
				retVal |= Jump::NW;
			break;
		}

		case Jump::E:
		{
			if(!map->getNodeFromMap(x, y-1) &&
					map->getNodeFromMap(x+1, y-1))
				retVal |= Jump::NE;
			if(!map->getNodeFromMap(x, y+1) &&
					map->getNodeFromMap(x+1, y+1))
				retVal |= Jump::SE;
			break;
		}

		case Jump::SE:
		{
			if(!map->getNodeFromMap(x, y-1) &&
					map->getNodeFromMap(x+1, y-1))
				retVal |= Jump::NE;
			if(!map->getNodeFromMap(x-1, y) && 
					map->getNodeFromMap(x-1, y+1))
				retVal |= Jump::SW;
			break;
		}
		case Jump::NONE:
		{
			break;
		}
	}
	return retVal;
}

// this function checks whether a proposed step in the grid is allowed
// and returns a corresponding boolean value.
//
// A "step" involves moving from one tile on the grid to an adjacent
// neighbour. Usually a straightforward operation, the only possible
// complication arises from whether or not "corner cutting" is allowed
// when stepping diagonally. When allowed, it is possible to make a
// diagonal move provided there is at least one valid 2-step path between
// the source and destination. When corner cutting is disallowed, both 2-step
// paths must be valid. 
bool
JumpPointLocator::canStep(int x, int y, Jump::Direction checkdir)
{
	assert(checkdir != Jump::NONE);
	if(!map->getNodeFromMap(x, y))
		return false;

	bool retVal = true;
	switch(checkdir)
	{
		case Jump::N:
			if(!map->getNodeFromMap(x, y-1))
				retVal =  false;
			break;

		case Jump::S:
			if(!map->getNodeFromMap(x, y+1))
				retVal =  false;
			break;

		case Jump::E:
			if(!map->getNodeFromMap(x+1, y))
				retVal =  false;
			break;

		case Jump::W:
			if(!map->getNodeFromMap(x-1, y))
				retVal =  false;
			break;

		case Jump::NE:
			if(!map->getNodeFromMap(x+1, y-1))
			{
				retVal =  false;
			}
			else if( map->getCutCorners() )
			{
				if( !map->getNodeFromMap(x+1, y) && 
					!map->getNodeFromMap(x, y-1) )
				{
					retVal =  false;
				}
			}
			else
			{
				if( !map->getNodeFromMap(x+1, y) ||
					!map->getNodeFromMap(x, y-1) )
				{
					retVal =  false;
				}
			}
			break;

		case Jump::SE:
			if(!map->getNodeFromMap(x+1, y+1))
			{
				retVal =  false;
			}
			else if( map->getCutCorners() )
			{
				if( !map->getNodeFromMap(x+1, y) && 
					!map->getNodeFromMap(x, y+1) )
				{
					retVal =  false;
				}
			}
			else
			{
				if( !map->getNodeFromMap(x+1, y) ||
					!map->getNodeFromMap(x, y+1) )
				{
					retVal =  false;
				}
			}
			break;

		case Jump::NW:
			if(!map->getNodeFromMap(x-1, y-1))
			{
				retVal =  false;
			}
			else if( map->getCutCorners() )
			{
				if( !map->getNodeFromMap(x-1, y) && 
					!map->getNodeFromMap(x, y-1) )
				{
					retVal =  false;
				}
			}
			else
			{
				if( !map->getNodeFromMap(x-1, y) ||
					!map->getNodeFromMap(x, y-1) )
				{
					retVal =  false;
				}
			}
			break;

		case Jump::SW:
			if(!map->getNodeFromMap(x-1, y+1))
			{
				retVal =  false;
			}
			else if( map->getCutCorners() )
			{
				if( !map->getNodeFromMap(x-1, y) && 
					!map->getNodeFromMap(x, y+1) )
				{
					retVal =  false;
				}
			}
			else
			{
				if( !map->getNodeFromMap(x-1, y) ||
					!map->getNodeFromMap(x, y+1) )
				{
					retVal =  false;
				}
			}
			break;


		case Jump::NONE:
			retVal = false;
	}

	return retVal;
}

