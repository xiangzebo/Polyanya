#pragma once
#include <vector>

namespace polyanya
{

struct Polygon
{
    // "int" here means an array index.
    std::vector<int> vertices;
    std::vector<int> polygons;
    std::vector<int> weights;
    bool is_one_way;
    double min_x, max_x, min_y, max_y;
};

}
