#include "Entrance.h"

Entrance::Entrance() :
	x(0), y(0), length(0), type(VERTICAL)
{
}

Entrance::Entrance(int x_, int y_, int length_, EntranceType type_)
	: x(x_), y(y_), length(length_), type(type_)
{
}

Entrance::~Entrance()
{
}
