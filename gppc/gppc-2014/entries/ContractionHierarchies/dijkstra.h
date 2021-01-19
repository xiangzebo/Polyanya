#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "heap.h"
#include <vector>
#include <limits>
#include <type_traits>
#include <cassert>




//! Generic implementation of dijkstra's algorithm. 
//!
//! Use the free make_dijkstra function to create an object. For example:
//!    auto dij = make_dijkstra<BoolFlags>(adj_graph);
//! 
//! The adj_graph is not copied but referenced. The BoolFlags parameter is obligatory
//! and indicates which type of flags to use for the visited flags. Typical choices
//! are BoolFlags and TimestampFlags depending on whether clear should be efficient
//! or not.
//!
//! GetWeight is a functor that maps an arc onto its weight. It can take five forms:
//!    get_weight(a)
//!    get_weight(t, a)
//!    get_weight(a, id)
//!    get_weight(t, a, id)
//!    get_weight(id)
//! The functor you pass must support exactly one of these five forms. 
//! a is the arc object. t is the tentative distance at a.source. Further id 
//! is the position within the underlying AdjGraph.
//!
//! You can prune an arc by letting get_weight return std::numeric_limits<int>::max();
//!
//! Further there exists ShouldExpandNode which takes the form:
//!    should_expand_node(v)
//! It is called after removing v from the queue but before relaxing all outgoing arcs.
//! The arcs are only relaxed if it returns true. Omitting should_expand_node results
//! in all nodes being expanded.
template<class Flags, class AdjGraph>
class Dijkstra{
private:
	min_id_heap<int>queue;
	std::vector<int>tentative_distance;
	std::vector<int>predecessor;
	Flags visited_flags;
public:
	const AdjGraph&graph;
	
	typedef typename AdjGraph::Arc Arc;
	typedef typename AdjGraph::ArcHead ArcHead;

	Dijkstra(const AdjGraph&graph):
		queue(graph.node_count()),
		tentative_distance(graph.node_count()), 
		predecessor(graph.node_count()),
		visited_flags(graph.node_count()),
		graph(graph){
	}

	void clear(){
		queue.clear();
		visited_flags.unset_all();
	}

	void add_source_node(int source_node){
		assert(0 <= source_node && source_node < graph.node_count() && "source_node is out of bounds");

		queue.push_or_decrease_key(source_node, 0);
		visited_flags.set(source_node);
		tentative_distance[source_node] = 0;
		predecessor[source_node] = source_node;
	}

	//! Settles one node. Must not be called if the algorithm has finished.
	//! Returns the settled node.
	template<class GetWeight, class ShouldExpandNode>
	int settle_next(const GetWeight&get_weight, const ShouldExpandNode&should_expand_node){
		assert(!is_finished());

		int x = queue.pop();
		if(should_expand_node(x))
			settle(x, get_weight);
		return x;
	}

	template<class GetWeight>
	int settle_next(const GetWeight&get_weight){
		return settle_next(get_weight, [](int){return true;});
	}

	bool is_finished()const{
		return queue.empty();
	}

	//! Extracts the distance of a shortest path from the source node to the target node.
	//! \note  If there is no path then the distance is std::numeric_limits<int>::max().
	int extract_distance(int target_node)const{
		assert(0 <= target_node && target_node < graph.node_count() && "target_node is out of bounds");
		if(visited_flags.is_set(target_node))
			return tentative_distance[target_node];
		else
			return std::numeric_limits<int>::max();
	}

	//! Checks whether the tentative value of target_node is certainly correct.
	bool is_certain(int target_node)const{
		assert(0 <= target_node && target_node < graph.node_count() && "target_node is out of bounds");
		return queue.empty() || extract_distance(target_node) <= queue.peek_min_key();
	}

	//! Checks whether a (possibly suboptimal) path to target_node was found. 
	bool path_to_exists(int target_node)const{
		return visited_flags.is_set(target_node);
	}

	//! Extracts a list of nodes in a shortest path from the source node to the target node.
	//! \note If there is no path then the list is empty.
	std::vector<int>extract_path(int target_node)const{
		assert(0 <= target_node && target_node < graph.node_count() && "target_node is out of bounds");
		if(!path_to_exists(target_node))
			return {};
		else{
			std::vector<int>path = {target_node};
			while(predecessor[target_node] != target_node){
				target_node = predecessor[target_node];
				path.push_back(target_node);
			}
			std::reverse(path.begin(), path.end());
			return std::move(path);
		}
	}

