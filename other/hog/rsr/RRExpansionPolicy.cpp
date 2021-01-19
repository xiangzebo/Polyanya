#include "RRExpansionPolicy.h"

#include "IncidentEdgesExpansionPolicy.h"
#include "EmptyCluster.h"
#include "EmptyClusterAbstraction.h"
#include "MacroNode.h"
#include "graphAbstraction.h"
#include "graph.h"
#include "ProblemInstance.h"

#include <cfloat>

RRExpansionPolicy::RRExpansionPolicy(graphAbstraction* map) :

	SelectiveExpansionPolicy()
{
	this->map = map;	

	primary = new IncidentEdgesExpansionPolicy(map);
	skipSecondary = false;
	whichSecondary = -1;
	numSecondary = 0;
	cost = 0;
}

RRExpansionPolicy::~RRExpansionPolicy()
{
	delete primary;
}

void RRExpansionPolicy::expand(node* target_) throw(std::logic_error)
{
	ExpansionPolicy::expand(target_);

	if(!primary->getProblemInstance())
		primary->setProblemInstance(problem->clone());
	primary->expand(target_);

	this->g = map->getAbstractGraph(target_->getLabelL(kAbstractionLevel));
	MacroNode* mnTarget = dynamic_cast<MacroNode*>(target_);
	assert(mnTarget);
	MacroNode* mnBackpointer = dynamic_cast<MacroNode*>(target_->backpointer);
	if(mnTarget && mnBackpointer && 
		mnTarget->getParentClusterId() == mnBackpointer->getParentClusterId())
	{
		skipSecondary = true;
	}
	else
	{
		skipSecondary = false;
	}

	whichSecondary = -1;
	numSecondary = mnTarget->numSecondaryEdges();
}

node* RRExpansionPolicy::first_impl()
{
	whichSecondary = -1;
	node* retVal = primary->first();
	cost = primary->cost_to_n();
	return retVal;
}

node* RRExpansionPolicy::n_impl()
{
	node* retVal = primary->n();
	if(retVal)
		cost = primary->cost_to_n();
	else
	{
		assert(primary->hasNext() == false);
		if(skipSecondary == false && whichSecondary < numSecondary)
		{
			assert(0 <= whichSecondary);
			MacroNode* mnTarget = dynamic_cast<MacroNode*>(target);
			edge* e = mnTarget->getSecondaryEdge(whichSecondary);
			assert(e);
			int neighbourid = e->getFrom()==mnTarget->getNum()?e->getTo():e->getFrom();
			retVal = g->getNode(neighbourid);
			assert(retVal);
			cost = e->getWeight();
		}
	}
	return retVal;
}

node* RRExpansionPolicy::next_impl()
{
	node* retVal = primary->next();
	if(retVal)
	{
		cost = primary->cost_to_n();
	}
	else if(skipSecondary == false)
	{
		whichSecondary++;
		if(whichSecondary < numSecondary)
		{
			MacroNode* mnTarget = dynamic_cast<MacroNode*>(target);
			edge* e = mnTarget->getSecondaryEdge(whichSecondary);
			assert(e);
			int neighbourid = e->getFrom()==mnTarget->getNum()?e->getTo():e->getFrom();
			retVal = g->getNode(neighbourid);
			assert(retVal);
			cost = e->getWeight();
		}
		else
		{
			cost = DBL_MAX;
		}
	}

	return retVal;
}

bool RRExpansionPolicy::hasNext()
{
	if(primary->hasNext())
		return true;

	if(!skipSecondary && 0 < numSecondary && whichSecondary < numSecondary)
		return true;

	return false;
}

double RRExpansionPolicy::cost_to_n()
{
	return cost;	
}

void
RRExpansionPolicy::label_n()
{
	node* tmp = n();
	if(tmp)
		tmp->backpointer = this->target;
}

