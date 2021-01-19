#ifndef OCTILEHEURISTICFACTORY_H
#define OCTILEHEURISTICFACTORY_H

// OctileHeuristicFactory.h
//
// A factory class for creating OctileHeuristic objects.
//
// @author: dharabor
// @created: 25/10/2010

#include "IHeuristicFactory.h"

class Heuristic;
class OctileHeuristicFactory : public IHeuristicFactory
{
	public:
		OctileHeuristicFactory();
		virtual ~OctileHeuristicFactory();
		virtual Heuristic* newHeuristic();
};

#endif

