#define __STDC_FORMAT_MACROS
#include "gridmap.h"
#include "online_jump_point_locator.h"
#include "offline_jump_point_locator2.h"

#include <assert.h>
#include <cstring>
#include <inttypes.h>
#include <stdio.h>

warthog::offline_jump_point_locator2::offline_jump_point_locator2(
		warthog::gridmap* map) : map_(map)
{
	if(map_->padded_mapsize() > ((1 << 23)-1)) 
	{
		// search nodes are stored as 32bit quantities.
		// bit 0 stores the expansion status
		// bits 1-23 store the unique node id
		// bits 24-31 store the direction from the parent
		std::cerr << "map size too big for this implementation of JPS+."
			<< " aborting."<< std::endl;
		exit(1);
	}
	preproc();
}

warthog::offline_jump_point_locator2::~offline_jump_point_locator2()
{
	delete [] db_;
}

void
warthog::offline_jump_point_locator2::preproc()
{
	if(load(map_->filename())) { return; }

	dbsize_ = 8*map_->padded_mapsize();
	db_ = new uint16_t[dbsize_];
	for(uint32_t i=0; i < dbsize_; i++) db_[i] = 0;

	warthog::online_jump_point_locator jpl(map_);
	for(uint32_t y = 0; y < map_->header_height(); y++)
	{
		for(uint32_t x = 0; x < map_->header_width(); x++)
		{
			uint32_t mapid = map_->to_padded_id(x, y);
//			std::cout << mapid << " ";
			for(int i = 0; i < 8; i++)
			{
				warthog::jps::direction dir = 
					(warthog::jps::direction)(1 << i);
//				std::cout << dir << ": ";
				uint32_t jumpnode_id;
				warthog::cost_t jumpcost;
				jpl.jump(dir, mapid,
						warthog::INF, jumpnode_id, jumpcost);
				
				// convert from cost to number of steps
				if(dir > 8)
				{
					jumpcost = (jumpcost / warthog::ROOT_TWO);
				}
				else
				{
					jumpcost = jumpcost / warthog::ONE;
				}
				uint32_t num_steps = (uint16_t)floor((jumpcost + 0.5));
//				std::cout << (jumpnode_id == warthog::INF ? 0 : num_steps) << " ";

				// set the leading bit if the jump leads to a dead-end
				if(jumpnode_id == warthog::INF)
				{
					db_[mapid*8 + i] |= 32768;
				}

				// truncate jump cost so we can fit the label into a single byte
				//if(num_steps > 32767)
				//{
				//	num_steps = 32767;
				//	jumpnode_id = 0; 
				//}

				db_[mapid*8 + i] |= num_steps;

				if(num_steps > 32768)
				{
					std::cerr << "label overflow; maximum jump distance exceeded. aborting\n";
					exit(1);
				}
			}
//			std::cout << std::endl;
		}
	}

	save(map_->filename());
}


bool
warthog::offline_jump_point_locator2::load(const char* filename)
{
	char fname[256];
	strcpy(fname, filename);
	strcat(fname, ".jps+");
	FILE* f = fopen(fname, "rb");
	std::cerr << "loading "<<fname << "... ";
	if(f == NULL) 
	{
		std::cerr << "no dice. oh well. keep going.\n"<<std::endl;
		return false;
	}
	
	fread(&dbsize_, sizeof(dbsize_), 1, f);
	std::cerr <<"#labels="<<dbsize_<<std::endl;

	db_ = new uint16_t[dbsize_];
	fread(db_, sizeof(uint16_t), dbsize_, f);
	fclose(f);
	return true;
}

void 
warthog::offline_jump_point_locator2::save(const char* filename)
{
	char fname[256];
	strcpy(fname, filename);
	strcat(fname, ".jps+");
	std::cerr << "saving to file "<<fname<<"; nodes="<<dbsize_<<" size: "<<sizeof(db_[0])<<std::endl;

	FILE* f = fopen(fname, "wb");
	if(f == NULL) 
	{
		std::cerr << "err; cannot write jump-point graph to file "
			<<fname<<". oh well. try to keep going.\n"<<std::endl;
		return;
	}

	fwrite(&dbsize_, sizeof(dbsize_), 1, f);
	fwrite(db_, sizeof(*db_), dbsize_, f);
	fclose(f);
	std::cerr << "jump-point graph saved to disk. file="<<fname<<std::endl;
}

