#include "searchinstance.h"
#include "expansion.h"
#include "geometry.h"
#include "searchnode.h"
#include "successor.h"
#include "vertex.h"
#include "mesh.h"
#include "point.h"
#include "consts.h"
#include <queue>
#include <vector>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <ctime>

namespace polyanya
{

PointLocation SearchInstance::get_point_location(Point p)
{
    assert(mesh != nullptr);
    PointLocation out = mesh->get_point_location(p);
    if (out.type == PointLocation::ON_CORNER_VERTEX_AMBIG)
    {
        // Add a few EPSILONS to the point and try again.
        static const Point CORRECTOR = {EPSILON * 10, EPSILON * 10};
        Point corrected = p + CORRECTOR;
        PointLocation corrected_loc = mesh->get_point_location(corrected);

        #ifndef NDEBUG
        if (verbose)
        {
            std::cerr << p << " " << corrected_loc << std::endl;
        }
        #endif

        switch (corrected_loc.type)
        {
            case PointLocation::ON_CORNER_VERTEX_AMBIG:
            case PointLocation::ON_CORNER_VERTEX_UNAMBIG:
            case PointLocation::ON_NON_CORNER_VERTEX:
                #ifndef NDEBUG
                if (verbose)
                {
                    std::cerr << "Warning: corrected " << p << " lies on vertex"
                              << std::endl;
                }
                #endif
            case PointLocation::NOT_ON_MESH:
                #ifndef NDEBUG
                if (verbose)
                {
                    std::cerr << "Warning: completely ambiguous point at " << p
                              << std::endl;
                }
                #endif
                break;

            case PointLocation::IN_POLYGON:
            case PointLocation::ON_MESH_BORDER:
            // Note that ON_EDGE should be fine: any polygon works and there's
            // no need to special case successor generation.
            case PointLocation::ON_EDGE:
                out.poly1 = corrected_loc.poly1;
                break;

            default:
                // Should be impossible to reach.
                assert(false);
                break;
        }
    }
    return out;
}

int SearchInstance::succ_to_node(
    SearchNodePtr parent, Successor* successors, int num_succ,
    SearchNode* nodes
)
{
    assert(mesh != nullptr);
    const Polygon& polygon = mesh->mesh_polygons[parent->next_polygon];
    const std::vector<int>& V = polygon.vertices;
    const std::vector<int>& P = polygon.polygons;

    double right_g = -1, left_g = -1;

    int out = 0;
    for (int i = 0; i < num_succ; i++)
    {
        const Successor& succ = successors[i];
        const int next_polygon = P[succ.poly_left_ind];
        if (next_polygon == -1)
        {
            continue;
        }

        // If the successor we're about to push pushes into a one-way polygon,
        // and the polygon isn't the end polygon, just continue.
        if (mesh->mesh_polygons[next_polygon].is_one_way &&
            next_polygon != end_polygon)
        {
            continue;
        }
        const int left_vertex  = V[succ.poly_left_ind];
        const int right_vertex = succ.poly_left_ind ?
                                 V[succ.poly_left_ind - 1] :
                                 V.back();

        // Note that g is evaluated twice here. (But this is a lambda!)
        // Always try to precompute before using this macro.
        // We implicitly set h to be zero and let search() update it.
        const auto p = [&](const int root, const double g)
        {
            if (root != -1)
            {
                assert(root >= 0 && root < (int) root_g_values.size());
                // Can POSSIBLY prune?
                if (root_search_ids[root] != search_id)
                {
                    // First time reaching root
                    root_search_ids[root] = search_id;
                    root_g_values[root] = g;
                }
                else
                {
                    // We've been here before!
                    // Check whether we've done better.
                    if (root_g_values[root] + EPSILON < g)
                    {
                        // We've done better!
                        return;
                    }
                    else
                    {
                        // This is better.
                        root_g_values[root] = g;
                    }
                }
            }
            nodes[out++] = {nullptr, root, succ.left, succ.right, left_vertex,
                right_vertex, next_polygon, g, g};
        };

        const Point& parent_root = (parent->root == -1 ?
                                    start :
                                    mesh->mesh_vertices[parent->root].p);
        #define get_g(new_root) parent->g + parent_root.distance(new_root)

        switch (succ.type)
        {
            case Successor::RIGHT_NON_OBSERVABLE:
                if (right_g == -1)
                {
                    right_g = get_g(parent->right);
                }
                p(parent->right_vertex, right_g);
                break;

            case Successor::OBSERVABLE:
                p(parent->root, parent->g);
                break;

            case Successor::LEFT_NON_OBSERVABLE:
                if (left_g == -1)
                {
                    left_g = get_g(parent->left);
                }
                p(parent->left_vertex, left_g);
                break;

            default:
                assert(false);
                break;
        }
        #undef get_h
        #undef get_g
    }

    return out;
}

void SearchInstance::set_end_polygon()
{
    // Any polygon is fine.
    end_polygon = get_point_location(goal).poly1;
}

void SearchInstance::gen_initial_nodes()
{
    // {parent, root, left, right, next_polygon, right_vertex, f, g}
    // be VERY lazy and abuse how our function expands collinear search nodes
    // if right_vertex is not valid, it will generate EVERYTHING
    // and we can set right_vertex if we want to omit generating an interval.
    const PointLocation pl = get_point_location(start);
    const double h = start.distance(goal);
    #define get_lazy(next, left, right) new (node_pool->allocate()) SearchNode \
        {nullptr, -1, start, start, left, right, next, h, 0}

    #define v(vertex) mesh->mesh_vertices[vertex]

    const auto push_lazy = [&](SearchNodePtr lazy)
    {
        const int poly = lazy->next_polygon;
        if (poly == -1)
        {
            return;
        }
        if (poly == end_polygon)
        {
            // Trivial case - we can see the goal from start!
            final_node = lazy;
            #ifndef NDEBUG
            if (verbose)
            {
                std::cerr << "got a trivial case!" << std::endl;
            }
            #endif
            // we should check final_node after each push_lazy
            return;
        }
        // iterate over poly, throwing away vertices if needed
        const std::vector<int>& vertices =
            mesh->mesh_polygons[poly].vertices;
        Successor* successors = new Successor [vertices.size()];
        int last_vertex = vertices.back();
        int num_succ = 0;
        for (int i = 0; i < (int) vertices.size(); i++)
        {
            const int vertex = vertices[i];
            if (vertex == lazy->right_vertex ||
                last_vertex == lazy->left_vertex)
            {
                last_vertex = vertex;
                continue;
            }
            successors[num_succ++] =
                {Successor::OBSERVABLE, v(vertex).p,
                 v(last_vertex).p, i};
            last_vertex = vertex;
        }
        SearchNode* nodes = new SearchNode [num_succ];
        const int num_nodes = succ_to_node(lazy, successors,
                                           num_succ, nodes);
        delete[] successors;
        for (int i = 0; i < num_nodes; i++)
        {
            SearchNodePtr n = new (node_pool->allocate())
                SearchNode(nodes[i]);
            const Point& n_root = (n->root == -1 ? start :
                                   mesh->mesh_vertices[n->root].p);
            n->f += get_h_value(n_root, goal, n->left, n->right);
            n->parent = lazy;
            #ifndef NDEBUG
            if (verbose)
            {
                std::cerr << "generating init node: ";
                print_node(n, std::cerr);
                std::cerr << std::endl;
            }
            #endif
            open_list.push(n);
        }
        delete[] nodes;
        nodes_generated += num_nodes;
        nodes_pushed += num_nodes;
    };

    switch (pl.type)
    {
        // Don't bother.
        case PointLocation::NOT_ON_MESH:
            break;

        // Generate all in an arbirary polygon.
        case PointLocation::ON_CORNER_VERTEX_AMBIG:
            // It's possible that it's -1!
            if (pl.poly1 == -1)
            {
                break;
            }
        case PointLocation::ON_CORNER_VERTEX_UNAMBIG:
        // Generate all in the polygon.
        case PointLocation::IN_POLYGON:
        case PointLocation::ON_MESH_BORDER:
        {
            SearchNodePtr lazy = get_lazy(pl.poly1, -1, -1);
            push_lazy(lazy);
            nodes_generated++;
        }
            break;

        case PointLocation::ON_EDGE:
            // Generate all in both polygons except for the shared side.
        {
            SearchNodePtr lazy1 = get_lazy(pl.poly2, pl.vertex1, pl.vertex2);
            SearchNodePtr lazy2 = get_lazy(pl.poly1, pl.vertex2, pl.vertex1);
            push_lazy(lazy1);
            nodes_generated++;
            if (final_node)
            {
                return;
            }
            push_lazy(lazy2);
            nodes_generated++;
        }
            break;


        case PointLocation::ON_NON_CORNER_VERTEX:
        {
            for (int& poly : v(pl.vertex1).polygons)
            {
                SearchNodePtr lazy = get_lazy(poly, pl.vertex1, pl.vertex1);
                push_lazy(lazy);
                nodes_generated++;
                if (final_node)
                {
                    return;
                }
            }
        }
            break;


        default:
            assert(false);
            break;
    }

    #undef v
    #undef get_lazy
}

#define root_to_point(root) ((root) == -1 ? start : mesh->mesh_vertices[root].p)

bool SearchInstance::search()
{
    timer.start();
    init_search();
    if (mesh == nullptr || end_polygon == -1)
    {
        timer.stop();
        return false;
    }

    if (final_node != nullptr)
    {
        timer.stop();
        return true;
    }

    while (!open_list.empty())
    {
        SearchNodePtr node = open_list.top(); open_list.pop();

        #ifndef NDEBUG
        if (verbose)
        {
            std::cerr << "popped off: ";
            print_node(node, std::cerr);
            std::cerr << std::endl;
        }
        #endif

        nodes_popped++;
        const int next_poly = node->next_polygon;
        if (next_poly == end_polygon)
        {
            // Make the TRUE final node.
            // (We usually push it onto the open list, but we know it's going
            // to be immediately popped off anyway.)

            // We need to find whether we need to turn left/right to ge
            // to the goal, so we do an orientation check like how we
            // special case triangle successors.

            const int final_root = [&]()
            {
                const Point& root = root_to_point(node->root);
                const Point root_goal = goal - root;
                // If root-left-goal is not CW, use left.
                if (root_goal * (node->left - root) < -EPSILON)
                {
                    return node->left_vertex;
                }
                // If root-right-goal is not CCW, use right.
                if ((node->right - root) * root_goal < -EPSILON)
                {
                    return node->right_vertex;
                }
                // Use the normal root.
                return node->root;
            }();

            const SearchNodePtr true_final =
                new (node_pool->allocate()) SearchNode
                {node, final_root, goal, goal, -1, -1, end_polygon,
                 node->f, node->g};

            nodes_generated++;

            timer.stop();

            #ifndef NDEBUG
            if (verbose)
            {
                std::cerr << "found end - terminating!" << std::endl;
            }
            #endif

            final_node = true_final;
            return true;
        }
        // We will never update our root list here.
        const int root = node->root;
        if (root != -1)
        {
            assert(root >= 0 && root < (int) root_g_values.size());
            if (root_search_ids[root] == search_id)
            {
                // We've been here before!
                // Check whether we've done better.
                if (root_g_values[root] + EPSILON < node->g)
                {
                    nodes_pruned_post_pop++;

                    #ifndef NDEBUG
                    if (verbose)
                    {
                        std::cerr << "node is dominated!" << std::endl;
                    }
                    #endif

                    // We've done better!
                    continue;
                }
            }
        }
        int num_nodes = 1;
        search_nodes_to_push[0] = *node;

        // We use a do while here because the first iteration is guaranteed
        // to work.
        do
        {
            SearchNode cur_node = search_nodes_to_push[0];
            // don't forget this!!!
            if (cur_node.next_polygon == end_polygon)
            {
                break;
            }
            int num_succ = get_successors(cur_node, start, *mesh,
                                          search_successors);
            successor_calls++;
            num_nodes = succ_to_node(&cur_node, search_successors,
                                     num_succ, search_nodes_to_push);
            if (num_nodes == 1)
            {
                // Did we turn?
                if (cur_node.g != search_nodes_to_push[0].g)
                {
                    // Turned. Set the parent of this, and set the current
                    // node pointer to this after allocating space for it.
                    search_nodes_to_push[0].parent = node;
                    node = new (node_pool->allocate())
                        SearchNode(search_nodes_to_push[0]);
                    nodes_generated++;
                }

                #ifndef NDEBUG
                if (verbose)
                {
                    std::cerr << "\tintermediate: ";
                    print_node(&search_nodes_to_push[0], std::cerr);
                    std::cerr << std::endl;
                }
                #endif
            }
        }
        while (num_nodes == 1); // if num_nodes == 0, we still want to break

        for (int i = 0; i < num_nodes; i++)
        {
            const SearchNode &cur_node = search_nodes_to_push[i];
            // We need to update the h value before we push!

            // if we turned before, AND we immediately broke out of the
            // do-while loop, then cur_node has a valid parent pointer
            // and cur_node is equivalent to node.
            // so: if cur_node has a parent pointer, then "node"
            // is the thing we want to push.
            SearchNodePtr n = nullptr;
            if (cur_node.parent)
            {
                // valid!
                n = node;
                // note that we should decrement nodes_generated here
                // as we overcount because of this
                nodes_generated--;
            }
            else
            {
                // need to allocate it in mempool
                // AND set a valid parent pointer
                n = new (node_pool->allocate()) SearchNode(cur_node);
                n->parent = node;
            }
            const Point& n_root = (n->root == -1 ? start :
                                   mesh->mesh_vertices[n->root].p);
            n->f += get_h_value(n_root, goal, n->left, n->right);

            #ifndef NDEBUG
            if (verbose)
            {
                std::cerr << "\tpushing: ";
                print_node(n, std::cerr);
                std::cerr << std::endl;
            }
            #endif

            open_list.push(n);
        }
        nodes_generated += num_nodes;
        nodes_pushed += num_nodes;
    }

    timer.stop();
    return false;
}

void SearchInstance::print_node(SearchNodePtr node, std::ostream& outfile)
{
    outfile << "root=" << root_to_point(node->root) << "; left=" << node->left
            << "; right=" << node->right << "; f=" << node->f << ", g="
            << node->g;
    /*
    outfile << "; col=" << [&]() -> std::string
            {
                switch (node->col_type)
                {
                    case SearchNode::NOT:
                        return "NOT";
                    case SearchNode::RIGHT:
                        return "RIGHT";
                    case SearchNode::LEFT:
                        return "LEFT";
                    case SearchNode::LAZY:
                        return "LAZY";
                    default:
                        return "";
                }
            }();
    */
}

void SearchInstance::get_path_points(std::vector<Point>& out)
{
    if (final_node == nullptr)
    {
        return;
    }
    out.clear();
    out.push_back(goal);
    SearchNodePtr cur_node = final_node;

    while (cur_node != nullptr)
    {
        if (root_to_point(cur_node->root) != out.back())
        {
            out.push_back(root_to_point(cur_node->root));
        }
        cur_node = cur_node->parent;
    }
    std::reverse(out.begin(), out.end());
}

void SearchInstance::print_search_nodes(std::ostream& outfile)
{
    if (final_node == nullptr)
    {
        return;
    }
    SearchNodePtr cur_node = final_node;
    while (cur_node != nullptr)
    {
        print_node(cur_node, outfile);
        outfile << std::endl;
        mesh->print_polygon(outfile, cur_node->next_polygon);
        outfile << std::endl;
        cur_node = cur_node->parent;
    }
}

#undef root_to_point

}
