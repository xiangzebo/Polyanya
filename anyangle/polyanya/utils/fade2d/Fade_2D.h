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
#include "Point2.h"
#include "Triangle2.h"
#include "TriangleAroundVertexIterator.h"
#include "Visualizer2.h"
#include "Zone2.h"
#include "ConstraintGraph2.h"
#include "Performance.h"
#include "MeshGenParams.h"
#include "MsgBase.h"
#include "SegmentChecker.h"
#include "testDataGenerators.h"

#if GEOM_PSEUDO3D==GEOM_TRUE
	#include "IsoContours.h"
#endif



#if GEOM_PSEUDO3D==GEOM_TRUE
	namespace GEOM_FADE25D {
#elif GEOM_PSEUDO3D==GEOM_FALSE
	namespace GEOM_FADE2D {
#else
	#error GEOM_PSEUDO3D is not defined
#endif



class Dt2; // Forward



/** \brief Delaunay triangulation - the main class
*
* Fade_2D represents a Delaunay triangulation in 2D or 2.5D (depends on the used namespace)
*/
class CLASS_DECLSPEC Fade_2D
{
public:
/** \brief Constructor of the main triangulation class
*
* @param numExpectedVertices specifies the number of points that will probably be inserted.
*
* By default, the internal data structures are initialized for 1000 points and
* they are automatically reconfigured later if necessary. Specifying \e numExpectedVertices
* saves memory but is not required.
*/

	Fade_2D(unsigned numExpectedVertices=1000)
	{
		initFade(numExpectedVertices);
	}


	~Fade_2D();



/** \brief Checks if a triangulation is valid
*
* This debug method checks the validity of the data structure. If \e bCheckEmptyCircleProperty
* is true, then (slow!) multiprecision arithmetic is used to re-check the empty circle property.
* A debug string can be added as optional parameter msg. Don't call this method unless you
* assume that something is wrong with the code.
*/

	bool checkValidity(bool bCheckEmptyCircleProperty,const std::string& msg="");


/** \brief Draws the triangulation as postscript file.
*
* show() is a convenience function for quick outputs with a default
* look. It is also possible to use the Visualizer2 class directly to
* draw arbitrary circles, line segments, vertices and labels with
* custom colors.
*
* @param postscriptFilename is the output name, i.e. "myFile.ps"
* @param bWithConstraints specifies if constraint segments shall be shown (default: true)
*/
 	void show(const std::string& postscriptFilename,bool bWithConstraints=true) const;


/** \brief Draws the triangulation as postscript file using an existing Visualizer2 object
*
* This overload of the show() method allows to add further geometric
* primitives to the Visualizer2 object before it is finally written.
*
* @param pVis is the pointer of a Visualizer2 object that may already contain geometric
* primitives or that may later be used to draw further elements
* @param bWithConstraints specifies if constraint segments shall be shown (default: true)
*
* @note The postscript file must be finalized with Visualizer2::writeFile().
*
*/
 	void show(Visualizer2* pVis,bool bWithConstraints=true) const;

#if GEOM_PSEUDO3D==GEOM_TRUE
	/** \brief Draws the triangulation in 3D.
	*
	* @note The free viewer Geomview can be used to view such files
	*/

	void showGeomview(const std::string& filename);
#endif

/** \brief Remove a single point
*
* @param pVertex is the point to be removed
*
*/

	void remove(Point2* pVertex);



/** \brief Insert a single point
*
* @param p is the point to be inserted.
* @return a pointer to the point in the triangulation
*
* The triangulation keeps a copy of \e p. The return value is a pointer to this copy.
* If duplicate points are inserted, the triangulation does not create new copies but
* returns a pointer to the copy of the very first insertion. @n
* @n
* @note This method offers a very good performance but it is still faster if all
* points are passed at once, if possible.
*/

	Point2* insert(const Point2& p);

/** \brief Insert a vector of points.
*
* @param vInputPoints contains the points to be inserted.
*
*/

	void insert(const std::vector<Point2>& vInputPoints);

/** \brief Insert points from a std::vector and store pointers in \e vHandles
*
* @param vInputPoints contains the points to be inserted.
* @param vHandles (empty) is used by Fade to return Point2 pointers
*
* Internally, the triangulation keeps copies of the inserted points which
* are returned in \e vHandles (in the same order). If duplicate points are
* contained in vInputPoints then only one copy will be made and a pointer
* to this unique copy will be stored in vHandles for every occurance.
*/


	void insert(const std::vector<Point2>& vInputPoints,std::vector<Point2*>& vHandles);

#if GEOM_PSEUDO3D==GEOM_TRUE
/** \brief Insert points from an array
*
* @param numPoints is the number of points to be inserted
* @param aCoordinates is an array of \e 3n double values, e.g. {x0,y0,z0,x1,y1,z1,...,xn,yn,zn}
* @param aHandles is an array with size \e n where pointers to the inserted points will be stored
*/
#else
/** \brief Insert points from an array
*
* @param numPoints is the number of points to be inserted
* @param aCoordinates is an array of \e 2n double values, e.g. {x0,y0,x1,y1,...,xn,yn}
* @param aHandles is an empty array with size \e n where pointers to the inserted points will be stored by Fade
*/
#endif
	void insert(int numPoints,double * aCoordinates,Point2 ** aHandles);


/** \brief Enable multithreading
*
* This is a feature for Linux systems: Fade2D can be executed on
* multiple CPU cores. The speed up is currently only 20% as the
* algorithm is only partially parallelized.
*
* \note Due to compatibility issues we had to deactivate the method,
* i.e., you can call it but it has no effect. Multithreading is
* currently revised and will be enabled for Linux and Windows as soon
* as possible.
*/

	void enableMultithreading();

/** \brief Locate a triangle which contains \e p
*
* \image html locate.jpg "Figure 1: Point location"
* \image latex locate.eps "Point location" width=7cm
*
* The Fade_2D class can be used as a data structure for point location.
* This method returns a pointer to a triangle which contains \e p.
*
* @param p is the query point
* @return a pointer to a Triangle2 object (or NULL if is2D()==false or if \e p is outside the triangulation)
*
*/


	Triangle2* locate(const Point2& p);


#if GEOM_PSEUDO3D==GEOM_TRUE
/** \brief Compute the height (z-coordinate) of a certain point
*
* If the coordinates (x,y) are inside the triangulation, this function computes
* the height and returns true. If the (x,y) is outside, the function returns false.
*
*/

	bool getHeight(double x,double y,double& height) const;
#endif

/** \brief Delaunay refinement
 *
 * Creates a mesh inside the area given by a Zone2 object.
 *
 * @param pZone is the zone whose triangles are refined. Allowed zoneLocation values are ZL_INSIDE and ZL_BOUNDED.
 * @param minAngleDegree (up to 30) is the minimum interior triangle angle
 * @param minEdgeLength is a lower threshold on the edge length. Triangles with smaller edges are not refined.
 * @param maxEdgeLength is an upper threshold on the edge length. Triangles with larger edges are always refined.
 * @param bAllowConstraintSplitting specifies if constraint edges may be splitted
 *
 * @note The behavior of the present method had to be changed in Fade v1.39:
 * Only ZL_INSIDE and ZL_BOUNDED zones are accepted. But you can easily
 * convert other types of zones to ZL_BOUNDED using @ref convertToBoundedZone "Zone2::convertToBoundedZone()".
 */

	void refine(	Zone2* pZone,
					double minAngleDegree,
					double minEdgeLength,
					double maxEdgeLength,
					bool bAllowConstraintSplitting
					);

/** \brief Delaunay refinement
 *
 * This is an advanced mesh generation method. The parameters are
 * encapsulated in the MeshGenParams class which provides default
 * parameters and which is intended as base class. Client code can
 * derive from MeshGenParams and overwrite the methods and parameters
 * to gain control over the mesh generation process.
*/
	void refineAdvanced(MeshGenParams* pParameters);






/** \brief Number of points
 *
 * @return the number of points in the triangulation
 *
 * @note if the returned number of points is smaller than the number of inserted points
 * then duplicate points have been inserted.
 */

	size_t numberOfPoints() const;

/** \brief Number of triangles
 *
 * @return the number of triangles in the triangulation (or 0 as long as is2D() is false).
 *
 */

	size_t numberOfTriangles() const;


/** \brief Get pointers to all triangles
*
* @param vAllTriangles is an empty vector of Triangle2 pointers.
*
* If is2D()==true then pointers to all existing triangles are stored in vAllTriangles.
* If is2D()==false then no triangles exist at all.
*/

	void getTrianglePointers(std::vector<Triangle2*>& vAllTriangles) const;

/** \brief Get pointers to all vertices
*
* @param vAllPoints is an empty vector of Point2 pointers.
*
* Stores pointers to all vertices of the triangulation in vAllPoints. The order in which
* the points are stored is \e not necessarily the insertion order. For geometrically identical
* points which have been inserted multiple times, only one pointer exists. Thus vAllPoints.size()
* can be smaller than the number of inserted points.
*
*/

	void getVertexPointers(std::vector<Point2*>& vAllPoints) const;



/** \brief Get adjacent triangle
*
* @return the triangle that has the edge (p0,p1) or NULL if no such edge is present
*
* @note Recall the counter-clockwise enumeration of vertices in a
* triangle. If (p0,p1) is used, one triangle adjacent to (p0,p1) is
* returned, using (p1,p0) one gets the other adjacent triangle.
*/
Triangle2* getAdjacentTriangle(Point2* p0,Point2* p1) const;

/** \brief Check if the triangulation contains triangles (which is the case if
* at least 3 non-collinear points exist in the triangulation
*
* As long as all inserted points are collinear the triangulation does not contain
* triangles. This is clearly the case as long as less than three input points are
* present but it may also be the case when 3 or more points have been inserted when
* all these points are collinear. These points are then in a pending state, i.e.
* they will be triangulated as soon as the first non-collinear point is inserted.
*
* \image html coll.jpg "Figure 2: Triangles are generated as soon as the first non-collinear point is inserted."
* \image latex coll.eps "Triangles are generated as soon as the first non-collinear point is inserted." width=7cm
*
* @return true if at least one triangle exists@n
* false otherwise
*
*/

	bool is2D() const;


/** \brief Add constraint edges (edges, polyline, polygon)
* \anchor createConstraint
* @param vSegments contains line segments which shall appear as edges
* of the triangulation. The order of the segments is not important.
* @param cis
* - CIS_CONSTRAINED_DELAUNAY inserts the segments without subdivision
* unless required (which is the case if existing vertices or constraint
* segments are crossed).
* - CIS_CONFORMING_DELAUNAY subdivides the segments if necessary to
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
* @param bOrientedSegments: Used to specify if the segments in vSegments
* are oriented (oriented, not ordered) in counterclockwise direction.
* A Zone2 object can later be made from the returned ConstrainedGraph2
* object only if this value is true \e or if vSegments forms a closed
* simple polygon (in this case the segments are internally reoriented).
* To maintain backwards compatibility bOrientedSegments is a default
* parameter and it defaults to false.
*
* @return a pointer to the new ConstraintGraph2 object@n
*
*
* \htmlonly <div class="center"> \endhtmlonly
*
* \image html crop_bareBike.jpg "Figure 3: Delaunay triangulation without constraints"
* \image latex crop_bareBike.eps "Delaunay triangulation without constraints" width=12cm
*
* \image html crop_ignoreBike.jpg "Figure 4: Constraint insertion with CIS_CONSTRAINED_DELAUNAY."
* \image latex crop_ignoreBike.eps "Constraint insertion with CIS_CONSTRAINED_DELAUNAY" width=12cm
*
* \image html crop_keepBike.jpg "Figure 5: Constraint insertion with CIS_CONFORMING_DELAUNAY. The inserted segments are automatically subdivided."
* \image latex crop_keepBike.eps "Constraint insertion with CIS_CONSTRAINED_DELAUNAY" width=12cm
*
* \htmlonly </div> \endhtmlonly
*
*/
	ConstraintGraph2* createConstraint(
		std::vector<Segment2>& vSegments,
		ConstraintInsertionStrategy cis,
		bool bOrientedSegments=false
		);






/** \brief Create a zone
*
* \anchor createZone
* A Zone2 object is an area of a triangulation, possibly bounded by
* a ConstraintGraph.
*
* @param zoneLoc is ZL_INSIDE, ZL_OUTSIDE or ZL_GLOBAL.
* @param pConstraintGraph points to a formerly created ConstraintGraph2
* object (which must contain a \e simple polygon) or is NULL in case of
* zoneLoc==ZL_GLOBAL.
* @param bVerbose is by default true and causes a warning if NULL is returned.
*
* @return a pointer to the new Zone2 object or NULL if no triangles
* exist or pConstraintGraph->isOriented() returns false.@n
*
* \see deleteZone(Zone2* pZone).
*
* \htmlonly <div class="center"> \endhtmlonly
* \image html bikeZones.jpg "Figure 6: Zones in a triangulation"
* \htmlonly </div> \endhtmlonly
*
* \image latex bikeZones.eps "Zones in a triangulation" width=12cm
*/
	Zone2* createZone(ConstraintGraph2* pConstraintGraph,ZoneLocation zoneLoc,bool bVerbose=true);


/** \brief Create a zone limited by multiple ConstraintGraph2 objects by growing from a start point
*
* A Zone2 object is an area of the traingulation, see \ref createZone
*
* @param vConstraintGraphs is a vector of ConstraintGraph objects
* @param zoneLoc must be ZL_GROW
* @param startPoint is the point from which the area is grown until
* the borders specified in vConstraintGraphs are reached
* @param bVerbose is by default true and causes a warning if NULL is returned.
* @return a pointer to the new Zone2 object (or NULL if zoneLoc!=ZL_GROW or no triangles exist)
*/
	Zone2* createZone(const std::vector<ConstraintGraph2*> vConstraintGraphs,ZoneLocation zoneLoc,const Point2& startPoint,bool bVerbose=true);

/** \brief Create a zone limited by a ConstraintGraph by growing from a start point
*
* A Zone2 object is an area of the traingulation, see \ref createZone
*
* @param pConstraintGraph is a constraint whose edges specify the area's border
* @param zoneLoc must be ZL_GROW
* @param startPoint is the point from which the area is grown until the borders specified in pConstraint are reached
* @param bVerbose is by default true and causes a warning if NULL is returned.
* @return a pointer to the new Zone2 object (or NULL if zoneLoc!=ZL_GROW or no triangles exist)
*
*/

	Zone2* createZone(ConstraintGraph2* pConstraintGraph,ZoneLocation zoneLoc,const Point2& startPoint,bool bVerbose=true);

/** \brief Create a zone defined by a vector of triangles
*
* A Zone2 object is an area of the traingulation, see \ref createZone
*
* @param vTriangles
* @param bVerbose is by default true and causes a warning if NULL is returned.
* @return a pointer to the new Zone2 object (or NULL if vTriangles is empty)
*
*/

	Zone2* createZone( std::vector<Triangle2*>& vTriangles,bool bVerbose=true );



/** \brief Delete a Zone2 object
*
* Zone2 objects are automatically destroyed with their Fade_2D objects.
* In addition this method provides the possibility to eliminate Zone2
* objects earlier.
*/
	void deleteZone(Zone2* pZone);


/** \brief Apply all formerly defined constraints and zones.
*
* For technical reasons all defined constraints are inserted at the end
* of the triangulation process. This step must be triggered manually,
* i.e., it is up to the user to call applyConstraintsAndZones() before
* the resulting triangulation is used. If afterwards more points are
* inserted, applyConstraintsAndZones() must be called again.
*
* Since version 1.33 of Fade intersecting constraint segments are
* supported. They are automatically subdivided and connected at their
* intersection points. The /e exact intersection point of two segments
* may not exist in IEEE754 double precision arithmetic. In this case
* the coordinates are rounded.
*/

	void applyConstraintsAndZones();


/** \brief Compute the axis-aligned bounding box of the points
*
* If no points have been inserted yet, then the returned Bbox2 object
* is invalid and its member function \e Bbox2::isValid() returns false.
*
*/
	Bbox2 computeBoundingBox() const;

/** \brief Check if an edge is a constraint edge
*
* Returns whether the edge in triangle \e pT which is opposite to the
* \e ith vertex is a constraint edge.
*
*/

	bool isConstraint(Triangle2* pT,int ith) const;

/** \brief Retrieve a ConstraintSegment2
*
* @return a pointer to the ConstraintSegment2 between p0 and p1 or NULL
* if the segment is not a constraint edge.
*/
	ConstraintSegment2* getConstraintSegment(Point2* p0,Point2* p1) const;


/** \brief Get incident triangles
*
* Stores pointers to all triangles around pVtx into vIncidentT
*/
	void getIncidentTriangles(Point2* pVtx,std::vector<Triangle2*>& vIncidentT) const;


/** \brief Write the current triangulation to an *.obj file
*
* Makes most sense in 2.5D but works also for 2D (all heights are set to zero then)
*
*/

	void writeObj(const std::string filename) const;

/** \brief Write a zone to an *.obj file
*
* Makes most sense in 2.5D but works also for 2D (all heights are set to zero then)
*
*/
	void writeObj(const std::string filename,Zone2* pZone) const;


/** \brief Write the current triangulation to an *.obj file
*
* Made for terrain visualizations in 2.5D but will work also for 2D.
*
*/
	void writeWebScene(const char* path) const;

/** \brief Write a zone to an *.obj file
*
* Made for terrain visualizations in 2.5D but will work also for 2D.
*
*/
	void writeWebScene(const char* path,Zone2* pZone) const;

/** \brief Register a message receiver
 *
 * @param msgType is the type of message the subscriber shall receive, e.g. MSG_PROGRESS or MSG_WARNING
 * @param pMsg is a pointer to a custom class derived from MsgBase
*/
void subscribe(MsgType msgType,MsgBase* pMsg);

/** \brief Unregister a message receiver
 *
 * @param msgType is the type of message the subscriber shall not receive anymore
 * @param pMsg is a pointer to a custom class derived from MsgBase
*/
void unsubscribe(MsgType msgType,MsgBase* pMsg);


/** \brief Check if an edge is a constraint edge
*
* Returns whether the edge (p0,p1) is a constraint edge.
*
*/

	bool isConstraint(Point2* p0,Point2* p1) const;

/**
*
* Prints informations about the currently used license
*/

	void printLicense() const;

/** \brief Development feature
*
* No documentation, this is a development tool.
*/

	bool checkZoneQuality(Zone2* pZone,double minAngle,const std::string& name,const AcceptExperimentalFeature& accept);

/** \brief Development feature
*
* No documentation, this is a development tool
*/

	void setName(const std::string& s);

/** \brief Development feature
*
* No documentation, this is a development tool
*/

	std::string getName() const;


/** \brief Import triangles
*
* This method imports triangles into an empty Fade object. Triangles
* do not necessarily need to satisfy the empty circle property.
*
* \param vPoints contains the input vertices (3 subsequent ones per triangle)
* \param bReorientIfNeeded specifies if the orientations of the point triples
* shall be checked and corrected. If the point triples are certainly oriented
* in counterclockwise order then the orientation test can be skipped.
* \param bCreateExtendedBoundingBox can be used to insert 4 dummy points
* of an extended bounding box. This is convenient in some cases. Use
* false if you are not sure.
* \return a pointer to a Zone2 object or NULL if the input data is invalid
*
* \note Imported triangles may change if points or constraint segments
* are inserted afterwards.
*/
	Zone2* importTriangles(	std::vector<Point2>& vPoints,
							bool bReorientIfNeeded,
							bool bCreateExtendedBoundingBox
						);

/** \brief Compute the orientation of 3 points
*
* \return ORIENTATION2_COLLINEAR, ORIENTATION2_CW (clockwise) or ORENTATION2_CCW (counterclockwise)
*/
	Orientation2 getOrientation(const Point2& p0,const Point2& p1,const Point2& p2);

/** \brief Cut through a triangulation
*
* \param knifeStart is one point of the knife segment
* \param knifeEnd is the second point of the knife segment
* \param bTurnEdgesIntoConstraints turns all 3 edges of each intersected
* triangle into constraint segments.
*
* This method inserts a constraint edge \e knife(\e knifeStart,\e knifeEnd).
* If existing edges \e E are intersected by \e knife, then \e knife is
* subdivided at the intersection points \e P.
*
* In any case \e knife will exist (in a possibly subdivided form) in
* the result. But a consequence of the insertion of the points \e P
* is that the edges \e E and even edges which are not intersected by
* \e knife may disappear. Use bTurnEdgesIntoConstraints=true to avoid
* that.
*
* \note The method calls applyConstraintsAndZones() internally to apply
* pre-existing constraint segments before the cut operation is performed.\n
*
* \note The intersection point of two line segments may not be exactly
* representable in double precision floating point arithmetic and thus
* tiny rounding errors may occur. As a consequence two very close
* intersection points may be rounded to the same coordinates. \n
*
* \note When more than one knife segment is inserted then the method
* void cutTriangles(std::vector<Segment2>& vSegments,bool bTurnEdgesIntoConstraints)
* should be used. The reason is that each individual cut operation
* changes the triangulation and thus iterative calls to the present
* version of the method can lead to a different result. \n
*
*
*/
	void cutTriangles(	const Point2& knifeStart,
						const Point2& knifeEnd,
						bool bTurnEdgesIntoConstraints);

/** \brief Cut through a triangulation
*
* \param vSegments are the knife segments
* \param bTurnEdgesIntoConstraints specifies if intersected edges shall automatically be turned into constraints
*
* Same method as void cutTriangles(const Point2& knifeStart,const Point2& knifeEnd,bool bTurnEdgesIntoConstraints)
* but it takes a vector of segments instead of a single segment. This
* is the recommended method to cut through a triangulation when more
* than one knife segment exists.
*
*/

	void cutTriangles(	std::vector<Segment2>& vSegments,
								bool bTurnEdgesIntoConstraints);


	// A development function, not for public use
	void internal(int au,int fu,std::string s="");
protected:
	void initFade(unsigned numExpectedVertices);
	Fade_2D(const Fade_2D&); // No copy constructor
	Fade_2D& operator=(const Fade_2D&); // No assignment allowed
	Dt2* pImpl;
};

/** \brief Fade2D version string
* This method returns a version string
*/
CLASS_DECLSPEC
std::string getFade2DVersion();
/** \brief Get the major version number
*/
CLASS_DECLSPEC
int getMajorVersionNumber();
/** \brief Get the minor version number
*/
CLASS_DECLSPEC
int getMinorVersionNumber();
/** \brief Get the revision version number
*/
CLASS_DECLSPEC
int getRevisionNumber();
/** \brief Check if a RELEASE or a DEBUG version is used.
*/
CLASS_DECLSPEC
bool isRelease();
/** \brief Get the border edges of a set of triangles
 *
 * \param vT are the input triangles
 * \param vBorderSegments is used to return all segments of triangles
 * in vT which have only one adjacent triangle
*/
CLASS_DECLSPEC
void getBorders(const std::vector<Triangle2*>& vT,std::vector<Segment2>& vBorderSegments);
/** \brief Sort a vector of Segments
 *
 * The segments in vRing are re-aligned and sorted such that subsequent
 * segments join at the endpoints.
*/
CLASS_DECLSPEC
bool sortRing(std::vector<Segment2>& vRing);


// License type
CLASS_DECLSPEC
void setLic(
	const std::string& l1,
	const std::string& l2,
	const std::string& dt,
	const std::string& s1,
	const std::string& s2_
	);
class Lic;
Lic* getLic();


/** \brief Read (x y) points
 *
 * Reads points from an ASCII file. Expected file format: Two
 * coordinates (x y) per line, whitespace separated.
 *
 * \cond SECTION_FADE25D
 * The z coordinate is set to 0.
 * \endcond
*/
CLASS_DECLSPEC
bool readXY(const char* filename,std::vector<Point2>& vPointsOut);

#if GEOM_PSEUDO3D==GEOM_TRUE
/** \brief Read (x y z) points
 *
 * Reads points from an ASCII file. Expected file format: Three
 * coordinates (x y z) per line, whitespace separated.
*/
CLASS_DECLSPEC
bool readXYZ(const char* filename,std::vector<Point2>& vPointsOut);

#endif

} // (namespace)




#ifndef FADE2D_EXPORT
	#include "License.h"
#endif

