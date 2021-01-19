/*
 * JPSPlus.cpp
 *
 * Copyright (c) 2015, Steve Rabin
 * All rights reserved.
 *
 * An explanation of the JPS+ algorithm and this code is contained in Chapter 14
 * of the book Game AI Pro 2, edited by Steve Rabin, CRC Press, 2015.
 * A copy of this code is on the website http://www.gameaipro.com.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * The name of the author may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY STEVE RABIN ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */ 

#include "JPSPlus.h"

#define USE_FAST_OPEN_LIST_AT_THE_COST_OF_MUCH_MORE_MEMORY	// Roughly 10 times faster, requiring ~100MB more memory

#define FIXED_POINT_MULTIPLIER 100000
#define FIXED_POINT_SHIFT(x) ((x) * FIXED_POINT_MULTIPLIER)
#define SQRT_2 141421
#define SQRT_2_MINUS_ONE 41421


JPSPlus::JPSPlus(DistantJumpPoints** map)
{
	m_map = map;
	m_currentIteration = 1;

	// Initialize nodes
	for (int r = 0; r<MAX_HEIGHT; r++)
	{
		for (int c = 0; c<MAX_WIDTH; c++)
		{
			PathfindingNode& node = m_mapNodes[r][c];
			node.m_row = r;
			node.m_col = c;
			node.m_listStatus = PathfindingNode::OnNone;
			node.m_iteration = 0;
		}
	}
}

JPSPlus::~JPSPlus()
{
}

bool JPSPlus::GetPath(void *data, xyLocJPS& s, xyLocJPS& g, std::vector<xyLocJPS> &path)
{
	JPSPlus* Search = (JPSPlus*)data;

	if (path.size() > 0)
	{
		path.push_back(g);
		return true;
	}

	{
		// Initialize map
		int startRow = s.y;
		int startCol = s.x;
		m_goalRow = g.y;
		m_goalCol = g.x;

		path.clear();

		m_goalNode = &m_mapNodes[m_goalRow][m_goalCol];
		m_currentIteration++;

#ifdef USE_FAST_OPEN_LIST_AT_THE_COST_OF_MUCH_MORE_MEMORY
		m_fastOpenList.Reset();
#else
		m_openList.reset();
		m_openList.reserve(3000);
#endif

		// Push the starting node onto the Open list
		PathfindingNode* startNode = &m_mapNodes[startRow][startCol];
		startNode->m_parent = NULL;
		startNode->m_givenCost = 0;
		startNode->m_finalCost = 0;
		startNode->m_listStatus = PathfindingNode::OnOpen;
		startNode->m_iteration = m_currentIteration;

#ifdef USE_FAST_OPEN_LIST_AT_THE_COST_OF_MUCH_MORE_MEMORY
		m_fastOpenList.Push(&m_mapNodes[startRow][startCol]);
#else
		m_openList.add(&m_mapNodes[startRow][startCol]);
#endif
	}

	PathStatus status = SearchLoop();

	if (status == PathFound)
	{
		FinalizePath(path);
		if (path.size() > 0)
		{
			path.pop_back();
			return false;
		}
		return true;
	}
	else
	{
		// No path
		return true;
	}
}

