#include "OfflineJumpPointLocator.h"
#include "JumpPointAbstraction.h"
#include <cstdlib>
#include <climits>

OfflineJumpPointLocator::OfflineJumpPointLocator(JumpPointAbstraction* _map)
	: JumpPointLocator(_map)
{
	assert(_map);
}

OfflineJumpPointLocator::~OfflineJumpPointLocator()
{
}

// Returns the jump point successor of node (x, y) in Direction d. 
// If the edge (target, neighbour) is a transition which involves jumping
// over the row or column of the goal node we return instead the node
// on the row or column of the goal node. 
//
// This procedure is necessary to ensure we do not miss the goal node
// while searching on a map of type JumpPointAbstraction.
node*
OfflineJumpPointLocator::findJumpNode(Jump::Direction d, int x, int y, 
		int gx, int gy)
{
	node* target = map->getNodeFromMap(x, y);
	int offset = calculateEdgeIndex(d);
	edge_iterator iter = target->getOutgoingEdgeIter() + offset;
	edge* e = target->edgeIterNextOutgoing(iter);

	graph* g = map->getAbstractGraph(0);
	node* n = 0;
	bool sterile = false;
	if(e->getFrom() == e->getTo())
	{
		sterile = true;
		n = findSterileJumpNode(d, x, y);
		if(n == 0)
			return n;
	}
	else
	{
		n = g->getNode(e->getTo());
	}

	int tx = target->getLabelL(kFirstData);
	int ty = target->getLabelL(kFirstData+1);

	int nx = n->getLabelL(kFirstData);
	int ny = n->getLabelL(kFirstData+1);

	int deltay = abs(gy - ty);
	int deltax = abs(gx - tx);

	// the edge (target, n) crosses both row and column of the goal node
	if(((tx < gx && gx <= nx) || (tx > gx && gx >= nx)) && 
			((ty < gy && gy <= ny) || (ty > gy && gy >= ny)))
	{
		// pick from the two possible n nodes the one closer to g
		if(deltax < deltay) 
		{
			nx = gx;
			ny = (ny > ty)?(ty + deltax):(ty - deltax);
		}
		else
		{
			ny = gy;
			nx = (nx > tx)?(tx + deltay):(tx - deltay);
		}
	}
	// the edge (target, n) crosses only the column of the goal node
	else if((tx < gx && gx <= nx) || (tx > gx && gx >= nx))
	{
		nx = gx;
		// (target, n) is a diagonal transition 
		if(ty != ny)
			ny = (ny > ty)?(ty + deltax):(ty - deltax);
	}
	// the edge (target, n) crosses only the row of the goal node
	else if((ty < gy && gy <= ny) || (ty > gy && gy >= ny))
	{
		ny = gy;

		// (target, n) is a diagonal transition 
		if(tx != nx)
			nx = (nx > tx)?(tx + deltay):(tx - deltay);
	}

	// never return sterile jump points (unless they are the goal or are
	// otherwise located on the same row or column as the goal)
	if(sterile)
	{
		if(nx == n->getLabelL(kFirstData) && ny == n->getLabelL(kFirstData+1))
		{
			if(nx == gx || ny == gy)
				n = map->getNodeFromMap(nx, ny);
			else 
				n = 0;
		}
		else
			n = map->getNodeFromMap(nx, ny);
	}
	else
	{
		n = map->getNodeFromMap(nx, ny);
	}

	return n;
}

