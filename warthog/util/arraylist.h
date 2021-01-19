#ifndef WARTHOG_ARRAYLIST_H
#define WARTHOG_ARRAYLIST_H

// arraylist.h
//
// A dumb but dynamically-sized array for both simple and complex types.
// The array is initialised with an initial size. New objects are added 
// to the end of the collection. When the number of elements exceeds 
// the capacity of the array, its size increases by a factor of 2.
//
// @author: dharabor
// @created: 22/05/2013
//

#include <cstring>
#include <stdint.h>

namespace warthog
{
	template <class T>
	class arraylist
	{
		public:
			typedef T* iterator;

			arraylist(size_t size=100) : max_size_(size), terminator_(0)
			{
				collection_ = new T[max_size_];	
				next_ = 0;
			}

			~arraylist()
			{
				delete [] collection_;
			}

			inline T
			at(uint32_t index)
			{
				return collection_[index];
			}

			inline T
			operator[](uint32_t index)
			{
				return collection_[index];
			}

			inline void
			push_back(T element)
			{
				if(next_ < max_size_)
				{
					collection_[next_] = element;	
					next_++;
				}
				else
				{
					grow(size()*2);
					collection_[next_] = element;
					next_++;
				}
			}

			inline void
			pop_back()
			{
				if(next_ > 0)
				{
					next_--;
				}
			}

			inline void
			clear() 
			{
				next_ = 0;
			}

			inline size_t
			size()
			{
				return next_;
			}

			size_t
			mem() { return sizeof(T)*max_size_;}


		private:
			T* collection_;
			size_t next_;
			uint32_t max_size_;
			const typename warthog::arraylist<T>::iterator terminator_;

			inline void 
			grow(size_t newsize)
			{
				if(newsize <= max_size_) { return; }

				T* bigcollection = new T[newsize];
				for(size_t i = 0; i < max_size_; i++)
				{
					bigcollection[i] = collection_[i];
				}
				delete [] collection_;
				collection_ = bigcollection;
				max_size_ = newsize;
			}

	};
}

#endif

