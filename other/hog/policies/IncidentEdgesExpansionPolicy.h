#ifndef INCIDENTEDGESEXPANSIONPOLICY_H
#define INCIDENTEDGESEXPANSIONPOLICY_H

// IncidentEdgesExpansionPolicy.h
//
// An expansion policy which iterates over all edges incident with a node
// in a graph.
//
// @author: dharabor
// @created: ??/09/2010


#include "SelectiveExpansionPolicy.h"
#include <stdexcept>

class node;
class graph;
class graphAbstraction;

class IncidentEdgesExpansionPolicy : public SelectiveExpansionPolicy
{
	public:
		IncidentEdgesExpansionPolicy(graphAbstraction* map);
		virtual ~IncidentEdgesExpansionPolicy();

		virtual void expand(node* n) throw(std::logic_error);
		virtual bool hasNext();
		virtual double cost_to_n();
		virtual void label_n();


	protected:
		virtual node* next_impl();
		virtual node* first_impl();
		virtual node* n_impl();

	private:
		int which;
		graph* g;
		graphAbstraction* map;
};

#endif
