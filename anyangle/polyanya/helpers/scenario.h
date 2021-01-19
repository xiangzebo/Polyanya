#pragma once
#include "point.h"
#include <iostream>
#include <vector>
#include <string>

namespace polyanya
{

struct Scenario
{
    int bucket;
    double xsize, ysize;
    Point start, goal;
    double gridcost;
};

bool load_scenarios(std::istream& infile, std::vector<Scenario>& out);

}
