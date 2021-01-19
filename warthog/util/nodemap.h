#ifndef WARTHOG_NODEMAP_H
#define WARTHOG_NODEMAP_H

#include "gridmap.h"

#include <cassert>

// nodemap.h
//
// Maps integer values from [0, N) to [0, N).
// This implementation is not hash-based and requires N * sizeof(int) space
//
// @author: dharabor
// 19/08/2012
//

namespace warthog
{

class nodemap
{
	public:
		nodemap(unsigned int size) 
			: size_(size), map_(new unsigned int[size_])
		{
			clear();
		}

		~nodemap()
		{
			delete [] map_;
		}

		void
		set_value(unsigned int index, unsigned int value)
		{
			assert(index < size_);
			map_[index] = value;
		}

		inline unsigned int
		get_value(unsigned int index)
		{
			if(index < size_)
			{
				return map_[index];
			}
			return warthog::INF;
		}

		void
		clear()
		{
			for(unsigned int i=0; i < size_; i++)
			{
				map_[i] = warthog::INF;
			}
		}

	private:
		unsigned int size_;
		unsigned int* map_;
};

}

#endif

