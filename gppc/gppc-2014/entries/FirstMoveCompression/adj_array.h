#ifndef ADJ_ARRAY_H
#define ADJ_ARRAY_H

#include <algorithm>
#include <cassert>

namespace detail{
	template<class Vector, class ArcSource>
	void build_begin_array(Vector&out_begin, int node_count, int arc_count, ArcSource source){
		out_begin.resize(node_count+1);

		std::fill(out_begin.begin(), out_begin.begin()+node_count, 0);

		for(int i=0; i<arc_count; ++i){
			int x = source(i);
			assert(x >= 0);
			assert(x<node_count);
			++out_begin[x];
		}
		
		int sum = 0;
		for(int i=0; i<=node_count; ++i){
			int tmp = out_begin[i];
			out_begin[i] = sum;
			sum += tmp;
		}
	}
}

// Builds an adjacency such that the outgoing arcs of node v have the ids 
// out_begin[v]...out_begin[v-1]. Further the out_dest array is filled with 
// some arc data. (Such as nearly always at least the arc's tail/target.)
//
// To acheive this the number the number of nodes and arcs must be given. Further
// source is a functor that maps an arc id onto its head/source node. arc_data
// maps an arc id onto the data that should be stored in out_dest.
template<class Vector1, class Vector2, class ArcSource, class ArcData>
void build_adj_array(Vector1&out_begin, Vector2&out_dest,
	int node_count, int arc_count, ArcSource source, ArcData arc_data){

	detail::build_begin_array(out_begin, node_count, arc_count, source);

	out_dest.resize(arc_count);

	for(int i=0; i<arc_count; ++i)
		out_dest[out_begin[source(i)]++] = arc_data(i);

	for(int i=0; i<arc_count; ++i)
		--out_begin[source(i)];
}

#endif

