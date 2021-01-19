#ifndef ORDER_H
#define ORDER_H
#include "list_graph.h"
#include "adj_array.h"
#include "cut_utility.h"
#include "vec_io.h"
#include <cassert>
#include <string>
#include <utility>
#include <stdexcept>

class NodeOrdering{
public:
	NodeOrdering(){}
	explicit NodeOrdering(int node_count):to_new_array(node_count, -1), to_old_array(node_count, -1){}

	void clear(){
		std::fill(to_new_array.begin(), to_new_array.end(), -1);
		std::fill(to_old_array.begin(), to_old_array.end(), -1);
	}

	int node_count()const{
		return to_new_array.size();
	}

	int to_new(int x)const{return to_new_array[x];}
	int to_old(int x)const{return to_old_array[x];}

	void map(int old_id, int new_id){
		assert(old_id != -1);
		assert(new_id != -1);

		if(to_new_array[old_id] != -1)
			to_old_array[to_new_array[old_id]] = -1;
		if(to_old_array[new_id] != -1)
			to_new_array[to_old_array[new_id]] = -1;
		to_new_array[old_id] = new_id;
		to_old_array[new_id] = old_id;
	}

	bool is_complete()const{
		return std::find(to_new_array.begin(), to_new_array.end(), -1) == to_new_array.end();
	}

	void sort_range(int new_id_begin, int new_id_end){
		std::sort(
			to_old_array.begin() + new_id_begin, 
			to_old_array.begin() + new_id_end
		);
		for(int i=new_id_begin; i<new_id_end; ++i)
			to_new_array[to_old_array[i]] = i;
	}
	
	bool next_range_permutation(int new_id_begin, int new_id_end){
		bool ret = std::next_permutation(
			to_old_array.begin() + new_id_begin, 
			to_old_array.begin() + new_id_end
		);
		for(int i=new_id_begin; i<new_id_end; ++i)
			to_new_array[to_old_array[i]] = i;
		return ret;
	}

	std::vector<int>store_range(int new_id_begin, int new_id_end){
		return std::vector<int>(
			to_old_array.begin() + new_id_begin, 
			to_old_array.begin() + new_id_end
		);
	}

	void load_range(int new_id_begin, const std::vector<int>&v){
		std::copy(v.begin(), v.end(), to_old_array.begin() + new_id_begin);
		for(int i=new_id_begin; i<new_id_begin+(int)v.size(); ++i)
			to_new_array[to_old_array[i]] = i;
	}

	void save(std::FILE*f)const{
		save_vector(f, to_new_array);
		save_vector(f, to_old_array);
	}

	void load(std::FILE*f){
		to_new_array = load_vector<int>(f);
		to_old_array = load_vector<int>(f);
	}

	friend bool operator==(const NodeOrdering&l, const NodeOrdering&r){
		return l.to_new_array == r.to_new_array && l.to_old_array == r.to_old_array;
	}

	void check_for_errors(){
		for(auto x:to_new_array){
			if(x < 0)
				throw std::runtime_error("to_new_array may not have 0 entries");
			if(x >= node_count())
				throw std::runtime_error("to_new_array may not have entries >= node_count");
		}
		for(auto x:to_old_array){
			if(x < 0)
				throw std::runtime_error("to_new_array may not have 0 entries");
			if(x >= node_count())
				throw std::runtime_error("to_new_array may not have entries >= node_count");
		}
		for(int i=0; i<node_count(); ++i){
			if(to_old_array[to_new_array[i]] != i)
				throw std::runtime_error("node ordering is no bijection");
		}
	}
private:
	std::vector<int>to_new_array, to_old_array;
};

inline
bool operator!=(const NodeOrdering&l, const NodeOrdering&r){
	return !(l == r);
}

NodeOrdering compute_real_dfs_order(const ListGraph&g);

template<class CutAlgo>
void compute_cut_order(
	int id_begin, const ListGraph&g, const std::vector<int>&g_to_top_level, const CutAlgo&algo, NodeOrdering&order,
	std::vector<int>&lower_deg, 
	std::vector<int>&higher_deg
){
	std::vector<bool>is_lower = remove_isolated_nodes_from_cut(g, algo(g));

	int lower_size = 0, upper_size = 0;
	for(auto x:is_lower)
		if(x)
			++lower_size;
		else
			++upper_size;

	if(std::find(is_lower.begin(), is_lower.end(), !is_lower[0]) == is_lower.end()){
		std::vector<int>inner_order(g.node_count());
		for(int i=0; i<g.node_count(); ++i)
			inner_order[i] = g_to_top_level[i];

		std::sort(inner_order.begin(), inner_order.end(), 
			[&](int u, int v){
				u = higher_deg[u]-lower_deg[u];
				v = higher_deg[v]-lower_deg[v];
				return u < v;
			}
		);

		for(int i=0; i<g.node_count(); ++i)
			order.map(inner_order[i], id_begin+i);	
	}else{
		long long int order_value = 0;
		for(int i=0; i<g.node_count(); ++i){
			if(is_lower[i]){
				order_value += lower_deg[g_to_top_level[i]];
				order_value -= higher_deg[g_to_top_level[i]];
			}else{
				order_value -= lower_deg[g_to_top_level[i]];
				order_value += higher_deg[g_to_top_level[i]];
			}
		}

		if(order_value < 0)
			for(int i=0; i<g.node_count(); ++i)
				is_lower[i] = !is_lower[i];

		std::vector<int>
			lower_to_top_level,
			higher_to_top_level;

		ListGraph 
			lower = extract_node_induced_subgraph(g, g_to_top_level, [&](int x){return is_lower[x];}, lower_to_top_level),
			higher = extract_node_induced_subgraph(g, g_to_top_level, [&](int x){return !is_lower[x];}, higher_to_top_level);

		for(auto a:g.arc){
			if(is_lower[a.source] && !is_lower[a.target]){
				++higher_deg[g_to_top_level[a.source]];
				++lower_deg[g_to_top_level[a.target]];
			}else if(!is_lower[a.source] && is_lower[a.target]){
				++higher_deg[g_to_top_level[a.target]];
				++lower_deg[g_to_top_level[a.source]];
			}
		}

		compute_cut_order(id_begin+lower.node_count(), std::move(higher), std::move(higher_to_top_level), algo, order, lower_deg, higher_deg);
		compute_cut_order(id_begin, std::move(lower), std::move(lower_to_top_level), algo, order, lower_deg, higher_deg);
	}
}

template<class CutAlgo>
NodeOrdering compute_cut_order(const ListGraph&g, const CutAlgo&algo){
	NodeOrdering order(g.node_count());
	std::vector<int>higher_deg(g.node_count(), 0), lower_deg(g.node_count(), 0);

	std::vector<int>g_to_top_level(g.node_count());
	for(int i=0; i<g.node_count(); ++i)
		g_to_top_level[i] = i;

	compute_cut_order(0, g, std::move(g_to_top_level), algo, order, lower_deg, higher_deg);
	assert(order.is_complete());
	return std::move(order);
}

#endif

