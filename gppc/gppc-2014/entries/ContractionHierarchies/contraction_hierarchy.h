#ifndef CONTRACTION_HIERARCHY_H
#define CONTRACTION_HIERARCHY_H

#include "timestamp_flags.h"
#include "dyn_adj_graph.h"
#include "bidirectional_dijkstra.h"
#include "range.h"
#include <vector>
#include <limits>
#include <cstdio>

//! A function object to test whether only paths exist with a distance longer than
//! a given value (or no path at all).
template<class AdjGraph>
class DoOnlyLongerPathsExists{
private:
	mutable BidirectionalDijkstra<TimestampFlags, AdjGraph> dij;
	int max_step_count;
public:
	DoOnlyLongerPathsExists(const AdjGraph&forward, const AdjGraph&backward, int max_step_count):
		dij(forward, backward),
		max_step_count(max_step_count){
		assert(max_step_count >= 2);
	}

	template<class GetWeight>
	bool operator()(int source, int target, int distance, const GetWeight&get_weight)const{
		if(source == target)
			return false;

		dij.start(source, target);

		int step_count = 0;
		dij.settle_until_certain(
			get_weight,
			[&](long long best_distance, long long forward_radius, long long backward_radius){
				if(best_distance <= distance)
					return true;

				if(best_distance <= forward_radius + backward_radius)
					return true;

				++step_count;
				if(step_count == max_step_count)
					return true;
				return false;
			}
		);		

		bool result = dij.get_current_shortest_path_distance() > distance;
		#ifndef NDEBUG
		// direct arc must be found
		if(result){
			for(auto a:dij.forward.graph.out(source))
				if(a.target == target){
					assert(get_weight(make_arc(source, a)) > distance);
				}
		}
		#endif

		return result;
	}
};

template<class AdjGraph>
auto make_do_only_longer_paths_exist_test(const AdjGraph&forward, const AdjGraph&backward, int max_step_count)
	->DoOnlyLongerPathsExists<AdjGraph>{
	return {forward, backward, max_step_count};
}

template<class ArcHead>
class ContractionCore{
public:
	typedef typename type_of_arc<ArcHead>::type Arc;

	DynAdjGraph<ArcHead> forward_graph, backward_graph;

	template<class ArcList>
	ContractionCore(int node_count, const ArcList&arc_list):
		forward_graph(node_count, arc_list),
		backward_graph(node_count, map_range(arc_list, [](Arc a){return reverse_arc(a);}))
	{}

	template<
		class IsShortcutNeeded, 
		class SequentielArcCombine,
		class ParallelArcCombine,
		class GetWeight
	>
	void contract_node(
		int v, 
		const IsShortcutNeeded&is_shortcut_needed,
		const SequentielArcCombine&seq_arc_combine,
		const ParallelArcCombine&para_arc_combine,
		const GetWeight&get_weight
	){
		auto forward_heads = forward_graph.remove_out_arcs(v);
		auto backward_heads = backward_graph.remove_out_arcs(v);
		 
		for(auto b:backward_heads)
			forward_graph.remove_arc(b.target, v);
		for(auto f:forward_heads)
			backward_graph.remove_arc(f.target, v);

		std::vector<Arc>potential_shortcut;

		for(auto b:backward_heads)
			for(auto f:forward_heads)
				if(f.target != b.target)
					potential_shortcut.push_back(
						seq_arc_combine(
							reverse_arc(make_arc(v, b)),
							make_arc(v, f)
						)
					);

		std::sort(
			std::begin(potential_shortcut), std::end(potential_shortcut), 
			[&](const Arc&l, const Arc&r){
				return get_weight(l) < get_weight(r);
			}
		);

		for(auto shortcut : potential_shortcut)
			if(is_shortcut_needed(shortcut)){
				forward_graph.add_or_combine(shortcut, para_arc_combine);
				backward_graph.add_or_combine(reverse_arc(shortcut), para_arc_combine);
			}

	}
};

template<class ArcList>
auto make_contraction_core(int node_count, const ArcList&arc_list)
	->ContractionCore<typename type_of_arc_head<typename type_of_range_element<ArcList>::type>::type>{
	return {node_count, arc_list};
}



struct ContractionArc{
	int source, target, weight, mid, hop_length;
};

struct ContractionArcHead{
	int target, weight, mid, hop_length;
};

inline
ContractionArcHead get_head(ContractionArc a){
	return {a.target, a.weight, a.mid, a.hop_length};
}

inline
ContractionArc make_arc(int source, ContractionArcHead h){
	return {source, h.target, h.weight, h.mid, h.hop_length};
}

