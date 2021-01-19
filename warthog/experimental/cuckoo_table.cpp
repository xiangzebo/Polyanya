#include "cuckoo_table.h"

warthog::cuckoo_table::cuckoo_table(unsigned int num_elements) :
   	max_buckets_(0), stored_elements_(0), f_(0), s_(0), verbose_(false)
{
	rehash(num_elements);
}

warthog::cuckoo_table::~cuckoo_table()
{
	delete [] f_;
	delete [] s_;
}

void
warthog::cuckoo_table::rehash(unsigned int num_elements)
{
	if(verbose_)
	{
		std::cerr << "verbose; cuckoo_table::rehash num_elements="<<num_elements<<"\n";
		this->metrics(std::cerr);
	}

	if(num_elements <= (max_buckets_*2*warthog::hash::BUCKET_SIZE))
	{
		std::cerr << "err; cuckoo_table::rehash "<<num_elements<<" <= " 
			<< max_buckets_*2*warthog::hash::BUCKET_SIZE<< std::endl;
		return;
	}

	// ensure table size is a power of two
	if(!(num_elements & (num_elements-1)))
	{
		num_elements = 1 << (int)ceil(log10(num_elements) / log10(2));
	}

	unsigned int oldmax_buckets = max_buckets_;
	warthog::cuckoo_bucket* oldf = f_;
	warthog::cuckoo_bucket* olds = s_;

	// create a larger table
	max_buckets_ = (num_elements >> warthog::hash::LOG2_BUCKET_SIZE)>>1;
	f_ = new cuckoo_bucket[max_buckets_];
	s_ = new cuckoo_bucket[max_buckets_];

	// rehash elements
	bool rehash_ok = true;
	for(unsigned int i=0; i < oldmax_buckets; i++)
	{
		for(unsigned int j=0; j < warthog::hash::BUCKET_SIZE; j++)
		{
			unsigned int value = oldf[i].get(j);
			if(value != UINT_MAX && !__insert(value))
			{
				rehash_ok = false;
				break;
			}
			value = olds[i].get(j);
			if(value != UINT_MAX && !__insert(olds[i].get(j)))
			{
				rehash_ok = false;
				break;
			}
		}
	}

	if(rehash_ok)
	{
		delete [] oldf;
		delete [] olds;
		if(verbose_)
		{
			std::cout << "rehash ok"<<std::endl;
		}
	}
	else
	{  
		// failed rehash; undo
		delete [] f_;
		delete [] s_;
		f_ = oldf;
		s_ = olds;
		max_buckets_ = oldmax_buckets;
		if(verbose_)
		{
			std::cout << "rehash failed"<<std::endl;
		}
	}
}

void
warthog::cuckoo_table::clear()
{
	for(unsigned int i=0; i < max_buckets_; i++)
	{
		f_[i].clear();
		s_[i].clear();
	}
	stored_elements_ = 0;
}
