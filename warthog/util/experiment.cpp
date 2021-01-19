#include "experiment.h"

#include <iomanip>

void
warthog::experiment::print(std::ostream& out)
{
	out << this->map() <<"\t";
	out << this->mapwidth() << "\t";
	out << this->mapheight() << "\t";
	out << this->startx() <<"\t";
	out << this->starty()<<"\t";
	out << this->goalx()<<"\t";
	out << this->goaly()<<"\t";
	out << std::fixed << std::setprecision(this->precision());
	out << this->distance();
}
