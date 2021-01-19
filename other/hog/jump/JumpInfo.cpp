#include "JumpInfo.h"

#include "constants.h"

#include <ostream>


JumpInfo::JumpInfo()
{ 
}

JumpInfo::~JumpInfo()  
{ 
}

JumpInfo::JumpInfo(JumpInfo& other)
{
	for(unsigned int i=0; i < other.nodecount(); i++)
	{
		this->jumpdirs.push_back(other.jumpdirs.at(i));
		this->jumpnodes.push_back(other.jumpnodes.at(i));
		this->jumpcosts.push_back(other.jumpcosts.at(i));
	}
}

void
JumpInfo::print(std::ostream& out)
{
	out << "JumpInfo: " << std::endl;
	for(unsigned int i=0; i < nodecount(); i++)
	{
		node* n = jumpnodes.at(i);
		int x = n->getLabelL(kFirstData);
		int y = n->getLabelL(kFirstData+1);
		double cost = jumpcosts.at(i);
		Jump::Direction dir = jumpdirs.at(i);

		out << "("<< x <<", "<< y <<") jumpcost: "<<cost << " lastdir: ";
		switch(dir)
		{
			case Jump::N:
				out << "N";
				break;
			case Jump::S:
				out << "S";
				break;
			case Jump::E:
				out << "E";
				break;
			case Jump::W:
				out << "W";
				break;
			case Jump::NE:
				out << "NE";
				break;
			case Jump::SE:
				out << "SE";
				break;
			case Jump::SW:
				out << "SW";
				break;
			case Jump::NW:
				out << "NW";
				break;
			case Jump::NONE:
				out << "NONE";
				break;
		}
		out << std::endl;
	}

}

