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

#include "Point2.h"

#include "common.h"
#if GEOM_PSEUDO3D==GEOM_TRUE
	namespace GEOM_FADE25D {
#elif GEOM_PSEUDO3D==GEOM_FALSE
	namespace GEOM_FADE2D {
#else
	#error GEOM_PSEUDO3D is not defined
#endif


//#include "TrPoint.h"
/** \brief Segment
*
*/
class Segment2
{
protected:
	Point2 src,trg;
public:
/** Create a Segment2
*
* @param src First endpoint (source)
* @param trg Second endpoint (target)
*/
	CLASS_DECLSPEC
	Segment2(Point2 src,Point2 trg);
/** Create a Segment2
* Default constructor
*/
	CLASS_DECLSPEC
	Segment2();
/** Get the source point
*
* @return the source point
*/
	CLASS_DECLSPEC
	Point2 getSrc() const;
/** Get the target point
*
* @return the target point
*/
	CLASS_DECLSPEC
	Point2 getTrg() const;

/** Get the squared length
*
*
*/
	CLASS_DECLSPEC
	double getSqLen2D() const;

#if GEOM_PSEUDO3D==GEOM_TRUE
/** Get the squared length (2.5D version)
*
*
*/
	CLASS_DECLSPEC
	double getSqLen25D() const;
#endif


/**
 * Internally swaps the source and target point
 */
	CLASS_DECLSPEC
	void swapSrcTrg();

	CLASS_DECLSPEC
	friend std::ostream &operator<<(std::ostream &stream, Segment2 seg);


/** operator==
*
* Undirected equality operator
*/
	CLASS_DECLSPEC
	bool operator==(const Segment2& other) const;

};



/**
\cond HIDDEN_SYMBOLS
*/
// Comparator class
struct Func_compareSegment
{
	bool operator()(const Segment2* seg0,const Segment2* seg1) const
	{
		if(seg0->getSrc()<seg1->getSrc()) return true;
		if(seg0->getSrc()>seg1->getSrc()) return false;
		if(seg0->getTrg()<seg1->getTrg()) return true;
		if(seg0->getTrg()>seg1->getTrg()) return false;
		return false;
	}
};
/**
\endcond
*/
} // (namespace)
