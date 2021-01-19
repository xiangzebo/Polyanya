#ifndef NODEFILTER_H
#define NODEFILTER_H

#include "graph.h"

class NodeFilter 
{

	public:
		NodeFilter() {}
		virtual ~NodeFilter() { }

		virtual bool filter(node* n) = 0;
};

#endif

