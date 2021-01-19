#ifndef PROBLEMINSTANCE_H
#define PROBLEMINSTANCE_H

// ProblemInstance.h
//
// Describes a particular problem instance in terms of:
// 	- Start node
// 	- Goal node
// 	- Map 
// 	- Heuristic
// 	- Whatever else is required
//
// 	This class is intended to be used as a means of passing information
// 	between the search algorithm and its expansion policy (without polluting
// 	the interface of the latter).
//
//	@author: dharabor
//	@created: 06/1/2011
//

class node;
class mapAbstraction;
class Heuristic;

class ProblemInstance
{
	public: 
		ProblemInstance(node* start, node* goal, mapAbstraction* map, 
				Heuristic* heuristic);
		virtual ~ProblemInstance();
		virtual ProblemInstance* clone();

		node* getStartNode();
		node* getGoalNode(); 
		mapAbstraction* getMap(); 
		Heuristic* getHeuristic();

	private:
		node *start, *goal;
		mapAbstraction* map;
		Heuristic* heuristic;
};

#endif

