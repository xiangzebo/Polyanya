#include "ExpansionPolicy.h"
#include "graph.h"
#include "mapAbstraction.h"
#include "ProblemInstance.h"

ExpansionPolicy::ExpansionPolicy()
{
	this->target = 0;
	this->problem = 0; 
	verbose = false;
}

ExpansionPolicy::~ExpansionPolicy() 
{
	if(problem)
	{
		delete problem;
		problem = 0;
	}
}

void 
ExpansionPolicy::expand(node* t) throw(std::logic_error)
{
	if(problem == 0)
		throw std::logic_error("ExpansionPolicy::expand "
				" ProblemInstance not set!");
	this->target = t;
}

ProblemInstance*
ExpansionPolicy::getProblemInstance()
{
	return problem;
}

void
ExpansionPolicy::setProblemInstance(ProblemInstance* problem_)
{
	if(problem)
	{
		delete problem;
		problem = 0;
	}

	this->problem = problem_;
}
