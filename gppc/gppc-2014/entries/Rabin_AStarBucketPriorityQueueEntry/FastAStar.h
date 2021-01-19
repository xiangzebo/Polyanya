/*
 * FastAStar.h
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

#pragma once
#include "PathfindingNode.h"
#include "GenericHeap.h"
#include "BucketPriorityQueue.h"

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

class FastAStar
{
public:
	FastAStar(std::vector<bool> &rawMap, int w, int h);
	~FastAStar();

	bool GetPath(xyLocJPS& s, xyLocJPS& g, std::vector<xyLocJPS> &path);

protected:

	PathStatus SearchLoop(void);
	void FinalizePath(std::vector<xyLocJPS> &finalPath);

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
	BucketPriorityQueue m_fastOpenList;
	int m_currentIteration;	// This allows us to know if a node has been touched this iteration (faster than clearing all the nodes before each search)
	PathfindingNode* m_goalNode;
	int m_goalRow, m_goalCol;
	PathfindingNode m_mapNodes[MAX_HEIGHT][MAX_WIDTH];
	std::vector<bool> m_rawMap;
	int m_width, m_height;
};

