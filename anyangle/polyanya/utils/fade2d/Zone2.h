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

class Dt2; // Fwd
class ConstraintGraph2; // Fwd
class Triangle2; // Fwd
class Point2; // Fwd
class Visualizer2; // Fwd
/** \brief Area in a triangulation.
*
*  \see \ref createZone in the Fade2D class
*
*/

class Zone2
{
public:
/**
 * internal use
 */
	CLASS_DECLSPEC
	Zone2(Dt2* pDt_,ZoneLocation zoneLoc_);
/**
 * internal use
 */
	CLASS_DECLSPEC
	Zone2(Dt2* pDt_,ZoneLocation zoneLoc_,ConstraintGraph2* pConstraintGraph_);
/**
 * internal use
 */
	Zone2(Dt2* pDt_,ZoneLocation zoneLoc_,const std::vector<ConstraintGraph2*>& vConstraintGraphs_);

/**
 * internal use
 */
	CLASS_DECLSPEC
	Zone2(Dt2* pDt_,const std::vector<ConstraintGraph2*>& vConstraintGraphs_,ZoneLocation zoneLoc_,std::vector<Point2>& vStartPoints);

/** \brief Get the zone location
*
* \returns ZL_INSIDE if the zone applies to the triangles inside one or more ConstraintGraph2 objects@n
* ZL_OUTSIDE if the zone applies to the outside triangles@n
* ZL_GLOBAL if the zone applies (dynamically) to all triangles@n
* ZL_RESULT if the zone is the result of a set operation@n
* ZL_GROW if the zone is specified by a set of constraint graphs and an inner point@n
* \image html in_and_outside_zone.jpg "An ouside zone and in inside zone"
* \image latex in_and_outside_zone.eps "An ouside zone and in inside zone" width=12cm
*/
	CLASS_DECLSPEC
	ZoneLocation getZoneLocation() const;

/** \brief Convert a zone to a bounded zone
*
* \anchor convertToBoundedZone
* The mesh generation algorithms refine() and refineAdvanced() require
* a zone object that is bounded by constraint segments. This is always
* the case for zones with zoneLocation ZL_INSIDE but other types of
* zones may be unbounded. For convenience this method is provided to
* create a bounded zone from a possibly unbounded one.
*
* @return a pointer to a new Zone2 object with zoneLocation ZL_RESULT_BOUNDED
* or @e this if this->getZoneLocation() is ZL_INSIDE.
*/
	CLASS_DECLSPEC
	Zone2* convertToBoundedZone();

/** \brief Show: A visualization method for zones
*
* @param postscriptFilename is the name of the output file.
* @param bShowFull specifies if only the zone or the full triangulation shall be drawn
* @param bWithConstraints specifies if constraint edges shall be drawn
*
*/
	CLASS_DECLSPEC
	void show(const std::string& postscriptFilename,bool bShowFull,bool bWithConstraints) const;

/** A visualization method for zones
*
* @param pVisualizer is a pointer to an existing Visualizer2 object.
* @note You must call pVisualizer->writeFile() before the program exits.
* @param bShowFull specifies if only the zone or the full triangulation shall be drawn
* @param bWithConstraints specifies if constraint edges shall be drawn
*/
	CLASS_DECLSPEC
	void show(Visualizer2* pVisualizer,bool bShowFull,bool bWithConstraints) const;

/**
 * internal use
 */
	CLASS_DECLSPEC
	bool assignDt2(Dt2* pDt_);
/** \brief Returns the triangles of the zone.
*
* @note Fade_2D::void applyConstraintsAndZones() must be called after the last
* insertion of points and constraints. Otherwise the result won't be valid.
*/
	CLASS_DECLSPEC
	void getTriangles(std::vector<Triangle2*>& vTriangles_) const;
/** \brief Get the associated constraint
* @return a pointer to the ConstraintGraph2 object which defines the zone.@n
* or NULL for ZL_RESULT-, ZL_GROW and ZL_GLOBAL_-zones.
*/
	CLASS_DECLSPEC
	ConstraintGraph2* getConstraintGraph() const;
	void getConstraintGraphs(std::vector<ConstraintGraph2*>& vConstraintGraphs_) const;
/** \brief Get a pointer to the associated Delaunay triangulation
*/
	CLASS_DECLSPEC
	Dt2* getDelaunayTriangulation() const;

/** \brief Get a the number of ConstraintGraph2 objects
*
* A Zone2 object might be defined by zero, one or more ConstraintGraph2 objects.
*/
	CLASS_DECLSPEC
	size_t numberOfConstraintGraphs() const;

/** \brief Development function
 */
	void debug(std::string name="");


/** \brief Get Area
 *
 * Returns the area of the zone.
 *
 * @note Make sure applyConstraintsAndZones() has been called to
 * establish the constraint edges.
 */
	double getArea() const;
protected:
	Zone2(const Zone2&);
	void getTriangles_RESULT(std::vector<Triangle2*>& vTriangles) const;
	void initWorkspace(bool bInside,std::vector<Triangle2*>& vWorkspace) const;
	void bfsFromWorkspace(std::vector<Triangle2*>& vWorkspace,std::vector<Triangle2*>& vTriangles) const;

	Zone2* ctbz_treatCC(std::vector<Triangle2*>& vOneCC);

	// Data
	Dt2* pDt;
	ZoneLocation zoneLoc;
	//Zone2* pInputZone0;
	//Zone2* pInputZone1;
	std::vector<ConstraintGraph2*> vConstraintGraphs;
	std::vector<Point2> vStartPoints;
	std::vector<Zone2*> vInputZones;
	CLASS_DECLSPEC
	friend Zone2* zoneUnion(Zone2* pZone0,Zone2* pZone1);
	CLASS_DECLSPEC
	friend Zone2* zoneIntersection(Zone2* pZone0,Zone2* pZone1);
	CLASS_DECLSPEC
	friend Zone2* zoneDifference(Zone2* pZone0,Zone2* pZone1);
	CLASS_DECLSPEC
	friend Zone2* zoneSymmetricDifference(Zone2* pZone0,Zone2* pZone1);

};

// Free functions

/** \brief Compute the union of two zones
* @return a new zone containing the union of the triangles of *pZone0 and *pZone1
*/
CLASS_DECLSPEC
Zone2* zoneUnion(Zone2* pZone0,Zone2* pZone1);
/** \brief Compute the intersection of two zones
* @return a new zone containing the intersection of *pZone0 and *pZone1
*/
CLASS_DECLSPEC
Zone2* zoneIntersection(Zone2* pZone0,Zone2* pZone1);
/** \brief Compute the difference of two zones
* @return a new zone containing the triangles of *pZone0 minus the ones of *pZone1
*/
CLASS_DECLSPEC
Zone2* zoneDifference(Zone2* pZone0,Zone2* pZone1);
/** \brief Compute the symmetric difference of two zones
* @return a new zone containing the triangles that are present in one of
* the zones but not in the other one.
*/
CLASS_DECLSPEC
Zone2* zoneSymmetricDifference(Zone2* pZone0,Zone2* pZone1);





} // (namespace)
