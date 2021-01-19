#ifndef INSERTIONPOLICY_H
#define INSERTIONPOLICY_H

// InsertionPolicy.h
//
// Defines a general class of insertion algorithms
// for use during hierarchical pathfinding.
//
// An insertion algorithm takes the start and goal nodes from a low-level graph
// (usually the grid, but not always) and inserts them into
// a higher level graph.
//
// As the insertion is usually temporary, this class also defines
// function stubs for a remove method -- which deletes inserted nodes.
//
// @author: dharabor
// @created: 08/03/2011
//

#include <iostream>
#include <stdexcept>
#include <vector>

class statCollection;
class node;
class InsertionPolicy
{
	public:
		InsertionPolicy();
		virtual ~InsertionPolicy();

		virtual node* insert(node* n) throw(std::invalid_argument) = 0;
		virtual void remove(node* n) throw(std::runtime_error) = 0;

		// metrics
		long getNodesExpanded() { return nodesExpanded; }
		long getNodesTouched() { return nodesTouched; }
		long getNodesGenerated() { return nodesGenerated; }
		double getSearchTime() { return searchTime; }
		void resetMetrics();

		inline void setVerbose(bool _v) { verbose = _v; }
		inline bool getVerbose() { return verbose; }

	protected:
		void addNode(node* n);
		bool removeNode(node* n);
		int getNumInserted() { return insertedNodes->size(); }
		node* getInsertedAtIndex(int idx) { return insertedNodes->at(idx); }

		long nodesExpanded;
		long nodesTouched;
		long nodesGenerated;
		double searchTime;

	private:
		std::vector<node*>* insertedNodes;
		bool verbose;

};

#endif

