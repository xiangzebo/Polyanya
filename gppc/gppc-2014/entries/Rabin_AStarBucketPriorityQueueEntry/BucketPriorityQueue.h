/*
 * BucketPriorityQueue.h
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
#include "UnsortedPriorityQueue.h"

// Cost of traversing 1 tile is expected to be 100000
// 150000 buckets is tuned for a maximum final cost 1.5 billion (true final cost of 15000)
// The number of buckets can be reduced if the maximum final cost is known to be lower (significant memory savings)
#define NUM_BUCKETS 150000

class BucketPriorityQueue
{
public:
	BucketPriorityQueue();
	~BucketPriorityQueue();

	inline void Reset() { m_lowestNonEmptyBin = NUM_BUCKETS; m_numNodesTracked = 0; }
	inline bool Empty() { return m_numNodesTracked == 0; }
	void Push(PathfindingNode* node);
	PathfindingNode* Pop(void);
	void DecreaseKey(PathfindingNode* node, unsigned int lastCost);

private:
	int m_lowestNonEmptyBin;
	int m_numNodesTracked;
	UnsortedPriorityQueue** m_bin;

	inline int GetBinIndex(unsigned int cost) { return cost / 10000; }
};

