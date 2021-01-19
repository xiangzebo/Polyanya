#ifndef WARTHOG_BLOCKLIST2_H
#define WARTHOG_BLOCKLIST2_H

// blocklist2.h
//
// A heap for allocated warthog::search_node objects.
// Stores addresses of allocated nodes and also acts as
// a closed list of sorts.
// This implementation is specialised for spatial networks.
// Addresses can be stored in one of several buckets where each
// bucket represents a cell in a square subdivision of the 
// spatial net. 
//
// Similar idea to warthog::blocklist2, this implementation does
// not do any reclaimation of memory. Nodes are allocated once
// and re-used next time. 
//
// @author: dharabor
// @created: 02/09/2012
//

#include "cpool.h"
#include "search_node.h"

#include <stdint.h>

namespace warthog
{

namespace blocklist2_ns
{
	static const uint32_t NBS = 64; // node block size; set this >= 8
	static const uint32_t LOG2_NBS = 6;
	static const uint32_t NBS_MASK = 63;
}

class blocklist2
{
	public:
		blocklist2(uint32_t mapwidth, uint32_t mapheight);
		~blocklist2();

		// return a warthog::search_node object corresponding to the given id.
		// if the node has already been generated, return a pointer to the 
		// previous instance; otherwise allocate memory for a new object.
		warthog::search_node*
		generate(uint32_t node_id);

		void
		clear();

		uint32_t
		mem();

	private:
		uint32_t num_blocks_;
		warthog::search_node*** blocks_;
		warthog::mem::cpool* blockspool_;
		warthog::mem::cpool* pool_;
};

}

#endif

