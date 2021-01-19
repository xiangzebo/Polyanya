#pragma once
#include "point.h"

namespace polyanya
{

struct Successor
{
    enum Type
    {
        RIGHT_NON_OBSERVABLE,
        OBSERVABLE,
        LEFT_NON_OBSERVABLE,
    };

    Type type;

    Point left, right;

    // Used to get next_polygon (Polygon.polygons[left_ind])
    // as well as right_vertex (Polygon.vertices[left_ind - 1])
    int poly_left_ind;

    friend std::ostream& operator<<(std::ostream& stream,
                                    const Successor& succ)
    {
        const std::string lookup[] = {
            "RIGHT_NON_OBSERVABLE",
            "OBSERVABLE",
            "LEFT_NON_OBSERVABLE",
        };
        return stream << "Successor(" << lookup[static_cast<int>(succ.type)]
                      << ", " << succ.left << " " << succ.right << ", "
                      << "poly_left=" << succ.poly_left_ind << ")";
    }
};

}
