// MacroNode.h
// 
//	@author: dharabor
//	@created: 03-04-2010
// 

#ifndef MACRONODE_H
#define MACRONODE_H

#include "ClusterNode.h"

class MacroNode : public ClusterNode
{
	public:
		MacroNode(const char* _n);
		MacroNode(const MacroNode* n);		
		virtual ~MacroNode();
		virtual graph_object* clone() const 
		{ 
			return new MacroNode(this); 
		}
		virtual void reset();

		inline void	setParent(MacroNode* _p) { p = _p; }
		inline MacroNode* getParent() { return p; }

		std::vector<edge*>::iterator secondaryEdgeIter() 
		{ 
			return secondaryEdges.begin(); 
		}

		void addSecondaryEdge(edge* e); 
		
		// get an edge using its index in the secondaryEdges collection
		edge* getSecondaryEdge(unsigned int index);

		// find an edge with id = edgeNum and remove it from secondaryEdges
		// NB: edgeNum != index of edge in collection
		void removeSecondaryEdge(unsigned int edgeNum);

		unsigned int numSecondaryEdges() { return secondaryEdges.size(); }
		void clearSecondaryEdges() { secondaryEdges.clear(); }

	private:
		std::vector<edge*> secondaryEdges;
		MacroNode* p;
		double pdist;
};

#endif

