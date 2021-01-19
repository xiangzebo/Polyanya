// (c) 2010 Geom e.U. Bernhard Kornberger, Graz/Austria. All rights reserved.
//
// This file is part of the Fade2D library. You can use it for your personal
// non-commercial research. Licensees holding a commercial license may use this
// file in accordance with the Commercial License Agreement provided
// with the Software.
//
// This software is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
// THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Please contact the author if any conditions of this licensing are not clear
// to you.
//
// Author: Bernhard Kornberger, bkorn (at) geom.at
// http://www.geom.at


#pragma once

#include "common.h"
#if GEOM_PSEUDO3D==GEOM_TRUE
	namespace GEOM_FADE25D {
#elif GEOM_PSEUDO3D==GEOM_FALSE
	namespace GEOM_FADE2D {
#else
	#error GEOM_PSEUDO3D is not defined
#endif

class Triangle2;
class Point2;

class Edge2
{
public:
	Edge2();
	Edge2(const Edge2& e_);
	Edge2(Triangle2* c_,int oppIdx_);
	~Edge2();

	bool operator<(const Edge2& e) const
	{
		if(c<e.c) return true;
		if(c>e.c) return false;
		if(oppIdx<e.oppIdx) return true;
		return false;
	}

	bool operator==(const Edge2& e) const
	{
		return(c==e.c && oppIdx==e.oppIdx);
	}

	bool operator!=(const Edge2& e) const
	{
		return((c!=e.c || oppIdx!=e.oppIdx));
	}

	void getPoints(Point2*& p0,Point2*& p1) const;
	void getTriangles(Triangle2*& c0,Triangle2*& c1);


	friend std::ostream &operator<<(std::ostream &stream, const Edge2& e);

protected:
	Triangle2* c;
	int oppIdx;
};


} // (namespace)
