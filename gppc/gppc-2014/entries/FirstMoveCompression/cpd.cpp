#include "cpd.h"

#include <fstream>
#include <stdexcept>
#include <cassert>

// compile with -O3 -DNDEBUG

static unsigned char find_first_allowed_out_arc(unsigned short allowed){
	assert(allowed != 0);
	for(int i=0; i<=0xF; ++i)
		if(allowed & (1u << i))
			return i;
	assert(false);
	return 0;
}

void CPD::append_row(int source_node, const std::vector<unsigned short>&allowed_first_move){
	auto get_allowed = [&](int x){
		if(x == source_node)
			return (unsigned short)0x7FFF;
		else if(allowed_first_move[x] == 0)
			return (unsigned short)0x8000;
		else
			return allowed_first_move[x];
	};

	int node_begin = 0;
	
	unsigned short allowed_up_to_now = get_allowed(0);
	for(int i=1; i<(int)allowed_first_move.size(); ++i){
		int allowed_next = allowed_up_to_now & get_allowed(i);
		if(allowed_next == 0){
			entry.push_back((node_begin << 4) | find_first_allowed_out_arc(allowed_up_to_now));
			node_begin = i;
			allowed_up_to_now = get_allowed(i);
		}else
			allowed_up_to_now = allowed_next;
	}
	entry.push_back((node_begin << 4) | find_first_allowed_out_arc(allowed_up_to_now));

	begin.push_back(entry.size());


	#ifndef NDEBUG
	int pos = 0;
	const int node_count = allowed_first_move.size();
	
	for(int target_node = 0; target_node < node_count; ++target_node){
		if(begin[source_node]+pos+1 != (int)begin[source_node+1] && entry[begin[source_node]+pos+1] <= ((target_node << 4) | 0xF)){
			++pos;
		}

		if(target_node != source_node && allowed_first_move[target_node] != 0){
			assert(allowed_first_move[target_node] & (1<<(entry[begin[source_node]+pos] & 0xF)));
		}
	}
	#endif
}

void CPD::append_rows(const CPD&other){
	int offset = begin.back();
	for(auto x:make_range(other.begin.begin()+1, other.begin.end()))
		begin.push_back(x + offset);
	std::copy(other.entry.begin(), other.entry.end(), back_inserter(entry));
}

