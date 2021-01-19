#ifndef WARTHOG_ONLINE_JUMP_POINT_LOCATOR2_H
#define WARTHOG_ONLINE_JUMP_POINT_LOCATOR2_H

// online_jump_point_locator2.h
//
// A class wrapper around some code that finds, online, jump point
// successors of an arbitrary nodes in a uniform-cost grid map.
//
// For theoretical details see:
// [Harabor D. and Grastien A, 2011, 
// Online Graph Pruning Pathfinding on Grid Maps, AAAI]
//
// @author: dharabor
// @created: 03/09/2012
//

#include "jps.h"

//class warthog::gridmap;
namespace warthog
{

class online_jump_point_locator2
{
	public: 
		online_jump_point_locator2(warthog::gridmap* map);
		~online_jump_point_locator2();

		void
		jump(warthog::jps::direction d, uint32_t node_id, uint32_t goalid, 
				std::vector<uint32_t>& jpoints,
				std::vector<warthog::cost_t>& costs);

		uint32_t 
		mem()
		{
			return sizeof(this) + rmap_->mem();
		}

	private:
		void
		jump_north(
				std::vector<uint32_t>& jpoints, 
				std::vector<warthog::cost_t>& costs);
		void
		jump_south(
				std::vector<uint32_t>& jpoints, 
				std::vector<warthog::cost_t>& costs);
		void
		jump_east(
				std::vector<uint32_t>& jpoints, 
				std::vector<warthog::cost_t>& costs);
		void
		jump_west(
				std::vector<uint32_t>& jpoints, 
				std::vector<warthog::cost_t>& costs);
		void
		jump_northeast(
				std::vector<uint32_t>& jpoints, 
				std::vector<warthog::cost_t>& costs);
		void
		jump_northwest(
				std::vector<uint32_t>& jpoints, 
				std::vector<warthog::cost_t>& costs);
		void
		jump_southeast(
				std::vector<uint32_t>& jpoints, 
				std::vector<warthog::cost_t>& costs);
		void
		jump_southwest(
				std::vector<uint32_t>& jpoints, 
				std::vector<warthog::cost_t>& costs);

		// these versions can be passed a map parameter to
		// use when jumping. they allow switching between
		// map_ and rmap_ (a rotated counterpart).
		void
		__jump_north(uint32_t node_id, uint32_t goal_id, 
				uint32_t& jumpnode_id, warthog::cost_t& jumpcost,
				warthog::gridmap* mymap);
		void
		__jump_south(uint32_t node_id, uint32_t goal_id, 
				uint32_t& jumpnode_id, warthog::cost_t& jumpcost, 
				warthog::gridmap* mymap);
		void
		__jump_east(uint32_t node_id, uint32_t goal_id, 
				uint32_t& jumpnode_id, warthog::cost_t& jumpcost, 
				warthog::gridmap* mymap);
		void
		__jump_west(uint32_t node_id, uint32_t goal_id, 
				uint32_t& jumpnode_id, warthog::cost_t& jumpcost, 
				warthog::gridmap* mymap);

		// these versions perform a single diagonal jump, returning
		// the intermediate diagonal jump point and the straight 
		// jump points that caused the jumping process to stop
		void
		__jump_northeast(
				uint32_t& node_id, uint32_t& rnode_id, 
				uint32_t goal_id, uint32_t rgoal_id,
				uint32_t& jumpnode_id, warthog::cost_t& jumpcost,
				uint32_t& jp1_id, warthog::cost_t& jp1_cost,
				uint32_t& jp2_id, warthog::cost_t& jp2_cost);
		void
		__jump_northwest(
				uint32_t& node_id, uint32_t& rnode_id, 
				uint32_t goal_id, uint32_t rgoal_id,
				uint32_t& jumpnode_id, warthog::cost_t& jumpcost,
				uint32_t& jp1_id, warthog::cost_t& jp1_cost,
				uint32_t& jp2_id, warthog::cost_t& jp2_cost);
		void
		__jump_southeast(
				uint32_t& node_id, uint32_t& rnode_id, 
				uint32_t goal_id, uint32_t rgoal_id,
				uint32_t& jumpnode_id, warthog::cost_t& jumpcost,
				uint32_t& jp1_id, warthog::cost_t& jp1_cost,
				uint32_t& jp2_id, warthog::cost_t& jp2_cost);
		void
		__jump_southwest(
				uint32_t& node_id, uint32_t& rnode_id, 
				uint32_t goal_id, uint32_t rgoal_id,
				uint32_t& jumpnode_id, warthog::cost_t& jumpcost,
				uint32_t& jp1_id, warthog::cost_t& jp1_cost,
				uint32_t& jp2_id, warthog::cost_t& jp2_cost);

		// functions to convert map indexes to rmap indexes
		inline uint32_t
		map_id_to_rmap_id(uint32_t mapid)
		{
			if(mapid == warthog::INF) { return mapid; }

			uint32_t x, y;
			uint32_t rx, ry;
			map_->to_unpadded_xy(mapid, x, y);
			ry = x;
			rx = map_->header_height() - y - 1;
			return rmap_->to_padded_id(rx, ry);
		}

		// convert rmap indexes to map indexes
		inline uint32_t
		rmap_id_to_map_id(uint32_t rmapid)
		{
			if(rmapid == warthog::INF) { return rmapid; }

			uint32_t x, y;
			uint32_t rx, ry;
			rmap_->to_unpadded_xy(rmapid, rx, ry);
			x = ry;
			y = rmap_->header_width() - rx - 1;
			return map_->to_padded_id(x, y);
		}

		warthog::gridmap*
		create_rmap();

		warthog::gridmap* map_;
		warthog::gridmap* rmap_;
		//uint32_t jumplimit_;

		uint32_t current_goal_id_;
		uint32_t current_rgoal_id_;
		uint32_t current_node_id_;
		uint32_t current_rnode_id_;
};

}

#endif

