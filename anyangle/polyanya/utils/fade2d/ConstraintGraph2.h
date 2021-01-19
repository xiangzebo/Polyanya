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
#include "Segment2.h"
#include "ConstraintSegment2.h"
#include "Edge2.h"
#include <map>


#include "common.h"
#if GEOM_PSEUDO3D==GEOM_TRUE
	namespace GEOM_FADE25D {
#elif GEOM_PSEUDO3D==GEOM_FALSE
	namespace GEOM_FADE2D {
#else
	#error GEOM_PSEUDO3D is not defined
#endif

class Dt2; // Forward
class ConstraintSegment2; // Forward
class GeomTest; // Forward


/** \brief ConstraintGraph2 is a set of enforced edges
*
* \see \ref createConstraint in the Fade2D class
*
* \image html crop_ignoreBike.jpg "Constraints in a triangulation"
* \image latex crop_ignoreBike.eps "Constraints in a triangulation" width=12cm
*/
class ConstraintGraph2
{
public:
/** \brief Internal use
 *
 * Constructor for one or more ConstraintSegment2 objects
*/
	CLASS_DECLSPEC
	ConstraintGraph2(
		Dt2* pDt2_,
		std::vector<ConstraintSegment2*>& vCSegments,
		ConstraintInsertionStrategy
		);

/** \brief Internal use
 *
 * Constructor for one or more ConstraintSegment2 objects whose
 * direction is given by mPPReverse. Used for bounded zones
*/
	CLASS_DECLSPEC
	ConstraintGraph2(
		Dt2* pDt2_,
		std::vector<ConstraintSegment2*>& vCSegments_,
		std::map<std::pair<Point2*,Point2*>,bool > mPPReverse,
		ConstraintInsertionStrategy cis_
	);

/** \brief Internal use
 *
 * Called by the two constructors
 */
void init(std::vector<ConstraintSegment2*>& vCSegments_);


/** \brief Does the constraint graph form a closed polygon?
*
* @note This method does not check if it is a simple polygon (one
* without self-intersections).
*/
	CLASS_DECLSPEC
	bool isPolygon() const;

/** \brief Are the segments of the constraint graph oriented?
*
* @return true if the constraint graph has been created with
* bOrientedSegments=true or if automatic reorientation was possible
* which is the case for simple polygons.
*/
	CLASS_DECLSPEC
	bool isOriented() const;

/** \brief Get the vertices of the constraint segments
*
* Use this method to retrieve the segments of *this in form of a vector
* of vertices. If *this is a closed polygon, then the points are ordered
* and oriented in counterclockwise direction, e.g. (a,b,b,c,c,d,d,a). If
* the *this is not a polygon, then the segments are returned in the
* original direction.
*
* @note If it was necessary to split the constraint segments, then the
* splitted segments are returned. If, in the above example, the constraint
* segment (a,b) crosses some previously inserted point x, then the
* result is (a,x,x,b,b,c,c,d,d,a).
*
*/
	CLASS_DECLSPEC
	void getPolygonVertices(std::vector<Point2*>& vTriangulationPoints_) ;

/** \brief Get the constraint insertion strategy
*
* \if SECTION_FADE25D
* @return CIS_CONFORMING_DELAUNAY, CIS_CONFORMING_DELAUNAY_SEGMENT_LEVEL or @n
* CIS_CONSTRAINED_DELAUNAY
* \else
* @return CIS_CONFORMING_DELAUNAY or CIS_CONSTRAINED_DELAUNAY
* \endif
*
*/
	CLASS_DECLSPEC
	ConstraintInsertionStrategy getInsertionStrategy() const;

/** \brief Check if an edge is a constraint
*
* Checks if the edge (p0,p1) is a constraint of *this
*/
	CLASS_DECLSPEC
	bool isConstraint(Point2* p0,Point2* p1) const;



/** \brief Visualization
*
*/
	CLASS_DECLSPEC
	void show(const std::string& name);



/** \brief Get the original ConstraintSegment2 objects
*
* Get the original, not subdivided ConstraintSegment2 objects. The
* ones which have been splitted are not alive anymore. But they
* have children (for which the same may hold).
*
*/
	CLASS_DECLSPEC
	void getOriginalConstraintSegments(std::vector<ConstraintSegment2*>& vConstraintSegments_) const;

/** \brief Update a constraint segment of *this
* Internal method
*/
	void updateSplittedConstraintSegment(ConstraintSegment2* pCSeg,bool bDirChange0,bool bDirChange1,ConstraintSegment2* pChild0,ConstraintSegment2* pChild1);
/**
* \return the Delaunay class it belongs to
*/
	Dt2* getDt2();
	void getAliveConstraintChain(std::vector<ConstraintSegment2*>& vAliveCSeg) ; // For debugging
protected:
	bool isReverse(ConstraintSegment2* pCSeg) const;
	bool checkAndSortPolygon(std::vector<ConstraintSegment2*>& vCSegments_);


	// Data
	Dt2* pDt2;
	GeomTest* pGeomPredicates;
	ConstraintInsertionStrategy cis;
	std::vector<ConstraintSegment2*> vCSegParents;
	bool bIsPolygon;
	std::map<ConstraintSegment2*,bool,func_ltDerefPtr<ConstraintSegment2*> > mCSegReverse;
	std::map<Point2*,size_t> mSplitPointNum;
	bool bIsOriented;
};

} // (namespace)
