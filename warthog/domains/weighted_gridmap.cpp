#include "gm_parser.h"
#include "weighted_gridmap.h"

#include <cassert>
#include <cstring>

warthog::weighted_gridmap::weighted_gridmap(unsigned int h, unsigned int w)
	: header_(h, w, "octile")
{	
	this->init_db();
}

warthog::weighted_gridmap::weighted_gridmap(const char* filename)
{
	strcpy(filename_, filename);
	warthog::gm_parser parser(filename);
	this->header_ = parser.get_header();

	init_db();
	// populate matrix
	for(unsigned int i = 0; i < parser.get_num_tiles(); i++)
	{
		char c = parser.get_tile_at(i);
		switch(c)
		{
            // explicit obstacle
			case '@':  
				this->set_label(to_padded_id(i), 0);
				assert(this->get_label(to_padded_id(i)) == 0);
				break;
            // other tiles have terrain cost equal to their ascii value
			default: 
				this->set_label(to_padded_id(i), c);
				assert(this->get_label(to_padded_id(i)) == c);
				break;
		}
	}
}

void
warthog::weighted_gridmap::init_db()
{
	// when storing the grid we pad the edges of the map.
	// this eliminates the need for bounds checking when
	// fetching the neighbours of a node. 
	this->padded_rows_before_first_row_ = 2;
	this->padded_rows_after_last_row_ = 2;
	this->padding_per_row_ = 1;

	this->padded_width_ = this->header_.width_ + this->padding_per_row_;
	this->padded_height_ = this->header_.height_ + 
		this->padded_rows_after_last_row_ +
		this->padded_rows_before_first_row_;

	this->db_size_ = this->padded_height_ * padded_width_;

	// create a one dimensional dbword array to store the grid
	this->db_ = new warthog::dbword[db_size_];
	for(unsigned int i=0; i < db_size_; i++)
	{
		db_[i] = 0; 
	}
}

warthog::weighted_gridmap::~weighted_gridmap()
{
	delete [] db_;
}

void 
warthog::weighted_gridmap::print(std::ostream& out)
{
	out << "printing padded map" << std::endl;
	out << "-------------------" << std::endl;
	out << "type "<< header_.type_ << std::endl;
	out << "height "<< this->height() << std::endl;
	out << "width "<< this->width() << std::endl;
	out << "map" << std::endl;
	for(unsigned int y=0; y < this->height(); y++)
	{
		for(unsigned int x=0; x < this->width(); x++)
		{
            warthog::dbword c = this->get_label(y*this->width()+x);
			out << (c == 0 ? '@': (char)c);
		}
		out << std::endl;
	}	
}
