#ifndef JUMPPOINTABSTRACTION_H
#define JUMPPOINTABSTRACTION_H

// JumpPointAbstraction.h
//
// Creates a graph in which every neighbour of each node
// is a jump point.
//
// @author: dharabor
// @created: 15/03/2011
//

#include "mapAbstraction.h"
#include "Jump.h"

#include <string>

class IEdgeFactory;
class INodeFactory;
class Map;
class node;
class edge;
class graph;

class JumpPointAbstraction : public mapAbstraction
{
	public:
		JumpPointAbstraction(Map*, INodeFactory*, IEdgeFactory*, 
				bool _allowDiagonals = true, bool _cutCorners = true,
				bool _verbose = false);
		JumpPointAbstraction(Map*, const char* graphfile,
				INodeFactory*, IEdgeFactory*, 
				bool _allowDiagonals = true, bool _cutCorners = true,
				bool _verbose = false);
		virtual ~JumpPointAbstraction();
		virtual mapAbstraction *clone(Map *);

		virtual bool pathable(node*, node*);
		virtual void verifyHierarchy();
		virtual void removeNode(node *n);
		virtual void removeEdge(edge *e, unsigned int absLevel);
		virtual void addNode(node *n);
		virtual void addEdge(edge *e, unsigned int absLevel);
		virtual void repairAbstraction();

		bool getVerbose() { return verbose; }
		void setVerbose(bool _verbose) { this->verbose = _verbose; }

	private:
		bool verbose;
		INodeFactory* nf;
		IEdgeFactory* ef;

		void makeJumpPointGraph();		
		void importGraph(const char* filename);
		node* findJumpNode(Jump::Direction d, int x, int y);
		node* findObstacleJumpNode(Jump::Direction d, int x, int y);
};

#endif

