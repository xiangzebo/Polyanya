#ifndef CPD_H
#define CPD_H

#include <vector>
#include <algorithm>
#include <string>
#include "adj_graph.h"
#include "binary_search.h"
#include "range.h"
#include "vec_io.h"

//! Compressed Path database. Allows to quickly query the first out arc id of
//! any shortest source-target-path. There may be at most 15 outgoing arcs for
//! any node.
class CPD{
public:
	CPD():begin{0}{}

	//! Adds a new node s to the CPD. first_move should be an array that 
	//! maps every target node onto a 15-bit bitfield that has a bit set
	//! for every valid first move. get_first_move is free to return any of
	//! them.
	void append_row(int source_node, const std::vector<unsigned short>&first_move);

	void append_rows(const CPD&other);

	//! Get the first move. 
	//! An ID of 0xF means that there is no path. 
	//! If source_node == target_node then return value is undefined. 
	unsigned char get_first_move(int source_node, int target_node)const{
		target_node <<= 4;
		target_node |= 0xF;
		return *binary_find_last_true(
			entry.begin() + begin[source_node],
			entry.begin() + begin[source_node+1],
			[=](int x){return x <= target_node;}
		)&0xF;
	}

	int node_count()const{
		return begin.size()-1;
	}

	int entry_count()const{
		return entry.size();
	}

	friend bool operator==(const CPD&l, const CPD&r){
		return l.begin == r.begin && l.entry == r.entry;
	}

	void save(std::FILE*f)const{
		save_vector(f, begin);
		save_vector(f, entry);
	}

	void load(std::FILE*f){
		begin = load_vector<int>(f);
		entry = load_vector<int>(f);
	}

private:
	std::vector<int>begin;
	std::vector<int>entry;
};

inline
bool operator!=(const CPD&l, const CPD&r){
	return !(l == r);
}

#endif
