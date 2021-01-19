#include "expansion.h"
#include "searchnode.h"
#include "successor.h"
#include "mesh.h"
#include "geometry.h"
#include "vertex.h"
#include "point.h"
#include "consts.h"
#include <cassert>
#include <vector>

namespace polyanya
{

// Gets the h value of a search node with interval l-r and root "root",
// given a goal.
double get_h_value(const Point& root, Point goal,
                   const Point& l, const Point& r)
{
    if (root == l || root == r)
    {
        return root.distance(goal);
    }
    // First, check whether goal and root are on the same side of the interval.
    // If either are collinear with r/l, reflecting does nothing.
    const Point lr = r - l;
    const Point lroot = root - l;
    Point lgoal = goal - l;
    if ((lroot * lr > 0) == (lgoal * lr > 0))
    {
        // Need to reflect.
        goal = reflect_point(goal, l, r);
        lgoal = goal - l;
    }
    // Now we do the actual line intersection test.
    const double denom = (goal - root) * lr;
    if (std::abs(denom) < EPSILON)
    {
        // Root, goal, L and R are ALL collinear!
        // Take the best one of root-L-goal and root-R-goal.

        // We can be sneaky and use the distance squared as we always want the
        // endpoint which is closest to the root.
        const double root_l = root.distance_sq(l);
        const double root_r = root.distance_sq(r);

        // If they're the same or within an epsilon we don't care which one we
        // use.

        if (root_l < root_r)
        {
            // L is better.
            return std::sqrt(root_l) + l.distance(goal);
        }
        else
        {
            // R is better.
            return std::sqrt(root_r) + r.distance(goal);
        }
    }
    const double lr_num = lgoal * lroot;
    const ZeroOnePos lr_pos = line_intersect_bound_check(lr_num, denom);
    switch (lr_pos)
    {
        case ZeroOnePos::LT_ZERO:
            // Too far left.
            // Use left end point.
            return root.distance(l) + l.distance(goal);

        case ZeroOnePos::EQ_ZERO:
        case ZeroOnePos::IN_RANGE:
        case ZeroOnePos::EQ_ONE:
            // Line goes through interval, so just use the direct distance.
            return root.distance(goal);

        case ZeroOnePos::GT_ONE:
            // Too far right.
            // Use right end point.
            return root.distance(r) + r.distance(goal);

        default:
            // Unreachable.
            assert(false);
            return -1;
    }
}

// Internal binary search helper.
// All indices must be within the range [0, 2 * N - 1] to make binary search
// easier. You can normalise an index with this macro:
#define normalise(index) (index) - ((index) >= N ? N : 0)
// Assume that there exists at least one element within the range which
// satisifies the predicate.
template<typename Type, typename Pred>
inline int binary_search(const std::vector<int>& arr, const int N,
                         const std::vector<Type>& objects, int lower, int upper,
                         const Pred pred, const bool is_upper_bound)
{
    if (lower == upper) return lower;
    int best_so_far = -1;
    while (lower <= upper)
    {
        const int mid = lower + (upper - lower) / 2;
        const bool matches_pred = pred(objects[arr[normalise(mid)]]);
        if (matches_pred)
        {
            best_so_far = mid;
        }
        // If we're looking for an upper bound:
            // If we match the predicate, go higher.
            // If not, go lower.
        // If we're looking for a lower bound:
            // If we match the predicate, go lower.
            // If not, go higher.
        if (matches_pred == is_upper_bound)
        {
            // Either "upper bound AND matches pred"
            // or "lower bound AND doesn't match pred"
            // We should go higher, so increase the lower bound.
            lower = mid + 1;
        }
        else
        {
            // The opposite.
            // Decrease the upper bound.
            upper = mid - 1;
        }
    }
    return best_so_far;
}

// TODO: Wrap this in a class so we don't have to keep passing the same params
// over and over again
// Generates the successors of the search node and sets them in the successor
// vector. Returns number of successors generated.
int get_successors(SearchNode& node, const Point& start, const Mesh& mesh,
                   Successor* successors)
{
    // If the next polygon is -1, we did a bad job at pruning...
    assert(node.next_polygon != -1);

    const Polygon& polygon = mesh.mesh_polygons[node.next_polygon];
    const std::vector<Vertex>& mesh_vertices = mesh.mesh_vertices;
    // V, P and N are solely used for conciseness
    const std::vector<int>& V = polygon.vertices;
    const int N = (int) V.size();

    const Point& root = (node.root == -1 ? start : mesh_vertices[node.root].p);

    int out = 0;

    assert(get_orientation(root, node.left, node.right) !=
           Orientation::CCW);

    {
        // Check collinearity.
        const Point root_l = node.left - root;
        const Point root_r = node.right - root;
        #define is_zero(n) (std::abs(n) < EPSILON)
        const bool root_eq_l = is_zero(root_l.x) && is_zero(root_l.y);
        const bool root_eq_r = is_zero(root_r.x) && is_zero(root_r.y);
        #undef is_zero

        if (root_eq_l || root_eq_r || is_collinear(root, node.right, node.left))
        {
            // It's collinear... but we don't know where to turn.
            // Find which endpoint is closer.
            // We can terminate early if we know the root is equal to one
            // of the endpoints.
            // Additionally, we can simply compare the absolute values of
            // the coordinates to find which is closer.
            Successor::Type succ_type;
            if (root_eq_l || (!root_eq_r &&
                (std::abs(root_l.x - root_r.x) < EPSILON ?
                 std::abs(root_l.y) < std::abs(root_r.y) :
                 std::abs(root_l.x) < std::abs(root_r.x)
                )))
            {
                // We should turn at L... if we can!
                if (!mesh_vertices[node.left_vertex].is_corner)
                {
                    return 0;
                }
                succ_type = Successor::LEFT_NON_OBSERVABLE;
            }
            else
            {
                // We should turn at R... if we can!
                if (!mesh_vertices[node.right_vertex].is_corner)
                {
                    return 0;
                }
                succ_type = Successor::RIGHT_NON_OBSERVABLE;
            }

            // We can be lazy and start iterating from any point.
            // We still need to exclude the current interval as a successor.
            int last_vertex = V.back();

            for (int i = 0; i < N; i++)
            {
                const int this_vertex = V[i];
                if (this_vertex == node.right_vertex)
                {
                    // The interval we're going to generate is the same as our
                    // current one, so skip it.
                    last_vertex = this_vertex;
                    continue;
                }
                const Point& left = mesh_vertices[this_vertex].p;
                const Point& right = mesh_vertices[last_vertex].p;
                successors[out++] = {succ_type, left, right, i};
                last_vertex = this_vertex;
            }
            return out;
        }
    }


    if (N == 3)
    {
        int p1; // V[p1] = t2. Used for poly_left_ind for 1-2 successors.
        int p2; // V[p2] = t3. Used for poly_left_ind for 2-3 successors.
        // Note that p3 is redundant, as that's the polygon we came from.

        // The right point of the triangle.
        const Point& t1 = mesh_vertices[node.right_vertex].p;
        // The middle point of the triangle.
        const Point& t2 = [&]() -> const Point&
        {
            // horrible hacky lambda which also sets p1/p2

            // Let's get p1, p2 and t2.
            if (V[0] == node.right_vertex)
            {
                // t1 = V[0], t2 = V[1], t3 = V[2]
                p1 = 1;
                p2 = 2;
                return mesh_vertices[V[1]].p;
            }
            else if (V[0] == node.left_vertex)
            {
                // t1 = V[1], t2 = V[2], t3 = V[0]
                p1 = 2;
                p2 = 0;
                return mesh_vertices[V[2]].p;
            }
            else
            {
                // t1 = V[2], t2 = V[0], t3 = V[1]
                p1 = 0;
                p2 = 1;
                return mesh_vertices[V[0]].p;
            }
        }();
        // The left point of the triangle.
        const Point& t3 = mesh_vertices[node.left_vertex].p;



        const Point& L = node.left;
        const Point& R = node.right;

        // Now we need to check the orientation of root-L-t2.
        // TODO: precompute a shared term for getting orientation,
        // like t2 - root.
        switch (get_orientation(root, L, t2))
        {
            case Orientation::CCW:
            {
                // LI in (1, 2)
                // RI in [1, 2)

                // TODO: precompute shared constants (assuming the compiler
                // doesn't)
                const Point LI = line_intersect(t1, t2, root, L);
                const Point RI = (R == t1 ? t1 :
                                  line_intersect(t1, t2, root, R));

                // observable(RI, LI)
                successors[0] = {
                    Successor::OBSERVABLE,
                    LI, RI,
                    p1 // a 1-2 successor
                };

                // if we can turn left
                if (mesh_vertices[node.left_vertex].is_corner && L == t3)
                {
                    // left_non_observable(LI, 2)
                    successors[1] = {
                        Successor::LEFT_NON_OBSERVABLE,
                        t2, LI,
                        p1 // a 1-2 successor
                    };
                    // left_collinear(2, 3)
                    successors[2] = {
                        Successor::LEFT_NON_OBSERVABLE,
                        t3, t2,
                        p2 // a 2-3 successor
                    };

                    return 3;
                }

                return 1;
            }

            case Orientation::COLLINEAR:
            {
                // LI = 2
                // RI in [1, 2)
                const Point RI = (R == t1 ? t1 :
                                  line_intersect(t1, t2, root, R));

                // observable(RI, 2)
                successors[0] = {
                    Successor::OBSERVABLE,
                    t2, RI,
                    p1 // a 1-2 successor
                };

                // if we can turn left
                if (mesh_vertices[node.left_vertex].is_corner && L == t3)
                {
                    // left_collinear(2, 3)
                    successors[1] = {
                        Successor::LEFT_NON_OBSERVABLE,
                        t3, t2,
                        p2 // a 2-3 successor
                    };

                    return 2;
                }

                return 1;
            }

            case Orientation::CW:
            {
                // LI in (2, 3]
                const Point LI = (L == t3 ? t3 :
                                  line_intersect(t2, t3, root, L));

                // Now we need to check the orientation of root-R-t2.
                switch (get_orientation(root, R, t2))
                {
                    case Orientation::CW:
                    {
                        // RI in (2, 3)
                        const Point RI = line_intersect(t2, t3, root, R);

                        // if we can turn right
                        if (mesh_vertices[node.right_vertex].is_corner &&
                            R == t1)
                        {
                            // right_collinear(1, 2)
                            successors[0] = {
                                Successor::RIGHT_NON_OBSERVABLE,
                                t2, t1,
                                p1 // a 1-2 successor
                            };

                            // right_non_observable(2, RI)
                            successors[1] = {
                                Successor::RIGHT_NON_OBSERVABLE,
                                RI, t2,
                                p2 // a 2-3 successor
                            };

                            // observable(RI, LI)
                            successors[2] = {
                                Successor::OBSERVABLE,
                                LI, RI,
                                p2 // a 2-3 successor
                            };

                            return 3;
                        }

                        // observable(RI, LI)
                        successors[0] = {
                            Successor::OBSERVABLE,
                            LI, RI,
                            p2 // a 2-3 successor
                        };

                        return 1;
                    }

                    case Orientation::COLLINEAR:
                    {
                        // RI = 2
                        // if we can turn right
                        if (mesh_vertices[node.right_vertex].is_corner &&
                            R == t1)
                        {
                            // right_collinear(1, 2)
                            successors[0] = {
                                Successor::RIGHT_NON_OBSERVABLE,
                                t2, t1,
                                p1 // a 1-2 successor
                            };

                            // observable(2, LI)
                            successors[1] = {
                                Successor::OBSERVABLE,
                                LI, t2,
                                p2 // a 2-3 successor
                            };

                            return 2;
                        }

                        // observable(2, LI)
                        successors[0] = {
                            Successor::OBSERVABLE,
                            LI, t2,
                            p2 // a 2-3 successor
                        };

                        return 1;
                    }

                    case Orientation::CCW:
                    {
                        // RI in [1, 2)
                        const Point RI = (R == t1 ? t1 :
                                          line_intersect(t1, t2, root, R));

                        // observable(RI, 2)
                        successors[0] = {
                            Successor::OBSERVABLE,
                            t2, RI,
                            p1 // a 1-2 successor
                        };

                        // observable(2, LI)
                        successors[1] = {
                            Successor::OBSERVABLE,
                            LI, t2,
                            p2 // a 2-3 successor
                        };

                        return 2;
                    }

                    default:
                        assert(false);
                }
            }

            default:
                assert(false);
        }
    }


    // It is not collinear.
    // Find the starting vertex (the "right" vertex).

    // Note that "_ind" means "index in V/P",
    // "_vertex" means "index of mesh_vertices".
    // "_vertex_obj" means "object of the vertex" and
    // "_p" means "point".
    const int right_ind = [&]() -> int
    {
        // TODO: Compare to std::find.
        int temp = 0; // position of vertex in V
        while (V[temp] != node.right_vertex)
        {
            temp++;
            assert(temp < N);
        }
        return temp;
    }();
    // Note that left_ind MUST be greater than right_ind.
    // This will make binary searching easier.
    const int left_ind = N + right_ind - 1;


    assert(V[normalise(left_ind)] == node.left_vertex);

    // Find whether we can turn at either endpoint.
    const Vertex& right_vertex_obj = mesh_vertices[node.right_vertex];
    const Vertex& left_vertex_obj  = mesh_vertices[V[normalise(left_ind)]];

    const Point& right_p = right_vertex_obj.p;
    const Point& left_p  = left_vertex_obj.p;
    const bool right_lies_vertex = right_p == node.right;
    const bool left_lies_vertex  = left_p == node.left;

    // Macro for getting a point from a polygon point index.
    #define index2point(index) mesh_vertices[V[index]].p

    // find the transition between non-observable-right and observable.
    // we will call this A, defined by:
    // "first P such that root-right-p is strictly CCW".
    // lower bound is right+1, as root-right-right is not CCW (it is collinear).
    // upper bound is left.
    // the "transition" will lie in the range [A-1, A)

    const Point root_right = node.right - root;
    const int A = [&]()
    {
        if (right_lies_vertex)
        {
            // Check whether root-right-right+1 is collinear or CCW.
            if (root_right *
                (index2point(normalise(right_ind + 1)) - node.right) >
                -EPSILON)
            {
                // Intersects at right, so...
                // we should use right_ind+1!
                return right_ind + 1;
            }
        }
        return binary_search(V, N, mesh_vertices, right_ind + 1, left_ind,
            [&root_right, &node](const Vertex& v)
            {
                // STRICTLY CCW.
                return root_right * (v.p - node.right) > EPSILON;
            }, false
        );
    }();
    assert(A != -1);
    const int normalised_A = normalise(A),
              normalised_Am1 = normalise(A-1);

    const Point& A_p = index2point(normalised_A);
    const Point& Am1_p = index2point(normalised_Am1);
    const Point right_intersect = right_lies_vertex && A == right_ind + 1 ? node.right : line_intersect(A_p, Am1_p, root, node.right);

    // find the transition between observable and non-observable-left.
    // we will call this B, defined by:
    // "first P such that root-left-p is strictly CW".
    // lower-bound is A - 1 (in the same segment as A).
    // upper bound is left-1, as we don't want root-left-left.
    // the "transition" will lie in the range (B, B+1]
    const Point root_left = node.left - root;
    const int B = [&]()
    {
        if (left_lies_vertex)
        {
            // Check whether root-left-left-1 is collinear or CW.
            if (root_left *
                (index2point(normalise(left_ind - 1)) - node.left) <
                EPSILON)
            {
                // Intersects at left, so...
                // we should use left_ind-1!
                return left_ind - 1;
            }
        }
        return binary_search(V, N, mesh_vertices, A - 1, left_ind - 1,
            [&root_left, &node](const Vertex& v)
            {
                // STRICTLY CW.
                return root_left * (v.p - node.left) < -EPSILON;
            }, true
        );
    }();
    assert(B != -1);
    const int normalised_B = normalise(B),
              normalised_Bp1 = normalise(B+1);
    const Point& B_p = index2point(normalised_B);
    const Point& Bp1_p = index2point(normalised_Bp1);
    const Point left_intersect = left_lies_vertex && B == left_ind - 1 ? node.left : line_intersect(B_p, Bp1_p, root, node.left);

    // Macro to update this_inde/last_ind.
    #define update_ind() last_ind = cur_ind++; if (cur_ind == N) cur_ind = 0
    if (right_lies_vertex && right_vertex_obj.is_corner)
    {
        // Generate non-observable.

        // Generate non-observable to Am1.
        // Generate non-observable from Am1 to intersect
        // if right_intersect != Am1_p.

        // We always generate successors from last_ind to cur_ind.
        // right_ind should always be normalised.
        assert(normalise(right_ind) == right_ind);
        int last_ind = right_ind;
        int cur_ind = normalise(right_ind + 1);

        // Generate non-observable to Am1.
        while (last_ind != normalised_Am1)
        {
            // Generate last-cur, turning at right.
            successors[out++] = {
                Successor::RIGHT_NON_OBSERVABLE,
                index2point(cur_ind), index2point(last_ind),
                cur_ind
            };

            update_ind();
        }
        assert(cur_ind == normalised_A);

        if (right_intersect != Am1_p)
        {
            // Generate Am1-right_intersect, turning at right.
            successors[out++] = {
                Successor::RIGHT_NON_OBSERVABLE,
                right_intersect, Am1_p,
                normalised_A
            };
        }
    }

    // Start at Am1.
    // last_node = right_intersect
    // If index is normalised_Bp1, go from last_node to left_intersect.
    // (And terminate too!)
    // Else, go to the end and set that as last_node

    // Special case when there are NO observable successors.
    if (A == B + 2)
    {
        // Do nothing.
    }
    // Special case when there only exists one observable successor.
    // Note that we used the non-normalised indices for this.
    else if (A == B + 1)
    {
        successors[out++] = {
            Successor::OBSERVABLE,
            left_intersect, right_intersect,
            normalised_A // (the same as normalised_Bp1)
        };
    }
    else
    {
        // Generate first (probably non-maximal) successor
        // (right_intersect-A)
        successors[out++] = {
            Successor::OBSERVABLE,
            A_p, right_intersect,
            normalised_A
        };

        // Generate all guaranteed-maximal successors.
        // Should generate B-A of them.
        int last_ind = normalised_A;
        int cur_ind = normalise(A+1);

        #ifndef NDEBUG
        int counter = 0;
        #endif

        while (last_ind != normalised_B)
        {
            #ifndef NDEBUG
            counter++;
            #endif

            // Generate last-cur.
            successors[out++] = {
                Successor::OBSERVABLE,
                index2point(cur_ind), index2point(last_ind),
                cur_ind
            };

            update_ind();
        }

        #ifndef DEBUG
        assert(counter == B - A);
        #endif

        // Generate last (probably non-maximal) successor
        // (B-left_intersect)
        successors[out++] = {
            Successor::OBSERVABLE,
            left_intersect, B_p,
            normalised_Bp1
        };
    }

    if (left_lies_vertex && left_vertex_obj.is_corner)
    {
        // Generate non-observable from left_intersect to Bp1_p
        // if left_intersect != Bp1_p.
        // Generate non-observable up to end.
        // Generate left_intersect-Bp1, turning at left.
        if (left_intersect != Bp1_p)
        {
            successors[out++] = {
                Successor::LEFT_NON_OBSERVABLE,
                Bp1_p, left_intersect,
                normalised_Bp1
            };
        }

        int last_ind = normalised_Bp1;
        int cur_ind = normalise(B + 2);

        const int normalised_left_ind = normalise(left_ind);
        while (last_ind != normalised_left_ind)
        {
            // Generate last_ind-cur_ind, turning at left.
            successors[out++] = {
                Successor::LEFT_NON_OBSERVABLE,
                index2point(cur_ind), index2point(last_ind),
                cur_ind
            };

            update_ind();
        }
    }

    #undef update_ind

    #undef index_to_point

    return out;
}

#undef normalise

}