// This method identifies sterile jump point successors of node (x, y). These
// nodes were not added to the jump point graph during pre-processing as they
// lie in dead-end locations that do not lead anywhere. 
// Identifying such nodes during search is only necessary to make sure we do not miss 
// the goal; they never need to be generated (unless such a node is the goal
// itself).
//
// The identification procedure is simpler than for regular jump points: just
// step in direction d until an obstacle, or the edge of the map, is reached.
// Then, return the traversable node immediately prior.
//
// @return: a sterile jump point successor, or 0 if no such node exists.
node*
OfflineJumpPointLocator::findSterileJumpNode(Jump::Direction d, int x, int y)
{
	//const int jumplimit = INT_MAX;
	node* n = 0; 

	switch(d)
	{
		case Jump::N:
		{
			for(int steps=1; steps <= jumplimit ; steps++)
			{
				int ny = y-steps;
				n = map->getNodeFromMap(x, ny);

				// the node just before an obstacle is a jump point
				if(n == 0)
				{
					n = map->getNodeFromMap(x, y-(steps-1));
					break;
				}
			}
			break;
		}

		case Jump::S:
		{
			for(int steps=1; steps <= jumplimit ; steps++)
			{
				int ny = y+steps;
				n = map->getNodeFromMap(x, ny);
				
				// the node just before an obstacle is a jump point
				if(n == 0)
				{
					n = map->getNodeFromMap(x, y+(steps-1));
					break;
					
				}
			}
			break;
		}

		case Jump::E:
		{
			for(int steps=1; steps <= jumplimit ; steps++)
			{
				int nx = x+steps;
				n = map->getNodeFromMap(nx, y);
				
				// the node just before an obstacle is a jump point
				if(n == 0)
				{
					n = map->getNodeFromMap(x+(steps-1), y);
					break;
				}
			}
			break;
		}

		case Jump::W:
		{

			for(int steps=1; steps <= jumplimit ; steps++)
			{
				int nx = x-steps;
				n = map->getNodeFromMap(nx, y);
				
				// the node just before an obstacle is a jump point
				if(n == 0)
				{
					n = map->getNodeFromMap(x-(steps-1), y);
					break;
				}
			}
			break;
		}

		case Jump::NE:
		{
			for(int steps=1; steps <= jumplimit ; steps++)
			{
				int nx = x+steps;
				int ny = y-steps;
				if(canStep(x+(steps-1), y-(steps-1), Jump::NE))
					n = map->getNodeFromMap(nx, ny);
				else
					n = 0;
				
				// the node just before an obstacle is a jump point
				if(n == 0)
				{
					n = map->getNodeFromMap(x+(steps-1), y-(steps-1));
					break;
					
				}
			}
			break;
		}

		case Jump::SE:
		{
			for(int steps=1; steps <= jumplimit ; steps++)
			{
				int nx = x+steps;
				int ny = y+steps;
				if(canStep(x+(steps-1), y+(steps-1), Jump::SE))
					n = map->getNodeFromMap(nx, ny);
				else
					n = 0;

				// the node just before an obstacle is a jump point
				if(n == 0)
				{
					n = map->getNodeFromMap(x+(steps-1), y+(steps-1));
					break;
					
				}
			}
			break;
		}

		case Jump::NW:
		{
			for(int steps=1; steps <= jumplimit ; steps++)
			{
				int nx = x-steps; 
				int ny = y-steps;
				if(canStep(x-(steps-1), y-(steps-1), Jump::NW))
					n = map->getNodeFromMap(nx, ny);
				else
					n = 0;

				// the node just before an obstacle is a jump point
				if(n == 0)
				{
					n = map->getNodeFromMap(x-(steps-1), y-(steps-1));
					break;
				}
			}
			break;
		}

		case Jump::SW:
		{
			for(int steps=1; steps <= jumplimit ; steps++)
			{
				int nx = x-steps;
				int ny = y+steps;
				if(canStep(x-(steps-1), y+(steps-1), Jump::SW))
					n = map->getNodeFromMap(nx, ny);
				else
					n = 0;

				// the node just before an obstacle is a jump point
				if(n == 0)
				{
					n = map->getNodeFromMap(x-(steps-1), y+(steps-1));
					break;
				}
			}
			break;
		}

		default:
			break;
	}

	node* tmp = map->getNodeFromMap(x, y);
	if(tmp->getNum() == n->getNum())
		n = 0;	

	return n;
}

// given a direction for a neighbour, return the corresponding index
// for the edge to that neighbour in the array of edges stored by the 
// target node.
int
OfflineJumpPointLocator::calculateEdgeIndex(Jump::Direction dir)
{
	int idx = 0;
	switch(dir)
	{
		case Jump::NONE:
			std::cerr << "JPAExpansionPolicy::n neighbour in direction Jump::NONE?!";
			exit(1);
		case Jump::N:
			break;
		case Jump::NE:
			idx++;
			break;
		case Jump::E:
			idx+=2;
			break;
		case Jump::SE:
			idx+=3;
			break;
		case Jump::S:
			idx+=4;
			break;
		case Jump::SW:
			idx+=5;
			break;
		case Jump::W:
			idx+=6;
			break;
		case Jump::NW:
			idx+=7;
			break;
	}
	return idx;
}