PathStatus JPSPlus::SearchLoop(void)
{
#ifdef USE_FAST_OPEN_LIST_AT_THE_COST_OF_MUCH_MORE_MEMORY
	if (!m_fastOpenList.Empty())
#else
	if (!m_openList.empty())
#endif
	{
		// Special case for the starting node

#ifdef USE_FAST_OPEN_LIST_AT_THE_COST_OF_MUCH_MORE_MEMORY
		PathfindingNode* currentNode = m_fastOpenList.Pop();
#else
		PathfindingNode* currentNode = m_openList.remove();
#endif

		if (currentNode == m_goalNode)
		{
			// Found goal
			return PathFound;
		}

		DistantJumpPoints* distantJumpPoints = &m_map[currentNode->m_row][currentNode->m_col];

		ExploreFromParentAllDirections(currentNode, distantJumpPoints);

		currentNode->m_listStatus = PathfindingNode::OnClosed;
	}

#ifdef USE_FAST_OPEN_LIST_AT_THE_COST_OF_MUCH_MORE_MEMORY
	while (!m_fastOpenList.Empty())
#else
	while (!m_openList.empty())
#endif
	{
#ifdef USE_FAST_OPEN_LIST_AT_THE_COST_OF_MUCH_MORE_MEMORY
		PathfindingNode* currentNode = m_fastOpenList.Pop();
#else
		PathfindingNode* currentNode = m_openList.remove();
#endif

		if (currentNode == m_goalNode)
		{
			// Found goal
			return PathFound;
		}

		int row = currentNode->m_row;
		int col = currentNode->m_col;
		int parentRow = currentNode->m_parent->m_row;
		int parentCol = currentNode->m_parent->m_col;

		DistantJumpPoints* distantJumpPoints = &m_map[row][col];

		// Explore nodes based on parent
		if (parentRow < row)
		{
			if (parentCol < col)
			{
				ExploreFromParentDownRight(currentNode, distantJumpPoints);
			}
			else if (parentCol > col)
			{
				ExploreFromParentDownLeft(currentNode, distantJumpPoints);
			}
			else
			{
				ExploreFromParentDown(currentNode, distantJumpPoints);
			}
		}
		else if (parentRow > row)
		{
			if (parentCol < col)
			{
				ExploreFromParentUpRight(currentNode, distantJumpPoints);
			}
			else if (parentCol > col)
			{
				ExploreFromParentUpLeft(currentNode, distantJumpPoints);
			}
			else
			{
				ExploreFromParentUp(currentNode, distantJumpPoints);
			}
		}
		else
		{
			if (parentCol < col)
			{
				ExploreFromParentRight(currentNode, distantJumpPoints);
			}
			else
			{
				ExploreFromParentLeft(currentNode, distantJumpPoints);
			}
		}

		currentNode->m_listStatus = PathfindingNode::OnClosed;
	}
	return NoPathExists;
}

void JPSPlus::FinalizePath(std::vector<xyLocJPS> &finalPath)
{
	PathfindingNode* prevNode = NULL;
	PathfindingNode* curNode = m_goalNode;

	while (curNode != NULL)
	{
		xyLocJPS loc;
		loc.x = curNode->m_col;
		loc.y = curNode->m_row;

		if (prevNode != NULL)
		{
			// Insert extra nodes if needed (may not be neccessary depending on final path use)
			int xDiff = curNode->m_col - prevNode->m_col;
			int yDiff = curNode->m_row - prevNode->m_row;

			int xInc = 0;
			int yInc = 0;

			if (xDiff > 0) { xInc = 1; }
			else if (xDiff < 0) { xInc = -1; xDiff = -xDiff; }

			if (yDiff > 0) { yInc = 1; }
			else if (yDiff < 0) { yInc = -1; yDiff = -yDiff; }

			int x = prevNode->m_col;
			int y = prevNode->m_row;
			int steps = xDiff - 1;
			if (yDiff > xDiff) { steps = yDiff - 1; }

			for (int i = 0; i < steps; i++)
			{
				x += xInc;
				y += yInc;

				xyLocJPS locNew;
				locNew.x = x;
				locNew.y = y;

				finalPath.push_back(locNew);
			}
		}

		finalPath.push_back(loc);
		prevNode = curNode;
		curNode = curNode->m_parent;
	}
	std::reverse(finalPath.begin(), finalPath.end());
}

