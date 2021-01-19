#pragma once
#include "common.h"

#if GEOM_PSEUDO3D==GEOM_TRUE
	namespace GEOM_FADE25D {
#elif GEOM_PSEUDO3D==GEOM_FALSE
	namespace GEOM_FADE2D {
#else
	#error GEOM_PSEUDO3D is not defined
#endif

class Fade_2D; // Forward
template <typename T> inline void unusedParameter(const T&){} // Avoids compiler warnings



/** \brief Parameters for the mesh generator
*
* This class serves as container for mesh generator parameters. Client
* code can provide a class which derives from MeshGenParams an which
* provides custom implementations of the getMaxTriangleArea(Triangle* pT)
* method or the getMaxEdgeLength(Triangle* pT) method in order to
* gain control over the local density of the generated mesh. When the
* meshing algorithm decides if a certain triangle T must be refined,
* then it calls these functions.
*/
class CLASS_DECLSPEC MeshGenParams
{
public:


/** \brief getMaxTriangleArea(Triangle2* pT)
 *
 * @param pZone_ is the zone to be remeshed. Its zoneLocation value
 * must be ZL_INSIDE or ZL_BOUNDED. Other types of zones can be
 * converted ZL_BOUNDED using @ref convertToBoundedZone "Zone2::convertToBoundedZone()
 */
	MeshGenParams(Zone2* pZone_):
#if GEOM_PSEUDO3D==GEOM_TRUE
		// *** The following two parameters exist only in Fade2.5D ***
		pHeightGuideTriangulation(NULL),
		maxHeightError(DBL_MAX),
#endif
		pZone(pZone_),
		minAngleDegree(20.0),
		minEdgeLength(1e-3),
		maxEdgeLength(DBL_MAX),
		maxTriangleArea(DBL_MAX),
		bAllowConstraintSplitting(true),
		growFactor(DBL_MAX),
		growFactorMinArea(1e-3),
		capAspectLimit(10.0)
	{
		if(	pZone->getZoneLocation()!=ZL_INSIDE &&
			pZone->getZoneLocation()!=ZL_BOUNDED )
		{
			std::cerr<<"\n\n\nMeshGenParams::MeshGenParams(..), ERROR: \n\tNew behavior since Fade 1.39: The zoneLocation must be ZL_INSIDE or ZL_BOUNDED. Please convert the zone using pZone->convertToBoundedZone(..)\n\n"<<std::endl;
			assert(false);
			return;
		}
	}

	virtual ~MeshGenParams()
	{}



/** \brief getMaxTriangleArea(Triangle2* pT)
 *
 * @param pT is the current triangle for which the meshing
 * algorithm checks if it must be refined.
 *
 * The default implementation returns maxTriangleArea (the default
 * value or one that has been set by the user). This method can be
 * overwritten by the client software in order to control the local
 * mesh density.
 * @note There is no getMaxTriangleArea(double x,double y) method
 * anymore. The parameter has been changed to Triangle2* to give
 * the client code more information.
 */
	virtual double getMaxTriangleArea(Triangle2* pT)
	{
		unusedParameter(pT); // avoids useless compiler warnings
		return maxTriangleArea;
	}


/** \brief getMaxEdgeLength(Triangle2* pT)
 *
 * @param pT is the current triangle for which the meshing
 * algorithm checks if it must be refined.
 *
 * The default implementation returns maxEdgeLengthArea (the default
 * value or one that has been set by the user). This method can be
 * overwritten by the client software in order to control the local
 * mesh density.
 * @note There is no getMaxEdgeLength(double x,double y) method
 * anymore. The parameter has been changed to Triangle2* to give
 * the client code more information.
 */
	virtual double getMaxEdgeLength(Triangle2* pT)
	{
		unusedParameter(pT); // avoids useless compiler warnings
		return maxEdgeLength;
	}


	// *** The following two parameters exist only in Fade2.5D ***
#if GEOM_PSEUDO3D==GEOM_TRUE
/** \brief pHeightGuideTriangulation
 *
 * An extra triangulation can optionally be set. When present, it serves
 * as height guide for newly created vertices.
 */
	Fade_2D* pHeightGuideTriangulation;

/** \brief maxHeightError
 *
 * If pHeightGuideTriangulation is set and the height error exceeds
 * locally maxHeightError then the triangulation is further refined.
 */
	double maxHeightError;
#endif




/** \brief Zone to be meshed.
*/
	Zone2* pZone;

/** \brief Minimum interior triangle angle
 *
 * Minimum interior angle: Default: 20.0, maximum: 30.0
*/
	double minAngleDegree;

/** \brief Minimum edge length
 *
 * Edges below the minimum length are not subdivided. This parameter
 * is useful to avoid tiny triangles. Default: 0.001
*/
	double minEdgeLength;

/** \brief Maximum edge length
 *
 * This value is returned by the default implementation of
 * getMaxEdgeLength(Triangle* pT). Larger edges are automatically
 * subdivided. If a custom implementation of getMaxEdgeLength(Triangle* pT)
 * is provided then this value is ignored. Default value: DBL_MAX.
 *
*/
	double maxEdgeLength;


/** \brief maxTriangleArea
 *
 * This value is returned by the default implementation of
 * getMaxTriangleArea(Triangle* pT). Larger triangles are
 * automatically subdivided. If a custom implementation of getMaxTriangleArea(Triangle* pT)
 * is provided then this value is ignored. Default value: DBL_MAX.
 */
	double maxTriangleArea;


/** \brief bAllowConstraintSplitting
 *
 * Defines if constraint segments can be splitted. Default: yes
*/
	bool bAllowConstraintSplitting;

/** \brief growFactor
 *
 * Limits the growth of adjacent triangles. The mesh is constructed such
 * that for any two adjacent triangles t0 and t1 (where t0 is the larger
 * one) area(t0)/area(t1) < growFactor. Recommendation: growFactor>5.0, Default: growFactor=DBL_MAX
*/
	double growFactor;

/** \brief growFactorMinArea
 *
 * The growFactor value is ignored for triangles with a smaller area
 * than growFactorMinArea. This value prevents generation of hundreds
 * of tiny triangles around one that is unusually small. Default: 0.001
 */
	double growFactorMinArea;

/** \brief capAspectLimit
 *
 * Limits the quotient edgeLength / height. Default value: 10.0
 */
	double capAspectLimit;

};


} // NAMESPACE
