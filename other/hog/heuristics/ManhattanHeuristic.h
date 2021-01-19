#ifndef MANHATTANHEURISTIC_H
#define MANHATTANHEURISTIC_H

#include "graph.h"
#include "Heuristic.h"

class ManhattanHeuristic : public Heuristic
{
	public:
		ManhattanHeuristic();
		virtual ~ManhattanHeuristic();
		
		virtual double h(node* first, node* second) const;
};

#endif