inline void JPSPlus::ExploreFromParentDown(PathfindingNode * currentNode, DistantJumpPoints * distantJumpPoints)
{
	if (distantJumpPoints->jumpDistance[Left] != 0) SearchLeft(currentNode, distantJumpPoints->jumpDistance[Left]);
	if (distantJumpPoints->jumpDistance[DownLeft] != 0) SearchDownLeft(currentNode, distantJumpPoints->jumpDistance[DownLeft]);
	if (distantJumpPoints->jumpDistance[Down] != 0) SearchDown(currentNode, distantJumpPoints->jumpDistance[Down]);
	if (distantJumpPoints->jumpDistance[DownRight] != 0) SearchDownRight(currentNode, distantJumpPoints->jumpDistance[DownRight]);
	if (distantJumpPoints->jumpDistance[Right] != 0) SearchRight(currentNode, distantJumpPoints->jumpDistance[Right]);
}

inline void JPSPlus::ExploreFromParentDownRight(PathfindingNode * currentNode, DistantJumpPoints * distantJumpPoints)
{
	if (distantJumpPoints->jumpDistance[Down] != 0) SearchDown(currentNode, distantJumpPoints->jumpDistance[Down]);
	if (distantJumpPoints->jumpDistance[DownRight] != 0) SearchDownRight(currentNode, distantJumpPoints->jumpDistance[DownRight]);
	if (distantJumpPoints->jumpDistance[Right] != 0) SearchRight(currentNode, distantJumpPoints->jumpDistance[Right]);
}

inline void JPSPlus::ExploreFromParentRight(PathfindingNode * currentNode, DistantJumpPoints * distantJumpPoints)
{
	if (distantJumpPoints->jumpDistance[Down] != 0) SearchDown(currentNode, distantJumpPoints->jumpDistance[Down]);
	if (distantJumpPoints->jumpDistance[DownRight] != 0) SearchDownRight(currentNode, distantJumpPoints->jumpDistance[DownRight]);
	if (distantJumpPoints->jumpDistance[Right] != 0) SearchRight(currentNode, distantJumpPoints->jumpDistance[Right]);
	if (distantJumpPoints->jumpDistance[UpRight] != 0) SearchUpRight(currentNode, distantJumpPoints->jumpDistance[UpRight]);
	if (distantJumpPoints->jumpDistance[Up] != 0) SearchUp(currentNode, distantJumpPoints->jumpDistance[Up]);
}

inline void JPSPlus::ExploreFromParentUpRight(PathfindingNode * currentNode, DistantJumpPoints * distantJumpPoints)
{
	if (distantJumpPoints->jumpDistance[Right] != 0) SearchRight(currentNode, distantJumpPoints->jumpDistance[Right]);
	if (distantJumpPoints->jumpDistance[UpRight] != 0) SearchUpRight(currentNode, distantJumpPoints->jumpDistance[UpRight]);
	if (distantJumpPoints->jumpDistance[Up] != 0) SearchUp(currentNode, distantJumpPoints->jumpDistance[Up]);
}

inline void JPSPlus::ExploreFromParentUp(PathfindingNode * currentNode, DistantJumpPoints * distantJumpPoints)
{
	if (distantJumpPoints->jumpDistance[Right] != 0) SearchRight(currentNode, distantJumpPoints->jumpDistance[Right]);
	if (distantJumpPoints->jumpDistance[UpRight] != 0) SearchUpRight(currentNode, distantJumpPoints->jumpDistance[UpRight]);
	if (distantJumpPoints->jumpDistance[Up] != 0) SearchUp(currentNode, distantJumpPoints->jumpDistance[Up]);
	if (distantJumpPoints->jumpDistance[UpLeft] != 0) SearchUpLeft(currentNode, distantJumpPoints->jumpDistance[UpLeft]);
	if (distantJumpPoints->jumpDistance[Left] != 0) SearchLeft(currentNode, distantJumpPoints->jumpDistance[Left]);
}

