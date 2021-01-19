#include "blocklist.h"
#include "gridmap_expansion_policy.h"
#include "helpers.h"
#include "problem_instance.h"

warthog::gridmap_expansion_policy::gridmap_expansion_policy(
		warthog::gridmap* map) : map_(map)
{
	nodepool_ = new warthog::blocklist(map->height(), map->width());
}

warthog::gridmap_expansion_policy::~gridmap_expansion_policy()
{
	delete nodepool_;
}

void 
warthog::gridmap_expansion_policy::expand(warthog::search_node* current,
		warthog::problem_instance* problem)
{
	reset();

	// get terrain type of each tile in the 3x3 square around (x, y)
	uint32_t tiles = 0;
	uint32_t nodeid = current->get_id();
	map_->get_neighbours(nodeid, (uint8_t*)&tiles);

//	#ifndef NDEBUG
//	uint32_t cx_, cy_;
//	warthog::helpers::index_to_xy(nodeid, map_->width(), cx_, cy_);
//	assert(tiles[0] == map_->get_label(cx_-1, cy_-1));
//	assert(tiles[1] == map_->get_label(cx_, cy_-1));
//	assert(tiles[2] == map_->get_label(cx_+1, cy_-1));
//	assert(tiles[3] == map_->get_label(cx_-1, cy_));
//	assert(tiles[4] == map_->get_label(cx_, cy_));
//	assert(tiles[5] == map_->get_label(cx_+1, cy_));
//	assert(tiles[6] == map_->get_label(cx_-1, cy_+1));
//	assert(tiles[7] == map_->get_label(cx_, cy_+1));
//	assert(tiles[8] == map_->get_label(cx_+1, cy_+1));
//	#endif

	// NB: no corner cutting or squeezing between obstacles!
	uint32_t nid_m_w = nodeid - map_->width();
	uint32_t nid_p_w = nodeid + map_->width();

	if((tiles & 514) == 514) // N
	{  
		neis_[num_neis_] = nodepool_->generate(nid_m_w);
		costs_[num_neis_] = warthog::ONE;
		num_neis_++;
	} 

	if((tiles & 1542) == 1542) // NE
	{ 
		neis_[num_neis_] = nodepool_->generate(nid_m_w + 1);
		costs_[num_neis_] = warthog::ROOT_TWO;
		num_neis_++;
	}

	if((tiles & 1536) == 1536) // E
	{
		neis_[num_neis_] = nodepool_->generate(nodeid + 1);
		costs_[num_neis_] = warthog::ONE;
		num_neis_++;
	}
	
	if((tiles & 394752) == 394752) // SE
	{	
		neis_[num_neis_] = nodepool_->generate(nid_p_w + 1);
		costs_[num_neis_] = warthog::ROOT_TWO;
		num_neis_++;
	}

	if((tiles & 131584) == 131584) // S
	{ 
		neis_[num_neis_] = nodepool_->generate(nid_p_w);
		costs_[num_neis_] = warthog::ONE;
		num_neis_++;
	}

	if((tiles & 197376) == 197376) // SW
	{ 
		neis_[num_neis_] = nodepool_->generate(nid_p_w - 1);
		costs_[num_neis_] = warthog::ROOT_TWO;
		num_neis_++;
	}

	if((tiles & 768) == 768) // W
	{ 
		neis_[num_neis_] = nodepool_->generate(nodeid - 1);
		costs_[num_neis_] = warthog::ONE;
		num_neis_++;
	}

	if((tiles & 771) == 771) // NW
	{ 
		neis_[num_neis_] = nodepool_->generate(nid_m_w - 1);
		costs_[num_neis_] = warthog::ROOT_TWO;
		num_neis_++;
	}
}

