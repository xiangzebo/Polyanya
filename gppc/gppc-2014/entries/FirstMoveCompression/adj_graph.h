#ifndef ADJ_GRAPH_H 
#define ADJ_GRAPH_H

#include "list_graph.h"
#include "range.h"
#include "adj_array.h"

struct OutArc{
	int target;
	int weight;
};

class AdjGraph{
public:
	AdjGraph(){}

	AdjGraph(ListGraph g){
		build_adj_array(
			out_begin, out_arc,
			g.node_count(), g.arc.size(),
			[&](int x){return g.arc[x].source;},
			[&](int x){return OutArc{g.arc[x].target, g.arc[x].weight};}
		);
	}

	AdjGraph&operator=(const ListGraph&o){
		return *this = AdjGraph(o);
	}

	int node_count()const{
		return out_begin.size()-1;	
	}

	Range<std::vector<OutArc>::const_iterator>out(int v)const{
		return make_range(out_arc.begin() + out_begin[v], out_arc.begin() + out_begin[v+1]);
	}

	OutArc out(int v, int i)const{
		return out_arc[out_begin[v] + i];
	}

	int out_deg(int v)const{
		return out_begin[v+1] - out_begin[v];
	}

private:
	std::vector<int>out_begin;
	std::vector<OutArc>out_arc;
};

#endif


