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

/** \brief Triangle
*
* Triangle2 is a triangle in the Fade_2D triangulation. It holds three Point2
* pointers to its corners. The corners are numbered in counterclockwise order.
* We refer to these indices as intra-triangle-indices.
*
* Each triangle has three neighbors which can be accessed through intra-triangle-indices:
* The i-th neighbor triangle of a certain triangle T is the one which shares an edge with T such
* that this edge does not include the i-th corner of T.
*
* \image html neig.jpg "Figure 1: Indices and neighborships, \e tb is the 0-th neighbor of \e ta and \e ta is the 2nd neighbor of \e tb."
* \image latex neig.eps "Indices and neighborships, \e tb is the 0-th neighbor of \e ta and \e ta is the 2nd neighbor of \e tb." width=6cm
*
* \see TriangleAroundVertexIterator to find out how to access all triangles incident
* to a certain vertex.
*/
class CLASS_DECLSPEC  Triangle2
{
public:
/** \brief Constructor
*
*/
	Triangle2();



/** \brief Get the \e i-th corner of the triangle
*
* \return a pointer to the i-th corner point of the triangle.
*
* \image html corners.jpg "Figure 2: Intra triangle indices are ordered counterclockwise"
* \image latex corners.eps "Intra triangle indices are ordered counterclockwise" width=4cm
*
* @param ith is the intra-triangle-index, ith={0,1,2}.
*/
	Point2* getCorner(const int ith) const;

#if GEOM_PSEUDO3D==GEOM_TRUE
/** \brief Get the dual Voronoi vertex
*
* @return a std::pair<Point2,bool>, where the first component is the dual Voronoi vertex
* of the triangle and the second component is a boolean value which is true if the vertex
* is accurate. The z-coordinate of the returned point is always 0. Use Fade_2D::getHeight(..)
* to determine the height value.
*
* @note The true dual Voronoi vertex of an almost collinear Delaunay triangle
* can be outside the bounds of floating point arithmetic. In such cases this
* method returns a point with very large coordinates but still inside the
* range of double precision floating point arithmetic, and it will inform
* the user by setting the boolean return value to false.
*
* @note Such cases can easily be avoided by insertion of four dummy vertices
* around the triangulation, e.g., at coordinates ten times larger than the
* domain of the data points. This will automatically restrict the Voronoi diagram
* of the data points to this range.
*
*
*/
std::pair<Point2,bool> getDual() const;
#else
/** \brief Get the dual Voronoi vertex
*
* @return a std::pair<Point2,bool>, where the first component is the dual Voronoi vertex
* of the triangle and the second component is a boolean value which is true if the vertex
* is accurate.
*
* @note The true dual Voronoi vertex of an almost collinear Delaunay triangle
* can be outside the bounds of floating point arithmetic. In such cases this
* method returns a point with very large coordinates but still inside the
* range of double precision floating point arithmetic, and it will inform
* the user by setting the boolean return value to false.
*
* @note Such cases can easily be avoided by insertion of four dummy vertices
* around the triangulation, e.g., at coordinates ten times larger than the
* domain of the data points. This will automatically restrict the Voronoi diagram
* of the data points to this range.
*/
std::pair<Point2,bool> getDual() const;
#endif

/** \brief Get the barycenter of a triangle
*
* @return the barycenter of the triangle.
*/
	Point2 getBarycenter() const;


#if GEOM_PSEUDO3D==GEOM_TRUE
/** \brief Get the normal vector of a triangle
*
* @return the normalized normal vector
*/
	Vector2 getNormalVector() const;
#endif

/** \brief Get the area of a triangle
*/
double getArea() const;

/** \brief Get the \e i-th neighbor triangle
*
* Returns the \e i-th neighbor triangle, i.e. the one opposite to the
* \e i-th corner.
* \image html getNeig.jpg "Figure 3: Neighbors of a triangle"
* \image latex getNeig.eps "Neighbors of a triangle" width=6cm
*
*
* @param ith is the intra-triangle-index of the opposite corner of @e *this
* @return the i-th neighbor triangle, i.e. the one opposite to the
* i-th vertex or NULL if no neighbor triangle exists which is the case
* at the convex hull edges of the triangulation.
*/
	Triangle2* getOppositeTriangle(const int ith) const;


/** \brief Get the index of \e p in the triangle
*
* \image html getITI.jpg "Figure 4: Intra triangle index of a vertex pointer" width=10cm
* \image latex getITI.eps "Intra triangle index of a vertex pointer" width=4cm
*
* @param p is a pointer to a vertex in @e *this
*
* @return the intra-triangle-index 0,1 or 2 of @e p in @e *this
*
*
*/
	int getIntraTriangleIndex(const Point2* p) const;
/** \brief Get the neighbor index of \e pTriangle
*
*
* \image html getITI_t.jpg "Figure 5: \e pTriangle is the 0-th neighbor of \e *this"
* \image latex getITI_t.eps "pTriangle is the 0-th neighbor of *this" width=8cm
*
*
* @param pTriangle is a neighbor triangle of *this.
*
* @return the intra-triangle-index of the vertex in \e *this which is opposite (i.e., does not
* touch the neighbor) \e pTriangle.
*/
	int getIntraTriangleIndex(const Triangle2* pTriangle) const;
/** \brief Method for internal use
 *
 *
 *  Internal use
*/
	bool getState() const;

/** \brief Squared edge length
*
* Returns the squared length of the \e ith edge.
* \if SECTION_FADE25D
* This method ignores the z-coordinate.
* \endif
*/
	double getSquaredEdgeLength(int ith) const;

/** \brief Method for internal use
 *
 * Internal use
*/
	void setState(bool bState_);
/** \brief Set the \e i-th neighbor triangle
*
* \image html getITI_t.jpg "Figure 6: Make @e pTriangle the 0-th neighbor of @e *this"
* \image latex getITI_t.eps "Make pTriangle the 0-th neighbor of *this" width=8cm
*
*
* @param ith is the index of the corner of \e *this which does not touch \e pTriangle
* @param pTriangle is a pointer to the triangle which shares two corners with \e *this
*/
	void setOppTriangle(const int ith, Triangle2* pTriangle);
/** \brief Set all corners
*/
	void setProperties( Point2* pI, Point2* pJ, Point2* pK);
/** \brief Set the \e i-th corner
*/
	void setVertexPointer(const int ith, Point2* pp);

