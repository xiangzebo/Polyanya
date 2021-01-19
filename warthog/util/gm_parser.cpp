#include "gm_parser.h"

#include <tr1/unordered_map>

warthog::gm_parser::gm_parser(const char* filename)
{
	std::fstream mapfs(filename, std::fstream::in);
	if(!mapfs.is_open())
	{
		std::cerr << "err; gm_parser::gm_parser "
			"cannot open map file: "<<filename << std::endl;
		exit(1);
	}

	this->parse_header(mapfs);
	this->parse_map(mapfs);
	mapfs.close();
}

warthog::gm_parser::~gm_parser()
{
}

void 
warthog::gm_parser::parse_header(std::fstream& mapfs)
{
	// read header fields
	std::tr1::unordered_map<std::string, std::string> contents;
	for(int i=0; i < 3; i++)
	{
		std::string hfield, hvalue;
		mapfs >> hfield;
		if(mapfs.good())
		{
			mapfs >> hvalue;
			if(mapfs.good())
			{
				contents[hfield] = hvalue;
			}
			else
			{
				std::cerr << "err; map load failed. could not read header." << 
					hfield << std::endl;
				exit(1);
			}
		}
		else
		{
			std::cerr << "err;  map load failed. format looks wrong."<<std::endl;
			exit(1);
		}
	}

	this->header_.type_ = contents[std::string("type")];
	if(this->header_.type_.compare("octile") != 0)
	{
		std::cerr << "err; map type " << this->header_.type_ << 
			"is unknown. known types: octile "<<std::endl;;
		exit(1);
	}

	this->header_.height_ = atoi(contents[std::string("height")].c_str());
	if(this->header_.height_ == 0)
	{
		std::cerr << "err; map file specifies invalid height. " << std::endl;
		exit(1);
	}

	this->header_.width_ = atoi(contents[std::string("width")].c_str());
	if(this->header_.width_ == 0)
	{
		std::cerr << "err; map file specifies invalid width. " << std::endl;
		exit(1);
	}

}

void 
warthog::gm_parser::parse_map(std::fstream& mapfs)
{
	std::string hfield;
	mapfs >> hfield;
	if(hfield.compare("map") != 0)
	{
		std::cerr << "err; map load failed. missing 'map' keyword." 
			<< std::endl;
	}

	// read map data
	int index = 0;
	int max_tiles = this->header_.height_*this->header_.width_;
	while(true)
	{
		char c = mapfs.get();	
		if( !mapfs.good() )
		{
			// eof
			break;
		}

		if(c == ' ' || c == '\t' || c == '\n' || c == '\r')
		{
			// skip whitespace
			continue;
		}

		if(index >= max_tiles)
		{
			index++;
			continue;
		}

		this->map_.push_back(c);
		index++;
	}

	if(index != max_tiles)
	{
		std::cerr << "err; expected " << max_tiles
			<< " tiles; read " << index <<" tiles." << std::endl;
		exit(1);
	}
}

