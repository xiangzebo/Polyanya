#ifndef ALTHEAP_H
#define ALTHEAP_H

/* Identical to HOG's regular heap except ties are broken
 * in favour of the node with the largest gcost.
 * Such nodes end up higher on the heap than other nodes 
 * with identical priority
 *
 * @author: dharabor
 * @created: 03/05/2010
 */


#include "heap.h"

class graph_object;
class Heuristic;
class altheap : public heap
{

	public:	
		altheap(Heuristic* heuristic, node* goal, int s = DEFAULT_SIZE);
		~altheap(); 

		inline void	setGoal(node* newgoal) { goal = newgoal; } 
		inline node* getGoal() { return goal; } 
		virtual bool lessThan(graph_object* first, graph_object* second);
	    virtual bool greaterThan(graph_object* first, graph_object* second);

	private:
		Heuristic* heuristic;
		node* goal;
};

#endif

