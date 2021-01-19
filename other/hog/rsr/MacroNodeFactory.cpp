#include "MacroNodeFactory.h"

MacroNodeFactory::MacroNodeFactory()
{
}

MacroNodeFactory::MacroNodeFactory(MacroNodeFactory* mnf)
{
}

MacroNodeFactory::~MacroNodeFactory()
{
}

MacroNode* MacroNodeFactory::newNode(const char* name) throw(std::invalid_argument)
{
	return new MacroNode(name);
}

// If n is of type MacroNode this function returns a deep copy of n.
// Otherwise, if n is of some other type the function returns 0;
MacroNode* MacroNodeFactory::newNode(const node* n) throw(std::invalid_argument)
{
	const MacroNode* mn = dynamic_cast<const MacroNode*>(n);
	if(mn)
	{
		return new MacroNode(mn);
	}
	return 0;
}

