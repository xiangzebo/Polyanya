/*
 * BucketPriorityQueue.cpp
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

#include "BucketPriorityQueue.h"
#include "UnsortedPriorityQueue.h"

BucketPriorityQueue::BucketPriorityQueue()
{
	Reset();
	m_bin = new UnsortedPriorityQueue*[NUM_BUCKETS];
	for (int m = 0; m < NUM_BUCKETS; m++)
	{
		m_bin[m] = new UnsortedPriorityQueue();
	}
}

BucketPriorityQueue::~BucketPriorityQueue()
{
}

void BucketPriorityQueue::Push(PathfindingNode* node)
{
	m_numNodesTracked++;
	int index = GetBinIndex(node->m_finalCost);

	m_bin[index]->Push(node);

	if (index < m_lowestNonEmptyBin)
	{
		m_lowestNonEmptyBin = index;
	}
}

PathfindingNode* BucketPriorityQueue::Pop(void)
{
	PathfindingNode* node = m_bin[m_lowestNonEmptyBin]->Pop();
	m_numNodesTracked--;

	if (m_numNodesTracked > 0)
	{
		// Find the next non-empty bin
		for (m_lowestNonEmptyBin; m_lowestNonEmptyBin < NUM_BUCKETS; m_lowestNonEmptyBin++)
		{
			if (!m_bin[m_lowestNonEmptyBin]->Empty(node->m_iteration))
			{
				break;
			}
		}
	}
	else
	{
		m_lowestNonEmptyBin = NUM_BUCKETS;
	}

	return node;
}

void BucketPriorityQueue::DecreaseKey(PathfindingNode* node, unsigned int lastCost)
{
	// Remove node
	int index = GetBinIndex(lastCost);
	m_bin[index]->Remove(node);

	// Push node
	index = GetBinIndex(node->m_finalCost);

	m_bin[index]->Push(node);

	if (index < m_lowestNonEmptyBin)
	{
		m_lowestNonEmptyBin = index;
	}
}
