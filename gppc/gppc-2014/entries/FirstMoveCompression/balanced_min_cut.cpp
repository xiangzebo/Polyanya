#include "balanced_min_cut.h"
#include "adj_array.h"
#include <metis.h>
#include <cassert>

// link with -lmetis
// compile with -O3 -DNDEBUG

std::vector<bool>balanced_min_cut(const ListGraph&g){
	assert(g.is_valid());

	#ifndef USE_CUT_ORDER
	return {};
	#else

	std::vector<int>out_begin;
	std::vector<int>out_dest;

	build_adj_array(out_begin, out_dest, 
		g.node_count(), g.arc.size(),
		[&](int x){return g.arc[x].source;},
		[&](int x){return g.arc[x].target;}
	);

	std::vector<int>part(g.node_count());

	// Call METIS
	int one = 1, two = 2, ignore, node_count = g.node_count();

	idx_t options[METIS_NOPTIONS];
	METIS_SetDefaultOptions(options);
	options[METIS_OPTION_NUMBERING] = 0;

	options[METIS_OPTION_CONTIG] = 1;
	options[METIS_OPTION_UFACTOR] = 100;

	//options[METIS_OPTION_NCUTS] = 10;

	METIS_PartGraphKway(
		(idx_t*)&node_count,
		(idx_t*)&one,
		(idx_t*)&out_begin[0],
		(idx_t*)&out_dest[0],
		nullptr,
		nullptr,
		nullptr,
		(idx_t*)&two,
		nullptr,
		nullptr,
		options,
		(idx_t*)&ignore,
		(idx_t*)&part[0]
	);

	std::vector<bool>part_ret(g.node_count());
	for(int i=0; i<g.node_count(); ++i)
		part_ret[i] = part[i] == 0;

	return std::move(part_ret);
	#endif
}

