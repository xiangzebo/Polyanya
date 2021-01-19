#include "FlexibleAStarFactory.h"

#include "FlexibleAStar.h"
#include "Heuristic.h"
#include "IHeuristicFactory.h"
#include "IExpansionPolicyFactory.h"

FlexibleAStarFactory::FlexibleAStarFactory(IExpansionPolicyFactory* epf,
		IHeuristicFactory* hf)
{
	this->epf = epf;
	this->hf = hf;
}

FlexibleAStarFactory::~FlexibleAStarFactory()
{
	delete epf;
	delete hf;
}

searchAlgorithm* FlexibleAStarFactory::newSearchAlgorithm()
{	
	ExpansionPolicy* policy = epf->newExpansionPolicy();
	Heuristic* h = hf->newHeuristic();
	FlexibleAStar* astar = new FlexibleAStar(policy, h);
	return astar;
}

