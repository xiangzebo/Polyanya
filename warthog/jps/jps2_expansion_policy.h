#ifndef WARTHOG_JPS2_EXPANSION_POLICY_H
#define WARTHOG_JPS2_EXPANSION_POLICY_H

// jps2_expansion_policy.h
//
// An experimental variation of warthog::jps2_expansion_policy,
// this version works with a modified version of online JPS
// which skips intermediate jump points (i.e. those jps 
// that do not have any forced neighbours)
//
// @author: dharabor
// @created: 06/01/2010

#include "blocklist.h"
#include "gridmap.h"
#include "helpers.h"
#include "jps.h"
#include "online_jump_point_locator2.h"
#include "problem_instance.h"
#include "search_node.h"

#include "stdint.h"

namespace warthog
{

class jps2_expansion_policy 
{
	public:
		jps2_expansion_policy(warthog::gridmap* map);
		~jps2_expansion_policy();

		// create a warthog::search_node object from a state description
		// (in this case, an id)
		inline warthog::search_node*
		generate(uint32_t node_id)
		{
			return nodepool_->generate(node_id);
		}


		// reset the policy and discard all generated nodes
		inline void
		clear()
		{
			reset();
			nodepool_->clear();
		}


		void 
		expand(warthog::search_node*, warthog::problem_instance*);

		inline void
		first(warthog::search_node*& ret, warthog::cost_t& cost)
		{
			which_ = 0;
			ret = neighbours_[which_];
			cost = costs_[which_];
		}

		inline bool
		has_next()
		{
			if((which_+1) < num_neighbours_) { return true; }
			return false;
		}

		inline void
		n(warthog::search_node*& ret, warthog::cost_t& cost)
		{
			ret = neighbours_[which_];
			cost = costs_[which_];
		}

		inline void
		next(warthog::search_node*& ret, warthog::cost_t& cost)
		{
			if(which_ < num_neighbours_)
			{
				which_++;
			}
			ret = neighbours_[which_];
			cost = costs_[which_];
		}

		inline uint32_t
		mem()
		{
			return sizeof(*this) + map_->mem() + nodepool_->mem() + jpl_->mem();
		}

		uint32_t 
		mapwidth()
		{
			return map_->width();
		}

	private:
		warthog::gridmap* map_;
		warthog::blocklist* nodepool_;
		online_jump_point_locator2* jpl_;
		uint32_t which_;
		uint32_t num_neighbours_;
		std::vector<warthog::search_node*> neighbours_;
		std::vector<warthog::cost_t> costs_;
		std::vector<uint32_t> jp_ids_;

		inline void
		reset()
		{
			which_ = 0;
			num_neighbours_ = 0;
			neighbours_.clear();
			costs_.clear();
			jp_ids_.clear();
		}

};

}

#endif

