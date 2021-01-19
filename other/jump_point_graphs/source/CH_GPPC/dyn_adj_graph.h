#ifndef DYN_ADJ_GRAPH_H
#define DYN_ADJ_GRAPH_H

#include "binary_search.h"
#include <type_traits>
#include <cassert>
#include <vector>
#include <algorithm>
#include "arc_gppc.h"
#include "range_gppc.h"

namespace ch_gppc {
template<class T_ArcHead>
class DynAdjGraph{
public:
	typedef T_ArcHead ArcHead;
	typedef typename type_of_arc<ArcHead>::type Arc;
	
	DynAdjGraph():num_arcs(0){}

	template<class ArcRange>
	DynAdjGraph(int node_count, const ArcRange&arc_range):
		data(node_count),
		num_arcs(range_size(arc_range)){
		std::vector<int>out_deg(node_count);
		for(auto x:arc_range)
			++out_deg[x.source];

		for(int i=0; i<node_count; ++i)
			data[i].reserve(2*out_deg[i]);

		for(auto x:arc_range)
			data[x.source].push_back(get_head(x));
		
		for(int i=0; i<node_count; ++i)
			std::sort(data[i].begin(), data[i].end(), 
				[](ArcHead l, ArcHead r){return l.target < r.target;}
			);
	}

	int node_count()const{
		return data.size();
	}
	
	int arc_count()const{
		return num_arcs;
	}

	int out_degree(int v)const{
		return data[v].size();
	}

	template<class Combiner>
	void add_or_combine(Arc a, const Combiner&combine){
		assert(0 <= a.source && a.source < node_count() && "source is out of bounds");
		assert(0 <= a.target && a.target < node_count() && "target is out of bounds");
		auto pos = binary_find_first_true(
			data[a.source].begin(), data[a.source].end(), 
			[&](ArcHead x){return x.target >= a.target;}
		);
		
		if(pos == data[a.source].end()){
			data[a.source].push_back(get_head(a));
			++num_arcs;
		}else if(pos->target != a.target){
			data[a.source].insert(pos, get_head(a));
			++num_arcs;
		}else
			*pos = get_head(combine(make_arc(a.source, *pos), a));

		assert(
			std::is_sorted(
				data[a.source].begin(), data[a.source].end(), 
				[](ArcHead l, ArcHead r){
					return l.target < r.target;
				}
			)
		);
	}

	bool remove_arc(int source_node, int target_node){
		auto 
			end = data[source_node].end(),
			pos = binary_find_first_true(
				data[source_node].begin(), end, 
				[&](ArcHead x){return x.target >= target_node;}
			);
		if(pos != end && pos->target == target_node){
			data[source_node].erase(pos);
			--num_arcs;
			return true;
		}else
			return false;
	}

	Range<typename std::vector<ArcHead>::const_iterator> out(int v)const{
		assert(0 <= v && v < node_count() && "v is out of bounds");
		return make_range(data[v].begin(), data[v].end());
	}

	Range<typename std::vector<ArcHead>::iterator> out(int v){
		assert(0 <= v && v < node_count() && "v is out of bounds");
		return make_range(data[v].begin(), data[v].end());
	}

	std::vector<ArcHead>remove_out_arcs(int v){
		assert(0 <= v && v < node_count() && "v is out of bounds");
		std::vector<ArcHead>ret;
		ret.swap(data[v]);
		num_arcs -= ret.size();
		return std::move(ret);
	}

private:
	std::vector<std::vector<ArcHead>>data;
	int num_arcs;
};

template<class ArcList>
auto make_dyn_adj_graph(int node_count, const ArcList&arc_list)
	->DynAdjGraph<decltype(get_head(arc_list[0]))>{
	return {node_count, arc_list};
}
}
#endif

