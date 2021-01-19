#include "MacroEdge.h"

MacroEdge::MacroEdge(unsigned int from, unsigned int to, double weight)
	: edge(from, to, weight)
{
	secondary = false;
}

MacroEdge::MacroEdge(MacroEdge& e) : edge(e)
{
	this->secondary = e.secondary;
}

MacroEdge::~MacroEdge()
{
}
