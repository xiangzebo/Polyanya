#ifndef IHEURISTICFACTORY_H
#define IHEURISTICFACTORY_H

// IHeuristicFactory.h
//
// A factory for different kinds of heuristic algorithms.
//
// @author: dharabor
// @created: 25/10/2010

class Heuristic;
class IHeuristicFactory
{
	public:
		IHeuristicFactory() { }
		virtual ~IHeuristicFactory() { }

		virtual Heuristic* newHeuristic() = 0;
};

#endif

