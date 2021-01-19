#include "OnlineJumpPointLocator.h"
#include "graph.h"
#include "mapAbstraction.h"

#include <climits>

OnlineJumpPointLocator::OnlineJumpPointLocator(mapAbstraction* _map) 
	: JumpPointLocator(_map)
{
}

OnlineJumpPointLocator::~OnlineJumpPointLocator()
{
}


// Finds a jump point successor of node (x, y) in Direction d.
// Also given is the location of the goal node (goalx, goaly) for a particular
// search instance. If encountered, the goal node is always returned as a 
// jump point successor.
//
// @return: a jump point successor or 0 if no such jump point exists.
node*
OnlineJumpPointLocator::findJumpNode(Jump::Direction d, int x, int y, 
		int goalx, int goaly)
{
	node* n = 0;
	switch(d)
	{
		case Jump::N:
		{
			for(int steps=1; steps <= jumplimit ; steps++)
			{
				int ny = y-steps;
				n = map->getNodeFromMap(x, ny);
				if(n == 0)
					break;

				if(x == goalx && ny == goaly)
					break;

				// n is a jump node if we cannot prove a symmetric path to 
				// either of two diagonal neighbours exists
				if(!canStep(x, y-(steps-1), Jump::NW) && 
						canStep(x, ny, Jump::W))
				{
					break;
				}

				if(!canStep(x, y-(steps-1), Jump::NE) && 
						canStep(x, ny, Jump::E))
				{
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
				if(n == 0)
					break;

				if(x == goalx && ny == goaly)
					break;
				
				// n is a jump node if we cannot prove a symmetric path to 
				// either of two diagonal neighbours exists
				if(!canStep(x, y+(steps-1), Jump::SW) && 
						canStep(x, ny, Jump::W))
				{
					break;
				}

				if(!canStep(x, y+(steps-1), Jump::SE) && 
						canStep(x, ny, Jump::E))
				{
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
				if(n == 0)
					break;

				if(nx == goalx && y == goaly)
					break;
				
				// n is a jump node if we cannot prove a symmetric path to 
				// either of two diagonal neighbours exists
				if(!canStep(x+(steps-1), y, Jump::NE) && 
						canStep(nx, y, Jump::N))
				{
					break;
				}

				if(!canStep(x+(steps-1), y, Jump::SE) && 
						canStep(nx, y, Jump::S))
				{
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
				if(n == 0)
					break;

				if(nx == goalx && y == goaly)
					break;

				
				// n is a jump node if we cannot prove a symmetric path to 
				// either of two diagonal neighbours exists
				if(!canStep(x-(steps-1), y, Jump::NW) && 
						canStep(nx, y, Jump::N))
				{
					break;
				}

				if(!canStep(x-(steps-1), y, Jump::SW) && 
						canStep(nx, y, Jump::S))
				{
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
				{
					n = map->getNodeFromMap(nx, ny);
				}
				else
				{
					n = 0;
				}

				// stop if we hit an obstacle (no jump node exists)
				if(n == 0)
					break;

				// stop if we hit the goal (always a jump node)
				if(nx == goalx && ny == goaly)
					break;

				// n is a jump node if we can reach other jump nodes by
				// travelling vertically or horizontally 
				if(findJumpNode(Jump::N, nx, ny, goalx, goaly) || 
					findJumpNode(Jump::E, nx, ny, goalx, goaly))
				{
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
				{
					n = map->getNodeFromMap(nx, ny);
				}
				else
				{
					n = 0;
				}

				if(n == 0)
					break;

				if(nx == goalx && ny == goaly)
					break;
				
				if(findJumpNode(Jump::S, nx, ny, goalx, goaly) || 
					findJumpNode(Jump::E, nx, ny, goalx, goaly))
				{
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
				{
					n = map->getNodeFromMap(nx, ny);
				}
				else
				{
					n = 0;
				}

				if(n == 0)
					break;

				if(nx == goalx && ny == goaly)
					break;

				if(findJumpNode(Jump::N, nx, ny, goalx, goaly) || 
					findJumpNode(Jump::W, nx, ny, goalx, goaly))
				{
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
				{
					n = map->getNodeFromMap(nx, ny);
				}
				else
				{
					n = 0;
				}

				if(n == 0)
					break;

				if(nx == goalx && ny == goaly)
					break;

				if(findJumpNode(Jump::S, nx, ny, goalx, goaly) ||
					findJumpNode(Jump::W, nx, ny, goalx, goaly))
				{
					break;
				}
			}
			break;
		}
		default:
			break;
	}
	return n;
}

