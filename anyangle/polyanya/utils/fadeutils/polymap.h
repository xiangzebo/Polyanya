#pragma once
#include <Fade_2D.h>
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>

namespace fadeutils
{

using namespace std;
using namespace GEOM_FADE2D;

typedef vector<Point2> Polygon;

void fail(const string& message);

vector<Polygon> *read_polys(istream& infile);

vector<ConstraintGraph2*> *create_constraint_graphs(const vector<Polygon> &polygons, Fade_2D &dt);

Zone2* create_traversable_zone(istream& infile, Fade_2D &dt);

}
