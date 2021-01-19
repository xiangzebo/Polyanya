#ifndef WARTHOG_CUCKOO_TABLE_H
#define WARTHOG_CUCKOO_TABLE_H

// cuckoo_table.h
//
// A hash table for mapping integer values to range of integers [0, N).
// This implementation uses a cuckoo hashing scheme over two arrays.
// The hashing function h1, used for the first array, is FNV32-1a
// The hashing function h2, used for the second array, is double FNV32-1a
//
// NB: This implementation assumes that all values being mapped are unique.
// NB2: The value UINT_MAX is reserved; it indicates an empty bucket.
//
// @author: dharabor
// @created: 10/08/2012
//

#include "constants.h"

#include <cstdlib>
#include <iostream>
#include <ostream>

namespace warthog
{

namespace hash
{

const unsigned int BUCKET_SIZE=2;
const unsigned int LOG2_BUCKET_SIZE =
   	ceil(log10(warthog::hash::BUCKET_SIZE) / log(2));

}

// a multi-element container for hash tables
class cuckoo_bucket
{
	public:
		cuckoo_bucket()
		{
			elts_ = new unsigned int[warthog::hash::BUCKET_SIZE];
			clear();
		}

		virtual ~cuckoo_bucket()
		{
			delete [] elts_;
		}
		
		inline bool
		insert(unsigned int value)
		{
			for(unsigned int i=0; i < warthog::hash::BUCKET_SIZE; i++)
			{
				if(elts_[i] == UINT_MAX)
				{
					elts_[i] = value;
					return true;
				}
			}
			return false;
		}

		inline unsigned int
		swap(unsigned int value)
		{
			unsigned int tmp = elts_[0];
			elts_[0] = value;
			return tmp;
		}

		inline bool 
		contains(unsigned int value)
		{
			for(unsigned int i=0; i < warthog::hash::BUCKET_SIZE; i++)
			{
				if(elts_[i] == value)
				{
					return true;
				}
			}
			return false;
		}

		inline bool
		erase(unsigned int value)
		{
			for(unsigned int i=0; i < warthog::hash::BUCKET_SIZE; i++)
			{
				if(elts_[i] == value)
				{
					elts_[i] = UINT_MAX;
					return true;
				}
			}
			return false;
		}
		
		inline void
		clear()
		{
			for(unsigned int i=0; i < warthog::hash::BUCKET_SIZE; i++)
			{
				elts_[i] = UINT_MAX;
			}
		}

		inline unsigned int 
		get(unsigned int index)
		{
			if(index > warthog::hash::BUCKET_SIZE)
			{
				return UINT_MAX;
			}
			return elts_[index];
		}

	private:
		unsigned int* elts_;
};

class cuckoo_table
{
	public:
		cuckoo_table(unsigned int num_elements) ;
		virtual ~cuckoo_table();

		void 
		clear();

		unsigned int 
		max_elements()
		{
			return max_buckets_*2*warthog::hash::BUCKET_SIZE;
		}

		void 
		rehash(unsigned int num_elements);

		// @return true if an element was erased; else false
		inline bool
		erase(unsigned int value)
		{
			if(!f_[h1(value)].erase(value))
			{
				return s_[h2(value)].erase(value);
			}
			return false;
		}

		// @return: true if insertion successful, else false
		inline bool 
		insert(unsigned int value)
		{
			if(value == UINT_MAX)
			{
				if(verbose_)
				{
					std::cerr << "err; cuckoo_table::insert "
						"value=UINT_MAX is not allowed\n";
				}
				return false;
			}

			if(!__insert(value))
			{
				rehash(this->max_elements()*2);
				if(!__insert(value))
				{
					std::cerr << "rehash failed; grr; exiting.\n";
					exit(1);
				}
			}
			stored_elements_++;
			return true;
		}

		
		inline bool
		contains(unsigned int value)
		{
			return 
				f_[h1(value)].contains(value) |
			   	s_[h2(value)].contains(value);
		}

		// @return load factor for the table
		inline double 
		load()
		{
			return stored_elements_ /
			   	(double)(max_buckets_*2* warthog::hash::BUCKET_SIZE);
		}

