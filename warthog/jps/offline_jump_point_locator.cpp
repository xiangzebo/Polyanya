#define __STDC_FORMAT_MACROS
#include "gridmap.h"
#include "online_jump_point_locator.h"
#include "offline_jump_point_locator.h"

#include <cstring>
#include <inttypes.h>
#include <stdio.h>

warthog::offline_jump_point_locator::offline_jump_point_locator(
		warthog::gridmap* map) : map_(map)
{
	preproc();
}

warthog::offline_jump_point_locator::~offline_jump_point_locator()
{
	delete [] db_;
}

void
warthog::offline_jump_point_locator::preproc()
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
warthog::offline_jump_point_locator::load(const char* filename)
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
warthog::offline_jump_point_locator::save(const char* filename)
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
warthog::offline_jump_point_locator::jump(warthog::jps::direction d, 
		uint32_t node_id, uint32_t goal_id, uint32_t& jumpnode_id, 
		warthog::cost_t& jumpcost)
{
	current_ = max_ = 0;
	switch(d)
	{
		case warthog::jps::NORTH:
			jump_north(node_id, goal_id, jumpnode_id, jumpcost);
			break;
		case warthog::jps::SOUTH:
			jump_south(node_id, goal_id, jumpnode_id, jumpcost);
			break;
		case warthog::jps::EAST:
			jump_east(node_id, goal_id, jumpnode_id, jumpcost);
			break;
		case warthog::jps::WEST:
			jump_west(node_id, goal_id, jumpnode_id, jumpcost);
			break;
		case warthog::jps::NORTHEAST:
			jump_northeast(node_id, goal_id, jumpnode_id, jumpcost);
			break;
		case warthog::jps::NORTHWEST:
			jump_northwest(node_id, goal_id, jumpnode_id, jumpcost);
			break;
		case warthog::jps::SOUTHEAST:
			jump_southeast(node_id, goal_id, jumpnode_id, jumpcost);
			break;
		case warthog::jps::SOUTHWEST:
			jump_southwest(node_id, goal_id, jumpnode_id, jumpcost);
			break;
		default:
			break;
	}
}

void
warthog::offline_jump_point_locator::jump_northwest(uint32_t node_id,
	  	uint32_t goal_id, uint32_t& jumpnode_id, warthog::cost_t& jumpcost)
{
	uint32_t mapw = map_->width();
	uint16_t label = db_[8*node_id + 5];
	uint16_t num_steps = label & 32767;

	// goal test (so many div ops! and branches! how ugly!)
	uint32_t id_delta = (mapw + 1) * num_steps;
	if((node_id - goal_id) < map_->padded_mapsize()) // heading toward the goal?
	{
		uint32_t gx, gy, nx, ny;
		map_->to_padded_xy(goal_id, gx, gy);
		map_->to_padded_xy(node_id, nx, ny);

		uint32_t ydelta = (ny - gy);
		uint32_t xdelta = (nx - gx); 
		if(xdelta < mapw && ydelta < map_->height())
		{
			jumpnode_id = warthog::INF;
			uint32_t nid, steps_to_nid;
			if(ydelta < xdelta && ydelta <= num_steps)
			{
				steps_to_nid = ydelta;
				nid = node_id - (mapw + 1) * steps_to_nid;
				jump_west(nid, goal_id, jumpnode_id, jumpcost);
				if(jumpnode_id == goal_id)
				{
					jumpnode_id = goal_id;
					jumpcost = steps_to_nid * warthog::ROOT_TWO + jumpcost;
					return;
				}
			}
			else if(xdelta <= num_steps)
			{
				steps_to_nid = xdelta;
				nid = node_id - (mapw + 1) * steps_to_nid;
				jump_north(nid, goal_id, jumpnode_id, jumpcost);
				if(jumpnode_id == goal_id)
				{
					jumpnode_id = goal_id;
					jumpcost = steps_to_nid * warthog::ROOT_TWO + jumpcost;
					return;
				}
			}
		}
	}

	// return the jump point; but only if it isn't sterile
	jumpnode_id = node_id - id_delta;
	jumpcost = num_steps * warthog::ROOT_TWO;
	if(label & 32768) { jumpnode_id = warthog::INF; }
}