inline void JPSPlus::ExploreFromParentUpLeft(PathfindingNode * currentNode, DistantJumpPoints * distantJumpPoints)
{
	if (distantJumpPoints->jumpDistance[Up] != 0) SearchUp(currentNode, distantJumpPoints->jumpDistance[Up]);
	if (distantJumpPoints->jumpDistance[UpLeft] != 0) SearchUpLeft(currentNode, distantJumpPoints->jumpDistance[UpLeft]);
	if (distantJumpPoints->jumpDistance[Left] != 0) SearchLeft(currentNode, distantJumpPoints->jumpDistance[Left]);
}

inline void JPSPlus::ExploreFromParentLeft(PathfindingNode * currentNode, DistantJumpPoints * distantJumpPoints)
{
	if (distantJumpPoints->jumpDistance[Up] != 0) SearchUp(currentNode, distantJumpPoints->jumpDistance[Up]);
	if (distantJumpPoints->jumpDistance[UpLeft] != 0) SearchUpLeft(currentNode, distantJumpPoints->jumpDistance[UpLeft]);
	if (distantJumpPoints->jumpDistance[Left] != 0) SearchLeft(currentNode, distantJumpPoints->jumpDistance[Left]);
	if (distantJumpPoints->jumpDistance[DownLeft] != 0) SearchDownLeft(currentNode, distantJumpPoints->jumpDistance[DownLeft]);
	if (distantJumpPoints->jumpDistance[Down] != 0) SearchDown(currentNode, distantJumpPoints->jumpDistance[Down]);
}

inline void JPSPlus::ExploreFromParentDownLeft(PathfindingNode * currentNode, DistantJumpPoints * distantJumpPoints)
{
	if (distantJumpPoints->jumpDistance[Left] != 0) SearchLeft(currentNode, distantJumpPoints->jumpDistance[Left]);
	if (distantJumpPoints->jumpDistance[DownLeft] != 0) SearchDownLeft(currentNode, distantJumpPoints->jumpDistance[DownLeft]);
	if (distantJumpPoints->jumpDistance[Down] != 0) SearchDown(currentNode, distantJumpPoints->jumpDistance[Down]);
}

inline void JPSPlus::ExploreFromParentAllDirections(PathfindingNode * currentNode, DistantJumpPoints * distantJumpPoints)
{
	SearchDown(currentNode, distantJumpPoints->jumpDistance[Down]);
	SearchDownRight(currentNode, distantJumpPoints->jumpDistance[DownRight]);
	SearchRight(currentNode, distantJumpPoints->jumpDistance[Right]);
	SearchUpRight(currentNode, distantJumpPoints->jumpDistance[UpRight]);
	SearchUp(currentNode, distantJumpPoints->jumpDistance[Up]);
	SearchUpLeft(currentNode, distantJumpPoints->jumpDistance[UpLeft]);
	SearchLeft(currentNode, distantJumpPoints->jumpDistance[Left]);
	SearchDownLeft(currentNode, distantJumpPoints->jumpDistance[DownLeft]);
}

void JPSPlus::SearchDown(PathfindingNode * currentNode, int jumpDistance)
{
	int row = currentNode->m_row;
	int col = currentNode->m_col;

	// Consider straight line to Goal
	if (col == m_goalCol && row < m_goalRow)
	{
		int absJumpDistance = jumpDistance;
		if (absJumpDistance < 0) { absJumpDistance = -absJumpDistance; }

		if ((row + absJumpDistance) >= m_goalRow)
		{
			PathfindingNode * newSuccessor = m_goalNode;
			int diff = m_goalRow - row;
			unsigned int givenCost = currentNode->m_givenCost + FIXED_POINT_SHIFT(diff);
			PushNewNode(newSuccessor, currentNode, givenCost);
			return;
		}
	}

	if (jumpDistance > 0)
	{
		// Directly jump
		int newRow = row + jumpDistance;
		unsigned int givenCost = currentNode->m_givenCost + FIXED_POINT_SHIFT(jumpDistance);
		PathfindingNode * newSuccessor = &m_mapNodes[newRow][col];
		PushNewNode(newSuccessor, currentNode, givenCost);
	}
}

