#ifndef OCTILEEXPANSIONPOLICY_H
#define OCTILEEXPANSIONPOLICY_H

#include "GridMapExpansionPolicy.h"

class mapAbstraction;
class OctileHeuristic;
class OctileExpansionPolicy : public GridMapExpansionPolicy
{
	public:
		OctileExpansionPolicy();
		virtual ~OctileExpansionPolicy();

		virtual node* n();
		virtual void label_n();
		virtual double cost_to_n();
};

#endif
