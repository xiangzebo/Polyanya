#pragma once
#include "point.h"

namespace polyanya
{

// A search node.
// Only makes sense given a mesh and an endpoint, which the node does not store.
// This means that the f value needs to be set manually.
struct SearchNode
{
    SearchNode* parent;
    // Note that all Points here will be in terms of a Cartesian plane.
    int root; // -1 if start

    // If possible, set the orientation of left / root / right to be
    // "if I'm standing at 'root' and look at 'left', 'right' is on my right"
    Point left, right;

    // The left vertex of the edge the interval is lying on.
    // When generating the successors of this node, end there.
    int left_vertex;

    // The right vertex of the edge the interval is lying on.
    // When generating the successors of this node, start there.
    int right_vertex;

    //current polygon.
    int cur_polygon;

    // Index of the polygon we're going to "push" into.
    // Every successor must lie within this polygon.
    int next_polygon;

    double f, g;

    // Comparison.
    // Always take the "smallest" search node in a priority queue.
    bool operator<(const SearchNode& other) const
    {
        if (this->f == other.f)
        {
            // If two nodes have the same f, the one with the bigger g
            // is "smaller" to us.
            return this->g > other.g;
        }
        return this->f < other.f;
    }

    bool operator>(const SearchNode& other) const
    {
        if (this->f == other.f)
        {
            return this->g < other.g;
        }
        return this->f > other.f;
    }

    friend std::ostream& operator<<(std::ostream& stream, const SearchNode& sn)
    {
        return stream << "SearchNode ([" << sn.root << ", [" << sn.left << ", "
                      << sn.right << "]], f=" << sn.f << ", g=" << sn.g
                      << ", poly=" << sn.next_polygon << ")";
    }
};

typedef SearchNode* SearchNodePtr;

}
