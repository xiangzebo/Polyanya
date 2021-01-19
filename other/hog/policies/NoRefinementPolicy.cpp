#include "NoRefinementPolicy.h"

#include "path.h"

NoRefinementPolicy::NoRefinementPolicy() : RefinementPolicy()
{
}

NoRefinementPolicy::~NoRefinementPolicy()
{
}

// @return: a copy of parameter abspath
path* NoRefinementPolicy::refine(path* abspath)
{
	if(!abspath || !abspath->next)
		return 0;

	path* thepath = 0;
	path* last = 0;
	for(path* mypath = abspath; mypath->next != 0; mypath = mypath->next)
	{
		path* current = new path(mypath->n, 0);

		if(thepath == 0)
			thepath = current;										
		else
		{
			thepath->tail()->next = current;
		}

		last = current;
	}
	return thepath;
}
