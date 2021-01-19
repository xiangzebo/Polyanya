#include "JumpPointExpansionPolicy.h"

#include "JumpPointLocator.h"
#include "fpUtil.h"
#include "Heuristic.h"
#include "mapAbstraction.h"
#include "ProblemInstance.h"

#include <limits.h>

JumpPointExpansionPolicy::JumpPointExpansionPolicy(JumpPointLocator* _jpl)
	: ExpansionPolicy()
{
	neighbourIndex = 0;
	jpl = _jpl; 
}

JumpPointExpansionPolicy::~JumpPointExpansionPolicy()
{
	neighbours.clear();
	delete jpl;
}


void 
JumpPointExpansionPolicy::expand(node* t) throw(std::logic_error)
{
	ExpansionPolicy::expand(t);


	neighbours.clear();
	computeNeighbourSet();

	neighbourIndex = 0;
}

// Computes the set of jump point successors for ::target
// (the node which is currently being expanded).
void 
JumpPointExpansionPolicy::computeNeighbourSet()
{
	mapAbstraction* map = problem->getMap();
	int x = target->getLabelL(kFirstData);
	int y = target->getLabelL(kFirstData+1);
	bool cutCorners = map->getCutCorners();

	// Compute the direction of travel used to reach the current node.
	// We look for successors in this same direction as well as in the direction of
	// any forced neighbours.
	Jump::Direction which = Jump::computeDirection(target->backpointer, target);
	switch(which)
	{
		case Jump::S:
		{
			node* n = findJumpNode(Jump::S, x, y);
			if(n)
				neighbours.push_back(n);

			if( cutCorners )
			{
				// add SE neighbour only if E neighbour is forced 
				if(!map->getNodeFromMap(x+1, y))
				{
					n = findJumpNode(Jump::SE, x, y); 
					if(n)
						neighbours.push_back(n);
				}
				// add SW neighbour only if W neighbour is forced
				if(!map->getNodeFromMap(x-1, y))
				{
					n = findJumpNode(Jump::SW, x, y); 
					if(n)
						neighbours.push_back(n);
				}
			}
			// special rules when corner cutting is disallowed
			else 
			{
				// E & SE neighbours are forced if NE neighbour is null 
				if(!map->getNodeFromMap(x+1, y-1))
				{
					n = findJumpNode(Jump::E, x, y); 
					if(n)
						neighbours.push_back(n);

					n = findJumpNode(Jump::SE, x, y); 
					if(n)
						neighbours.push_back(n);
				}
				// W & SW neighbour are forced if NW neighbour is null 
				if(!map->getNodeFromMap(x-1, y-1))
				{
					n = findJumpNode(Jump::W, x, y); 
					if(n)
						neighbours.push_back(n);

					n = findJumpNode(Jump::SW, x, y); 
					if(n)
						neighbours.push_back(n);
				}
			}

			break;
		}

		case Jump::SW:
		{
			node* n = findJumpNode(Jump::SW, x, y);
			if(n)
				neighbours.push_back(n);

			n = findJumpNode(Jump::S, x, y);
			if(n)
				neighbours.push_back(n);

			n = findJumpNode(Jump::W, x, y);
			if(n)
				neighbours.push_back(n);

			if( cutCorners )
			{
				// add NW neighbour only if N neighbour is forced
				if(!map->getNodeFromMap(x, y-1))
				{
					n = findJumpNode(Jump::NW, x, y); 
					if(n)
						neighbours.push_back(n);
				}

				// add SE neighbour only if E neighbour is forced
				if(!map->getNodeFromMap(x+1, y))
				{
					n = findJumpNode(Jump::SE, x, y);
					if(n)
						neighbours.push_back(n);
				}
			}
			break;
		}

		case Jump::W:
		{
			node* n = findJumpNode(Jump::W, x, y);
			if(n)
				neighbours.push_back(n);

			if( cutCorners )
			{
				// add NW neighbour only if N neighbour is forced	
				if(!map->getNodeFromMap(x, y-1))
				{
					n = findJumpNode(Jump::NW, x, y);
					if(n)
						neighbours.push_back(n);
				}
				// add SW neighbour only if S neighbour is forced
				if(!map->getNodeFromMap(x, y+1))
				{
					n = findJumpNode(Jump::SW, x, y); 
					if(n)
						neighbours.push_back(n);
				}
			}
			// special rules when corner cutting is disallowed
			else
			{
				// N & NW are forced if NE neighbour is null
				if(!map->getNodeFromMap(x+1, y-1))
				{
					n = findJumpNode(Jump::N, x, y);
					if(n)
						neighbours.push_back(n);

					n = findJumpNode(Jump::NW, x, y);
					if(n)
						neighbours.push_back(n);
				}
				// S & SW are forced if SE neighbour is null
				if(!map->getNodeFromMap(x+1, y+1))
				{
					n = findJumpNode(Jump::S, x, y); 
					if(n)
						neighbours.push_back(n);

					n = findJumpNode(Jump::SW, x, y); 
					if(n)
						neighbours.push_back(n);
				}
			}
			break;
		}

		case Jump::NW:
		{
			node* n = findJumpNode(Jump::NW, x, y); 
			if(n)
				neighbours.push_back(n);

			n = findJumpNode(Jump::N, x, y);
			if(n)
				neighbours.push_back(n);

			n = findJumpNode(Jump::W, x, y);
			if(n)
				neighbours.push_back(n);

			if( cutCorners )
			{
				// add SW neighbour only if S neighbour is forced
				if(!map->getNodeFromMap(x, y+1))
				{
					n = findJumpNode(Jump::SW, x, y); 
					if(n)
						neighbours.push_back(n);
				}

				// add NE neighbour only if E neighbour is forced
				if(!map->getNodeFromMap(x+1, y))
				{
					n = findJumpNode(Jump::NE, x, y); 
					if(n)
						neighbours.push_back(n);
				}
			}
			break;
		}

		case Jump::N:
		{
			node* n = findJumpNode(Jump::N, x, y);
			if(n)
				neighbours.push_back(n);

			if( cutCorners )
			{
				// add NE neighbour only if E neighbour is forced 
				if(!map->getNodeFromMap(x+1, y))
				{
					n = findJumpNode(Jump::NE, x, y); 
					if(n)
						neighbours.push_back(n);
				}
				// add NW neighbour only if W neighbour is forced
				if(!map->getNodeFromMap(x-1, y))
				{
					n = findJumpNode(Jump::NW, x, y);
					if(n)
						neighbours.push_back(n);
				}
			}
			else
			{
				// E & NE are forced if SE neighbour is null
				if(!map->getNodeFromMap(x+1, y+1))
				{
					n = findJumpNode(Jump::E, x, y); 
					if(n)
						neighbours.push_back(n);

					n = findJumpNode(Jump::NE, x, y); 
					if(n)
						neighbours.push_back(n);
				}
				// W & NW are forced if SW neighbour is null
				if(!map->getNodeFromMap(x-1, y+1))
				{
					n = findJumpNode(Jump::W, x, y);
					if(n)
						neighbours.push_back(n);

					n = findJumpNode(Jump::NW, x, y);
					if(n)
						neighbours.push_back(n);
				}
			}
			break;
		}

		case Jump::NE:
		{
			node* n = findJumpNode(Jump::NE, x, y); 
			if(n)
				neighbours.push_back(n);

			n = findJumpNode(Jump::N, x, y);
			if(n)
				neighbours.push_back(n);

			n = findJumpNode(Jump::E, x, y);
			if(n)
				neighbours.push_back(n);

			if( cutCorners )
			{
				// add SE neighbour only if S neighbour is forced
				if(!map->getNodeFromMap(x, y+1))
				{
					n = findJumpNode(Jump::SE, x, y);
					if(n)
						neighbours.push_back(n);
				}

				// add NW neighbour only if W neighbour is forced
				if(!map->getNodeFromMap(x-1, y))
				{
					n = findJumpNode(Jump::NW, x, y);
					if(n)
						neighbours.push_back(n);
				}
			}
			break;
		}

		case Jump::E:
		{
			node* n = findJumpNode(Jump::E, x, y);
			if(n)
				neighbours.push_back(n);

			if( cutCorners )
			{
				// add NE neighbour only if N neighbour is forced 
				if(!map->getNodeFromMap(x, y-1))
				{
					n = findJumpNode(Jump::NE, x, y);
					if(n)
						neighbours.push_back(n);
				}
				// add SE neighbour only if S neighbour is forced
				if(!map->getNodeFromMap(x, y+1))
				{
					n = findJumpNode(Jump::SE, x, y);
					if(n)
						neighbours.push_back(n);
				}
			}
			// special jumping rules when corner cutting is disallowed
			else
			{
				// N and NE neighbours are forced if NW is null 
				if(!map->getNodeFromMap(x-1, y-1))
				{
					n = findJumpNode(Jump::N, x, y);
					if(n)
						neighbours.push_back(n);

					n = findJumpNode(Jump::NE, x, y);
					if(n)
						neighbours.push_back(n);

				}
				// S and SE are forced if SW is null
				if(!map->getNodeFromMap(x-1, y+1))
				{
					n = findJumpNode(Jump::S, x, y);
					if(n)
						neighbours.push_back(n);

					n = findJumpNode(Jump::SE, x, y);
					if(n)
						neighbours.push_back(n);
				}
			}
			break;
		}

		case Jump::SE:
		{
			node* n = findJumpNode(Jump::SE, x, y);
			if(n)
				neighbours.push_back(n);

			n = findJumpNode(Jump::S, x, y);
			if(n)
				neighbours.push_back(n);

			n = findJumpNode(Jump::E, x, y);
			if(n)
				neighbours.push_back(n);

			if( cutCorners )
			{
				// add NE neighbour only if N neighbour is forced
				if(!map->getNodeFromMap(x, y-1))
				{
					n = findJumpNode(Jump::NE, x, y); 
					if(n)
						neighbours.push_back(n);
				}

				// add SW neighbour only if W neighbour is forced
				if(!map->getNodeFromMap(x-1, y))
				{
					n = findJumpNode(Jump::SW, x, y);
					if(n)
						neighbours.push_back(n);
				}
			}
			break;
		}
		case Jump::NONE:
		{
			// when a node has no parent (usually only the start node)
			// we generate jump point successors in every traversable direction
			node* n = findJumpNode(Jump::N, x, y);
			if(n)
				neighbours.push_back(n);

			n = findJumpNode(Jump::S, x, y);
			if(n)
				neighbours.push_back(n);

			n = findJumpNode(Jump::E, x, y);
			if(n)
				neighbours.push_back(n);

			n = findJumpNode(Jump::W, x, y);
			if(n)
				neighbours.push_back(n);

			n = findJumpNode(Jump::NE, x, y);
			if(n)
				neighbours.push_back(n);

			n = findJumpNode(Jump::NW, x, y);
			if(n)
				neighbours.push_back(n);

			n = findJumpNode(Jump::SE, x, y);
			if(n)
				neighbours.push_back(n);

			n = findJumpNode(Jump::SW, x, y);
			if(n)
				neighbours.push_back(n);

		}
	}
}

