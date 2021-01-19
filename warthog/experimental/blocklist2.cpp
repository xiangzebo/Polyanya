#include "blocklist2.h"
#include "helpers.h"
#include "search_node.h"

warthog::blocklist2::blocklist2(uint32_t mapheight, uint32_t mapwidth)
	: blocks_(0), pool_(0)
{
	num_blocks_ = ((mapwidth*mapheight) >> warthog::blocklist2_ns::LOG2_NBS)+1;
	blockspool_ = 
		new warthog::mem::cpool(sizeof(void*)*warthog::blocklist2_ns::NBS, 1);
	pool_ = new warthog::mem::cpool(sizeof(warthog::search_node));

	blocks_ = new warthog::search_node**[num_blocks_];
	for(uint32_t i=0; i < num_blocks_; i++)
	{
		warthog::search_node** list = new (blockspool_->allocate())
		   	warthog::search_node*[warthog::blocklist2_ns::NBS];
		for(uint32_t j = 0; j < warthog::blocklist2_ns::NBS; j+=8)
		{
			list[j] = 0;
			list[j+1] = 0;
			list[j+2] = 0;
			list[j+3] = 0;
			list[j+4] = 0;
			list[j+5] = 0;
			list[j+6] = 0;
			list[j+7] = 0;
		}
		blocks_[i] = list;
	}
}

warthog::blocklist2::~blocklist2()
{
	pool_->reclaim();
	blockspool_->reclaim();
	delete pool_;
	delete blockspool_;
}

warthog::search_node*
warthog::blocklist2::generate(uint32_t node_id)
{
	uint32_t block_id = node_id >> warthog::blocklist2_ns::LOG2_NBS;
	uint32_t list_id = node_id &  warthog::blocklist2_ns::NBS_MASK;
	assert(block_id <= num_blocks_);

	// look for node_id in an existing block
	warthog::search_node* mynode = blocks_[block_id][list_id];
	if(mynode == 0)
	{
		// not in any existing block; generate it
		mynode = new (pool_->allocate()) warthog::search_node(node_id);
		blocks_[block_id][list_id] = mynode;
	}

	return mynode;
}

void
warthog::blocklist2::clear()
{
}

uint32_t
warthog::blocklist2::mem()
{
	uint32_t bytes = sizeof(*this) + blockspool_->mem() +
		pool_->mem() + num_blocks_*sizeof(warthog::search_node**);

	return bytes;
}