void JPSPlus::SearchDownRight(PathfindingNode * currentNode, int jumpDistance)
{
	int row = currentNode->m_row;
	int col = currentNode->m_col;

	// Check for goal in general direction (straight line to Goal or Target Jump Point)
	if (row < m_goalRow && col < m_goalCol)
	{
		int absJumpDistance = jumpDistance;
		if (absJumpDistance < 0) { absJumpDistance = -absJumpDistance; }

		int diffRow = m_goalRow - row;
		int diffCol = m_goalCol - col;
		int smallerDiff = diffRow;
		if (diffCol < smallerDiff) { smallerDiff = diffCol; }

		if (smallerDiff <= absJumpDistance)
		{
			int newRow = row + smallerDiff;
			int newCol = col + smallerDiff;
			unsigned int givenCost = currentNode->m_givenCost + (SQRT_2 * smallerDiff);
			PathfindingNode * newSuccessor = &m_mapNodes[newRow][newCol];
			PushNewNode(newSuccessor, currentNode, givenCost);
			return;
		}
	}

	if (jumpDistance > 0)
	{
		// Directly jump
		int newRow = currentNode->m_row + jumpDistance;
		int newCol = currentNode->m_col + jumpDistance;
		unsigned int givenCost = currentNode->m_givenCost + (SQRT_2 * jumpDistance);
		PathfindingNode * newSuccessor = &m_mapNodes[newRow][newCol];
		PushNewNode(newSuccessor, currentNode, givenCost);
	}
}

void JPSPlus::SearchRight(PathfindingNode * currentNode, int jumpDistance)
{
	int row = currentNode->m_row;
	int col = currentNode->m_col;

	// Consider straight line to Goal
	if (row == m_goalRow && col < m_goalCol)
	{
		int absJumpDistance = jumpDistance;
		if (absJumpDistance < 0) { absJumpDistance = -absJumpDistance; }

		if ((col + absJumpDistance) >= m_goalCol)
		{
			PathfindingNode * newSuccessor = m_goalNode;
			int diff = m_goalCol - col;
			unsigned int givenCost = currentNode->m_givenCost + FIXED_POINT_SHIFT(diff);
			PushNewNode(newSuccessor, currentNode, givenCost);
			return;
		}
	}

	if (jumpDistance > 0)
	{
		// Directly jump
		int newCol = col + jumpDistance;
		unsigned int givenCost = currentNode->m_givenCost + FIXED_POINT_SHIFT(jumpDistance);
		PathfindingNode * newSuccessor = &m_mapNodes[row][newCol];
		PushNewNode(newSuccessor, currentNode, givenCost);
	}
}

void JPSPlus::SearchUpRight(PathfindingNode * currentNode, int jumpDistance)
{
	int row = currentNode->m_row;
	int col = currentNode->m_col;

	// Check for goal in general direction (straight line to Goal or Target Jump Point)
	if (row > m_goalRow && col < m_goalCol)
	{
		int absJumpDistance = jumpDistance;
		if (absJumpDistance < 0) { absJumpDistance = -absJumpDistance; }

		int diffRow = row - m_goalRow;
		int diffCol = m_goalCol - col;
		int smallerDiff = diffRow;
		if (diffCol < smallerDiff) { smallerDiff = diffCol; }

		if (smallerDiff <= absJumpDistance)
		{
			int newRow = row - smallerDiff;
			int newCol = col + smallerDiff;
			unsigned int givenCost = currentNode->m_givenCost + (SQRT_2 * smallerDiff);
			PathfindingNode * newSuccessor = &m_mapNodes[newRow][newCol];
			PushNewNode(newSuccessor, currentNode, givenCost);
			return;
		}
	}

	if (jumpDistance > 0)
	{
		// Directly jump
		int newRow = row - jumpDistance;
		int newCol = col + jumpDistance;
		unsigned int givenCost = currentNode->m_givenCost + (SQRT_2 * jumpDistance);
		PathfindingNode * newSuccessor = &m_mapNodes[newRow][newCol];
		PushNewNode(newSuccessor, currentNode, givenCost);
	}
}

