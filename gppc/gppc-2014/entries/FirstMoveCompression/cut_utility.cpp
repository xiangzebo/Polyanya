#include "cut_utility.h"
#include <cassert>
#include <vector>
#include <algorithm>

// compile with -O3 -DNDEBUG

std::vector<bool>remove_isolated_nodes_from_cut(const ListGraph&g, std::vector<bool>is_in_cut){
	assert(g.is_valid());
	assert(g.node_count() == (int)is_in_cut.size());

	std::vector<int>is_isolated(g.node_count());

	#ifndef NDEBUG
	std::fill(is_isolated.begin(), is_isolated.end(), true);
	for(auto&a:g.arc){
		is_isolated[a.source] = false;
		is_isolated[a.target] = false;
	}
	for(int i=0; i<g.node_count(); ++i)
		assert(!is_isolated[i] && "degree 0 node is always isolated...");
	#endif

	auto update_isolated_flag = [&](){
		std::fill(is_isolated.begin(), is_isolated.end(), true);
		for(auto&a:g.arc){
			if(is_in_cut[a.source] == is_in_cut[a.target]){
				is_isolated[a.source] = false;
				is_isolated[a.target] = false;
			}
		}
	};

	update_isolated_flag();

	for(int i=0; i<g.node_count(); ++i)
		if(is_isolated[i] && is_in_cut[i])
			is_in_cut[i] = false;

	update_isolated_flag();

	for(int i=0; i<g.node_count(); ++i)
		if(is_isolated[i] && !is_in_cut[i])
			is_in_cut[i] = true;

	#ifndef NDEBUG
	update_isolated_flag();
	for(int i=0; i<g.node_count(); ++i)
		assert(!is_isolated[i]);
	#endif

	return std::move(is_in_cut);
}

