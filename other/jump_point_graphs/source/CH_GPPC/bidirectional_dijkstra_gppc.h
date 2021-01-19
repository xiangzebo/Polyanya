#ifndef BIDIRECTIONAL_DIJKSTRA_H
#define BIDIRECTIONAL_DIJKSTRA_H

#include <limits>
#include <type_traits>
#include <cassert>
#include "dijkstra_gppc.h"

namespace ch_gppc {


template<class Flags, class ForwardAdjGraph, class BackwardAdjGraph = ForwardAdjGraph>
class BidirectionalDijkstraGPPC{
private:
	int best_middle_node;
	int best_distance;
	int num_expansions;
public:
	DijkstraGPPC<Flags, ForwardAdjGraph>forward;
	DijkstraGPPC<Flags, BackwardAdjGraph>backward;

	BidirectionalDijkstraGPPC(const ForwardAdjGraph&forward, const BackwardAdjGraph&backward):
		forward(forward),
		backward(backward){
		assert(forward.node_count() == backward.node_count());
	}

	int GetNumExpansions() {
	  return num_expansions;
	}

	//! Clears the datastructures and sets the source node.
	void start(int source_node, int target_node){
		assert(0 <= source_node && source_node < forward.graph.node_count() && "source_node is out of bounds");
		assert(0 <= target_node && target_node < backward.graph.node_count() && "target_node is out of bounds");

		forward.clear();
		forward.add_source_node(source_node);
		backward.clear();
		backward.add_source_node(target_node);

		if(source_node == target_node){
			best_distance = 0;
			best_middle_node = source_node;
		}else
			best_distance = std::numeric_limits<int>::max();

		num_expansions = 0;
	}

	//! Settles one node. Must not be called if the algorithm has finished.
	//! Returns the settled node.
	template<class GetForwardWeight, class GetBackwardWeight, class ShouldExpandForwardNode, class ShouldExpandBackwardNode>
	int settle_next(
		const GetForwardWeight&get_forward_weight, const GetBackwardWeight&get_backward_weight,
		const ShouldExpandForwardNode&should_expand_forward_node, const ShouldExpandBackwardNode&should_expand_backward_node
	){
		int x;
		bool was_path_found = false;
		num_expansions++;

		assert(!forward.is_finished() || !backward.is_finished());
			
		bool is_forward_next = forward.get_current_radius() < backward.get_current_radius(); 

		if(forward.is_finished())
			is_forward_next = false;
		else if(backward.is_finished())
			is_forward_next = true;

		if(is_forward_next){
			x = forward.settle_next(get_forward_weight, should_expand_forward_node);
			was_path_found = backward.path_to_exists(x);
		}else{
			x = backward.settle_next(get_backward_weight, should_expand_backward_node);
			was_path_found = forward.path_to_exists(x);
		}

		if(was_path_found){
			assert(forward.extract_distance(x) != std::numeric_limits<int>::max());
			assert(backward.extract_distance(x) != std::numeric_limits<int>::max());

			int distance = forward.extract_distance(x) + backward.extract_distance(x);
			if(distance < best_distance){
				best_distance = distance;
				best_middle_node = x;
			}
		}

		return x;
	}

	bool is_finished()const{
		return forward.is_finished() && backward.is_finished();
	}

	template<class GetWeight>
	int settle_next(const GetWeight&get_weight){
		return settle_next(get_weight, get_weight, [](int){return true;}, [](int){return true;});
	}

	//! Settles nodes until a shortest path has certainly been found.
	template<class GetWeight, class CanStop>
	void settle_until_certain(const GetWeight&get_weight, const CanStop&can_stop){
		settle_until_certain(get_weight, get_weight, can_stop);

	}

	//! Settles nodes until a shortest path has certainly been found.
	template<class GetForwardWeight, class GetBackwardWeight, class CanStop>
	void settle_until_certain(const GetForwardWeight&get_forward_weight, const GetBackwardWeight&get_backward_weight, const CanStop&can_stop){
		static_assert(
			std::is_same<
				decltype(can_stop(0, 0, 0)), 
				bool
			>::value, 
			"can_stop must return a boolean"
		);

		while(
			(!forward.is_finished() || !backward.is_finished()) && 
			!can_stop(get_current_shortest_path_distance(), forward.get_current_radius(), backward.get_current_radius())
		){
			settle_next(get_forward_weight, get_backward_weight, [](int){return true;}, [](int){return true;});
		}

	}

	//! Returns the distance of the shortest currently discovered path.
	int get_current_shortest_path_distance()const{
		return best_distance;
	}

	int get_current_middle_node()const{
		return best_middle_node;
	}

	//! Returns the shortest currently discovered path.
	std::vector<int>get_current_shortest_path()const{
		if(best_distance == std::numeric_limits<int>::max())
			return {};
		else
			return get_via_path(best_middle_node);
	}

	std::vector<int>get_via_path(int via)const{
		if(!forward.path_to_exists(via) || !backward.path_to_exists(via))
			return {};
		else{
			auto
				forward_path = forward.extract_path(via),
				backward_path = backward.extract_path(via);
			assert(!backward_path.empty());
			std::copy(backward_path.rbegin()+1, backward_path.rend(), std::back_inserter(forward_path));
			return std::move(forward_path);
		}
	}
};

template<class Flags, class ForwardAdjGraph, class BackwardAdjGraph>
BidirectionalDijkstraGPPC<Flags, ForwardAdjGraph, BackwardAdjGraph>make_bidirectional_dijkstra(const ForwardAdjGraph&forward, const BackwardAdjGraph&backward){
	return BidirectionalDijkstraGPPC<Flags, ForwardAdjGraph, BackwardAdjGraph>(forward, backward);
}


}
#endif

