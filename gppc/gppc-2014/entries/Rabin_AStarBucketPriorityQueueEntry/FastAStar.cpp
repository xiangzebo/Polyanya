/*
 * FastAStar.cpp
 *
 * Copyright (c) 2015, Steve Rabin
 * All rights reserved.
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

#include "FastAStar.h"

#define ONLY_VISIT_REASONABLE_NEIGHBORS	// 3% to 17% speed-up
#define OCTILE_HEURISTIC				// 1% to 15% speed-up
#define USE_FAST_OPEN_LIST_AT_THE_COST_OF_MUCH_MORE_MEMORY	// Roughly 10 times faster, requiring >100MB more memory

#define FIXED_POINT_MULTIPLIER 100000
#define FIXED_POINT_SHIFT(x) ((x) * FIXED_POINT_MULTIPLIER)
#define SQRT_2 141421
#define SQRT_2_MINUS_ONE 41421


FastAStar::FastAStar(std::vector<bool> &rawMap, int w, int h)
{
	m_rawMap = rawMap;
	m_width = w;
	m_height = h;

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

FastAStar::~FastAStar()
{
}

bool FastAStar::GetPath(xyLocJPS& s, xyLocJPS& g, std::vector<xyLocJPS> &path)
{
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

PathStatus FastAStar::SearchLoop(void)
{
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

		static const int offsetX[] = { 0,  1, 1, 1, 0, -1, -1, -1};
		static const int offsetY[] = {-1, -1, 0, 1, 1,  1,  0, -1};

#ifdef ONLY_VISIT_REASONABLE_NEIGHBORS
		static const bool dir[10][8] = {
			{ false, false, true , true, true, false, false, false},	// DownRight
			{ true , true, true, true , true, false, false, false },	// Down
			{ true , true, true, false, false, false, false, false },	// DownLeft
			{ false, false, true , true, true, true, true, false },		// Right
			{ false, false, false, false, false, false, false, false},	// NULL
			{ true, true, true, false, false, false, true, true },		// Left
			{ false, false, false, false, true, true, true, false },	// UpRight
			{ true, false, false, false, true, true, true, true },		// Up
			{ true, false, false, false, false, false, true, true },	// UpLeft
			{ true, true, true, true, true, true, true, true },			// All
		};

		int dirIndex = 9;
		if(currentNode->m_parent != NULL)
		{
			int rowDiff = 1 + currentNode->m_parent->m_row - currentNode->m_row;
			int colDiff = 1 + currentNode->m_parent->m_col - currentNode->m_col;
			dirIndex = colDiff + (rowDiff * 3);
		}
#endif

		for (int i = 0; i < 8; ++i)
		{
#ifdef ONLY_VISIT_REASONABLE_NEIGHBORS
			// Check if useless direction based on parent
			if(!dir[dirIndex][i])
				continue;
#endif

			unsigned int neighborRow = currentNode->m_row + offsetX[i];

			// Out of grid bounds?
			if (neighborRow >= m_width)
				continue;

			unsigned int neighborCol = currentNode->m_col + offsetY[i];

			// Out of grid bounds?
			if (neighborCol >= m_width)
				continue;

			// Valid tile - get the node
			PathfindingNode* newSuccessor = &m_mapNodes[neighborRow][neighborCol];

			// Blocked?
			if (!m_rawMap[neighborCol + (neighborRow * m_width)])
				continue;

			// Diagonal blocked?
			bool isDiagonal = (i & 0x1) == 1;
			if (isDiagonal && (!m_rawMap[currentNode->m_col + ((currentNode->m_row + offsetX[i]) * m_width)] ||
							   !m_rawMap[currentNode->m_col + offsetY[i] + (currentNode->m_row * m_width)]))
			{
				continue;
			}

			unsigned int givenCost = currentNode->m_givenCost + (isDiagonal ? SQRT_2 : FIXED_POINT_MULTIPLIER);

			if (newSuccessor->m_iteration != m_currentIteration)
			{	
				// Place it on the Open list

#ifdef OCTILE_HEURISTIC
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
#else
				// Euclidean heuristic
				unsigned int diffrow = m_goalRow - newSuccessor->m_row;
				unsigned int diffcolumn = m_goalCol - newSuccessor->m_col;
				unsigned int heuristicCost = (unsigned int)FIXED_POINT_SHIFT(sqrtf(((diffrow * diffrow) + (diffcolumn * diffcolumn))));
#endif

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
				// We found a cheaper way to this node - update it

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

		currentNode->m_listStatus = PathfindingNode::OnClosed;

	}
	return NoPathExists;
}

void FastAStar::FinalizePath(std::vector<xyLocJPS> &finalPath)
{
	PathfindingNode* curNode = m_goalNode;

	while (curNode != NULL)
	{
		xyLocJPS loc;
		loc.x = curNode->m_col;
		loc.y = curNode->m_row;

		finalPath.push_back(loc);
		curNode = curNode->m_parent;
	}
	std::reverse(finalPath.begin(), finalPath.end());
}
