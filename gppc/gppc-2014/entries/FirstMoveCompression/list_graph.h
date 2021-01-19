#ifndef LIST_GRAPH_H
#define LIST_GRAPH_H
#include <vector>
#include <algorithm>
#include <string>
#include <cassert>
#include "range.h"

struct Arc{
	int source, target, weight;
};

inline bool operator==(Arc l, Arc r){
	return l.source == r.source && l.target == r.target;
}

inline bool operator!=(Arc l, Arc r){
	return !(l == r);
}



struct ListGraph{
	ListGraph():n(0){}
	explicit ListGraph(int node_count):n(node_count){}

	std::vector<Arc>arc;

	int node_count()const{
		return n;
	}

	bool is_valid()const{
		bool ok = true;
		for(auto a:arc){
			ok &= (a.source != -1);
			ok &= (a.source >= 0);
			ok &= (a.source < node_count());
			ok &= (a.target != -1);
			ok &= (a.target >= 0);
			ok &= (a.target < node_count());
		}
		return ok;
	}

	int n;
};

inline bool operator==(const ListGraph&l, const ListGraph&r){
	return l.arc == r.arc && l.n == r.n;
}

inline bool operator!=(const ListGraph&l, const ListGraph&r){
	return !(l == r);
}

template<class IsNodeInSubgraph>
ListGraph extract_node_induced_subgraph(
	const ListGraph&g,
	const std::vector<int>&g_to_top_level,
	const IsNodeInSubgraph&is_in_s, 
	std::vector<int>&g_to_s, //! Maps an ID from g onto the id in the subgraph or -1
	std::vector<int>&s_to_top_level
){
	assert(g.is_valid());
	assert(g.node_count() == (int)g_to_top_level.size());
	int s_node_count = 0;
	for(int i=0; i<g.node_count(); ++i)
		if(is_in_s(i))
			++s_node_count;

	ListGraph s(s_node_count);
	s_to_top_level.resize(s_node_count);

	g_to_s.resize(g.node_count());

	int n = 0;
	for(int i=0; i<g.node_count(); ++i)
		if(is_in_s(i)){
			s_to_top_level[n] = g_to_top_level[i];
			g_to_s[i] = n;
			++n;
		}else
			g_to_s[i] = -1;

	for(auto a:g.arc)
		if(g_to_s[a.source] != -1 && g_to_s[a.target] != -1)
			s.arc.push_back({g_to_s[a.source], g_to_s[a.target]});

	assert(s.is_valid());
	return std::move(s);
}

template<class IsNodeInSubgraph>
ListGraph extract_node_induced_subgraph(const ListGraph&g, const IsNodeInSubgraph&is_in_s){
	std::vector<int>ignore1(g.node_count()), ignore2, ignore3;
	return extract_node_induced_subgraph(g, ignore1, is_in_s, ignore2, ignore3);
}

template<class IsNodeInSubgraph>
ListGraph extract_node_induced_subgraph(const ListGraph&g, const std::vector<int>&g_to_top_level, const IsNodeInSubgraph&is_in_s, std::vector<int>&s_to_top_level){
	std::vector<int>ignore;
	return extract_node_induced_subgraph(g, g_to_top_level, is_in_s, ignore, s_to_top_level);
}

#endif
