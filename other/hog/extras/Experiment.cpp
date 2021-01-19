#include "Experiment.h"

#include <ostream>
#include <iomanip>
#include <iostream>


Experiment::Experiment(int sx,int sy,int gx,int gy,int b, double d, string m)
	: startx(sx),starty(sy),goalx(gx),goaly(gy),scaleX(kNoScaling),
	scaleY(kNoScaling),bucket(b),distance(d),map(m),precision(4)
{
}

Experiment::Experiment(int sx,int sy,int gx,int gy,int sizeX, int sizeY,int b, 
		double d, string m)
	: startx(sx),starty(sy),goalx(gx),goaly(gy),scaleX(sizeX),
	scaleY(sizeY),bucket(b),distance(d),map(m),precision(4)
{
}

Experiment::~Experiment()
{
}

void Experiment::print(std::ostream& out)
{
	out << this->bucket << "\t";
	out << this->getMapName() <<"\t";
	out << this->getXScale() << "\t";
	out << this->getYScale() << "\t";
	out << this->getStartX() <<"\t";
	out << this->getStartY()<<"\t";
	out << this->getGoalX()<<"\t";
	out << this->getGoalY()<<"\t";
	out << std::fixed << std::setprecision(this->getPrecision());
	out << this->getDistance();
}
