#ifndef CORRIDORFILTER_H
#define CORRIDORFILTER_H

// CorridorFilter.h
//
// A filter that checks if a given node belongs to a collection
// of nodes. Useful during node expansion when the search must be
// limited to some arbitrary set of nodes (like a cluster or corridor).
//
// @author: dharabor
// @created: 24/09/2010

#include "NodeFilter.h"
#include <map>

class node;
class CorridorFilter : public NodeFilter
{
	public:
		CorridorFilter();
		virtual ~CorridorFilter();

		virtual bool filter(node* n);
		void setCorridorNodes(std::map<int, node*>* _nodes);

	private:
		std::map<int, node*>* corridorNodes;
};

#endif

