#ifndef WARTHOG_HELPERS_H
#define WARTHOG_HELPERS_H

// helpers.h
//
// Helper functions that don't fit anywhere else.
//
// @author: dharabor
// @created: 21/08/2012
//

namespace warthog
{
namespace helpers
{

// convert id into x/y coordinates on a grid of width 'mapwidth'
inline void
index_to_xy(unsigned int id, unsigned int mapwidth, 
		unsigned int& x, unsigned int& y)
{	
	y = id / mapwidth;
	x = id % mapwidth;
//	x = id;
//	y = 0;
//	for( ; x >= mapwidth ; x -= mapwidth)
//	{
//		y++; 
//	}
}

}
}

#endif


