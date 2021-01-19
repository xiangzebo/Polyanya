#ifndef MANHATTANHEURISTICFACTORY_H
#define MANHATTANHEURISTICFACTORY_H

// ManhattanHeuristicFactory.h
//
// A factory class for creating ManhattanHeuristic objects.
//
// @author: dharabor
// @created: 25/10/2010

#include "IHeuristicFactory.h"

class Heuristic;
class ManhattanHeuristicFactory : public IHeuristicFactory
{
	public:
		ManhattanHeuristicFactory();
		virtual ~ManhattanHeuristicFactory();
		virtual Heuristic* newHeuristic();
};

#endif

