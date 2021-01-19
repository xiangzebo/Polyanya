#ifndef RSRSEARCH_H
#define RSRSEARCH_H

// RSRSearch.h
// 
// A convenience wrapper for configuring various types of search 
// algorithms that use Rectangular Symmetry Reduction.
//
// There is also a small optimisation where the optimal path is returned 
// without search if the start and goal are located in the same cluster.
// 
// @author: dharabor
// @created: 03/06/2011
//

#include "searchAlgorithm.h"

class FlexibleAStar;
class Heuristic;
class InsertionPolicy;
class node;
class path;
class reservationProvider;

class RSRSearch : public searchAlgorithm
{
	public: 
		RSRSearch(bool bfReduction, Heuristic* heuristic);
		virtual ~RSRSearch();

		virtual const char* getName() { return name.c_str(); }
		virtual path *getPath(graphAbstraction *aMap, node *from, node *to, 
				reservationProvider *rp = 0);	
		bool usingBranchingFactorReduction() { return bfReduction; }

	private:
		path* getClusterPath(node* from, node* to);
		FlexibleAStar* alg;
		std::string name;
		bool bfReduction;
		InsertionPolicy* inserter;
		Heuristic* heuristic;
};

#endif

