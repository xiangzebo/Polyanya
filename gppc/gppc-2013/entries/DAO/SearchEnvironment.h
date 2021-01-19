/*
 * SearchEnvironment.h
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

#ifndef SEARCHENVIRONMENT_H
#define SEARCHENVIRONMENT_H

#include "Map.h"
#include "MinimalSectorAbstraction.h"
#include <vector>

/**
 * SearchEnvironment
 *
 * This is a generic interface onto any environment which allows it to be
 * searched using a generic version of AStar.
 */
class SearchEnvironment
{
public:
    virtual ~SearchEnvironment() {}
    virtual void GetNeighbors(uint32_t nodeID, std::vector<uint32_t> &neighbors) = 0;
    virtual double HCost(uint32_t node1, uint32_t node2) = 0;
    virtual double GCost(uint32_t node1, uint32_t node2) = 0;
    virtual bool GoalTest(uint32_t node, uint32_t goal) { return (node == goal); }
};

/**
* MapSearchEnvironment
 *
 * This is an implementation of a search environment for performing searches
 * on maps. 32-bit nodes are mapped to maps using 16 bits for each of the x-
 * and y-coordinates
 */
class MapSearchEnvironment : public SearchEnvironment
{
public:
    MapSearchEnvironment(Map *_map) :map(_map) {  }
    ~MapSearchEnvironment() {}
    void GetNeighbors(uint32_t nodeID, std::vector<uint32_t> &neighbors);
    double HCost(uint32_t node1, uint32_t node2);
    double GCost(uint32_t node1, uint32_t node2);
private:
    Map *map;
};

/**
* MinimalSearchEnvironment
 *
 * This is an implementation of a search environment for performing searches
 * on the map abstraction. 32-bit nodes are mapped to the abstraction using
 * 16 bits for each of the sector and region
 */
class MinimalSearchEnvironment : public SearchEnvironment
{
public:
    MinimalSearchEnvironment(MinimalSectorAbstraction *_msa);
    void GetNeighbors(uint32_t nodeID, std::vector<uint32_t> &neighbors);
    double HCost(uint32_t node1, uint32_t node2);
    double GCost(uint32_t node1, uint32_t node2);
private:
    MinimalSectorAbstraction *msa;
};


#endif
