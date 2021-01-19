#ifndef RREXPANSIONPOLICY_H
#define RREXPANSIONPOLICY_H

#include "SelectiveExpansionPolicy.h"

// RRExpansionPolicy.h
//
// An expansion policy for Empty Rectangular Rooms.
// Each Empty Rectangular Room consists of a set of MacroNode objects 
// that define the perimeter of each room. 
//
// Each MacroNode is associated with 2 sets of incident edges: a primary 
// set of edges and a secondary set of edges.
//
// Primary Edges:
//  These connect the target node to neighbours immediately
//  adjacent or which are located on an orthogonal side of the ERR.
//
// Secondary Edges:
//   These connect the target node to neighbours which are located on the
//   opposide side of the ERR.
//
// During most node expansion operations both primary and secondary neighbours
// will be considered. 
//
// In some cases however the set of secondary neighbours can be skipped without
// affecting the optimality or completeness of the search.
// This optimisation is performed if it can be established that the parent of 
// the target node is from the same ERR as the target.
// 
// See [Harabor & Botea 2011] for more details.
//
//
// @author: dharabor
// @created: 28/11/2010

#include <stdexcept>

class graph;
class graphAbstraction;
class IncidentEdgesExpansionPolicy;
class RRExpansionPolicy : public SelectiveExpansionPolicy
{
	public:
		RRExpansionPolicy(graphAbstraction* map);
		virtual ~RRExpansionPolicy();

		virtual void expand(node* t) throw(std::logic_error);
		virtual bool hasNext();
		virtual double cost_to_n();
		virtual void label_n();

	protected:
		virtual node* first_impl();
		virtual node* next_impl();
		virtual node* n_impl();

	private:
		bool skipSecondary;
		int whichSecondary;
		int numSecondary;
		double cost;

		graphAbstraction* map;
		graph* g;
		IncidentEdgesExpansionPolicy* primary;
};

#endif