void
warthog::offline_jump_point_locator2::jump(warthog::jps::direction d, 
		uint32_t node_id, uint32_t goal_id, 
		std::vector<uint32_t>& neighbours, std::vector<warthog::cost_t>& costs)
{
	switch(d)
	{
		case warthog::jps::NORTH:
			jump_north(node_id, goal_id, 0, neighbours, costs);
			break;
		case warthog::jps::SOUTH:
			jump_south(node_id, goal_id, 0, neighbours, costs);
			break;
		case warthog::jps::EAST:
			jump_east(node_id, goal_id, 0, neighbours, costs);
			break;
		case warthog::jps::WEST:
			jump_west(node_id, goal_id, 0, neighbours, costs);
			break;
		case warthog::jps::NORTHEAST:
			jump_northeast(node_id, goal_id, neighbours, costs);
			break;
		case warthog::jps::NORTHWEST:
			jump_northwest(node_id, goal_id, neighbours, costs);
			break;
		case warthog::jps::SOUTHEAST:
			jump_southeast(node_id, goal_id, neighbours, costs);
			break;
		case warthog::jps::SOUTHWEST:
			jump_southwest(node_id, goal_id, neighbours, costs);
			break;
		default:
			break;
	}
}

void
warthog::offline_jump_point_locator2::jump_northwest(uint32_t node_id,
	  	uint32_t goal_id,
		std::vector<uint32_t>& neighbours, std::vector<warthog::cost_t>& costs)

{
	uint16_t label = 0;
	uint16_t num_steps = 0;
	uint32_t mapw = map_->width();
	uint32_t diag_step_delta = (mapw + 1);
	
	// keep jumping until we hit a dead-end. take note of the
	// points reachable by a vertical or horizontal jump from
	// each intermediate location that we reach diagonally.
	uint32_t jump_from = node_id;
	
	// step diagonally to an intermediate location jump_from
	label = db_[8*jump_from + 5];
	num_steps += label & 32767;
	jump_from = node_id - num_steps * diag_step_delta;
	while(!(label & 32768))
	{
		// north of jump_from
		uint16_t label_straight1 = db_[8*jump_from]; 
		if(!(label_straight1 & 32768)) 
		{ 
			uint32_t jp_cost = (label_straight1 & 32767);
			uint32_t jp_id = jump_from - mapw *  jp_cost;
			*(((uint8_t*)&jp_id)+3) = warthog::jps::NORTH;
			neighbours.push_back(jp_id);
			costs.push_back(jp_cost * warthog::ONE + num_steps * warthog::ROOT_TWO);
		}
		// west of jump_from
		uint16_t label_straight2 = db_[8*jump_from+3]; // west of next jp
		if(!(label_straight2 & 32768)) 
		{ 
			uint32_t jp_cost = (label_straight2 & 32767);
			uint32_t jp_id = jump_from - jp_cost;
			*(((uint8_t*)&jp_id)+3) = warthog::jps::WEST;
			neighbours.push_back(jp_id);
			costs.push_back(jp_cost * warthog::ONE + num_steps * warthog::ROOT_TWO);
		}
		label = db_[8*jump_from + 5];
		num_steps += label & 32767;
		jump_from = node_id - num_steps * diag_step_delta;
	}

	// goal test (so many div ops! and branches! how ugly!)
	if((node_id - goal_id) < map_->padded_mapsize()) // heading toward the goal?
	{
		uint32_t gx, gy, nx, ny;
		map_->to_padded_xy(goal_id, gx, gy);
		map_->to_padded_xy(node_id, nx, ny);

		uint32_t ydelta = (ny - gy);
		uint32_t xdelta = (nx - gx); 
		if(xdelta < mapw && ydelta < map_->height())
		{
			if(ydelta < xdelta && ydelta <= num_steps)
			{
				uint32_t jp_id = node_id - diag_step_delta * ydelta;
				warthog::cost_t jp_cost = warthog::ROOT_TWO * ydelta;
				jump_west(jp_id, goal_id, jp_cost, neighbours, costs);
			}
			else if(xdelta <= num_steps)
			{
				uint32_t jp_id = node_id - diag_step_delta * xdelta;
				warthog::cost_t jp_cost = warthog::ROOT_TWO * xdelta;
				jump_north(jp_id, goal_id, jp_cost, neighbours, costs);
			}
		}
	}
}