		inline void metrics(std::ostream& out)
		{
			out
			<< "cuckoo_table:load="<<this->load() << "\n"
			<< "cuckoo_table::stored_elements_="<<stored_elements_<<"\n"
			<< "cuckoo_table::max_elements: " << max_buckets_*2*warthog::hash::BUCKET_SIZE<<"\n";
		}

		inline void 
		set_verbose(bool verbose)
		{
			this->verbose_ = verbose;
		}

		inline bool
		get_verbose()
		{
			return this->verbose_;
		}



	private:
		cuckoo_table(const cuckoo_table& other) {}
		cuckoo_table& operator=(const cuckoo_table& other) 
		{ 
			return *this; 
		}

		// @return fnv_hash(value) -- truncated to the maximum number of
		// buckets available in the table.
		unsigned int 
		h1(unsigned int value)
		{
			return this->fnv_hash(value) & (max_buckets_-1);
		}

		// @return ~h1(value) -- truncated to the maxmimum number of buckets 
		// available in the table
		unsigned int 
		h2(unsigned int value)
		{
			value += (value << 12);
			value ^= (value >> 22);
			value += (value << 4);
			value ^= (value >> 9);
			value += (value << 10);
			value ^= (value >> 2);
			value += (value << 7);
			value ^= (value >> 12);
			return value & (max_buckets_-1);
		}

		// 32bit FNV variant due to Bret Mulvey; uses a modified mixin to 
		// improve avalanching properties.
		// See http://bretm.home.comcast.net/~bretm/hash/6.html
		unsigned int 
		fnv_hash(unsigned int value)
		{
			unsigned int hash = warthog::FNV32_offset_basis;
			for( ; value > 0; value >>= 8)
			{
				hash = (hash ^ (value & 255)) * warthog::FNV32_prime;
				hash += hash << 13;
				hash ^= hash >> 7;
				hash += hash << 3;
				hash ^= hash >> 17;
				hash += hash << 5;
			}
			return hash;
		}

		inline bool
		__insert(unsigned int value)
		{
			unsigned int fpos = h1(value);
			unsigned int spos = h2(value);

			if(verbose_)
			{
				std::cerr << "cuckoo_table::__insert value="<<value << " candidate buckets "<<fpos<<" and "<<spos<<" ";
			}


			// check if element is already in the table
			if(f_[fpos].contains(value) || s_[spos].contains(value))
			{
				//std::cerr << value << " already exists "<<std::endl;
				return true;
			}

			// try to insert element into the table
			unsigned int tmp = value;
			for(unsigned int attempt=0; attempt < 10; attempt++)
			{
				if(f_[fpos].insert(tmp))
				{
					if(verbose_)
					{
						std::cerr << " placing into f @ "<<fpos<<std::endl;
					}
					return true;
				}
				if(s_[spos].insert(tmp))
				{
					if(verbose_)
					{
						std::cerr << " placing into s @ "<<spos<<std::endl;
					}
					return true;
				}

				// can't find an empty slot; cuckoo insert
				if(verbose_)
				{
					std::cerr << " cuckoo "<<tmp<<" into f @"<<fpos<<std::endl;
				}
				tmp = f_[fpos].swap(tmp);
				spos = h2(tmp);
				if(verbose_)
				{
					std::cerr << " cuckoo "<<tmp<<" into s @"<<spos<<std::endl;
				}
				tmp = s_[spos].swap(tmp);
				fpos = this->h1(tmp);
				spos = this->h2(tmp);
				if(verbose_)
				{
					std::cerr << " value="<<tmp<< " candidate buckets "<<fpos<<" and "<<spos<<"\n";
				}
			}

			if(verbose_)
			{
				std::cerr << " insertion failed; undoing"<<std::endl;
				if(tmp != value)
				{
					// undo failed insertion
					this->erase(value);
					if(!__insert(tmp))
					{
						std::cerr << "err; cuckoo_table::__insert cannot undo failed insertion. fatal."<<std::endl;
						exit(1);
					}
				}
			}
			return false;
		}

		unsigned int max_buckets_;
		unsigned int stored_elements_;
		warthog::cuckoo_bucket* f_;
		warthog::cuckoo_bucket* s_;
		bool verbose_;
};

}

#endif

