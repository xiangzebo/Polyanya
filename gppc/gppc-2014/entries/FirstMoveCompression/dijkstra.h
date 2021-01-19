#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include <algorithm>
#include <vector>
#include <limits>

#include "adj_graph.h"
#include "heap.h"

class Dijkstra{
public:
	Dijkstra(const AdjGraph&g):
		g(g), q(g.node_count()), dist(g.node_count()), allowed(g.node_count()){}

	const std::vector<unsigned short>&run(int source_node){
		std::fill(dist.begin(), dist.end(), std::numeric_limits<int>::max());
		std::fill(allowed.begin(), allowed.end(), 0);

		dist[source_node] = 0;		
		allowed[source_node] = 0;

		auto reach = [&](int v, int d, unsigned short first_move){
			if(d < dist[v]){
				q.push_or_decrease_key(v, d);
				dist[v] = d;
				allowed[v] = first_move;
			}else if(d == dist[v]){
				allowed[v] |= first_move;
			}
		};

		for(int i=0; i<g.out_deg(source_node); ++i){
			auto a = g.out(source_node, i);
			reach(a.target, a.weight, 1 << i);
		}

		while(!q.empty()){
			int x = q.pop();

			for(auto a:g.out(x))
				reach(a.target, dist[x] + a.weight, allowed[x]);
		}

		#ifndef NDEBUG
		for(int u=0; u<g.node_count(); ++u)
			for(auto uv : g.out(u)){
				int v = uv.target;
				assert(dist[u] >= dist[v] - uv.weight);
			}
		#endif

		return allowed;
	}

	int distance(int target)const{
		return dist[target];
	}
private:
	const AdjGraph&g;
	min_id_heap<int>q;
	std::vector<int>dist;
	std::vector<unsigned short>allowed;
};

#endif