void
warthog::offline_jump_point_locator2::jump_northeast(uint32_t node_id,
	  	uint32_t goal_id, 
		std::vector<uint32_t>& neighbours, std::vector<warthog::cost_t>& costs)
{
	uint16_t label = 0;
	uint16_t num_steps = 0;
	uint32_t mapw = map_->width();
	uint32_t diag_step_delta = (mapw - 1);
	
	uint32_t jump_from = node_id;
	// step diagonally to an intermediate location jump_from
	label = db_[8*jump_from + 4];
	num_steps += label & 32767;
	jump_from = node_id - num_steps * diag_step_delta;
	while(!(label & 32768))
	{

		// north of jump_from
		uint16_t label_straight1 = db_[8*jump_from]; 
		if(!(label_straight1 & 32768)) 
		{ 
			uint32_t jp_cost = (label_straight1 & 32767);
			uint32_t jp_id = jump_from - mapw *  jp_cost;
			*(((uint8_t*)&jp_id)+3) = warthog::jps::NORTH;
			neighbours.push_back(jp_id);
			costs.push_back(jp_cost * warthog::ONE + num_steps * warthog::ROOT_TWO);
		}
		// east of jump_from
		uint16_t label_straight2 = db_[8*jump_from+2]; 
		if(!(label_straight2 & 32768)) 
		{ 
			uint32_t jp_cost = (label_straight2 & 32767);
			uint32_t jp_id = jump_from + jp_cost;
			*(((uint8_t*)&jp_id)+3) = warthog::jps::EAST;
			neighbours.push_back(jp_id);
			costs.push_back(jp_cost * warthog::ONE + num_steps * warthog::ROOT_TWO);
		}
		label = db_[8*jump_from + 4];
		num_steps += label & 32767;
		jump_from = node_id - num_steps * diag_step_delta;
	}

	// goal test (so many div ops! and branches! how ugly!)
	if((node_id - goal_id) < map_->padded_mapsize()) // heading toward the goal?
	{
		uint32_t gx, gy, nx, ny;
		map_->to_padded_xy(goal_id, gx, gy);
		map_->to_padded_xy(node_id, nx, ny);

		uint32_t ydelta = (ny - gy);
		uint32_t xdelta = (gx - nx); 
		if(xdelta < mapw && ydelta < map_->height())
		{
			if(ydelta < xdelta && ydelta <= num_steps)
			{
				uint32_t jp_id = node_id - diag_step_delta * ydelta;
				warthog::cost_t jp_cost = warthog::ROOT_TWO * ydelta;
				jump_east(jp_id, goal_id, jp_cost, neighbours, costs);
			}
			else if(xdelta <= num_steps)
			{
				uint32_t jp_id = node_id - diag_step_delta * xdelta;
				warthog::cost_t jp_cost = warthog::ROOT_TWO * xdelta;
				jump_north(jp_id, goal_id, jp_cost, neighbours, costs);
			}
		}
	}
}

void
warthog::offline_jump_point_locator2::jump_southwest(uint32_t node_id,
	  	uint32_t goal_id, 
		std::vector<uint32_t>& neighbours, std::vector<warthog::cost_t>& costs)
{
	uint32_t mapw = map_->width();
	uint32_t diag_step_delta = (mapw - 1);
	uint16_t label, num_steps;
	label = num_steps = 0;

	uint32_t jump_from = node_id;
	// step diagonally to an intermediate location jump_from
	label = db_[8*jump_from + 7];
	num_steps += label & 32767;
	jump_from = node_id + num_steps * diag_step_delta;
	while(!(label & 32768))
	{
		// south of jump_from
		uint16_t label_straight1 = db_[8*jump_from+1]; 
		if(!(label_straight1 & 32768)) 
		{ 
			uint32_t jp_cost = (label_straight1 & 32767);
			uint32_t jp_id = jump_from + mapw *  jp_cost;
			*(((uint8_t*)&jp_id)+3) = warthog::jps::SOUTH;
			neighbours.push_back(jp_id);
			costs.push_back(jp_cost * warthog::ONE + num_steps * warthog::ROOT_TWO);
		}
		// west of jump_from
		uint16_t label_straight2 = db_[8*jump_from+3]; 
		if(!(label_straight2 & 32768)) 
		{ 
			uint32_t jp_cost = (label_straight2 & 32767);
			uint32_t jp_id = jump_from - jp_cost;
			*(((uint8_t*)&jp_id)+3) = warthog::jps::WEST;
			neighbours.push_back(jp_id);
			costs.push_back(jp_cost * warthog::ONE + num_steps * warthog::ROOT_TWO);
		}
		label = db_[8*jump_from + 7];
		num_steps += label & 32767;
		jump_from = node_id + num_steps * diag_step_delta;
	}

	// goal test (so many div ops! and branches! how ugly!)
	if((goal_id - node_id) < map_->padded_mapsize()) // heading toward the goal?
	{
		uint32_t gx, gy, nx, ny;
		map_->to_padded_xy(goal_id, gx, gy);
		map_->to_padded_xy(node_id, nx, ny);

		uint32_t ydelta = (gy - ny);
		uint32_t xdelta = (nx - gx); 
		if(xdelta < mapw && ydelta < map_->height())
		{
			if(ydelta < xdelta && ydelta <= num_steps)
			{
				uint32_t jp_id = node_id + diag_step_delta * ydelta;
				warthog::cost_t jp_cost = warthog::ROOT_TWO * ydelta;
				jump_west(jp_id, goal_id, jp_cost, neighbours, costs);
			}
			else if(xdelta <= num_steps)
			{
				uint32_t jp_id = node_id + diag_step_delta * xdelta;
				warthog::cost_t jp_cost = warthog::ROOT_TWO * xdelta;
				jump_south(jp_id, goal_id, jp_cost, neighbours, costs);
			}
		}
	}
}

