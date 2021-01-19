#ifndef WARTHOG_OCTILE_HEURISTIC_H
#define WARTHOG_OCTILE_HEURISTIC_H

// octile_heuristic.h
//
// Analogue of Manhattan Heuristic but for 8C grids (cf. 4C).
//
// @author: dharabor
// @created: 21/08/2012
//

#include "constants.h"
#include "helpers.h"

#include <cstdlib>

namespace warthog
{

class octile_heuristic
{
	public:
		octile_heuristic(unsigned int mapwidth, unsigned int mapheight) 
	    	: mapwidth_(mapwidth)//, mapheight_(mapheight) 
            { }
		~octile_heuristic() { }

		inline warthog::cost_t
		h(int32_t x, int32_t y, int32_t x2, int32_t y2)
		{
            // NB: precision loss when warthog::cost_t is an integer
			double dx = abs(x-x2);
			double dy = abs(y-y2);
			if(dx < dy)
			{
				return dx * warthog::ROOT_TWO + (dy - dx) * warthog::ONE;
			}
			return dy * warthog::ROOT_TWO + (dx - dy) * warthog::ONE;
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
		unsigned int mapwidth_;
		//unsigned int mapheight_;
};

}

#endif

