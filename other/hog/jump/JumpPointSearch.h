#ifndef JUMPPOINTSEARCH_H
#define JUMPPOINTSEARCH_H

// JumpPointSearch.h
//
// A convenience wrapper for constructing and managing different types of
// Jump Point Search.
//
// @author: dharabor
// @created: 28/02/2012
//

#include "searchAlgorithm.h"

#include <climits>

class FlexibleAStar;
class Heuristic;
class mapAbstraction;
class path;
class JumpPointSearch : public searchAlgorithm
{
	public:
		JumpPointSearch(Heuristic* heuristic, mapAbstraction* map, 
				bool online=true, unsigned int maxdepth=0, 
				unsigned int jumplimit=INT_MAX);
		virtual ~JumpPointSearch();

		virtual const char *getName() { return name.c_str(); }
		virtual path *getPath(graphAbstraction *aMap, node *from, node *to, 
				reservationProvider *rp = 0);	
		void resetMetrics();

	private:
		void addIntermediateNodes(path* thepath);

		bool online;
		unsigned int maxdepth;
		Heuristic* heuristic;
		mapAbstraction* map;
		FlexibleAStar* astar;
		std::string name;
};

#endif

