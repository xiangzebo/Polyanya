#ifndef ADJ_GRAPH_H
#define ADJ_GRAPH_H

#include "arc.h"
#include "range.h"
#include <type_traits>
#include <cassert>
#include <vector>
#include "vec_io.h"

//! A static adjacency array that allows for every node to quickly iterate over
//! its outgoing arcs' heads. An ArcHead is a structure that may contain 
//! arbitrary information. Usually it has a target member that stores the arc's
//! target node but even this is optional.
template<class T_ArcHead>
class AdjGraph{
public:
	typedef T_ArcHead ArcHead;
	typedef typename type_of_arc<ArcHead>::type Arc;
	
	//! Creates an empty adjacency array.
	AdjGraph():out_begin{0}, out_head{}{}

private:
	template<class ArcRange, class GetSource, class GetHead>
	void build_adj_array(int source_node_count, const ArcRange&arc_range, const GetSource&get_source, const GetHead&get_head){
		// FIXME: If C++11 support was better then build_adj_array would be a constructor and the other constructors would
		//        be implemented using constructor forwarding. However, until better support arrives, we are stuck with this.

		out_begin.resize(source_node_count+1);
		std::fill(out_begin.begin(), out_begin.end(), 0);
		out_head.resize(range_size(arc_range));

		for(auto x:arc_range)
			++out_begin[get_source(x)];

		int sum = 0;
		for(auto&x:out_begin){
			int tmp = sum;
			sum += x;
			x = tmp;
		}

		// Assign the arc heads using out_begin as counter.
		for(auto x:arc_range)
			out_head[out_begin[get_source(x)]++] = get_head(x);

		// Reset out_begin to the correct value.
		for(auto x:arc_range)
			--out_begin[get_source(x)];
	}

public:

	template<class ArcRange, class GetSource, class GetHead>
	AdjGraph(int source_node_count, const ArcRange&arc_range, const GetSource&get_source, const GetHead&get_head){
		assert(source_node_count >= 0 && "source_node_count must be positive");
		build_adj_array(source_node_count, arc_range, get_source, get_head);
		
	}

	//! Creates an adjacency array given an arc range. The source of an arc 
	//! x must be accesible as x.source and its head as get_head(x).
	//!
	//! The outgoing arcs of a node are arranged in the same order that they
	//! appear in arc_range.
	template<class ArcRange>
	AdjGraph(int node_count, const ArcRange&arc_range){
		assert(node_count >= 0 && "node_count must be positive");
		typedef typename type_of_range_element<ArcRange>::type Arc;
		build_adj_array(node_count, arc_range, [](Arc a){return a.source;}, [](Arc a){return get_head(a);});
	}

	int node_count()const{
		return out_begin.size()-1;
	}
	
	int arc_count()const{
		return out_head.size();
	}

	//! The out-degree of node v.
	int out_degree(int v)const{
		assert(0 <= v && v < node_count() && "v is out of bounds");
		return out_begin[v+1] - out_begin[v];
	}

	//! Returns a range over the v's outgoing arcs' heads.
	Range<typename std::vector<ArcHead>::const_iterator> out(int v)const{
		assert(0 <= v && v < node_count() && "v is out of bounds");
		return make_range(out_head.begin() + out_begin[v], out_head.begin() + out_begin[v+1]);
	}

	Range<typename std::vector<ArcHead>::iterator> out(int v){
		assert(0 <= v && v < node_count() && "v is out of bounds");
		return make_range(out_head.begin() + out_begin[v], out_head.begin() + out_begin[v+1]);
	}

	Range<CountIterator> arc_id_out(int v)const{
		assert(0 <= v && v < node_count() && "v is out of bounds");
		return count_range(out_begin[v], out_begin[v+1]);
	}

	const ArcHead&head(int arc_id)const{
		assert(0 <= arc_id && arc_id < arc_count() && "arc_id is out of bounds");
		return out_head[arc_id];
	}

	ArcHead&head(int arc_id){
		assert(0 <= arc_id && arc_id < arc_count() && "arc_id is out of bounds");
		return out_head[arc_id];
	}

	void save(std::FILE*f){
		save_vector(f, out_begin);
		save_vector(f, out_head);
	}

	void load(std::FILE*f){
		out_begin = load_vector<int>(f);
		out_head = load_vector<ArcHead>(f);
	}

	// Members are not private because OpenMP for does not support range-based-for loops.
	std::vector<int>out_begin;
	std::vector<ArcHead>out_head;
};

template<class G>
int get_arc_id(const G&g, int x, int y){
	for(auto xy:g.arc_id_out(x))
		if(g.head(xy).target == y)
			return xy;
	return -1;
}

template<class ArcRange>
auto make_adj_graph(int node_count, const ArcRange&arc_range)
	->AdjGraph<typename type_of_arc_head<typename type_of_range_element<ArcRange>::type>::type>{
	return {node_count, arc_range};
}

template<class ArcRange, class GetSource, class GetHead>
auto make_adj_graph(int node_count, const ArcRange&arc_range, const GetSource&get_source, const GetHead&get_head)
	->AdjGraph<typename std::decay<decltype(get_head(*std::begin(arc_range)))>::type>{
	return {node_count, arc_range, get_source, get_head};
}

#endif

