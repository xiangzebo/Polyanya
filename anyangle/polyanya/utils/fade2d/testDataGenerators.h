#pragma once
#include "Point2.h"
#include "Segment2.h"
#include <vector>




#if GEOM_PSEUDO3D==GEOM_TRUE
	namespace GEOM_FADE25D {
#elif GEOM_PSEUDO3D==GEOM_FALSE
	namespace GEOM_FADE2D {
#else
	#error GEOM_PSEUDO3D is not defined
#endif


/** \defgroup dataGenerators Test Data Generators
 *
 * \image html random_objects.png "Random objects (points, lines, polygons, polylines)"
 *
* # Generate random polygons and other test objects
* Theory, careful programming and automated software stress tests.
* Neither of them can replace the other one. Testing with random data
* helps to discover errors early. Fade provides random object
* generators for your automated software stress tests:
*
 * - Random simple polygons
 * - Random segments
 * - Random point clouds
 * - Random numbers.
 * - Polylines from sine functions
*
*
* If you discover an error in your software you must be able to reproduce
* the input data that has triggered your bug. For this reason the random
* object generators take a seed value to initialize the internal random
* number generators. A certain seed value always leads to the same
* sequence of objects. Only when the special seed value 0 is used then
* the random number generators are initialized from the system time.
 *
 *  @{
 */

/** @brief Generate random numbers
 *
 * @param num Number of random numbers to be generated
 * @param min Lower bound
 * @param max Upper bound
 * @param [out] vRandomNumbersOut is the output vector
 * @param seed initializes the random number generator RNG
 * (default: 0...mapped to a random seed, other values: constant initialization)
 *
 * \note Reproducable random numbers are often desirable when software
 * is tested with random geometric constructions. Thus each seed value
 * different from \e 0 leads to its own, reproducible, output sequence.
 * In contrast the \p seed value \e 0 is mapped to random initialization
 * of the RNG. In this case the RNG will produce a different output
 * sequence each time it is called.
 */
CLASS_DECLSPEC
void generateRandomNumbers(	size_t num,
							double min,
							double max,
							std::vector<double>& vRandomNumbersOut,
							unsigned int seed=0
							);

/** @brief Generate random points
 *
 * @param numRandomPoints Number of points to be generated
 * @param min Lower bound (x,y)
 * @param max Upper bound (x,y)
 * @param [out] vRandomPointsOut is the output vector
 * @param seed initializes the random number generator RNG
 * (default: 0...mapped to a random seed, other values: constant initialization)
 *
 *
 *
* \image html randomPoints.png "Point generator"
* \image latex randomPoints.eps "Point generator" width=15cm
*
*/
CLASS_DECLSPEC
void generateRandomPoints(	size_t numRandomPoints,
							double min,
							double max,
							std::vector<Point2>& vRandomPointsOut,
							unsigned int seed=0
						);

/** @brief Generate a random simple polygon
 *
 * @param numSegments Number of segments to be generated
 * @param min Lower bound (x,y)
 * @param max Upper bound (x,y)
 * @param [out] vPolygonOut is the output vector
 * @param seed initializes the random number generator RNG
 * (default: 0...mapped to a random seed, other values: constant initialization)
 *
 *
* \image html randomPolygon.png "Polygon generator: Random simple polygon"
* \image latex randomPolygon.eps "Polygon generator: Random simple polygon" width=15cm
 */
CLASS_DECLSPEC
void generateRandomPolygon(	size_t numSegments,
							double min,
							double max,
							std::vector<Segment2>& vPolygonOut,
							unsigned int seed=0
						);

/** @brief Generate random line segments
 *
 * @param numSegments Number of segments to be generated
 * @param min Lower bound (x,y)
 * @param max Upper bound (x,y)
 * @param maxLen Maximal segment length
 * @param [out] vSegmentsOut is the output vector
 * @param seed initializes the random number generator RNG
 * (default: 0...mapped to a random seed, other values: constant initialization)
*
 *
 *
* \image html randomSegments.png "Segment generator: Random line segments"
* \image latex randomSegments.eps "Segment generator: Random line segments" width=15cm
 */
CLASS_DECLSPEC
void generateRandomSegments(size_t numSegments,
							double min,
							double max,
							double maxLen,
							std::vector<Segment2>& vSegmentsOut,
							unsigned int seed);

/** @brief Generate segments from a sine function
 *
 * @param numSegments Number of segments to be generated
 * @param numPeriods Number of periods of the sine function
 * @param xOffset Offset of the output x-coordinates
 * @param yOffset Offset of the output y-coordinates
 * @param xFactor Factor to scale the sine function in x direction
 * @param yFactor Factor to scale the sine function in y direction
 * @param bSwapXY Swap the x and y coordinate of the function
 * @param [out] vSineSegmentsOut is the output vector
 *
 * * \image html sinePolyline.png "Polyline generator: Polylines from sine functions"
* \image latex sinePolyline.eps "Polyline generator: Polylines from sine functions" width=15cm
 */
CLASS_DECLSPEC
void generateSineSegments(
							int numSegments,
							int numPeriods,
							double xOffset,
							double yOffset,
							double xFactor,
							double yFactor, // 0 causes absolute values
							bool bSwapXY,
							std::vector<Segment2>& vSineSegmentsOut
							);



/** @}*/
} // Namespace
