/*
 * UnsortedPriorityQueue.cpp
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

#include "UnsortedPriorityQueue.h"

#define ALL_NODES_EQUAL_COST	// ~40% faster and still results in optimal paths in virtually all cases

UnsortedPriorityQueue::UnsortedPriorityQueue(void)
: m_nextFreeNode(0), m_iteration(0)
{

}

void UnsortedPriorityQueue::Push(PathfindingNode* node)
{
	if (m_iteration != node->m_iteration)
	{
		m_nextFreeNode = 0;
		m_iteration = node->m_iteration;
	}

	m_nodeArray[m_nextFreeNode++] = node;
}

void UnsortedPriorityQueue::Remove(PathfindingNode* node)
{
	for (int i = 0; i < m_nextFreeNode; ++i)
	{
		if (m_nodeArray[i] == node)
		{
			// Delete off Open list (put last node where this one was)
			m_nodeArray[i] = m_nodeArray[--m_nextFreeNode];
			return;
		}
	}
}

PathfindingNode* UnsortedPriorityQueue::Pop(void)
{
#ifdef ALL_NODES_EQUAL_COST
	// Just pop last node (all nodes are very close to each other in cost)
	return m_nodeArray[--m_nextFreeNode];
#else
	// Find cheapest node
	unsigned int cheapestNodeCostFinal = m_nodeArray[0]->m_finalCost;
	int cheapestNodeIndex = 0;

	for (int i = 1; i < m_nextFreeNode; ++i)
	{
		if (m_nodeArray[i]->m_finalCost < cheapestNodeCostFinal)
		{
			cheapestNodeCostFinal = m_nodeArray[i]->m_finalCost;
			cheapestNodeIndex = i;
		}
	}

	// Remember cheapest node
	PathfindingNode* cheapestNode = m_nodeArray[cheapestNodeIndex];

	// Delete off Open list (put last node where this one was)
	m_nodeArray[cheapestNodeIndex] = m_nodeArray[--m_nextFreeNode];

	return cheapestNode;
#endif
}
