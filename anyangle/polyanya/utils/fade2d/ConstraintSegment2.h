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
#include <set>

#include "common.h"
#if GEOM_PSEUDO3D==GEOM_TRUE
	namespace GEOM_FADE25D {
#elif GEOM_PSEUDO3D==GEOM_FALSE
	namespace GEOM_FADE2D {
#else
	#error GEOM_PSEUDO3D is not defined
#endif

class Point2; // FWD
class ConstraintGraph2; // FWD


/** \brief Constraint Insertion Strategy
* determines how a constraint edge shall be inserted:
*
* - CIS_CONSTRAINED_DELAUNAY inserts a segment without subdivision
* unless required (which is the case if existing vertices or constraint
* segments are crossed).
* - CIS_CONFORMING_DELAUNAY subdivides a segment if necessary to
* maintain the empty circle property of the surrounding triangles.
* In general this insertion strategy creates more but better shaped
* triangles than CIS_CONSTRAINED_DELAUNAY.
* \if SECTION_FADE25D
* The height (z-coordinate) of new split points is adapted to existing
* triangles.
* - CIS_CONFORMING_DELAUNAY_SEGMENT_LEVEL is the same as the strategy
* CIS_CONFORMING_DELAUNAY except that split points are not projected
* to an existing surface but their height is interpolated between the
* endpoints of the segment to be inserted.
* \endif
*
* \note In former library versions the terms CIS_IGNORE_DELAUNAY
* and CIS_KEEP_DELAUNAY were used but these were misleading and
* are now deprecated. For backwards compatibility they are kept.
*/
enum ConstraintInsertionStrategy
{
	CIS_CONFORMING_DELAUNAY=0,
	CIS_CONSTRAINED_DELAUNAY=1,
#if GEOM_PSEUDO3D==GEOM_TRUE
	CIS_CONFORMING_DELAUNAY_SEGMENT_LEVEL=2,
#endif
	CIS_KEEP_DELAUNAY=0, // Deprecated name
	CIS_IGNORE_DELAUNAY=1 // Deprecated name
};

/** \brief Constraint segment
*/
class ConstraintSegment2
{
public:
	ConstraintSegment2(Point2* p0_,Point2* p1_,ConstraintInsertionStrategy cis_);
	~ConstraintSegment2();
/** \brief Get the source point
* \return the source point
*/
	CLASS_DECLSPEC
	Point2* getSrc() const;
/** \brief Get the target point
* \return the source point
*/
	CLASS_DECLSPEC
	Point2* getTrg() const;
/**
* \return if the object is alive
*/
	CLASS_DECLSPEC
	bool isAlive() const;
/**
* \return the constraint insertion strategy (CIS)
*/
	ConstraintInsertionStrategy getCIS() const;

/** \brief operator<(..)
* Compares by vertex pointers
*/

	CLASS_DECLSPEC
	bool operator<(const ConstraintSegment2& pOther) const;

/** \brief Split a constraint segment
*
* internal use.
*/
	CLASS_DECLSPEC
	bool split(Point2* pSplit);
/** \brief Split a constraint segment
*
* internal use
*/
	CLASS_DECLSPEC
	bool splitAndRemovePrev(Point2* pSplit);
/** \brief Add an owner
*
* Sets a specific ConstraintGraph2 as owner of the current ConstraintSegment2.
* internal use.
*/
	void addOwner(ConstraintGraph2* pOwner);
/** \brief Remove an owner
* Removes a specific ConstraintGraph2 as owner of the current ConstraintSegment2.
* Mostly for internal use.
*/
	void removeOwner(ConstraintGraph2* pOwner);
	CLASS_DECLSPEC
/** \brief Get all children
* Recursively retrieve all children of the current ConstraintSegment2.
*/
	void getChildrenRec(std::vector<ConstraintSegment2*>& vChildConstraintSegments);
	CLASS_DECLSPEC
/** \brief Get the children and the split point
* Retrieve the two direct children of the current ConstraintSegment2 as well as the split point.
*/
	void getChildrenAndSplitPoint(ConstraintSegment2*& pCSeg0,ConstraintSegment2*& pCSeg1,Point2*& pSplitPoint);
	CLASS_DECLSPEC
	friend std::ostream &operator<<(std::ostream &stream, const ConstraintSegment2& cSeg);
/** \brief Return the number of owners
* Returns the number of ConstraintGraphs where the current ConstraintSegment2 is a member
*/
	CLASS_DECLSPEC
	size_t getNumberOfOwners() const;


/** \brief Set adjacent area
* A customer specific method, not thought for public use. If the
* current constraint segment is a border segment, then the area of
* the (non-existing) outside triangle can manually be deposited
* here. This value is used by the extended meshing method when a
* grow factor is given or ignored if not set.
* \note Only use if you know what you are doing.
*/
	CLASS_DECLSPEC
	void setAdjacentArea(double adjacentArea_);

/** \brief Get adjacent area
* A customer specific method, not thought for public use. Returns
* a previously via setAdjacentArea(double adjacentArea_) deposited
* value. If not explicitly set, DBL_MAX is returned.
*/
	CLASS_DECLSPEC
	double getAdjacentArea() const;


protected:
	Point2 *p0,*p1;
	ConstraintInsertionStrategy cis;
	std::set<ConstraintGraph2*> sOwners;
	std::vector<ConstraintSegment2*> vChildren;
	double adjacentArea;
};

} // NAMESPACE