void
warthog::offline_jump_point_locator2::jump_southeast(uint32_t node_id,
	  	uint32_t goal_id, 
		std::vector<uint32_t>& neighbours, std::vector<warthog::cost_t>& costs)
	
{
	uint16_t label = 0;
	uint16_t num_steps = 0;
	uint32_t mapw = map_->width();
	uint32_t diag_step_delta = (mapw + 1);
	
	uint32_t jump_from = node_id;
	
	// step diagonally to an intermediate location jump_from
	label = db_[8*jump_from + 6];
	num_steps += label & 32767;
	jump_from = node_id + num_steps * diag_step_delta;
	while(!(label & 32768))
	{
		// south of jump_from
		uint16_t label_straight1 = db_[8*jump_from + 1]; 
		if(!(label_straight1 & 32768)) 
		{ 
			uint32_t jp_cost = (label_straight1 & 32767);
			uint32_t jp_id = jump_from + mapw * jp_cost;
			*(((uint8_t*)&jp_id)+3) = warthog::jps::SOUTH;
			neighbours.push_back(jp_id);
			costs.push_back(jp_cost * warthog::ONE + num_steps * warthog::ROOT_TWO);
		}
		// east of jump_from
		uint16_t label_straight2 = db_[8*jump_from + 2]; 
		if(!(label_straight2 & 32768)) 
		{ 
			uint32_t jp_cost = (label_straight2 & 32767);
			uint32_t jp_id = jump_from + jp_cost;
			*(((uint8_t*)&jp_id)+3) = warthog::jps::EAST;
			neighbours.push_back(jp_id);
			costs.push_back(jp_cost * warthog::ONE + num_steps * warthog::ROOT_TWO);
		}
		// step diagonally to an intermediate location jump_from
		label = db_[8*jump_from + 6];
		num_steps += label & 32767;
		jump_from = node_id + num_steps * diag_step_delta;
	}

	// goal test (so many div ops! and branches! how ugly!)
	if((goal_id - node_id) < map_->padded_mapsize()) // heading toward the goal?
	{
		uint32_t gx, gy, nx, ny;
		map_->to_padded_xy(goal_id, gx, gy);
		map_->to_padded_xy(node_id, nx, ny);

		uint32_t ydelta = (gy - ny);
		uint32_t xdelta = (gx - nx); 
		if(xdelta < mapw && ydelta < map_->height())
		{
			if(ydelta < xdelta && ydelta <= num_steps)
			{
				uint32_t jp_id = node_id + diag_step_delta * ydelta;
				warthog::cost_t jp_cost = warthog::ROOT_TWO * ydelta;
				jump_east(jp_id, goal_id, jp_cost, neighbours, costs);
			}
			else if(xdelta <= num_steps)
			{
				uint32_t jp_id = node_id + diag_step_delta * xdelta;
				warthog::cost_t jp_cost = warthog::ROOT_TWO * xdelta;
				jump_south(jp_id, goal_id, jp_cost, neighbours, costs);
			}
		}
	}
}

