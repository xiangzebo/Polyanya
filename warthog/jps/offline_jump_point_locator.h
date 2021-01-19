#ifndef WARTHOG_OFFLINE_JUMP_POINT_LOCATOR_H
#define WARTHOG_OFFLINE_JUMP_POINT_LOCATOR_H

// offline_jump_point_locator.h
//
// Identifies jump points using a pre-computed database that stores
// distances from each node to jump points in every direction.
//
// @author: dharabor
// @created: 05/05/2013
//

#include "jps.h"

namespace warthog
{

class gridmap;
class offline_jump_point_locator
{
	public:
		offline_jump_point_locator(warthog::gridmap* map);
		~offline_jump_point_locator();

		void
		jump(warthog::jps::direction d, uint32_t node_id, uint32_t goalid, 
				uint32_t& jumpnode_id, warthog::cost_t& jumpcost);

		uint32_t
		mem()
		{
			return sizeof(this) + sizeof(*db_)*dbsize_;
		}


	private:

		void
		preproc();

		bool
		load(const char* filename);

		void 
		save(const char* filename);

		void
		jump_northwest(uint32_t node_id, uint32_t goal_id, 
				uint32_t& jumpnode_id, warthog::cost_t& jumpcost);
		void
		jump_northeast(uint32_t node_id, uint32_t goal_id, 
				uint32_t& jumpnode_id, warthog::cost_t& jumpcost);
		void
		jump_southwest(uint32_t node_id, uint32_t goal_id, 
				uint32_t& jumpnode_id, warthog::cost_t& jumpcost);
		void
		jump_southeast(uint32_t node_id, uint32_t goal_id, 
				uint32_t& jumpnode_id, warthog::cost_t& jumpcost);
		void
		jump_north(uint32_t node_id, uint32_t goal_id, 
				uint32_t& jumpnode_id, warthog::cost_t& jumpcost);
		void
		jump_south(uint32_t node_id, uint32_t goal_id, 
				uint32_t& jumpnode_id, warthog::cost_t& jumpcost);
		void
		jump_east(uint32_t node_id, uint32_t goal_id, 
				uint32_t& jumpnode_id, warthog::cost_t& jumpcost);
		void
		jump_west(uint32_t node_id, uint32_t goal_id, 
				uint32_t& jumpnode_id, warthog::cost_t& jumpcost);

		warthog::gridmap* map_;
		uint32_t dbsize_;
		uint16_t* db_;	

		//uint32_t jumppoints_[3];
		//warthog::cost_t costs_[3];
		uint32_t max_;
		uint32_t current_;
};

}

#endif

