#ifndef WARTHOG_ZERO_HEURISTIC_H
#define WARTHOG_ZERO_HEURISTIC_H

// zero_heuristic.h
//
// @author: dharabor
// @created: 2014-10-22
//

#include "constants.h"
#include "helpers.h"

#include <cstdlib>

namespace warthog
{

class zero_heuristic
{
	public:
		zero_heuristic(unsigned int mapwidth, unsigned int mapheight) {}
		~zero_heuristic() {}

		inline warthog::cost_t
		h(unsigned int x, unsigned int y, unsigned int x2, unsigned int y2)
		{
            return 0;
		}

		inline warthog::cost_t
		h(unsigned int id, unsigned int id2)
		{
            return 0;
		}
};

}

#endif

