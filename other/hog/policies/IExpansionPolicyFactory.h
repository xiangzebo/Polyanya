#ifndef IEXPANSIONPOLICYFACTORY_H
#define IEXPANSIONPOLICYFACTORY_H

class ExpansionPolicy;
class IExpansionPolicyFactory
{
	public:
		IExpansionPolicyFactory() { }
		virtual ~IExpansionPolicyFactory() { }

		virtual ExpansionPolicy* newExpansionPolicy() = 0;
};

#endif