void
warthog::offline_jump_point_locator2::jump_north(uint32_t node_id,
	  	uint32_t goal_id, warthog::cost_t cost_to_node_id,
		std::vector<uint32_t>& neighbours, std::vector<warthog::cost_t>& costs)
{
	uint16_t label = db_[8*node_id];
	uint16_t num_steps = label & 32767;

	// do not jump over the goal
	uint32_t id_delta = num_steps * map_->width();
	uint32_t goal_delta = node_id - goal_id;
	if(id_delta >= goal_delta)
	{
		uint32_t gx = goal_id % map_->width();
		uint32_t nx = node_id % map_->width();
		if(nx == gx) 
		{ 
			*(((uint8_t*)&goal_id)+3) = warthog::jps::NORTH;
			neighbours.push_back(goal_id);
			costs.push_back((goal_delta / map_->width() * warthog::ONE) + cost_to_node_id);
			return;
		}
	}

	// return the jump point at hand (if it isn't sterile)
	if(!(label & 32768)) 
	{ 
		uint32_t jp_id = node_id - id_delta;
		*(((uint8_t*)&jp_id)+3) = warthog::jps::NORTH;
		neighbours.push_back(jp_id);
		costs.push_back(num_steps * warthog::ONE + cost_to_node_id);
	}
}

void
warthog::offline_jump_point_locator2::jump_south(uint32_t node_id,
	  	uint32_t goal_id, warthog::cost_t cost_to_node_id, 
		std::vector<uint32_t>& neighbours, std::vector<warthog::cost_t>& costs)
{
	uint16_t label = db_[8*node_id + 1];
	uint16_t num_steps = label & 32767;
	
	// do not jump over the goal
	uint32_t id_delta = num_steps * map_->width();
	uint32_t goal_delta = goal_id - node_id;
	if(id_delta >= goal_delta)
	{
		uint32_t gx = goal_id % map_->width();
		uint32_t nx = node_id % map_->width();
		if(nx == gx) 
		{ 
			*(((uint8_t*)&goal_id)+3) = warthog::jps::SOUTH;
			neighbours.push_back(goal_id);
			costs.push_back((goal_delta / map_->width() * warthog::ONE) + cost_to_node_id);
			return;
		}
	}

	// return the jump point at hand (if it isn't sterile)
	if(!(label & 32768))
	{
		uint32_t jp_id = (node_id + id_delta);
		*(((uint8_t*)&jp_id)+3) = warthog::jps::SOUTH;
		neighbours.push_back(jp_id);
		costs.push_back(num_steps * warthog::ONE + cost_to_node_id);
	}
}

void
warthog::offline_jump_point_locator2::jump_east(uint32_t node_id,
	  	uint32_t goal_id, warthog::cost_t cost_to_node_id,
		std::vector<uint32_t>& neighbours, std::vector<warthog::cost_t>& costs)
{
	uint16_t label = db_[8*node_id + 2];
	uint32_t num_steps = label & 32767;

	// do not jump over the goal
	uint32_t goal_delta = goal_id - node_id;
	if(num_steps >= goal_delta)
	{
		*(((uint8_t*)&goal_id)+3) = warthog::jps::EAST;
		neighbours.push_back(goal_id);
		costs.push_back(goal_delta * warthog::ONE + cost_to_node_id);
		return;
	}

	// return the jump point at hand
	if(!(label & 32768))
	{
		uint32_t jp_id = (node_id + num_steps);
		*(((uint8_t*)&jp_id)+3) = warthog::jps::EAST;
		neighbours.push_back(jp_id);
		costs.push_back(num_steps * warthog::ONE + cost_to_node_id);
	}
}

void
warthog::offline_jump_point_locator2::jump_west(uint32_t node_id,
	  	uint32_t goal_id, warthog::cost_t cost_to_node_id,
		std::vector<uint32_t>& neighbours, std::vector<warthog::cost_t>& costs)
{
	uint16_t label = db_[8*node_id + 3];
	uint32_t num_steps = label & 32767;

	// do not jump over the goal
	uint32_t goal_delta = node_id - goal_id;
	if(num_steps >= goal_delta)
	{
		*(((uint8_t*)&goal_id)+3) = warthog::jps::WEST;
		neighbours.push_back(goal_id);
		costs.push_back(goal_delta * warthog::ONE + cost_to_node_id);
		return;
	}

	// return the jump point at hand
	if(!(label & 32768))
	{
		uint32_t jp_id = node_id - num_steps;
		*(((uint8_t*)&jp_id)+3) = warthog::jps::WEST;
		neighbours.push_back(jp_id);
		costs.push_back(num_steps * warthog::ONE + cost_to_node_id);
	}
}