template<class ArcList, class OnNewForwardArc, class OnNewBackwardArc>
std::vector<int>compute_online_contraction_order(
	int node_count,
	const ArcList&arc_list,
	int max_step_count,
	const OnNewForwardArc&on_forward_dag,
	const OnNewBackwardArc&on_backward_dag
){
	auto core = make_contraction_core(
		node_count, map_range(
			arc_list, 
			[](WeightedArc a){
				return ContractionArc{a.source, a.target, a.weight, -1, 1};
			}
		)
	);

	auto do_only_longer_path_exist = make_do_only_longer_paths_exist_test(
		core.forward_graph, core.backward_graph,
		max_step_count
	);

	
	std::vector<int>level(node_count, 0);

	std::vector<int>removed_neighbours(node_count, 0);
	
	auto estimate_importance = [&](int v){
		int in_deg = core.backward_graph.out_degree(v);
		int out_deg = core.forward_graph.out_degree(v);

		if(in_deg <= 1 || out_deg <= 1)
			return -2;

		if(in_deg == 2 && out_deg == 2){
			auto x = core.forward_graph.out(v);
			int a = std::begin(x)->target;
			int b = std::next(std::begin(x))->target;
			auto y = core.backward_graph.out(v);
			int c = std::begin(y)->target;
			int d = std::next(std::begin(y))->target;
			if((a == c || a == d) && (b == c || b == d))
				return -1;
		}

		int arcs_removed = 0;
		int hops_removed = 0;

		for(auto b:core.backward_graph.out(v)){
			++arcs_removed;
			hops_removed += b.hop_length;
		}

		for(auto f:core.forward_graph.out(v)){
			++arcs_removed;
			hops_removed += f.hop_length;
		}

		int shortcuts_added = 0;
		int hops_added = 0;
		for(auto b:core.backward_graph.out(v))
			for(auto f:core.forward_graph.out(v)){

				if(
					do_only_longer_path_exist(
						b.target,
						f.target,
						b.weight + f.weight,
						[&](ContractionArc a){
							if(a.target == v || a.source == v)
								return std::numeric_limits<int>::max();
							else
								return a.weight;
						}
					)
				){
					hops_added += b.hop_length;
					hops_added += f.hop_length;
				}
			}


		if(arcs_removed == 0  || hops_removed == 0)
			return std::numeric_limits<int>::max();
		else{
			const int fractal_part = 1024;
			return fractal_part*shortcuts_added / arcs_removed + fractal_part*hops_added / hops_removed + fractal_part*level[v];
		}
	};

	std::vector<bool>was_contracted(node_count, false);
	
	auto contract = [&](int v){
		for(auto b:core.backward_graph.out(v)){
			level[b.target] = std::max(level[b.target], level[v]+1);
			++removed_neighbours[b.target];
			on_backward_dag(WeightedArc{v, b.target, b.weight}, b.mid);
		}

		for(auto f:core.forward_graph.out(v)){
			level[f.target] = std::max(level[f.target], level[v]+1);
			++removed_neighbours[f.target];
			on_forward_dag(WeightedArc{v, f.target, f.weight}, f.mid);
		}

		core.contract_node(v,
			[&](ContractionArc a){
				bool ret = do_only_longer_path_exist(
					a.source,
					a.target,
					a.weight,
					[](ContractionArc a){return a.weight;}
				);
				return ret;
			},
			[=](ContractionArc l, ContractionArc r){
				return ContractionArc{
					l.source, 
					r.target, 
					l.weight + r.weight, 
					v,
					l.hop_length + r.hop_length
				};
			},
			[](ContractionArc l, ContractionArc r){
				if(l.weight < r.weight)
					return l;
				else
					return r;
			},
			[](ContractionArc a){
				return a.weight;
			}
		);

		was_contracted[v] = true;
		assert(core.forward_graph.arc_count() == core.backward_graph.arc_count());
	};

	std::vector<int>order(node_count);
	min_id_heap<int>queue(node_count);
	
	std::printf("Building Queue ... \n");

	queue.fill(
		map_range(
			count_range(node_count), 
			[&](int x){
				return std::make_pair(x, estimate_importance(x));
			}
		)
	);

	
	std::vector<int>neighbours;
	std::vector<bool>neighbour_found(node_count, false);

	std::printf("Contracting Nodes ... \n");
	for(int i=0; i<node_count; ++i){

		order[i] = queue.pop();

		neighbours.clear();
		for(auto a:core.backward_graph.out(order[i])){
			neighbours.push_back(a.target);
			neighbour_found[a.target] = true;
		}
		for(auto a:core.forward_graph.out(order[i])){
			if(!neighbour_found[a.target]){
				neighbours.push_back(a.target);
				neighbour_found[a.target] = true;
			}
		}

		contract(order[i]);
		
		for(auto u:neighbours){
			queue.push_or_set_key(u, estimate_importance(u));
			neighbour_found[u] = false;
		}
	}

	return std::move(order);
}

#endif

