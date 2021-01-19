/*
 * GenericAStar.h
 *
 * Copyright (c) 2007, Nathan Sturtevant
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of Alberta nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY NATHAN STURTEVANT ``AS IS'' AND ANY
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


#ifdef _MSC_VER
#include "stdafx.h"
#endif

#ifndef GENERICSTAR_H
#define GENERICSTAR_H

#ifdef _MSC_VER
#include <hash_map>
#else

#include <ext/hash_map>
#include <stdint.h>
#ifndef UINT32_MAX
#define UINT32_MAX        4294967295U
#endif

#endif

#include "FPUtil.h"
#include "GenericHeap.h"
#include "SearchEnvironment.h" // for the SearchEnvironment class

/**
* These are utility classes needed for GenericAStar.
 */
namespace GenericAStarUtil
{
    /**
     * A SearchNode is a fairly heavy representation of a node used for
     * search within GenericAStar. It makes no assumptions about the
     * underlying implementation. For grid-based worlds, for instance, we
     * can use integer values for the f- and g-costs, if we make diagonals
     * 1.5 and then multiply the value by 2.
     *
     * Using optimized data structures here will speed A* by a factor of
     * two or more.
     */
    class SearchNode {
public:
        SearchNode(double fCst=0, double gCst=0, uint32_t curr=0, uint32_t prev=0)
        :fCost(fCst), gCost(gCst), currNode(curr), prevNode(prev) {}

        SearchNode(uint32_t curr)
        :fCost(0), gCost(0), currNode(curr), prevNode(0) {}

        double fCost;
        double gCost;
        uint32_t currNode;
        uint32_t prevNode;
    };
        
    /**
    * \brief Compare two search nodes and return true if they are equal
     */
    struct SearchNodeEqual {
        /**
        * \brief Compare two search nodes and return true if they are equal
         */
        bool operator()(const SearchNode &i1, const SearchNode &i2)
        { return (i1.currNode == i2.currNode); } };
    
    struct SearchNodeCompare {
        /**
        * \brief Compare two search nodes and return true if i2 should be
         * opened before i1.
         */
        bool operator()(const SearchNode &i1, const SearchNode &i2)
        {
            if (fequal(i1.fCost, i2.fCost))
            {
                return (fless(i1.gCost, i2.gCost));
            }
            return (fgreater(i1.fCost, i2.fCost));
        } };
        
    struct SearchNodeHash {
        /**
        * \brief Return a hash-key for the search node.
         */
        size_t operator()(const SearchNode &x) const
        { return (size_t)(x.currNode); }
    };
    
    typedef GenericHeap<GenericAStarUtil::SearchNode, GenericAStarUtil::SearchNodeHash,
        GenericAStarUtil::SearchNodeEqual, GenericAStarUtil::SearchNodeCompare> PQueue;
    
#ifdef _MSC_VER
    typedef stdext::hash_map<uint32_t, GenericAStarUtil::SearchNode > NodeLookupTable;
#else
    typedef __gnu_cxx::hash_map<uint32_t, GenericAStarUtil::SearchNode > NodeLookupTable;
#endif
    
}


typedef GenericAStarUtil::NodeLookupTable::const_iterator closedList_iterator;


/**
* GenericAStar
 *
 * This is a generic implementation of A* which will run on any problem
 * which has keys which fit into 32 bits. It makes as few assumptions as
 * possible about the underlying problem. It is used to search both the
 * abstract and low-level space for this demo code.
 *
 * This implementation also has low-level access to the closed list
 * and can be run incrementally by initializing a search and then doing
 * single steps of the search one at a time. Combined, these make it easy
 * to set up a visualization of the search.
 *
 */
class GenericAStar {
public:
    GenericAStar() {}
    virtual ~GenericAStar() {}
    void GetPath(SearchEnvironment *env, uint32_t from, uint32_t to,
                             std::vector<uint32_t> &thePath);

    bool InitializeSearch(SearchEnvironment *env, uint32_t from, uint32_t to,
                                                std::vector<uint32_t> &thePath);
    bool DoSingleSearchStep(std::vector<uint32_t> &thePath);
    uint32_t CheckNextNode();
    void ExtractPathToStart(uint32_t n, std::vector<uint32_t> &thePath);

    virtual const char *GetName();
    
    void PrintStats();
    long GetNodesExpanded() { return nodesExpanded; }
    long GetNodesTouched() { return nodesTouched; }
    void ResetNodeCount() { nodesExpanded = nodesTouched = 0; }
    int GetMemoryUsage();

    closedList_iterator GetClosedListIter() const;
    uint32_t ClosedListIterNext(closedList_iterator&) const;

private:
    long nodesTouched, nodesExpanded;

    uint32_t GetNextNode();
    void UpdateWeight(uint32_t currOpenNode, uint32_t neighbor);
    void AddToOpenList(uint32_t currOpenNode, uint32_t neighbor);
    GenericAStarUtil::PQueue openQueue;
    GenericAStarUtil::NodeLookupTable closedList; //openList
    uint32_t goal, start;

    std::vector<uint32_t> neighbors;
    SearchEnvironment *env;
};

#endif
