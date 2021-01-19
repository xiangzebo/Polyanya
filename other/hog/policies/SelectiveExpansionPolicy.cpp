#include "SelectiveExpansionPolicy.h"

#include "graph.h"
#include "NodeFilter.h"

SelectiveExpansionPolicy::SelectiveExpansionPolicy() :
	ExpansionPolicy()
{
}

SelectiveExpansionPolicy::~SelectiveExpansionPolicy()
{
	for(unsigned int i=0; i<filters.size(); i++)
	{
		NodeFilter* nf = filters.at(i);
		delete nf;
	}

	filters.clear();
}

node* SelectiveExpansionPolicy::first()
{
	if(!target) 
		return 0;

	node* retVal = first_impl();
	if(filter(retVal))
		retVal = next();

	return retVal;
}

node* SelectiveExpansionPolicy::n()
{
	if(!target)
		return 0;

	node* retVal = n_impl();
	if(!retVal && hasNext())
		retVal = next();

	return retVal;
}

node* SelectiveExpansionPolicy::next()
{
	if(!target)
		return 0;

	node *retVal = next_impl();
	while(filter(retVal))
	{
		retVal = 0;
		if(hasNext())
			retVal = next_impl();
		else
			break;
	}

	return retVal;
}

void SelectiveExpansionPolicy::addFilter(NodeFilter* nf)
{
	filters.push_back(nf);
}

bool SelectiveExpansionPolicy::filter(node* n)
{
	for(unsigned int i = 0; i<filters.size(); i++)
	{
		NodeFilter* nf = filters.at(i);
		if(nf->filter(n))
			return true;
	}
	return false;
}
