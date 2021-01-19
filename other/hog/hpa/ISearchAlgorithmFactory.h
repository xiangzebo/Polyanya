#ifndef ISEARCHALGORITHMFACTORY_H
#define SEARCHALGORITHMFACTORY_H

// ISearchAlgorithmFactory
//
// An interface for searchAlgorithm factories.
//
// @author: dharabor
// @created: 25/10/2010

#include "searchAlgorithm.h"

class ISearchAlgorithmFactory
{
	public:
		ISearchAlgorithmFactory() { }
		virtual ~ISearchAlgorithmFactory() { }

		virtual searchAlgorithm* newSearchAlgorithm() = 0;
};

#endif

