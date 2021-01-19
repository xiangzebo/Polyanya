#ifndef WARTHOG_HASH_TABLE_H
#define WARTHOG_HASH_TABLE_H

#include <climits>
#include <iostream>

namespace warthog
{

struct list_element
{
	list_element()
	{
		next = 0;
		value = UINT_MAX;
	}

	~list_element()
	{
		if(next)
		{
			delete next;
		}
	}

	warthog::list_element* next;
	unsigned int value;
};

class hash_table
{

	public:
		hash_table() : elements_(0), size_(0), stored_(0)
		{
			rehash(4);
		}

		~hash_table()
		{
			for(unsigned int i=0; i < size_; i++)
			{
				delete elements_[i];
			}
			delete [] elements_;
		}

		inline bool 
		contains(unsigned int value)
		{
			unsigned int index = jenkins32(value);
			if(elements_[index])
			{
				warthog::list_element* n = elements_[index];
				while(n)
				{
					if(n->value == value)
					{
						return true;
					}
					n = n->next;
				}
			}
			return false;
		}

		inline int
		insert(unsigned int value)
		{
			if(size_ == stored_)
			{
				rehash(size_*2);
			}

			unsigned int index = jenkins32(value);
			if(elements_[index] == 0)
			{
				warthog::list_element*  foo = new warthog::list_element();
				foo->value = value;
				elements_[index] = foo;
			}
			else
			{
				warthog::list_element* n = elements_[index];
				while(n)
				{
					if(n->value == value)
					{
						return index;
						//std::cout << "hash_table::insert duplicate element "<<value<<std::endl;
					}
					n = n->next;
				}
				warthog::list_element*  foo = new warthog::list_element();
				foo->value = value;
				foo->next = elements_[index];
				elements_[index] = foo;
			}
			//std::cout << "hash_table::insert inserted "<<value <<" at "<<index<<std::endl;
			stored_++;
			return index;
		}

		void
		rehash(unsigned int size)
		{
			if(size <= size_)
			{
				return;
			}

			//std::cout << "rehash; newsize: "<<size<<std::endl;

			warthog::list_element** oldelements = elements_;
			unsigned int oldsize = size_;

			// create the new array
			size_ = size;
			elements_ = new warthog::list_element*[size];
			for(unsigned int i=0; i < size_; i++)
			{
				elements_[i] = 0;
			}

			// rehash
			stored_ = 0;
			for(unsigned int i=0; i < oldsize; i++)
			{
				warthog::list_element* n = oldelements[i];
				while(n)
				{
					insert(n->value);
					n = n->next;
				}

				if(oldelements[i])
				{
					delete oldelements[i];
				}

			}
			delete [] oldelements;
		}

		void
		print()
		{
			for(unsigned int i=0; i < size_; i++)
			{
				warthog::list_element* n = elements_[i];
				while(n)
				{
					//std::cout << elements_[i]->value <<" -> " <<i<<std::endl;
					n = n->next;
				}
			}
		}	

	private:

		// Bob Jenkins' 32bit hash
		inline unsigned int
		jenkins32(unsigned int key)
		{
			key += (key << 12);
			key ^= (key >> 22);
			key += (key << 4);
			key ^= (key >> 9);
			key += (key << 10);
			key ^= (key >> 2);
			key += (key << 7);
			key ^= (key >> 12);
			return key & (size_-1);
		}

		warthog::list_element** elements_;
		unsigned int size_;
		unsigned int stored_;
};

}

#endif

