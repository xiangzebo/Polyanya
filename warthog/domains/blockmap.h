#ifndef WARTHOG_BLOCKMAP_H
#define WARTHOG_BLOCKMAP_H

// blockmap.h
//
// A blockmap is a division of gridmap into a set of rectangular blocks.
// The idea is to create a cache-efficient method for accessing sections
// of very large grid domains.
//
// @author: dharabor
// @created: 13/08/2012
//

#include "constants.h"
#include "gridmap.h"

#include <iostream>

namespace warthog
{
	
// blockmap constants
static const unsigned int BLOCKSIZE = 256; // NB: must be a power of 2!!
static const unsigned int LOG_BLOCKSIZE = ceil(log10(BLOCKSIZE) / log10(2));

class gm_parser;
class gridmap;
class blockmap
{
	public:
		blockmap(const char* filename);
		virtual ~blockmap();

		void print(std::ostream& out);

		inline bool
		get_label(unsigned int x, unsigned int y)
		{
			unsigned int block_x = x >> warthog::LOG_BLOCKSIZE;
			unsigned int block_y = y >> warthog::LOG_BLOCKSIZE;
			x = x & (warthog::BLOCKSIZE-1);
			y = y & (warthog::BLOCKSIZE-1);
			gridmap* block = blocks_[block_x][block_y];
			return block->get_label(block->to_padded_id(x, y));
		}

		inline void 
		set_label(unsigned int x, unsigned int y, bool label)
		{
			unsigned int block_x = x >> warthog::LOG_BLOCKSIZE;
			unsigned int block_y = y >> warthog::LOG_BLOCKSIZE;
			x = x & (warthog::BLOCKSIZE-1);
			y = y & (warthog::BLOCKSIZE-1);
			gridmap* block = blocks_[block_x][block_y];
			return block->set_label(block->to_padded_id(x, y), label);
		}

		inline unsigned int height() { return header_->height_; }
		inline unsigned int width() { return header_->width_; }

		inline unsigned int 
		get_num_blocks() 
		{ 
			return this->dbheight()*this->dbwidth(); 
		}

	private:
		inline unsigned int dbheight()
		{
			return (header_->height_ >> warthog::LOG_BLOCKSIZE)+1;
		}

		inline unsigned int dbwidth()
		{
			return (header_->width_ >> warthog::LOG_BLOCKSIZE)+1;
		}

		gm_header* header_;
		gridmap*** blocks_; // gridmap* matrix; each BLOCKSIZE*BLOCKSIZE cells
};

}

#endif

