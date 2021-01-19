#include "order.h"
#include <fstream>
#include <stdexcept>
#include <utility>
#include <vector>
#include <stack>

// compile with -O3 -DNDEBUG

NodeOrdering compute_real_dfs_order(const ListGraph&g){

	std::vector<int>out_begin, out_dest;
	auto source_node = [&](int x){
		return g.arc[x].source;
	};

	auto target_node = [&](int x){
		return g.arc[x].target;
	};

	build_adj_array(
		out_begin, out_dest, 
		g.node_count(), g.arc.size(),
		source_node, target_node
	);

	NodeOrdering order(g.node_count());

	std::vector<bool>was_pushed(g.node_count(), false);
	std::vector<bool>was_popped(g.node_count(), false);
	std::vector<int>next_out = out_begin;
	std::stack<int>to_visit;
	int next_id = 0;
	for(int source_node=0; source_node<g.node_count(); ++source_node){
		if(!was_pushed[source_node]){

			to_visit.push(source_node);
			was_pushed[source_node] = true;

			while(!to_visit.empty()){
				int x = to_visit.top();
				to_visit.pop();
				if(!was_popped[x]){
					order.map(x, next_id++);
					was_popped[x] = true;
				}

				while(next_out[x] != out_begin[x+1]){
					int y = out_dest[next_out[x]];
					if(was_pushed[y])
						++next_out[x];
					else{
						was_pushed[y] = true;
						to_visit.push(x);
						to_visit.push(y);
						++next_out[x];
						break;
					}
				}
			}
		}
	}
	assert(order.is_complete());
	return std::move(order);
}

