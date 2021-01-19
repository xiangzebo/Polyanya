/*
 * SearchEnvironment.cpp
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

#include "SearchEnvironment.h"

/**
* MapSearchEnvironment::GetNeighbors()
 *
 * \brief Return the neighbors in a simple map
 *
 * \param nodeID The current location stored as (x<<16)|(y)
 * \param nodeID Array holding the neighbors upon return
 * \return none
 */
void MapSearchEnvironment::GetNeighbors(uint32_t nodeID, std::vector<uint32_t> &neighbors)
{
    int x1, y1;
    bool up=false, down=false;
    x1 = nodeID>>16; y1 = nodeID&0xFFFF;
    if ((map->GetTerrainType(x1, y1+1) == kGround))
    {
        down = true;
        neighbors.push_back((x1<<16)|(y1+1));
    }
    if ((map->GetTerrainType(x1, y1-1) == kGround))
    {
        up = true;
        neighbors.push_back((x1<<16)|(y1-1));
    }
    if ((map->GetTerrainType(x1-1, y1) == kGround))
    {
        if ((up && (map->GetTerrainType(x1-1, y1-1) == kGround)))
            neighbors.push_back(((x1-1)<<16)|(y1-1));
        if ((down && (map->GetTerrainType(x1-1, y1+1) == kGround)))
            neighbors.push_back(((x1-1)<<16)|(y1+1));
        neighbors.push_back(((x1-1)<<16)| y1);
    }
    if ((map->GetTerrainType(x1+1, y1) == kGround))
    {
        if ((up && (map->GetTerrainType(x1+1, y1-1) == kGround)))
            neighbors.push_back(((x1+1)<<16)|(y1-1));
        if ((down && (map->GetTerrainType(x1+1, y1+1) == kGround)))
            neighbors.push_back(((x1+1)<<16)|(y1+1));
        neighbors.push_back(((x1+1)<<16)| y1);
    }
}

/**
* MapSearchEnvironment::HCost()
 *
 * \brief Return the heuristic between two locations
 *
 * \param nodeID The first location [stored as (x<<16)|(y)]
 * \param nodeID The second location [stored as (x<<16)|(y)]
 * \return none
 */
double MapSearchEnvironment::HCost(uint32_t node1, uint32_t node2)
{
    int x1, x2, y1, y2;
    x1 = node1>>16; y1 = node1&0xFFFF;
    x2 = node2>>16; y2 = node2&0xFFFF;
    double a = ((x1>x2)?(x1-x2):(x2-x1));
    double b = ((y1>y2)?(y1-y2):(y2-y1));
    return (a>b)?(b*ROOT_TWO+a-b):(a*ROOT_TWO+b-a);
}

/**
 * MapSearchEnvironment::GCost()
 *
 * \brief Return the actual cost between two locations
 *
 * \param nodeID The first location [stored as (x<<16)|(y)]
 * \param nodeID The second location [stored as (x<<16)|(y)]
 * \return none
 */
double MapSearchEnvironment::GCost(uint32_t node1, uint32_t node2)
{
    return HCost(node1, node2);
}


/*********************************************************************/


MinimalSearchEnvironment::MinimalSearchEnvironment(MinimalSectorAbstraction *_msa)
    :msa(_msa)
{}

/**
* MinimalSearchEnvironment::GetNeighbors()
 *
 * \brief Return the neighbors of a node in the abstraction
 *
 * \param nodeID The current location stored as (sector<<16)|(region)
 * \param nodeID Array holding the neighbors upon return
 * \return none
 */
void MinimalSearchEnvironment::GetNeighbors(uint32_t nodeID,
                                            std::vector<uint32_t> &neighbors)
{
    static std::vector<tempEdgeData> edges;
    edges.resize(0);
    // sector ... abstraction
    msa->GetNeighbors(nodeID>>16, nodeID&0xFFFF, edges);
    for (unsigned int x = 0; x < edges.size(); x++)
    {
        int sector = msa->GetAdjacentSector(nodeID>>16, edges[x].direction);
        int region = edges[x].to;
        neighbors.push_back((sector<<16)|region);
    }
}

/**
* MinimalSearchEnvironment::HCost()
 *
 * \brief Return the heuristic between two locations
 *
 * \param nodeID The first location [stored as (sector<<16)|(region)]
 * \param nodeID The second location [stored as (sector<<16)|(region)]
 * \return none
 */
double MinimalSearchEnvironment::HCost(uint32_t node1, uint32_t node2)
{
    unsigned int x1, x2, y1, y2;
    msa->GetXYLocation(node1>>16, node1&0xFFFF, x1, y1);
    msa->GetXYLocation(node2>>16, node2&0xFFFF, x2, y2);
    double a = ((x1>x2)?(x1-x2):(x2-x1));
    double b = ((y1>y2)?(y1-y2):(y2-y1));
    double val = (a>b)?(b*ROOT_TWO+a-b):(a*ROOT_TWO+b-a);
    return val;
}

/**
* MinimalSearchEnvironment::GCost()
 *
 * \brief Return the actual cost between two locations
 *
 * \param nodeID The first location [stored as (sector<<16)|(region)]
 * \param nodeID The second location [stored as (sector<<16)|(region)]
 * \return none
 */
double MinimalSearchEnvironment::GCost(uint32_t node1, uint32_t node2)
{
    return HCost(node1, node2);
}
