#include "IncidentEdgesPolicyFactory.h"

#include "IncidentEdgesExpansionPolicy.h"
#include "graphAbstraction.h"

IncidentEdgesPolicyFactory::IncidentEdgesPolicyFactory(graphAbstraction* map)
{
	this->map = map;
}

IncidentEdgesPolicyFactory::~IncidentEdgesPolicyFactory()
{
}

ExpansionPolicy* IncidentEdgesPolicyFactory::newExpansionPolicy()
{
	IncidentEdgesExpansionPolicy* policy = new IncidentEdgesExpansionPolicy(map);
	return policy;
}
