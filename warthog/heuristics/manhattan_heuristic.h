#ifndef WARTHOG_MANHATTAN_HEURISTIC_H
#define WARTHOG_MANHATTAN_HEURISTIC_H

// manhattan_heuristic.h
//
// @author: dharabor
// @created: 21/08/2012
//

#include "constants.h"
#include "helpers.h"

#include <cstdlib>

namespace warthog
{

class manhattan_heuristic
{
	public:
		manhattan_heuristic(unsigned int mapwidth, unsigned int mapheight)
		 : mapwidth_(mapwidth)//, mapheight_(mapheight) 
         {}

		~manhattan_heuristic() {}

		inline warthog::cost_t
		h(unsigned int x, unsigned int y, unsigned int x2, unsigned int y2)
		{
            // NB: precision loss when warthog::cost_t is an integer
			return (abs((int32_t)x-(int32_t)x2) + abs((int32_t)y-(int32_t)y2)) * warthog::ONE;
		}

		inline warthog::cost_t
		h(unsigned int id, unsigned int id2)
		{
			unsigned int x, x2;
			unsigned int y, y2;
			warthog::helpers::index_to_xy(id, mapwidth_, x, y);
			warthog::helpers::index_to_xy(id2,mapwidth_, x2, y2);
			return this->h(x, y, x2, y2);
		}

	private:
		//unsigned int mapwidth_, mapheight_;
		unsigned int mapwidth_;

};

}

#endif