void JPSPlus::SearchUp(PathfindingNode * currentNode, int jumpDistance)
{
	int row = currentNode->m_row;
	int col = currentNode->m_col;

	// Consider straight line to Goal
	if (col == m_goalCol && row > m_goalRow)
	{
		int absJumpDistance = jumpDistance;
		if (absJumpDistance < 0) { absJumpDistance = -absJumpDistance; }

		if ((row - absJumpDistance) <= m_goalRow)
		{
			PathfindingNode * newSuccessor = m_goalNode;
			int diff = row - m_goalRow;
			unsigned int givenCost = currentNode->m_givenCost + FIXED_POINT_SHIFT(diff);
			PushNewNode(newSuccessor, currentNode, givenCost);
			return;
		}
	}

	if (jumpDistance > 0)
	{
		// Directly jump
		int newRow = row - jumpDistance;
		unsigned int givenCost = currentNode->m_givenCost + FIXED_POINT_SHIFT(jumpDistance);
		PathfindingNode * newSuccessor = &m_mapNodes[newRow][col];
		PushNewNode(newSuccessor, currentNode, givenCost);
	}
}

void JPSPlus::SearchUpLeft(PathfindingNode * currentNode, int jumpDistance)
{
	int row = currentNode->m_row;
	int col = currentNode->m_col;

	// Check for goal in general direction (straight line to Goal or Target Jump Point)
	if (row > m_goalRow && col > m_goalCol)
	{
		int absJumpDistance = jumpDistance;
		if (absJumpDistance < 0) { absJumpDistance = -absJumpDistance; }

		int diffRow = row - m_goalRow;
		int diffCol = col - m_goalCol;
		int smallerDiff = diffRow;
		if (diffCol < smallerDiff) { smallerDiff = diffCol; }

		if (smallerDiff <= absJumpDistance)
		{
			int newRow = row - smallerDiff;
			int newCol = col - smallerDiff;
			unsigned int givenCost = currentNode->m_givenCost + (SQRT_2 * smallerDiff);
			PathfindingNode * newSuccessor = &m_mapNodes[newRow][newCol];
			PushNewNode(newSuccessor, currentNode, givenCost);
			return;
		}
	}

	if (jumpDistance > 0)
	{
		// Directly jump
		int newRow = row - jumpDistance;
		int newCol = col - jumpDistance;
		unsigned int givenCost = currentNode->m_givenCost + (SQRT_2 * jumpDistance);
		PathfindingNode * newSuccessor = &m_mapNodes[newRow][newCol];
		PushNewNode(newSuccessor, currentNode, givenCost);
	}
}

void JPSPlus::SearchLeft(PathfindingNode * currentNode, int jumpDistance)
{
	int row = currentNode->m_row;
	int col = currentNode->m_col;

	// Consider straight line to Goal
	if (row == m_goalRow && col > m_goalCol)
	{
		int absJumpDistance = jumpDistance;
		if (absJumpDistance < 0) { absJumpDistance = -absJumpDistance; }

		if ((col - absJumpDistance) <= m_goalCol)
		{
			PathfindingNode * newSuccessor = m_goalNode;
			int diff = col - m_goalCol;
			unsigned int givenCost = currentNode->m_givenCost + FIXED_POINT_SHIFT(diff);
			PushNewNode(newSuccessor, currentNode, givenCost);
			return;
		}
	}

	if (jumpDistance > 0)
	{
		// Directly jump
		int newCol = col - jumpDistance;
		unsigned int givenCost = currentNode->m_givenCost + FIXED_POINT_SHIFT(jumpDistance);
		PathfindingNode * newSuccessor = &m_mapNodes[row][newCol];
		PushNewNode(newSuccessor, currentNode, givenCost);
	}
}