node* 
JumpPointExpansionPolicy::first()
{
	if(neighbours.size() > 0)
		return neighbours.at(0);
	return 0;
}

node* 
JumpPointExpansionPolicy::next()
{
	node* nextnode = 0;

	if(hasNext())
	{
		neighbourIndex++;
		nextnode = n();
	}

	return nextnode;
}

node* 
JumpPointExpansionPolicy::n()
{
	node* retVal = 0;
	unsigned int numNeighbours = neighbours.size();
	if(numNeighbours > 0 && neighbourIndex < numNeighbours)
	{
		retVal = neighbours.at(neighbourIndex);
	}
	return retVal;
}

double 
JumpPointExpansionPolicy::cost_to_n()
{
	node* current = n();
	return problem->getHeuristic()->h(target, current);
}

bool 
JumpPointExpansionPolicy::hasNext()
{
	if(neighbourIndex+1 < neighbours.size())
		return true;
	return false;
}



// Finds the nearest jump node neighbour (if any) for the target node
// in a given direction.
//
// @return: a jump node (null  if none is found)
node*
JumpPointExpansionPolicy::findJumpNode(Jump::Direction d, int x, int y)
{
	int goalx = problem->getGoalNode()->getLabelL(kFirstData);
	int goaly = problem->getGoalNode()->getLabelL(kFirstData+1);
	return jpl->findJumpNode(d, x, y, goalx, goaly);
}

void
JumpPointExpansionPolicy::label_n()
{
	node* tmp = this->n();
	if(tmp)
		tmp->backpointer = this->target;
}
