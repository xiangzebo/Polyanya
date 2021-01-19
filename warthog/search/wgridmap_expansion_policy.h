#ifndef WARTHOG_WGRIDMAP_EXPANSION_POLICY_H
#define WARTHOG_WGRIDMAP_EXPANSION_POLICY_H

// wgridmap_expansion_policy.h
//
// An expansion policy for square gridmaps with
// non-uniform costs.
//
// @author: dharabor
// @created: 2014-09-17
//

#include "blocklist.h"
#include "search_node.h"
#include "weighted_gridmap.h"

#include <memory>

namespace warthog
{

class wgridmap_expansion_policy
{
	public:
		wgridmap_expansion_policy(warthog::weighted_gridmap* map);
		~wgridmap_expansion_policy();

		warthog::search_node*
		generate(uint32_t node_id)
		{
			warthog::search_node* n = nodepool_->generate(node_id);
			assert(n->get_id() == node_id);
			return n;
		}

		inline void
		clear()
		{
			this->reset();
			nodepool_->clear();
		}

		inline void
		reset()
		{
			num_neis_ = 0;
			which_ = 0;

			for(int i=0; i < 10; i++)
			{
				neis_[i] = 0;
				costs_[i] = warthog::INF;
			}
		}

		void 
		expand(warthog::search_node*, warthog::problem_instance*);

		// fetches the first neighbour of (cx_, cy_). 
		// also resets the current neigbour iterator
		inline void
		first(warthog::search_node*& n, warthog::cost_t& cost)
		{
			which_ = 0;
			n = neis_[which_];
			cost = costs_[which_];
		}

		// return in @param n the id of the current neighbour of (cx_, cy_) -- the node
		// being expanded. 
		// ::end is returned if all neighbours have been processed.
		inline void
		n(warthog::search_node*& n, warthog::cost_t& cost)
		{
			n = neis_[which_];
			cost = costs_[which_];
		}


		// @return the node id of the next neighbour of (cx_, cy_)
		// ::end() is returned if there is no next neighbour
		inline void
		next(warthog::search_node*& n, warthog::cost_t& cost)
		{
			if(which_ < num_neis_) { which_++; }
			n = neis_[which_];
			cost = costs_[which_];
		}

		// @return true if (cx_, cy_) has more neighbours to process.
		// false otherwise.
		bool 
		has_next()
		{
			if((which_+1) < num_neis_)
			{
				return true;
			}
			return false;
		}

		uint32_t
		mem()
		{
			return sizeof(*this) +
				map_->mem() + 
				nodepool_->mem();
		}

		uint32_t 
		mapwidth()
		{
			return map_->width();
		}

	private:
		warthog::weighted_gridmap* map_;
		warthog::blocklist* nodepool_;

		// 3x3 square of tiles + one terminal element
		warthog::search_node* neis_[10]; 
		warthog::cost_t costs_[10];
		uint32_t which_; // current neighbour
		uint32_t num_neis_; // might have < 9 (some could be blocked)
};

}

#endif

