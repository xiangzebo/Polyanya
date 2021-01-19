#include <deque>
#include <vector>
#include <algorithm>
#include <assert.h>
#include "Entry.h"
#include "adj_graph.h"

#include <cstdio>
#include "contraction_hierarchy.h"
#include "mapper.h"
#include "vec_io.h"
using namespace std;


const char *GetName()
{
	return "Basic-Contraction-Hierarchy";
}

struct UnpackTriple{
	int source, target, mid;
};



struct ShortcutArc{
	int source, target, weight, mid;
};

struct ShortcutArcHead{
	int target, weight, mid;
};

inline ShortcutArcHead get_head(ShortcutArc a){
	return {a.target, a.weight, a.mid};
}

inline ShortcutArc make_arc(int source, ShortcutArcHead head){
	return {source, head.target, head.weight, head.mid};
}

#define USE_STALL_ON_DEMAND
//#define USE_NODE_REORDER

void PreprocessMap(std::vector<bool> &bits, int width, int height, const char *filename)
{
	Mapper mapper(bits, width, height);
	printf("width = %d, height = %d, node_count = %d\n", width, height, mapper.node_count());

	auto arc_list = extract_graph(mapper);
	const int node_count = mapper.node_count();

	std::vector<ShortcutArc>search_arc_list;

	auto order = compute_online_contraction_order(
		node_count, arc_list, 100000000,
		[&](WeightedArc a, int m){
			search_arc_list.push_back({a.source, a.target, a.weight, m});
		}, 
		[&](WeightedArc, int){}
	);

	#ifdef USE_NODE_REORDER
	auto rank = inverse_permutation(order);

	for(auto&x:search_arc_list){
		x.source = rank[x.source];
		x.target = rank[x.target];
		if(x.mid != -1)
			x.mid = rank[x.mid];
	}
	#endif

	AdjGraph<ShortcutArcHead> search_graph(node_count, search_arc_list);

	printf("#input arcs = %d, #search graph arcs = %d\n", (int)arc_list.size(), (int)search_arc_list.size());

	printf("Saving data to %s\n", filename);
	FILE*f = fopen(filename, "wb");
	search_graph.save(f);
	#ifdef USE_NODE_REORDER
	save_vector(f, order);
	#endif
	fclose(f);

	printf("Done\n");

}

struct State{
	Mapper mapper;
	AdjGraph<ShortcutArcHead>search_graph;
	BidirectionalDijkstra<TimestampFlags, AdjGraph<ShortcutArcHead>, AdjGraph<ShortcutArcHead>>*bidij;
};

void *PrepareForSearch(std::vector<bool> &bits, int w, int h, const char *filename)
{
	printf("Loading preprocessing data\n");
	State*state = new State;

	state->mapper = Mapper(bits, w, h);

	FILE*f = fopen(filename, "rb");
	state->search_graph.load(f);
	#ifdef USE_NODE_REORDER
	auto order = load_vector<int>(f);
	#endif
	fclose(f);

	#ifdef USE_NODE_REORDER
	state->mapper.reorder(order, inverse_permutation(order));
	#endif
	state->bidij = new BidirectionalDijkstra<TimestampFlags, AdjGraph<ShortcutArcHead>, AdjGraph<ShortcutArcHead>>(state->search_graph, state->search_graph);

	printf("input arc count = %d, search graph arc count = %d\n", (int)extract_graph(state->mapper).size(), (int)state->search_graph.arc_count());
	printf("Loading done\n");
	return state;
}


bool GetPath(void *data, xyLoc s, xyLoc t, std::vector<xyLoc> &path)
{
	State*state = static_cast<State*>(data);
	(void)state;
	
	int source_node = state->mapper(s);
	int target_node = state->mapper(t);

	auto get_forward_weight = [&](ShortcutArc a){
		return a.weight;
	};

	auto get_backward_weight = [&](ShortcutArc a){
		return a.weight;
	};

	#ifndef USE_STALL_ON_DEMAND	
	auto should_forward_search_continue = [&](int v){
		return true;
	};

	auto should_backward_search_continue = [&](int v){
		return true;
	};
	#else
	auto should_forward_search_continue = [&](int v){
		for(auto a:state->search_graph.out(v)){
			if(state->bidij->forward.extract_distance(a.target) < state->bidij->forward.extract_distance(v) - a.weight){
				return false;
			}
		}
		return true;	
	};
	auto should_backward_search_continue = [&](int v){
		for(auto a:state->search_graph.out(v)){
			if(state->bidij->backward.extract_distance(a.target) < state->bidij->backward.extract_distance(v) - a.weight){
				return false;
			}
		}
		return true;	
	};
	#endif

	state->bidij->start(source_node, target_node);
	while(
		   state->bidij->get_current_shortest_path_distance() > state->bidij->forward.get_current_radius()
		|| state->bidij->get_current_shortest_path_distance() > state->bidij->backward.get_current_radius() 
	){
		state->bidij->settle_next(get_forward_weight, get_backward_weight, should_forward_search_continue, should_backward_search_continue);
	}

	auto up_down_path = state->bidij->get_current_shortest_path();

	if(up_down_path.empty())
		return true;

	auto get_mid = [&](int x, int y){
		#ifdef USE_NODE_REORDER
		if(y < x)
			std::swap(x, y);
		for(auto h:state->search_graph.out(x))
			if(h.target == y)
				return h.mid;
		#else
		for(auto h:state->search_graph.out(x))
			if(h.target == y)
				return h.mid;
		for(auto h:state->search_graph.out(y))
			if(h.target == x)
				return h.mid;
		#endif
		return -1;
	};

	while(up_down_path.size() != 1){
		int last = up_down_path[up_down_path.size()-1];
		int second_last = up_down_path[up_down_path.size()-2];
		auto mid = get_mid(second_last, last);
		if(mid == -1){
			up_down_path.pop_back();
			path.push_back(state->mapper(last));
		}else{
			up_down_path.back() = mid;
			up_down_path.push_back(last);
		}
	}

	path.push_back(s);

	std::reverse(path.begin(), path.end());


	return true;
}

