#ifndef DEFAULTREFINEMENTPOLICY_H
#define DEFAULTREFINEMENTPOLICY_H

#include "RefinementPolicy.h"

class FlexibleAStar;
class mapAbstraction;
class path;
class DefaultRefinementPolicy : public RefinementPolicy
{
	public:
		DefaultRefinementPolicy(mapAbstraction* _map);
		virtual ~DefaultRefinementPolicy();

		virtual path* refine(path* abspath);
		inline void setVerbose(bool _verbose) { verbose = _verbose; }
		inline bool getVerbose() { return verbose; }

	private:
		mapAbstraction* map_;
		bool verbose;
};


#endif

