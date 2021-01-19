#ifndef NOREFINEMENTPOLICY_H
#define NOREFINEMENTPOLICY_H

// NoRefinementPolicy.h
//
// The class to use when you want to run a hierarchical 
// search but do not want to refine the abstract path.
//
// @author: dharabor
// @created: 08/03/2011
//

#include "RefinementPolicy.h"

class NoRefinementPolicy : public RefinementPolicy
{
	public: 
		NoRefinementPolicy();
		virtual ~NoRefinementPolicy();

		virtual path* refine(path* thepath);
};

#endif

