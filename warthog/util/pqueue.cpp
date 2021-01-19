#include "pqueue.h"

warthog::pqueue::pqueue(unsigned int s, bool minqueue) 
	: maxsize_(s), minqueue_(minqueue), queuesize_(0), elts_(0)
{
	resize(s);
}

warthog::pqueue::~pqueue()
{
	delete [] elts_;
}

void 
warthog::pqueue::push(warthog::search_node* val)
{
	if(contains(val))
	{
		return;
	}

	if(queuesize_+1 > maxsize_)
	{
		resize(maxsize_*2);
	}
	unsigned int priority = queuesize_;
	elts_[priority] = val;
	val->set_priority(priority);
	queuesize_++;
	heapify_up(priority);
}

warthog::search_node*
warthog::pqueue::pop()
{
	if (queuesize_ == 0)
	{
		return 0;
	}

	warthog::search_node *ans = elts_[0];
	queuesize_--;

	if(queuesize_ > 0)
	{
		elts_[0] = elts_[queuesize_];
		elts_[0]->set_priority(0);
		heapify_down(0);
	}
	return ans;
}

void 
warthog::pqueue::heapify_up(unsigned int index)
{
	assert(index < queuesize_);
	while(index > 0)
	{
		unsigned int parent = (index-1) >> 1;
		if(rotate(*elts_[parent], *elts_[index]))
		{
			swap(parent, index);
			index = parent;
		}
		else
		{
			break;
		}
	}
}

void 
warthog::pqueue::heapify_down(unsigned int index)
{
	unsigned int first_leaf_index = queuesize_ >> 1;
	while(index < first_leaf_index)
	{
		// find smallest (or largest, depending on heap type) child
		unsigned int child1 = (index<<1)+1;
		unsigned int child2 = (index<<1)+2;
		unsigned int which = child1;
		if((child2 < queuesize_) && *elts_[child2] < *elts_[child1])
		{
			which = child2;
		}
//		if ((child2 < queuesize_) && rotate(*elts_[child1], *elts_[child2]))
//		{
//			which = child2;
//		}

		// swap child with parent if necessary
		if(*elts_[which] < *elts_[index])
		//if (rotate(*elts_[index], *elts_[which]))
		{
			swap(index, which);
			index = which;
		}
		else
		{
			break;
		}
	}
}

void 
warthog::pqueue::decrease_key(warthog::search_node* val)
{	
	assert(val->get_priority() < queuesize_);
	if(minqueue_)
	{
		heapify_up(val->get_priority());
	}
	else
	{
		heapify_down(val->get_priority());
	}
}

void 
warthog::pqueue::increase_key(warthog::search_node* val)
{
	assert(val->get_priority() < queuesize_);
	if(minqueue_)
	{
		heapify_down(val->get_priority());
	}
	else
	{
		heapify_up(val->get_priority());
	}
}

void
warthog::pqueue::resize(unsigned int newsize)
{
//	std::cout << "pqueue::resize oldsize: "<<queuesize_<<" newsize " << newsize<<std::endl;
	if(newsize < queuesize_)
	{
		std::cerr << "err; pqueue::resize newsize < queuesize " << std::endl;
		exit(1);
	}

 	warthog::search_node** tmp = new search_node*[newsize];
	for(unsigned int i=0; i < queuesize_; i++)
	{
		tmp[i] = elts_[i];
	}
	delete [] elts_;
	elts_ = tmp;
	maxsize_ = newsize;
}

void
warthog::pqueue::clear()
{
	for(unsigned int i=0; i < queuesize_; i++)
	{
		elts_[i] = 0;
	}
	queuesize_ = 0;
}

void 
warthog::pqueue::print(std::ostream& out)
{
	for(unsigned int i=0; i < queuesize_; i++)
	{
		elts_[i]->print(out);
		out << std::endl;
	}
}

