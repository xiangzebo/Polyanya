// MacroNodeFactory.h
//
// Factory object for creating MacroNode objects.
//
// @author: dharabor
// @created: 03-04-2010
//

#ifndef MACRONODEFACTORY_H
#define MACRONODEFACTORY_H

#include "INodeFactory.h"
#include "MacroNode.h"

class MacroNodeFactory : public INodeFactory
{
	public:
		MacroNodeFactory();
		MacroNodeFactory(MacroNodeFactory* mnf);
		virtual ~MacroNodeFactory();
		virtual MacroNodeFactory* clone() { return new MacroNodeFactory(); }

		virtual MacroNode* newNode(const char* name) throw(std::invalid_argument);
		virtual MacroNode* newNode(const node* n) throw(std::invalid_argument);
};

#endif

