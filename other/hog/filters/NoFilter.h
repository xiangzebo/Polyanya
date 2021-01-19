#ifndef NOFILTER_H
#define NOFILTER_H

// NoFilter.h
//
// Default node filter. Accepts everything.
//
// @author: dharabor
// @created: 24/09/2010
//

#include "NodeFilter.h"

class NoFilter : public NodeFilter
{
	public:
		NoFilter() { }
		virtual ~NoFilter() { } 

		virtual bool filter(node* n) { return true; }
};

#endif

