#ifndef INCIDENTEDGESPOLICYFACTORY_H
#define INCIDENTEDGESPOLICYFACTORY_H

// IncidentEdgesPolicyFactory.h
//
// A factory for IncidentEdgesExpansionPolicy objects.
//
// @author: dharabor
// @created: 25/10/2010
//

#include "IExpansionPolicyFactory.h"

class graphAbstraction;
class ExpansionPolicy;
class IncidentEdgesPolicyFactory : public IExpansionPolicyFactory
{
	public:
		IncidentEdgesPolicyFactory(graphAbstraction* map);
		virtual ~IncidentEdgesPolicyFactory();

		virtual ExpansionPolicy* newExpansionPolicy();

	private:
		graphAbstraction* map;
};

#endif