	bool hasVertex(Point2* pVtx) const;

	// DEBUG
	CLASS_DECLSPEC
	friend std::ostream &operator<<(std::ostream &stream, const Triangle2& c);

protected:
	Point2* aVertexPointer[3];
	Triangle2* aOppTriangles[3];
	bool bState;
};

namespace{
inline bool checkRange(int ith)
{
	if(ith==0 || ith==1 || ith==2) return true;
	return false;
}

}
inline Triangle2* Triangle2::getOppositeTriangle(const int ith) const
{
	assert(checkRange(ith));
	return aOppTriangles[ith];
}

inline void Triangle2::setOppTriangle(const int ith, Triangle2* pNeig)
{
	assert(checkRange(ith));
	aOppTriangles[ith]=pNeig;
}

inline bool Triangle2::getState() const
{
	return bState;
}

inline void Triangle2::setState(bool bState_)
{
	bState=bState_;
}

inline int Triangle2::getIntraTriangleIndex(const Point2* pp) const
{
	if(aVertexPointer[0]==pp) return 0;
	if(aVertexPointer[1]==pp) return 1;
#ifndef NDEBUG
	if(aVertexPointer[2]!=pp)
	{
		std::cout<<"BUG: Triangle2::getIntraTriangleIndex failed for"<<std::endl;
		std::cout<<*pp<<std::endl;
		std::cout<<*this<<std::endl;
		assert(false);
	}
#endif

	return 2;
}

inline int Triangle2::getIntraTriangleIndex(const Triangle2* pTriangle) const
{
	if(getOppositeTriangle(0)==pTriangle) return 0;
	if(getOppositeTriangle(1)==pTriangle) return 1;

#ifndef NDEBUG
	if(getOppositeTriangle(2)!=pTriangle)
	{
		std::cout<<"Triangle2::getIntraTriangleIndex, pTriangle is not a neighbor of the current triangle"<<std::endl;
		std::cout<<"Current triangle: "<<*this<<std::endl;
		std::cout<<"pTriangle: "<<*pTriangle<<std::endl;
		assert(false);
	}
#endif
	return 2;
}

inline Point2* Triangle2::getCorner(const int ith) const
{
	assert(checkRange(ith));
	return aVertexPointer[ith];
}

inline void Triangle2::setVertexPointer(const int ith, Point2* pp)
{
	aVertexPointer[ith]=pp;
}

inline void Triangle2::setProperties( Point2* pI, Point2* pJ, Point2* pK)
{
	assert((pI!=NULL && pJ!=NULL && pK!=NULL));
	aVertexPointer[0]=pI;
	aVertexPointer[1]=pJ;
	aVertexPointer[2]=pK;
	pI->setIncidentTriangle(this);
	pJ->setIncidentTriangle(this);
	pK->setIncidentTriangle(this);
	aOppTriangles[0]=NULL;
	aOppTriangles[1]=NULL;
	aOppTriangles[2]=NULL;
	bState=false;

}







} // (namespace)
