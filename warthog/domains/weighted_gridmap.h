#ifndef WARTHOG_WEIGHTED_GRIDMAP_H
#define WARTHOG_WEIGHTED_GRIDMAP_H

// weighted_gridmap.h
//
// A weighted_gridmap implementation. We store the map as a flat
// array and represent individual cells using single byte quantities.
// We add some padding around the map to speed up access operations:
//  - a terminator character is added to indicate end-of-row.
//  - a line of terminator characters are added before the first row.
//  - a line of terminator characters are added after the last row.
//
// @author: dharabor
// @created: 2014-09-10
// 

#include "constants.h"
#include "helpers.h"
#include "gm_parser.h"

#include <climits>
#include <stdint.h>

namespace warthog
{

class weighted_gridmap
{
	public:
		weighted_gridmap(uint32_t height, unsigned int width);
		weighted_gridmap(const char* filename);
		~weighted_gridmap();
        
		// here we convert from the coordinate space of 
		// the original grid to the coordinate space of db_. 
		inline uint32_t
		to_padded_id(uint32_t node_id)
		{
			return node_id + 
				// padded rows before the actual map data starts
				padded_rows_before_first_row_*padded_width_ +
			   	// padding from each row of data before this one
				(node_id / header_.width_) * padding_per_row_;
		}

		inline uint32_t
		to_padded_id(uint32_t x, uint32_t y)
		{
			return to_padded_id(y * this->header_width() + x);
		}

		inline void
		to_unpadded_xy(uint32_t padded_id, uint32_t& x, uint32_t& y)
		{
			padded_id -= padded_rows_before_first_row_* padded_width_;
			y = padded_id / padded_width_;
			x = padded_id % padded_width_;
		}

        // read all tiles in the 3x3 square centred on @param db_id
        // return: 
        //  @param ids stores the ids of all tiles
        //  @param costs stores the terrain cost of all tiles
		inline void
		get_neighbours(uint32_t db_id, uint32_t ids[9], warthog::dbword costs[9])
		{
            // calculate ids of all adjacent neighbours
            ids[4] = db_id;
            ids[3] = db_id-1; // west
            ids[5] = db_id+1; // east
            ids[1] = db_id - this->padded_width_; // north
            ids[7] = db_id + this->padded_width_; // south
            ids[0] = ids[3] - this->padded_width_; // northwest
            ids[2] = ids[5] - this->padded_width_; // northeast
            ids[6] = ids[3] + this->padded_width_; // southwest
            ids[8] = ids[5] + this->padded_width_; // southeast

            // read terrain costs
            costs[0] = db_[ids[0]];
            costs[1] = db_[ids[1]];
            costs[2] = db_[ids[2]];
            costs[3] = db_[ids[3]];
            costs[4] = db_[ids[4]];
            costs[5] = db_[ids[5]];
            costs[6] = db_[ids[6]];
            costs[7] = db_[ids[7]];
            costs[8] = db_[ids[8]];
		}

		// get the label associated with the padded coordinate pair (x, y)
		inline warthog::dbword
		get_label(uint32_t x, unsigned int y)
		{
			return this->get_label(y*padded_width_+x);
		}

		inline warthog::dbword
		get_label(uint32_t padded_id)
		{
            return db_[padded_id];
		}

        inline warthog::dbword*
        get_label_ptr(uint32_t padded_id)
        {
            return &db_[padded_id];
        }

		// set the label associated with the padded coordinate pair (x, y)
		inline void
		set_label(uint32_t x, unsigned int y, warthog::dbword label)
		{
			this->set_label(y*padded_width_+x, label);
		}

		inline void 
		set_label(uint32_t padded_id, warthog::dbword label)
		{
            db_[padded_id] = label;
		}

		inline uint32_t 
		height() const
		{ 
			return this->padded_height_;
		} 

		inline uint32_t 
		width() const 
		{ 
			return this->padded_width_;
		}

		inline uint32_t 
		header_height()
		{
			return this->header_.height_;
		}

		inline uint32_t 
		header_width()
		{
			return this->header_.width_;
		}

		inline const char*
		filename()
		{
			return this->filename_;
		}

		void 
		print(std::ostream&);
		
		uint32_t 
		mem()
		{
			return sizeof(*this) +
			sizeof(warthog::dbword) * db_size_;
		}


	private:
		char filename_[256];
		warthog::gm_header header_;

		warthog::dbword* db_;

        uint32_t db_size_;
        uint32_t padding_per_row_;
		uint32_t padded_rows_before_first_row_;
		uint32_t padded_rows_after_last_row_;
        uint32_t padded_width_;
        uint32_t padded_height_;

		weighted_gridmap(const warthog::weighted_gridmap& other) {}
		weighted_gridmap& operator=(const warthog::weighted_gridmap& other) { return *this; }
		void init_db();
};

}

#endif

