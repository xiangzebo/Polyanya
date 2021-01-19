/*
 * UnsortedPriorityQueue.h
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

// This array is dangerous since there are no safeguards if the max size is exceeded
// However, any method to detect a problem and deal gracefully with it will sacrifice speed
// The current size is very safe, but in practice a value of 100 can be used (which would halve the memory required).
#define UNSORTED_ARRAY_SIZE 3000

class UnsortedPriorityQueue
{
public:

	UnsortedPriorityQueue();
	~UnsortedPriorityQueue() {}

	inline bool Empty(int iteration) { if(m_iteration == iteration) { return m_nextFreeNode == 0; } else { return true; } }
	inline int GetIteration() { return m_iteration; }
	void Push(PathfindingNode* node);
	void Remove(PathfindingNode* node);
	PathfindingNode* Pop(void);

private:
	int m_nextFreeNode;
	int m_iteration;
	PathfindingNode* m_nodeArray[UNSORTED_ARRAY_SIZE];

};

