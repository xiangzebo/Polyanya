#include "helpers.h"
#include "problem_instance.h"
#include "wgridmap_expansion_policy.h"

warthog::wgridmap_expansion_policy::wgridmap_expansion_policy(
		warthog::weighted_gridmap* map) : map_(map)
{
	nodepool_ = new warthog::blocklist(map->height(), map->width());
}

warthog::wgridmap_expansion_policy::~wgridmap_expansion_policy()
{
	delete nodepool_;
}

void 
warthog::wgridmap_expansion_policy::expand(warthog::search_node* current,
		warthog::problem_instance* problem)
{
	reset();

	// get terrain type of each tile in the 3x3 square around (x, y)
    uint32_t tile_ids[9];
    warthog::dbword tiles[9];
	uint32_t nodeid = current->get_id();
	map_->get_neighbours(nodeid, tile_ids, tiles);

    // NB: 
    // 1. neighbours generated in clockwise order starting from direction N.
    // 2. transition costs to each neighbour are calculated as the average of 
    // terrain values for all tiles touched by the agent when moving.
    // CAVEAT EMPTOR: precision loss from integer maths.
    if(tiles[1]) // N neighbour
    {
		neis_[num_neis_] = nodepool_->generate(tile_ids[1]);
		costs_[num_neis_] = ((tiles[1] + tiles[4]) * warthog::ONE) >> 1; 
		num_neis_++;
    }
    if(tiles[1] & tiles[2] & tiles[5]) // NE neighbour
    {
		neis_[num_neis_] = nodepool_->generate(tile_ids[2]);
		costs_[num_neis_] = ((tiles[1] + tiles[2] + tiles[4] + tiles[5]) * 
                warthog::ROOT_TWO) >> 2; 
		num_neis_++;
    }
    if(tiles[5]) // E
    {
		neis_[num_neis_] = nodepool_->generate(tile_ids[5]);
		costs_[num_neis_] = ((tiles[5] + tiles[4]) * warthog::ONE) >> 1; 
		num_neis_++;
    }
    if(tiles[5] & tiles[7] & tiles[8]) // SE
    {
		neis_[num_neis_] = nodepool_->generate(tile_ids[8]);
		costs_[num_neis_] = ((tiles[4] + tiles[5] + tiles[7] + tiles[8]) * 
                warthog::ROOT_TWO) >> 2; 
		num_neis_++;
    }
    if(tiles[7]) // S
    {
		neis_[num_neis_] = nodepool_->generate(tile_ids[7]);
		costs_[num_neis_] = ((tiles[7] + tiles[4]) * warthog::ONE) >> 1; 
		num_neis_++;
    }
    if(tiles[3] & tiles[6] & tiles[7]) // SW
    {
		neis_[num_neis_] = nodepool_->generate(tile_ids[6]);
		costs_[num_neis_] = ((tiles[3] + tiles[4] + tiles[6] + tiles[7]) * 
                warthog::ROOT_TWO) >> 2; 
		num_neis_++;
    }
    if(tiles[3]) // W
    {
		neis_[num_neis_] = nodepool_->generate(tile_ids[3]);
		costs_[num_neis_] = ((tiles[3] + tiles[4]) * warthog::ONE) >> 1; 
		num_neis_++;
    }
    if(tiles[0] & tiles[1] & tiles[3]) // NW neighbour
    {
		neis_[num_neis_] = nodepool_->generate(tile_ids[0]);
		costs_[num_neis_] = ((tiles[0] + tiles[1] + tiles[3] + tiles[4]) * 
                warthog::ROOT_TWO) >> 2; 
		num_neis_++;
    }
}

