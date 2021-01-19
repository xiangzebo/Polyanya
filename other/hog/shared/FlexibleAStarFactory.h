#ifndef FLEXIBLEASTARFACTORY_H
#define FLEXIBLEASTARFACTORY_H

#include "ISearchAlgorithmFactory.h"

class IExpansionPolicyFactory;
class IHeuristicFactory;
class FlexibleAStarFactory : public ISearchAlgorithmFactory
{
	public:
		FlexibleAStarFactory(IExpansionPolicyFactory* epf, IHeuristicFactory* hf);
		virtual ~FlexibleAStarFactory();
		virtual searchAlgorithm* newSearchAlgorithm();

	private:
		IExpansionPolicyFactory* epf;
		IHeuristicFactory* hf;

};

#endif

