#include "geometry.h"
#include "point.h"
#include "consts.h"
#include <cmath>
#include <cassert>

namespace polyanya
{

// Given two line segments ab and cd defined as:
//   ab(t) = a + (b-a) * t
//   cd(t) = c + (d-c) * t
// find ab_num, cd_num, denom such that
//   ab(ab_num / denom) = cd(cd_num / denom).
// If denom is close to 0, it will be set to 0.
void line_intersect_time(const Point& a, const Point& b,
                         const Point& c, const Point& d,
                         double& ab_num, double& cd_num, double& denom)
{
    denom = (b - a) * (d - c);
    if (std::abs(denom) < EPSILON)
    {
        denom = 0.0; // to make comparison easy
        ab_num = 1;
        cd_num = 1;
    }
    else
    {
        const Point ac = c - a;
        ab_num = ac * (d - a);
        cd_num = ac * (b - c);

        #ifndef NDEBUG
        // If we're debugging, double check that our results are right.
        const Point ab_point = get_point_on_line(a, b, ab_num / denom);
        const Point cd_point = get_point_on_line(c, d, cd_num / denom);
        assert(ab_point == cd_point);
        #endif
    }
}

// Reflects the point across the line lr.
Point reflect_point(const Point& p, const Point& l, const Point& r)
{
    // I have no idea how the below works.
    const double denom = r.distance_sq(l);
    if (std::abs(denom) < EPSILON)
    {
        // A trivial reflection.
        // Should be p + 2 * (l - p) = 2*l - p.
        return 2 * l - p;
    }
    const double numer = (r - p) * (l - p);

    // The vector r - l rotated 90 degrees counterclockwise.
    // Can imagine "multiplying" the vector by the imaginary constant.
    const Point delta_rotated = {l.y - r.y, r.x - l.x};

    #ifndef NDEBUG
    // If we're debugging, ensure that p + (numer / denom) * delta_rotated
    // lies on the line lr.
    assert(get_orientation(l, p + (numer / denom) * delta_rotated, r) ==
           Orientation::COLLINEAR);
    #endif

    return p + (2.0 * numer / denom) * delta_rotated;
}

}
