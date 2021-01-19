#ifndef WARTHOG_PQUEUE_H
#define WARTHOG_PQUEUE_H

// pqueue.h
//
// A min priority queue. Loosely based on an implementation from HOG
// by Nathan Sturtevant.
//
// @author: dharabor
// @created: 09/08/2012
//

#include "search_node.h"

#include <cassert>
#include <iostream>

namespace warthog
{

class search_node;
class pqueue 
{
	public:
		pqueue(unsigned int size, bool minqueue);
		~pqueue();

		// removes all elements from the pqueue
		void 
		clear();

		// reprioritise the specified element (up or down)
		void
		decrease_key(warthog::search_node* val);

		void 
		increase_key(warthog::search_node* val);

		// add a new element to the pqueue
		void 
		push(warthog::search_node* val);

		// remove the top element from the pqueue
		warthog::search_node*
		pop();

		// @return true if the priority of the element is 
		// otherwise
		inline bool
		contains(warthog::search_node* n)
		{
			unsigned int priority = n->get_priority();
			if(priority < queuesize_ && &*n == &*elts_[priority])
			{
				return true;
			}
			return false;
		}

		// retrieve the top element without removing it
		inline warthog::search_node*
		peek()
		{
			if(queuesize_ > 0)
			{
				return this->elts_[0];
			}
			return 0;
		}

		inline unsigned int
		size()
		{
			return queuesize_;
		}

		inline bool
		is_minqueue() 
		{ 
			return minqueue_; 
		} 
		
		void
		print(std::ostream& out);

		unsigned int
		mem()
		{
			return maxsize_*sizeof(warthog::search_node*)
				+ sizeof(*this);
		}

	private:
		unsigned int maxsize_;
		bool minqueue_;
		unsigned int queuesize_;
		warthog::search_node** elts_;

		// reorders the subpqueue containing elts_[index]
		void 
		heapify_up(unsigned int);
		
		// reorders the subpqueue under elts_[index]
		void 
		heapify_down(unsigned int);

		// allocates more memory so the pqueue can grow
		void 
		resize(unsigned int newsize);
	
		// returns true if:
		//   minqueue is true and the priority of second < first
		//   minqueue is false and the priority of second > first
		inline bool 
		rotate(warthog::search_node& first, warthog::search_node& second)
		{
			if(minqueue_)
			{
				if(second < first)
				{
					return true;
				}
			}
			else
			{
				if(second > first)
				{
					return true;
				}
			}
			return false;
		}

		// swap the positions of two nodes in the underlying array
		inline void 
		swap(unsigned int index1, unsigned int index2)
		{
			assert(index1 < queuesize_ && index2 < queuesize_);

			warthog::search_node* tmp = elts_[index1];
			elts_[index1] = elts_[index2];
			elts_[index1]->set_priority(index1);
			elts_[index2] = tmp;
			tmp->set_priority(index2);
		}
};

}

#endif

