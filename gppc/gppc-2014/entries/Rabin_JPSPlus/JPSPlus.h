/*
 * JPSPlus.h
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

#pragma once
#include "PathfindingNode.h"
#include "PrecomputeMap.h"
#include "GenericHeap.h"

#define MAX_WIDTH 2048
#define MAX_HEIGHT 2048

struct xyLocJPS {
	int16_t x;
	int16_t y;
};

enum PathStatus
{
	Working,
	PathFound,
	NoPathExists
};

class JPSPlus
{
public:
	JPSPlus(DistantJumpPoints** map);
	~JPSPlus();

	bool GetPath(void *data, xyLocJPS& s, xyLocJPS& g, std::vector<xyLocJPS> &path);

protected:

	PathStatus SearchLoop(void);
	void FinalizePath(std::vector<xyLocJPS> &finalPath);

	void ExploreFromParentDown(PathfindingNode * currentNode, DistantJumpPoints * distantJumpPoints);
	void ExploreFromParentDownRight(PathfindingNode * currentNode, DistantJumpPoints * distantJumpPoints);
	void ExploreFromParentRight(PathfindingNode * currentNode, DistantJumpPoints * distantJumpPoints);
	void ExploreFromParentUpRight(PathfindingNode * currentNode, DistantJumpPoints * distantJumpPoints);
	void ExploreFromParentUp(PathfindingNode * currentNode, DistantJumpPoints * distantJumpPoints);
	void ExploreFromParentUpLeft(PathfindingNode * currentNode, DistantJumpPoints * distantJumpPoints);
	void ExploreFromParentLeft(PathfindingNode * currentNode, DistantJumpPoints * distantJumpPoints);
	void ExploreFromParentDownLeft(PathfindingNode * currentNode, DistantJumpPoints * distantJumpPoints);
	void ExploreFromParentAllDirections(PathfindingNode * currentNode, DistantJumpPoints * distantJumpPoints);

	void SearchDown(PathfindingNode * currentNode, int jumpDistance);
	void SearchDownRight(PathfindingNode * currentNode, int jumpDistance);
	void SearchRight(PathfindingNode * currentNode, int jumpDistance);
	void SearchUpRight(PathfindingNode * currentNode, int jumpDistance);
	void SearchUp(PathfindingNode * currentNode, int jumpDistance);
	void SearchUpLeft(PathfindingNode * currentNode, int jumpDistance);
	void SearchLeft(PathfindingNode * currentNode, int jumpDistance);
	void SearchDownLeft(PathfindingNode * currentNode, int jumpDistance);

	void PushNewNode(PathfindingNode * newSuccessor, PathfindingNode * currentNode, unsigned int givenCost);

	struct PathfindingNodeEqual {
		bool operator()(const PathfindingNode* i1, const PathfindingNode* i2)
		{
			return (i1 == i2);
		}
	};

	struct PathfindingNodeCmp : std::binary_function<bool, PathfindingNode* const, PathfindingNode* const> {
		bool operator()(PathfindingNode* const lhs, PathfindingNode* const rhs) const {
			return lhs->m_finalCost > rhs->m_finalCost;
		}
	};

	struct SearchNodeHash {
		size_t operator()(const PathfindingNode* x) const
		{
			return (size_t)(x->m_col + (x->m_row * MAX_WIDTH));
		}
	};

	typedef GenericHeap<PathfindingNode*, SearchNodeHash,
		PathfindingNodeEqual, PathfindingNodeCmp> PQueue;

	PQueue m_openList;
	DistantJumpPoints** m_map;
	int m_currentIteration = 0;	// This allows us to know if a node has been touched this iteration (faster than clearing all the nodes before each search)
	PathfindingNode* m_goalNode;
	int m_goalRow, m_goalCol;
	PathfindingNode m_mapNodes[MAX_HEIGHT][MAX_WIDTH];
};

