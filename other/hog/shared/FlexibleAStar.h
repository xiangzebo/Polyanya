#ifndef FLEXIBLEASTAR_H
#define FLEXIBLEASTAR_H

// FlexibleAStar.h
//
// A standard implementation of the A* search algorithm which can be
// readily modified with alternative node expansion algorithms and different 
// heuristics.
//
// @author: dharabor
// @created: 26/09/2010


#include "searchAlgorithm.h"
#include <map>
#include <ext/hash_map>
#include <string>

class DebugUtility;
class ExpansionPolicy;
class altheap;
class Heuristic;

typedef __gnu_cxx::hash_map<int,  node*> ClosedList; 
class FlexibleAStar : public searchAlgorithm
{
	public:
		FlexibleAStar(ExpansionPolicy*, Heuristic*);
		virtual ~FlexibleAStar();

		virtual const char *getName();
		virtual path *getPath(graphAbstraction *aMap, node *from, node *goal,
				reservationProvider *rp = 0);

		Heuristic* getHeuristic() { return heuristic; }
		ExpansionPolicy* getExpander() { return policy; }
		bool markForVis;	

	protected:
		ExpansionPolicy* policy;
		Heuristic* heuristic;

		path* search(node* from, node* goal);
		void relaxNode(node* from, node* to, node* goal, double cost, 
			altheap* openList);
		void expand(node* current, node* goal, altheap* openList,
				ClosedList* closedList);
		path* extractBestPath(node* goal);

	private:
		void closeNode(node* current, ClosedList* closedList);
		bool checkParameters(node* from, node* to);
		DebugUtility* debug;
};

#endif
