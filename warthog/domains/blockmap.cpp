#include "blockmap.h"
#include "gm_parser.h"
#include "gridmap.h"

warthog::blockmap::blockmap(const char* filename)
{
	warthog::gm_parser parser(filename);
	this->header_ = new gm_header(parser.get_header());

	blocks_ = new gridmap**[this->dbwidth()];
	for(unsigned int i=0; i < this->dbwidth(); i++)
	{
		blocks_[i] = new gridmap*[this->dbheight()];
		for(unsigned int j=0; j < this->dbheight(); j++)
		{
			unsigned int blockwidth = BLOCKSIZE;
			if((i*BLOCKSIZE+blockwidth) > header_->width_)
			{
				blockwidth = header_->width_ % BLOCKSIZE;
			}

			unsigned int blockheight = BLOCKSIZE;
			if((i*BLOCKSIZE+blockheight) > header_->height_)
			{
				blockheight = header_->height_ % BLOCKSIZE;
			}

			blocks_[i][j] = new gridmap(blockheight, blockwidth);
		}
	}

	// set terrain type of each tile
	for(unsigned int i=0; i < parser.get_num_tiles(); i++)
	{
		char c = parser.get_tile_at(i);
		unsigned int x = i % this->width();
		unsigned int y = i / this->width();

		switch(c)
		{
			case 'S':
			case 'W': 
			case 'T':
			case '@':
			case 'O':
				this->set_label(x, y, false); // obstacle
				break;
			default:
				this->set_label(x, y, true); // traversable
				break;
		}
	}
}

warthog::blockmap::~blockmap()
{
	for(unsigned int i = 0; i < this->dbwidth(); i++)
	{
		for(unsigned int j = 0; j < this->dbheight(); j++)
		{
			delete blocks_[i][j];
		}
		delete [] blocks_[i];
	}
	delete [] blocks_;
	delete header_;
}

void 
warthog::blockmap::print(std::ostream& out)
{
	for(unsigned int y = 0; y < this->height(); y++)
	{
		for(unsigned int x = 0; x < this->width(); x++)
		{
			out << this->get_label(x, y);
		}
		out << std::endl;
	}
}
