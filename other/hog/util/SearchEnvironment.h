/*
 *  SearchEnvironment.h
 *  hog
 *
 *  Created by Nathan Sturtevant on 4/5/07.
 *  Copyright 2007 Nathan Sturtevant, University of Alberta. All rights reserved.
 *
 */

#ifndef SEARCHENVIRONMENT_H
#define SEARCHENVIRONMENT_H

#include "graph.h"
#include "map.h"

#define __STDC_CONSTANT_MACROS
#include <stdint.h>
// this is defined in stdint.h, but it doesn't always get defined correctly
// even when __STDC_CONSTANT_MACROS is defined before including stdint.h
// because stdint might be included elsewhere first...
#ifndef UINT32_MAX
#define UINT32_MAX        4294967295U
#endif

class SearchEnvironment
{
public:
	virtual ~SearchEnvironment() {}
	virtual void getNeighbors(uint32_t nodeID, std::vector<uint32_t> &neighbors) = 0;
	virtual double heuristic(uint32_t node1, uint32_t node2) = 0;
	virtual double gcost(uint32_t node1, uint32_t node2) = 0;
	virtual bool goalTest(uint32_t node, uint32_t goal) { return (node == goal); }
};

class MapSearchEnvironment : public SearchEnvironment
{
public:
	MapSearchEnvironment(Map *_map) :map(_map) {  }
	~MapSearchEnvironment() {}
	void getNeighbors(uint32_t nodeID, std::vector<uint32_t> &neighbors);
	double heuristic(uint32_t node1, uint32_t node2);
	double gcost(uint32_t node1, uint32_t node2);
private:
	Map *map;
};

class GraphSearchEnvironment : public SearchEnvironment
{
public:
	GraphSearchEnvironment(graph *_graph) :g(_graph) {  }
	~GraphSearchEnvironment() {}
	void getNeighbors(uint32_t nodeID, std::vector<uint32_t> &neighbors);
	double heuristic(uint32_t node1, uint32_t node2);
	double gcost(uint32_t node1, uint32_t node2);
private:
	graph *g;
};

#endif