void JPSPlus::SearchDownLeft(PathfindingNode * currentNode, int jumpDistance)
{
	int row = currentNode->m_row;
	int col = currentNode->m_col;

	// Check for goal in general direction (straight line to Goal or Target Jump Point)
	if (row < m_goalRow && col > m_goalCol)
	{
		int absJumpDistance = jumpDistance;
		if (absJumpDistance < 0) { absJumpDistance = -absJumpDistance; }

		int diffRow = m_goalRow - row;
		int diffCol = col - m_goalCol;
		int smallerDiff = diffRow;
		if (diffCol < smallerDiff) { smallerDiff = diffCol; }

		if (smallerDiff <= absJumpDistance)
		{
			int newRow = row + smallerDiff;
			int newCol = col - smallerDiff;
			unsigned int givenCost = currentNode->m_givenCost + (SQRT_2 * smallerDiff);
			PathfindingNode * newSuccessor = &m_mapNodes[newRow][newCol];
			PushNewNode(newSuccessor, currentNode, givenCost);
			return;
		}
	}

	if (jumpDistance > 0)
	{
		// Directly jump
		int newRow = row + jumpDistance;
		int newCol = col - jumpDistance;
		unsigned int givenCost = currentNode->m_givenCost + (SQRT_2 * jumpDistance);
		PathfindingNode * newSuccessor = &m_mapNodes[newRow][newCol];
		PushNewNode(newSuccessor, currentNode, givenCost);
	}
}

void JPSPlus::PushNewNode(PathfindingNode * newSuccessor, PathfindingNode * currentNode, unsigned int givenCost)
{
	if (newSuccessor->m_iteration != m_currentIteration)
	{
		// Place node on the Open list (we've never seen it before)

		// Compute heuristic using octile calculation (optimized: minDiff * SQRT_2_MINUS_ONE + maxDiff)
		unsigned int diffrow = abs(m_goalRow - newSuccessor->m_row);
		unsigned int diffcolumn = abs(m_goalCol - newSuccessor->m_col);
		unsigned int heuristicCost;
		if (diffrow <= diffcolumn)
		{
			heuristicCost = (diffrow * SQRT_2_MINUS_ONE) + FIXED_POINT_SHIFT(diffcolumn);
		}
		else
		{
			heuristicCost = (diffcolumn * SQRT_2_MINUS_ONE) + FIXED_POINT_SHIFT(diffrow);
		}

		newSuccessor->m_parent = currentNode;
		newSuccessor->m_givenCost = givenCost;
		newSuccessor->m_finalCost = givenCost + heuristicCost;
		newSuccessor->m_listStatus = PathfindingNode::OnOpen;
		newSuccessor->m_iteration = m_currentIteration;

#ifdef USE_FAST_OPEN_LIST_AT_THE_COST_OF_MUCH_MORE_MEMORY
		m_fastOpenList.Push(newSuccessor);
#else
		m_openList.add(newSuccessor);
#endif
	}
	else if (givenCost < newSuccessor->m_givenCost && newSuccessor->m_listStatus == PathfindingNode::OnOpen)
	{
		// We found a cheaper way to this node - update node

		// Extract heuristic cost (was previously calculated)
		unsigned int heuristicCost = newSuccessor->m_finalCost - newSuccessor->m_givenCost;
		unsigned int lastCost = newSuccessor->m_finalCost;

		newSuccessor->m_parent = currentNode;
		newSuccessor->m_givenCost = givenCost;
		newSuccessor->m_finalCost = givenCost + heuristicCost;

#ifdef USE_FAST_OPEN_LIST_AT_THE_COST_OF_MUCH_MORE_MEMORY
		m_fastOpenList.DecreaseKey(newSuccessor, lastCost);
#else
		m_openList.decreaseKey(newSuccessor);
#endif
	}
}