void
warthog::offline_jump_point_locator::jump_northeast(uint32_t node_id,
	  	uint32_t goal_id, uint32_t& jumpnode_id, warthog::cost_t& jumpcost)
{
	uint16_t label = db_[8*node_id + 4];
	uint16_t num_steps = label & 32767;
	uint32_t mapw = map_->width();

	// goal test (so many div ops! and branches! how ugly!)
	uint32_t id_delta = (mapw - 1) * num_steps;
	if((node_id - goal_id) < map_->padded_mapsize())
	{
		uint32_t gx, gy, nx, ny;
		map_->to_padded_xy(goal_id, gx, gy);
		map_->to_padded_xy(node_id, nx, ny);

		uint32_t ydelta = (ny - gy);
		uint32_t xdelta = (gx - nx); 
		if(xdelta < mapw && ydelta < map_->height())
		{
			jumpnode_id = warthog::INF;
			uint32_t nid, steps_to_nid;
			if(ydelta < xdelta && ydelta <= num_steps)
			{
				steps_to_nid = ydelta;
				nid = node_id - (mapw - 1) * steps_to_nid;
				jump_east(nid, goal_id, jumpnode_id, jumpcost);
				if(jumpnode_id == goal_id)
				{
					jumpnode_id = goal_id;
					jumpcost = steps_to_nid * warthog::ROOT_TWO + jumpcost;
					return;
				}
			}
			else if(xdelta <= num_steps)
			{
				steps_to_nid = xdelta;
				nid = node_id - (mapw - 1) * steps_to_nid;
				jump_north(nid, goal_id, jumpnode_id, jumpcost);
				if(jumpnode_id == goal_id)
				{
					jumpnode_id = goal_id;
					jumpcost = steps_to_nid * warthog::ROOT_TWO + jumpcost;
					return;
				}
			}
		}
	}

	// return the jump point; but only if it isn't sterile
	jumpnode_id = node_id - id_delta;
	jumpcost = num_steps * warthog::ROOT_TWO;
	if(label & 32768) { jumpnode_id = warthog::INF; }
}

void
warthog::offline_jump_point_locator::jump_southwest(uint32_t node_id,
	  	uint32_t goal_id, uint32_t& jumpnode_id, warthog::cost_t& jumpcost)
{
	uint16_t label = db_[8*node_id + 7];
	uint16_t num_steps = label & 32767;
	uint32_t mapw = map_->width();


	// goal test (so many div ops! and branches! how ugly!)
	if((goal_id - node_id) < map_->padded_mapsize())
	{
		uint32_t gx, gy, nx, ny;
		map_->to_padded_xy(goal_id, gx, gy);
		map_->to_padded_xy(node_id, nx, ny);

		uint32_t ydelta = (gy - ny);
		uint32_t xdelta = (nx - gx); 
		if(xdelta < mapw && ydelta < map_->height())
		{
			jumpnode_id = warthog::INF;
			uint32_t nid, steps_to_nid;
			if(ydelta < xdelta && ydelta <= num_steps)
			{
				steps_to_nid = ydelta;
				nid = node_id + (mapw - 1) * steps_to_nid;
				jump_west(nid, goal_id, jumpnode_id, jumpcost);
				if(jumpnode_id == goal_id)
				{ 
					jumpnode_id = goal_id;
					jumpcost = steps_to_nid * warthog::ROOT_TWO + jumpcost;
					return; 
				}
			}
			else if(xdelta <= num_steps)
			{
				steps_to_nid = xdelta;
				nid = node_id + (mapw - 1) * steps_to_nid;
				jump_south(nid, goal_id, jumpnode_id, jumpcost);
				if(jumpnode_id == goal_id)
				{ 
					jumpnode_id = goal_id;
					jumpcost = steps_to_nid * warthog::ROOT_TWO + jumpcost;
					return; 
				}
			}
		}

	}

	// return the jump point; but only if it isn't sterile
	jumpnode_id = node_id + (mapw - 1) * num_steps;
	jumpcost = num_steps * warthog::ROOT_TWO;
	if(label & 32768) { jumpnode_id = warthog::INF; }
}

