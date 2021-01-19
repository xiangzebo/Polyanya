#ifndef MACROEDGEFACTORY_H
#define MACROEDGEFACTORY_H

#include "IEdgeFactory.h"
#include "MacroEdge.h"

class MacroEdgeFactory : public IEdgeFactory
{
	public:
		MacroEdgeFactory();
		~MacroEdgeFactory();

		virtual MacroEdge* newEdge(unsigned int fromId, unsigned int toId, 
				double weight);
};

#endif
