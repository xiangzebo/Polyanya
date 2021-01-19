#pragma once
#include "mesh.h"
#include "point.h"

namespace polyanya
{
    //Throw snell ray, and return the point on the edge of mesh2Pass
    Point projectRay(Point start, Point middle, int poly2Leave,int poly2Pass, Mesh* mesh);
    //Find the snell ray from start point reaching target point, return the point where snellray meets the middle edge
    Point computeAngle(Point start, Point target,int poly2Leave, int poly2Pass, Mesh* mesh);
}