void
warthog::offline_jump_point_locator::jump_southeast(uint32_t node_id,
	  	uint32_t goal_id, uint32_t& jumpnode_id, warthog::cost_t& jumpcost)
{
	uint16_t label = db_[8*node_id + 6];
	uint16_t num_steps = label & 32767;
	uint32_t mapw = map_->width();
	
	// goal test (so many div ops! and branches! how ugly!)
	if((goal_id - node_id) < map_->padded_mapsize())
	{
		uint32_t gx, gy, nx, ny;
		map_->to_padded_xy(goal_id, gx, gy);
		map_->to_padded_xy(node_id, nx, ny);

		uint32_t ydelta = (gy - ny);
		uint32_t xdelta = (gx - nx); 
		if(xdelta < mapw && ydelta < map_->height())
		{
			uint32_t nid, steps_to_nid;
			jumpnode_id = warthog::INF;
			if(ydelta < xdelta && ydelta <= num_steps)
			{
				steps_to_nid = ydelta;
				nid = node_id + (mapw + 1) * steps_to_nid;
				jump_east(nid, goal_id, jumpnode_id, jumpcost);
				if(jumpnode_id == goal_id)
				{ 
					jumpnode_id = goal_id;
					jumpcost = steps_to_nid * warthog::ROOT_TWO + jumpcost;
					return; 
				}
			}
			else if(xdelta <= num_steps)
			{
				steps_to_nid = xdelta;
				nid = node_id + (mapw + 1) * steps_to_nid;
				jump_south(nid, goal_id, jumpnode_id, jumpcost);
				if(jumpnode_id == goal_id)
				{ 
					jumpnode_id = goal_id;
					jumpcost = steps_to_nid * warthog::ROOT_TWO + jumpcost;
					return; 
				}
			}
		}
	}


	jumpnode_id = node_id + (mapw + 1) * num_steps;
	jumpcost = num_steps * warthog::ROOT_TWO;
	if(label & 32768) { jumpnode_id = warthog::INF; }
}

void
warthog::offline_jump_point_locator::jump_north(uint32_t node_id,
	  	uint32_t goal_id, uint32_t& jumpnode_id, warthog::cost_t& jumpcost)
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
			jumpnode_id = goal_id; 
			jumpcost = (goal_delta / map_->width()) * warthog::ONE;
			return;
		}
	}

	// return the jump point at hand
	jumpnode_id = node_id - id_delta;
	jumpcost = num_steps * warthog::ONE;
	if(label & 32768) { jumpnode_id = warthog::INF; }
}

void
warthog::offline_jump_point_locator::jump_south(uint32_t node_id,
	  	uint32_t goal_id, uint32_t& jumpnode_id, warthog::cost_t& jumpcost)
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
			jumpnode_id = goal_id; 
			jumpcost = goal_delta / map_->width() * warthog::ONE;
			return;
		}
	}

	// return the jump point at hand
	jumpnode_id = node_id + id_delta;
	jumpcost = num_steps * warthog::ONE;
 	if(label & 32768) { jumpnode_id = warthog::INF; }
}

void
warthog::offline_jump_point_locator::jump_east(uint32_t node_id,
	  	uint32_t goal_id, uint32_t& jumpnode_id, warthog::cost_t& jumpcost)
{
	uint16_t label = db_[8*node_id + 2];

	// do not jump over the goal
	uint32_t id_delta = label & 32767;
	uint32_t goal_delta = goal_id - node_id;
	if(id_delta >= goal_delta)
	{
		jumpnode_id = goal_id;
		jumpcost = goal_delta * warthog::ONE;
		return;
	}

	// return the jump point at hand
	jumpnode_id = node_id + id_delta;
	jumpcost = id_delta * warthog::ONE;
	if(label & 32768) { jumpnode_id = warthog::INF; }
}

void
warthog::offline_jump_point_locator::jump_west(uint32_t node_id,
	  	uint32_t goal_id, uint32_t& jumpnode_id, warthog::cost_t& jumpcost)
{
	uint16_t label = db_[8*node_id + 3];

	// do not jump over the goal
	uint32_t id_delta = label & 32767;
	uint32_t goal_delta = node_id - goal_id;
	if(id_delta >= goal_delta)
	{
		jumpnode_id = goal_id;
		jumpcost = goal_delta * warthog::ONE;
		return;
	}

	// return the jump point at hand
	jumpnode_id = node_id - id_delta;
	jumpcost = id_delta * warthog::ONE;
	if(label & 32768) { jumpnode_id = warthog::INF; }
}

