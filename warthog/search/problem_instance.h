#ifndef WARTHOG_PROBLEM_INSTANCE_H
#define WARTHOG_PROBLEM_INSTANCE_H

#include "search_node.h"

namespace warthog
{

class problem_instance
{
	public:
		problem_instance() :  goal_(0), start_(0)  { }
		~problem_instance() { } 

		inline void
		set_goal(uint32_t goal_id) { goal_ = goal_id; }

		inline uint32_t
		get_goal() { return goal_; }

		inline uint32_t
		get_start() { return start_; }

		inline void
		set_start(uint32_t start_id) { start_ = start_id; }

		inline uint32_t
		get_searchid() { return search_id_; } 

		inline void
		set_searchid(uint32_t search_id) { search_id_ = search_id; }

	private:
		uint32_t goal_;
		uint32_t start_;
		uint32_t search_id_;

		// no copy
		problem_instance(const warthog::problem_instance& other) { }
		warthog::problem_instance& 
		operator=(const warthog::problem_instance& other) { return *this; } 
};

}

#endif

