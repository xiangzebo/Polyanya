#pragma once
#include "point.h"
#include <vector>

namespace polyanya
{

// A point in the polygon mesh.
struct Vertex
{
    Point p;
    // "int" here means an array index.
    std::vector<int> polygons;

    bool is_corner;
    bool is_ambig;
};

}
