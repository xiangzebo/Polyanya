#include "blocklist.h"
#include "helpers.h"
#include "search_node.h"

warthog::blocklist::blocklist(uint32_t mapheight, uint32_t mapwidth)
	: blocks_(0), pool_(0)
{
	num_blocks_ = ((mapwidth*mapheight) >> warthog::blocklist_ns::LOG2_NBS)+1;
	blocks_ = new warthog::search_node**[num_blocks_];
	for(uint32_t i=0; i < num_blocks_; i++)
	{
		blocks_[i] = 0;
	}
	blockspool_ = 
		new warthog::mem::cpool(sizeof(void*)*warthog::blocklist_ns::NBS, 1);
	pool_ = new warthog::mem::cpool(sizeof(warthog::search_node));
}

warthog::blocklist::~blocklist()
{
	clear();
	delete pool_;
	delete blockspool_;
}

warthog::search_node*
warthog::blocklist::generate(uint32_t node_id)
{
	uint32_t block_id = node_id >> warthog::blocklist_ns::LOG2_NBS;
	uint32_t list_id = node_id &  warthog::blocklist_ns::NBS_MASK;
	assert(block_id <= num_blocks_);

	if(!blocks_[block_id])
	{
		// add a new block of nodes
		//std::cerr << "generating block: "<<block_id<<std::endl;
		warthog::search_node** list = new (blockspool_->allocate())
		   	warthog::search_node*[warthog::blocklist_ns::NBS];
		uint32_t i = 0;
		for( ; i < warthog::blocklist_ns::NBS; i+=8)
		{
			list[i] = 0;
			list[i+1] = 0;
			list[i+2] = 0;
			list[i+3] = 0;
			list[i+4] = 0;
			list[i+5] = 0;
			list[i+6] = 0;
			list[i+7] = 0;
		}
		// generate node_id
		warthog::search_node* mynode = new (pool_->allocate())
			warthog::search_node(node_id);
		list[list_id] = mynode;
		blocks_[block_id] = list;
		return mynode;
	}

	// look for node_id in an existing block
	warthog::search_node* mynode = blocks_[block_id][list_id];
	if(mynode)
	{
		return mynode;
	}

	// not in any existing block; generate it
	mynode = new (pool_->allocate()) warthog::search_node(node_id);
	blocks_[block_id][list_id] = mynode;
	return mynode;
}

void
warthog::blocklist::clear()
{
	for(uint32_t i=0; i < num_blocks_; i++)
	{
		if(blocks_[i] != 0)
		{
			//std::cerr << "deleting block: "<<i<<std::endl;
			blocks_[i] = 0;
		}
	}
	pool_->reclaim();
	blockspool_->reclaim();
}

uint32_t
warthog::blocklist::mem()
{
	uint32_t bytes = sizeof(*this) + blockspool_->mem() +
		pool_->mem() + num_blocks_*sizeof(warthog::search_node**);

	return bytes;
}
