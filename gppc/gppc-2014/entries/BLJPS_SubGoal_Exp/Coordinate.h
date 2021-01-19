#pragma once
#include <algorithm>
typedef struct Coordinate
{
	short x,y;
	Coordinate()
	{
	}
	Coordinate(short _x,short _y) : x(_x),y(_y)
	{
	}
	float dist(const Coordinate& rhs)
	{
		int absX =abs(x-rhs.x);
		int absY =abs(y-rhs.y);

		int diagDist = std::min(absX,absY);
		int straightDist = std::max(absX, absY) - diagDist;
		return diagDist*1.414213562373095f+straightDist;
	}
	float distSqrt(const Coordinate& rhs)
	{
		int absX =abs(x-rhs.x);
		int absY =abs(y-rhs.y);

		int diagDist = std::min(absX,absY);
		int straightDist = std::max(absX,absY)-diagDist;
		return diagDist*1.414213562373095f+straightDist;
	}
	void add(const Coordinate& rhs)
	{
		x+=rhs.x;
		y+=rhs.y;
	}

} xyLoc;
