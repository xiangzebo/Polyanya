#include "mesh.h"
#include <vector>
#include <iostream>
#include <map>
#include <string>
#include <cmath>
#include <cassert>
#include <algorithm>

namespace polyanya
{

Mesh::Mesh(std::istream& infile)
{
    read(infile);
    precalc_point_location();
}

void Mesh::read(std::istream& infile)
{
    #define fail(message) std::cerr << message << std::endl; exit(1);
    std::string header;
    int version;

    if (!(infile >> header))
    {
        fail("Error reading header");
    }
    if (header != "mesh")
    {
        std::cerr << "Got header '" << header << "'" << std::endl;
        fail("Invalid header (expecting 'mesh')");
    }

    if (!(infile >> version))
    {
        fail("Error getting version number");
    }
    if (version != 2)
    {
        std::cerr << "Got file with version " << version << std::endl;
        fail("Invalid version (expecting 2)");
    }

    int V, P;
    if (!(infile >> V >> P))
    {
        fail("Error getting V and P");
    }
    if (V < 1)
    {
        std::cerr << "Got " << V << " vertices" << std::endl;
        fail("Invalid number of vertices");
    }
    if (P < 1)
    {
        std::cerr << "Got " << P << " polygons" << std::endl;
        fail("Invalid number of polygons");
    }

    mesh_vertices.resize(V);
    mesh_polygons.resize(P);


    for (int i = 0; i < V; i++)
    {
        Vertex& v = mesh_vertices[i];
        v.is_corner = false;
        v.is_ambig = false;
        if (!(infile >> v.p.x >> v.p.y))
        {
            fail("Error getting vertex point");
        }
        int neighbours;
        if (!(infile >> neighbours))
        {
            fail("Error getting vertex neighbours");
        }
        if (neighbours < 2)
        {
            std::cerr << "Got " << neighbours << " neighbours" << std::endl;
            fail("Invalid number of neighbours around a point");
        }
        v.polygons.resize(neighbours);
        for (int j = 0; j < neighbours; j++)
        {
            int polygon_index;
            if (!(infile >> polygon_index))
            {
                fail("Error getting a vertex's neighbouring polygon");
            }
            if (polygon_index >= P)
            {
                std::cerr << "Got a polygon index of " \
                          << polygon_index << std::endl;
                fail("Invalid polygon index when getting vertex");
            }
            v.polygons[j] = polygon_index;
            if (polygon_index == -1)
            {
                if (v.is_corner)
                {
                    if (!v.is_ambig)
                    {
                        v.is_ambig = true;
                    }
                }
                else
                {
                    v.is_corner = true;
                }
            }
        }
    }


    max_poly_sides = 0;
    for (int i = 0; i < P; i++)
    {
        Polygon& p = mesh_polygons[i];
        int n;
        if (!(infile >> n))
        {
            fail("Error getting number of vertices of polygon");
        }
        if (n < 3)
        {
            std::cerr << "Got " << n << " vertices" << std::endl;
            fail("Invalid number of vertices in polygon");
        }
        p.vertices.resize(n);
        p.polygons.resize(n);
        if (n > max_poly_sides)
        {
            max_poly_sides = n;
        }

        for (int j = 0; j < n; j++)
        {
            int vertex_index;
            if (!(infile >> vertex_index))
            {
                fail("Error getting a polygon's vertex");
            }
            if (vertex_index >= V)
            {
                std::cerr << "Got a vertex index of " \
                          << vertex_index << std::endl;
                fail("Invalid vertex index when getting polygon");
            }
            p.vertices[j] = vertex_index;
            if (j == 0)
            {
                p.min_x = mesh_vertices[vertex_index].p.x;
                p.min_y = mesh_vertices[vertex_index].p.y;
                p.max_x = mesh_vertices[vertex_index].p.x;
                p.max_y = mesh_vertices[vertex_index].p.y;
            }
            else
            {
                p.min_x = std::min(p.min_x, mesh_vertices[vertex_index].p.x);
                p.min_y = std::min(p.min_y, mesh_vertices[vertex_index].p.y);
                p.max_x = std::max(p.max_x, mesh_vertices[vertex_index].p.x);
                p.max_y = std::max(p.max_y, mesh_vertices[vertex_index].p.y);
            }
        }
        // mesh min/max
        if (i == 0)
        {
            min_x = p.min_x;
            min_y = p.min_y;
            max_x = p.max_x;
            max_y = p.max_y;
        }
        else
        {
            min_x = std::min(min_x, p.min_x);
            min_y = std::min(min_y, p.min_y);
            max_x = std::max(max_x, p.max_x);
            max_y = std::max(max_y, p.max_y);
        }

        bool found_trav = false;
        p.is_one_way = true;
        for (int j = 0; j < n; j++)
        {
            int polygon_index;
            if (!(infile >> polygon_index))
            {
                fail("Error getting a polygon's neighbouring polygon");
            }
            if (polygon_index >= P)
            {
                std::cerr << "Got a polygon index of " \
                          << polygon_index << std::endl;
                fail("Invalid polygon index when getting polygon");
            }
            if (polygon_index != -1)
            {
                if (found_trav)
                {
                    if (p.is_one_way)
                    {
                        p.is_one_way = false;
                    }
                }
                else
                {
                    found_trav = true;
                }
            }
            p.polygons[j] = polygon_index;
        }
    }

    double temp;
    if (infile >> temp)
    {
        fail("Error parsing mesh (read too much)");
    }
    #undef fail
}

void Mesh::precalc_point_location()
{
    for (Vertex& v : mesh_vertices)
    {
        slabs[v.p.x] = std::vector<int>(0); // initialises the vector
    }
    for (int i = 0; i < (int) mesh_polygons.size(); i++)
    {
        const Polygon& p = mesh_polygons[i];
        const auto low_it = slabs.lower_bound(p.min_x);
        const auto high_it = slabs.upper_bound(p.max_x);

        for (auto it = low_it; it != high_it; it++)
        {
            it->second.push_back(i);
        }
    }
    for (auto& pair : slabs)
    {
        std::sort(pair.second.begin(), pair.second.end(),
            [&](const int& a, const int& b) -> bool
            {
                // Sorts based on the midpoints.
                // If tied, sort based on width of poly.
                const Polygon& ap = mesh_polygons[a];
                const Polygon& bp = mesh_polygons[b];
                const double as = ap.min_y + ap.max_y, bs = bp.min_y + ap.max_y;
                if (as == bs) {
                    return (ap.max_y - ap.min_y) > (bp.max_y - bp.min_y);
                }
                return as < bs;
            }
        );
    }
}

// Finds out whether the polygon specified by "poly" contains point P.
PolyContainment Mesh::poly_contains_point(int poly, Point& p)
{
    // The below is taken from
    // "An Efficient Test for a Point to Be in a Convex Polygon"
    // from the Wolfram Demonstrations Project
    // demonstrations.wolfram.com/AnEfficientTestForAPointToBeInAConvexPolygon/

    // Assume points are in counterclockwise order.
    const Polygon& poly_ref = mesh_polygons[poly];
    if (p.x < poly_ref.min_x - EPSILON || p.x > poly_ref.max_x + EPSILON ||
        p.y < poly_ref.min_y - EPSILON || p.y > poly_ref.max_y + EPSILON)
    {
        return {PolyContainment::OUTSIDE, -1, -1, -1};
    }
    const Point& last_point_in_poly = mesh_vertices[poly_ref.vertices.back()].p;
    const Point ZERO = {0, 0};

    Point last = last_point_in_poly - p;
    if (last == ZERO)
    {
        return {PolyContainment::ON_VERTEX, -1, poly_ref.vertices.back(), -1};
    }

    int last_index = poly_ref.vertices.back();
    for (int i = 0; i < (int) poly_ref.vertices.size(); i++)
    {
        const int point_index = poly_ref.vertices[i];
        const Point cur = mesh_vertices[point_index].p - p;
        if (cur == ZERO)
        {
            return {PolyContainment::ON_VERTEX, -1, point_index, -1};
        }
        const double cur_a = last * cur;
        if (std::abs(cur_a) < EPSILON)
        {
            // The line going from cur to last goes through p.
            // This means that they are collinear.
            // The associated polygon should simply be polygons[i] in version
            // 2 of the file format.

            // Ensure that cur = c*last where c is negative.
            // If not, this means that the point is either outside or that this
            // segment is collinear to an adjacent one.
            if (cur.x)
            {
                if (!((cur.x > 0) ^ (last.x > 0)))
                {
                    last = cur;
                    last_index = point_index;
                    continue;
                }
            }
            else
            {
                if (!((cur.y > 0) ^ (last.y > 0)))
                {
                    last = cur;
                    last_index = point_index;
                    continue;
                }
            }
            return {PolyContainment::ON_EDGE, poly_ref.polygons[i],
                    point_index, last_index};
        }

        // Because we assume that the points are counterclockwise,
        // we can immediately terminate when we see a negatively signed area.
        if (cur_a < 0)
        {
            return {PolyContainment::OUTSIDE, -1, -1, -1};
        }
        last = cur;
        last_index = point_index;
    }
    return {PolyContainment::INSIDE, -1, -1, -1};
}

// Finds where the point P lies in the mesh.
PointLocation Mesh::get_point_location(Point& p)
{
    if (p.x < min_x - EPSILON || p.x > max_x + EPSILON ||
        p.y < min_y - EPSILON || p.y > max_y + EPSILON)
    {
        return {PointLocation::NOT_ON_MESH, -1, -1, -1, -1};
    }
    auto slab = slabs.upper_bound(p.x);
    if (slab == slabs.begin())
    {
        return {PointLocation::NOT_ON_MESH, -1, -1, -1, -1};
    }
    slab--;
    const std::vector<int>& polys = slab->second;
    const auto close_it = std::lower_bound(polys.begin(), polys.end(), p.y,
        [&](const int& poly_index, const double& y_coord) -> bool
        {
            // Sorts based on the midpoints.
            // If tied, sort based on width of poly.
            const Polygon& poly = mesh_polygons[poly_index];
            return poly.min_y + poly.max_y < y_coord * 2;
        }
    );
    const int close_index = close_it - polys.begin()
                            - (close_it == polys.end());
    // The plan is to take an index and repeatedly do:
    // +1, -2, +3, -4, +5, -6, +7, -8, ...
    // until it hits the edge. If it hits an edge, instead iterate normally.
    const int ps = polys.size();
    int i = close_index;
    int next_delta = 1;
    int walk_delta = 0; // way to go when walking normally

    while (i >= 0 && i < ps)
    {
        const int polygon = polys[i];
        const PolyContainment result = poly_contains_point(polygon, p);
        switch (result.type)
        {
            case PolyContainment::OUTSIDE:
                // Does not contain: try the next one.
                break;

            case PolyContainment::INSIDE:
                // This one strictly contains the point.
                return {PointLocation::IN_POLYGON, polygon, -1, -1, -1};

            case PolyContainment::ON_EDGE:
                // This one lies on the edge.
                // Chek whether the other one is -1.
                return {
                    (result.adjacent_poly == -1 ?
                     PointLocation::ON_MESH_BORDER :
                     PointLocation::ON_EDGE),
                    polygon, result.adjacent_poly,
                    result.vertex1, result.vertex2
                };

            case PolyContainment::ON_VERTEX:
                // This one lies on a corner.
            {
                const Vertex& v = mesh_vertices[result.vertex1];
                if (v.is_corner)
                {
                    if (v.is_ambig)
                    {
                        return {PointLocation::ON_CORNER_VERTEX_AMBIG, -1, -1,
                                result.vertex1, -1};
                    }
                    else
                    {
                        return {PointLocation::ON_CORNER_VERTEX_UNAMBIG,
                                polygon, -1, result.vertex1, -1};
                    }
                }
                else
                {
                    return {PointLocation::ON_NON_CORNER_VERTEX,
                            polygon, -1,
                            result.vertex1, -1};
                }
            }

            default:
                // This should not be reachable
                assert(false);
        }


        // do stuff
        if (walk_delta == 0)
        {
            const int next_i = i + next_delta * (2 * (next_delta & 1) - 1);
            if (next_i < 0)
            {
                // was going to go too far to the left.
                // start going right
                walk_delta = 1;
            }
            else if (next_i >= ps)
            {
                walk_delta = -1;
            }
            else
            {
                i = next_i;
                next_delta++;
            }
        }

        if (walk_delta != 0)
        {
            i += walk_delta;
        }
    }
    // Haven't returned yet, therefore P does not lie on the mesh.
    return {PointLocation::NOT_ON_MESH, -1, -1, -1, -1};
}

PointLocation Mesh::get_point_location_naive(Point& p)
{
    for (int polygon = 0; polygon < (int) mesh_polygons.size(); polygon++)
    {
        const PolyContainment result = poly_contains_point(polygon, p);
        switch (result.type)
        {
            case PolyContainment::OUTSIDE:
                // Does not contain: try the next one.
                break;

            case PolyContainment::INSIDE:
                // This one strictly contains the point.
                return {PointLocation::IN_POLYGON, polygon, -1, -1, -1};

            case PolyContainment::ON_EDGE:
                // This one lies on the edge.
                // Chek whether the other one is -1.
                return {
                    (result.adjacent_poly == -1 ?
                     PointLocation::ON_MESH_BORDER :
                     PointLocation::ON_EDGE),
                    polygon, result.adjacent_poly,
                    result.vertex1, result.vertex2
                };

            case PolyContainment::ON_VERTEX:
                // This one lies on a corner.
            {
                const Vertex& v = mesh_vertices[result.vertex1];
                if (v.is_corner)
                {
                    if (v.is_ambig)
                    {
                        return {PointLocation::ON_CORNER_VERTEX_AMBIG, -1, -1,
                                result.vertex1, -1};
                    }
                    else
                    {
                        return {PointLocation::ON_CORNER_VERTEX_UNAMBIG,
                                polygon, -1, result.vertex1, -1};
                    }
                }
                else
                {
                    return {PointLocation::ON_NON_CORNER_VERTEX,
                            polygon, -1,
                            result.vertex1, -1};
                }
            }

            default:
                // This should not be reachable
                assert(false);
        }
    }
    // Haven't returned yet, therefore P does not lie on the mesh.
    return {PointLocation::NOT_ON_MESH, -1, -1, -1, -1};
}

void Mesh::print(std::ostream& outfile)
{
    outfile << "mesh with " << mesh_vertices.size() << " vertices, " \
            << mesh_polygons.size() << " polygons" << std::endl;
    outfile << "vertices:" << std::endl;
    for (Vertex vertex : mesh_vertices)
    {
        outfile << vertex.p << " " << vertex.is_corner << std::endl;
    }
    outfile << std::endl;
    outfile << "polygons:" << std::endl;
    for (Polygon polygon : mesh_polygons)
    {
        for (int vertex : polygon.vertices)
        {
            outfile << mesh_vertices[vertex].p << " ";
        }
        outfile << std::endl;
    }
}

void Mesh::print_polygon(std::ostream& outfile, int index)
{
    if (index == -1)
    {
        outfile << "P!!!";
        return;
    }
    outfile << "P" << index << " [";
    Polygon& poly = mesh_polygons[index];
    const auto vertices = poly.vertices;
    const int size = (int) vertices.size();
    for (int i = 0; i < size; i++)
    {
        print_vertex(outfile, vertices[i]);
        if (i != size - 1)
        {
            outfile << ", ";
        }
    }
    outfile << "]";
}

void Mesh::print_vertex(std::ostream& outfile, int index)
{
    outfile << "V" << index << " " <<  mesh_vertices[index].p;
}

}