	int extract_predecessor(int v)const{
		return predecessor[v];
	}

	int extract_path_hop_length(int target_node)const{
		assert(0 <= target_node && target_node < graph.node_count() && "target_node is out of bounds");
		if(!path_to_exists(target_node))
			return std::numeric_limits<int>::max();
		else{
			int length = 0;
			while(predecessor[target_node] != target_node){
				target_node = predecessor[target_node];
				++length;
			}
			return length;
		}
	}
	
	//! Returns the radius of the current radius. Formally this is the largest number s such that all shortest paths with a length strictly below s have been found.
	int get_current_radius()const{
		if(queue.empty())
			return std::numeric_limits<int>::max();
		else
			return queue.peek_min_key();
	}

private:
	#define MAKE_EVALUATE_WEIGHT(allowed_parameter_list)\
		template<class GetWeight>\
		auto evaluate_weight(const GetWeight&get_weight, int t, Arc a, int id =-1)\
			->decltype(get_weight allowed_parameter_list){return get_weight allowed_parameter_list ;}\

	MAKE_EVALUATE_WEIGHT((a))
	MAKE_EVALUATE_WEIGHT((t, a))
	MAKE_EVALUATE_WEIGHT((t, a, id))
	MAKE_EVALUATE_WEIGHT((a, id))
	MAKE_EVALUATE_WEIGHT((id))

	#undef MAKE_EVALUATE_WEIGHT

	template<class GetWeight>
	std::false_type does_get_weight_need_an_arc_id(const GetWeight&get_weight, decltype(get_weight(std::declval<Arc>()))*dummy=0){return {};}

	template<class GetWeight>
	std::false_type does_get_weight_need_an_arc_id(const GetWeight&get_weight, decltype(get_weight(0, std::declval<Arc>()))*dummy=0){return {};}

	template<class GetWeight>
	std::true_type does_get_weight_need_an_arc_id(const GetWeight&get_weight, decltype(get_weight(0, std::declval<Arc>(), 0))*dummy=0){return {};}

	template<class GetWeight>
	std::true_type does_get_weight_need_an_arc_id(const GetWeight&get_weight, decltype(get_weight(std::declval<Arc>(), 0))*dummy=0){return {};}

	template<class GetWeight>
	std::true_type does_get_weight_need_an_arc_id(const GetWeight&get_weight, decltype(get_weight(0))*dummy=0){return {};}


	void relax(int source, int target, int weight){
		assert(weight >= 0 && "weight must be non-negative for Dijkstra's algorithm to work");
		int target_tentative_distance = tentative_distance[target];
		if(!visited_flags.is_set(target))
			target_tentative_distance = std::numeric_limits<int>::max();

		// -weight instead of +weight because of optential overflows
		if(tentative_distance[source] < target_tentative_distance - weight){
			visited_flags.set(target);
			tentative_distance[target] = tentative_distance[source] + weight;
			predecessor[target] = source;
			queue.push_or_decrease_key(target, tentative_distance[target]);
		}
	}

	template<class GetWeight> 	
	void settle(int v, const GetWeight&get_weight){
		settle(v, get_weight, does_get_weight_need_an_arc_id(get_weight));
	}
	
	template<class GetWeight> 	
	void settle(int v, const GetWeight&get_weight, std::true_type){
		for(auto arc_id : graph.arc_id_out(v))
			relax(v, graph.head(arc_id).target, evaluate_weight(get_weight, tentative_distance[v], make_arc(v, graph.head(arc_id)), arc_id));
	}

	template<class GetWeight> 	
	void settle(int v, const GetWeight&get_weight, std::false_type){
		for(auto x:graph.out(v))
			relax(v, x.target, evaluate_weight(get_weight, tentative_distance[v], make_arc(v, x)));
	}
};

template<class Flags, class AdjGraph>
Dijkstra<Flags, AdjGraph>make_dijkstra(const AdjGraph&graph){
	return Dijkstra<Flags, AdjGraph>(graph);
}

#endif

