#include "jps2_expansion_policy.h"

warthog::jps2_expansion_policy::jps2_expansion_policy(warthog::gridmap* map)
{
	map_ = map;
	nodepool_ = new warthog::blocklist(map->height(), map->width());
	jpl_ = new warthog::online_jump_point_locator2(map);
	reset();

	neighbours_.reserve(100);
	costs_.reserve(100);
	jp_ids_.reserve(100);
}

warthog::jps2_expansion_policy::~jps2_expansion_policy()
{
	delete jpl_;
	delete nodepool_;
}

void 
warthog::jps2_expansion_policy::expand(
		warthog::search_node* current, warthog::problem_instance* problem)
{
	reset();

	// compute the direction of travel used to reach the current node.
	warthog::jps::direction dir_c = current->get_pdir();

	// get the tiles around the current node c
	uint32_t c_tiles;
	uint32_t current_id = current->get_id();
	map_->get_neighbours(current_id, (uint8_t*)&c_tiles);

	// look for jump points in the direction of each natural 
	// and forced neighbour
	uint32_t succ_dirs = warthog::jps::compute_successors(dir_c, c_tiles);
	uint32_t goal_id = problem->get_goal();

	for(uint32_t i = 0; i < 8; i++)
	{
		warthog::jps::direction d = (warthog::jps::direction) (1 << i);
		if(succ_dirs & d)
		{
			jpl_->jump(d, current_id, goal_id, jp_ids_, costs_);
		}
	}

	uint32_t searchid = problem->get_searchid();
	uint32_t id_mask = (1 << 24)-1;
	for(uint32_t i = 0; i < jp_ids_.size(); i++)
	{
		// bits 0-23 store the id of the jump point
		// bits 24-31 store the direction to the parent
		uint32_t jp_id = jp_ids_.at(i);
		warthog::jps::direction pdir = (warthog::jps::direction)*(((uint8_t*)(&jp_id))+3);

		warthog::search_node* mynode = nodepool_->generate(jp_id & id_mask);
		neighbours_.push_back(mynode);
		if(mynode->get_searchid() != searchid) { mynode->reset(searchid); }

		// stupid hack
		if((current->get_g() + costs_.at(i)) < mynode->get_g())
		{
			mynode->set_pdir(pdir);
		}
	}
	if(problem->get_start() == current_id && neighbours_.size() == 0)
	{
		std::cerr << "stop plz.." << std::endl;
	}
	num_neighbours_ = neighbours_.size();

	// terminator (historical; yeah, this code is stupid)
	neighbours_.push_back(0);
	costs_.push_back(0);
}

