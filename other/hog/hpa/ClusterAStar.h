/*
 *  ClusterAStar.h
 *  hog
 *
	A simple variant on the standard A* implementation which supports limiting search to one or two clusters.
	Specifically, if useCorridor is true, the parentClusterId of the start node is used to limit which nodes are added to the open list.
	If the corridor is unspecified, a regular A* search is run on the map. 	NB: assumes the graph is composed of nodes of type ClusterNode.
		
	This class also adds additional metrics for measuring search performance:
	 - Total time to search (in milliseconds).
	 - Peak memory (records max size of open list during the search).
	
 *  Created by Daniel Harabor on 14/11/08.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef CLUSTERASTAR_H
#define CLUSTERASTAR_H

#include <ext/hash_map>
#include <string>
#include <map>
#include "aStar3.h"
#include "graph.h"

class statCollection;
class path;
class graphAbstraction;
class edge;
class statCollection;

class AbstractClusterAStar : public aStarOld
{
	typedef  __gnu_cxx::hash_map<int, node*> ClosedList;

	public:	
		AbstractClusterAStar() { corridorNodes = NULL; verbose = false; markForVis=true; }
		virtual ~AbstractClusterAStar() {}
		virtual const char* getName() = 0;
		virtual path *getPath(graphAbstraction *aMap, node *from, node *to, reservationProvider *rp = 0) = 0;
		
		void setCorridorNodes(std::map<int, node*>* _nodes) 
		{ 
			corridorNodes = _nodes; 
			assert(corridorNodes == _nodes); 
		}

		void printPath(path* p); // debugging function
		virtual void printStats();

		virtual void expand(node* current_, node* goal, edge_iterator begin, unsigned int card, 
				heap* openList, ClosedList& closedList, graph* g);

		bool markForVis;	
		//bool verbose;

	protected:
		void printNode(std::string msg, node* n, node* goal=0);
		bool isInCorridor(node* n);

		virtual void processNeighbour(node* current, edge* e, 
				node* to,heap* openList, ClosedList& closedList, graph* g);
		virtual void closeNode(node* current, ClosedList& closedList);

		virtual bool evaluate(node* current, node* target, edge* e) = 0; 
		virtual path *search(graph* g, node *from, node *to);
		
		std::map<int, node*> *corridorNodes;
};

class ClusterAStar : public AbstractClusterAStar
{
	public:
		#ifdef UNITTEST
			friend class ClusterAStarTest; 
		#endif
		
		ClusterAStar();
		virtual	~ClusterAStar();

		virtual path *getPath(graphAbstraction *aMap, node *from, node *to, reservationProvider *rp = 0);
		virtual const char* getName() { return "ClusterAStar"; }
		virtual void logFinalStats(statCollection *stats);

		virtual double h(node* a, node* b) throw(std::invalid_argument);

	protected:
		bool evaluate(node* current, node* target, edge* e=0);
		bool checkParameters(graphAbstraction* aMap, node* from, node* to);
};

#endif

