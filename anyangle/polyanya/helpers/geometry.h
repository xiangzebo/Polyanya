#pragma once
#include "point.h"

namespace polyanya
{

void line_intersect_time(const Point& a, const Point& b,
                         const Point& c, const Point& d,
                         double& ab_num, double& cd_num, double& denom);

// Returns the line intersect between ab and cd as fast as possible.
// Uses a and b for the parameterisation.
// ASSUMES NO COLLINEARITY.
inline Point line_intersect(const Point& a, const Point& b,
                            const Point& c, const Point& d)
{
    const Point ab = b - a;
    return a + ab * (((c - a) * (d - a)) / (ab * (d - c)));
}

enum struct ZeroOnePos
{
    LT_ZERO,  // n < 0
    EQ_ZERO,  // n = 0
    IN_RANGE, // 0 < n < 1
    EQ_ONE,   // n = 1
    GT_ONE,   // n > 1
};

// Returns where num / denom is in the range [0, 1].
inline ZeroOnePos line_intersect_bound_check(
    const double num, const double denom
)
{
    // Check num / denom == 0.
    if (std::abs(num) < EPSILON)
    {
        return ZeroOnePos::EQ_ZERO;
    }
    // Check num / denom == 1.
    // Note: Checking whether it is accurately near 1 requires us to check
    // |num - denom| < EPSILON^2 * denom
    // which requires a multiplication. Instead, we can assume that denom
    // is less than 1/EPSILON and check
    // |num - denom| < EPSILON
    // instead. This is less accurate but faster.
    if (std::abs(num - denom) < EPSILON)
    {
        return ZeroOnePos::EQ_ONE;
    }

    // Now we finally check whether it's greater than 1 or less than 0.
    if (denom > 0)
    {
        if (num < 0)
        {
            // strictly less than 0
            return ZeroOnePos::LT_ZERO;
        }
        if (num > denom)
        {
            // strictly greater than 1
            return ZeroOnePos::GT_ONE;
        }
    }
    else
    {
        if (num > 0)
        {
            // strictly less than 0
            return ZeroOnePos::LT_ZERO;
        }
        if (num < denom)
        {
            // strictly greater than 1
            return ZeroOnePos::GT_ONE;
        }
    }
    return ZeroOnePos::IN_RANGE;
}

// Given two points a, b and a number t, compute the point
//  a + (b-a) * t
inline Point get_point_on_line(const Point& a, const Point& b, const double t)
{
    return a + (b - a) * t;
}
Point reflect_point(const Point& p, const Point& l, const Point& r);

enum struct Orientation
{
	CCW,       // counterclockwise
	COLLINEAR, // collinear
	CW,        // clockwise
};

inline Orientation get_orientation(
    const Point& a, const Point& b, const Point& c
)
{
    const double cross = (b - a) * (c - b);
    if (std::abs(cross) < EPSILON)
    {
        return Orientation::COLLINEAR;
    } else if (cross > 0)
    {
        return Orientation::CCW;
    }
    else
    {
        return Orientation::CW;
    }
}

inline bool is_collinear(const Point& a, const Point& b, const Point& c)
{
    return std::abs((b - a) * (c - b)) < EPSILON;
}

}
