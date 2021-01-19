#ifndef EXPANSIONPOLICY_H
#define EXPANSIONPOLICY_H

// ExpansionPolicy.h
//
// Provides a generic interface for iterating over the neighbours
// of a node during expansion.
//
// @author: dharabor
// @created: 21/09/2010

#include <stdexcept>

class node;
class ProblemInstance;
class ExpansionPolicy
{
	public:
		ExpansionPolicy();
		virtual ~ExpansionPolicy();

		// initialises the policy with a target node
		virtual void expand(node* t) throw(std::logic_error);

		// return the first neighbour
		virtual node* first() = 0;

		// return the next neighbour
		virtual node* next() = 0;

		// return the current neighbour
		virtual node* n() = 0;

		// cost of the edge (target, n)
		virtual double cost_to_n() = 0;

		// update any labels associated with the current neighbour.
		// backpointers are good example but also any implementation specific
		// stuff that relates one node to another and which the search algorithm
		// shouldn't know/care about.
		virtual void label_n() = 0; 

		// returns true until all remaining neighbours are iterated over
		virtual bool hasNext() = 0;

		virtual void setProblemInstance(ProblemInstance* p);

		node* getTarget() const { return target;}
		ProblemInstance* getProblemInstance(); 

		bool verbose;

	protected:
		//node* n_;
		node* target;
		ProblemInstance* problem;
};

#endif

