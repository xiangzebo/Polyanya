#ifndef RECURSIVEJUMPPOINTEXPANSIONPOLICY_H
#define RECURSIVEJUMPPOINTEXPANSIONPOLICY_H

// RecursiveJumpPointExpansionPolicy.h
//
// This expansion policy expands on the ideas implemented in
// JumpPointExpansionPolicy. Specifically:
// Rather than generate the set of immediately adjacent jump points as successors
// we will instead try to jump further when we can prove that the branching
// factor of the currently expanding node is 1.
// To prove such a thing, we employ a recursive procedure that looks
// at the children of each successor node. If none can be found we say that the 
// successor is sterile and reduce the branching factor of the expanding node by 1.
//
// When we can reduce the branching factor of the expanding node to exactly 1, we
// jump past the remaining successor (without generating it) and on to a node that is
// further away -- which we generate instead as a successor.
//
// For theoretical details see: [Harabor & Grastien, 2012]
//
// nstead of generating only the set 
// of adjacent jump points as successors during node expansion, we 
// @author: dharabor
// @created: 06/01/2010

#include "ExpansionPolicy.h"
#include "Jump.h"
#include "JumpInfo.h"

#include <vector>
#include <stdexcept>

class JumpPointLocator;
class RecursiveJumpPointExpansionPolicy : 
	public ExpansionPolicy
{
	// a container for storing last_dir values of generated nodes
	typedef std::vector<Jump::Direction> DirectionList;

	public:
		RecursiveJumpPointExpansionPolicy(JumpPointLocator* jpl, 
				unsigned int maxdepth=3);
		virtual ~RecursiveJumpPointExpansionPolicy();

		JumpInfo* getJumpInfo();
		int getMaxDepth() { return MAX_DEPTH; } 

		// ExpansionPolicy stuff
		virtual void expand(node* t) throw(std::logic_error);
		virtual node* first();
		virtual node* next();
		virtual node* n();
		virtual double cost_to_n();
		virtual bool hasNext();
		virtual void reset() { directions.clear(); }
		virtual void setProblemInstance(ProblemInstance* p);
		int jumplimit; 

	private:
		void computeNeighbourSet();
		void findJumpNode(Jump::Direction dir, JumpInfo& out);
		JumpInfo* findBranchingNode(node* from, Jump::Direction last_dir, 
				unsigned int depth);
		void addNeighbour(JumpInfo& out);
		void label_n();
		Jump::Direction getDirection(node* n);

		// the set of reachable jump points from ::target (and their costs)
		std::vector<JumpInfo*> neighbours; // jump point successors for ::target
		DirectionList directions; // last_dir labels for every generated node

		unsigned int neighbourIndex; // index of the neighbour returned by ::n()
		JumpPointLocator* jpl;
		unsigned int MAX_DEPTH; // maximum recursion depth

};

#endif

