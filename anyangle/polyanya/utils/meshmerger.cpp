// Takes mesh from stdin, outputs to stdout.
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <cassert>
#include <numeric>
#include <climits>
#include <cmath>
#include <queue>
using namespace std;

bool pretty = false;

// We need union find!
struct UnionFind
{
    vector<int> parent;

    UnionFind(int n) : parent(n)
    {
        iota(parent.begin(), parent.end(), 0);
    }

    int find(int x)
    {
        if (x == -1)
        {
            return -1;
        }
        if (parent[x] != x)
        {
            parent[x] = find(parent[x]);
        }
        return parent[x];
    }

    // can't use "union" as that's a keyword!
    // also: don't use union by rank as we need find(x) == x after merge.
    void merge(int x, int y)
    {
        x = find(x);
        y = find(y);
        parent[y] = x;
    }
};

// We need a circular linked list of sorts.
// This is going to be used a lot - for polys around point and for merging the
// two polygon arrays together.
// We'll just use a std::shared_ptr to handle our memory...
struct ListNode
{
    ListNode* next;
    int val;

    ListNode* go(int n) const
    {
        ListNode* out = next;
        for (int i = 1; i < n; i++)
        {
            out = out->next;
        }
        return out;
    }
};

typedef ListNode* ListNodePtr;

vector<ListNodePtr> list_nodes;

ListNodePtr make_node(ListNodePtr next, int val)
{
    ListNodePtr out = new ListNode {next, val};
    list_nodes.push_back(out);
    return out;
}

void delete_nodes()
{
    for (auto x : list_nodes)
    {
        delete x;
    }
}

struct Point
{
    double x, y;

    Point operator+(const Point& other) const
    {
        return {x + other.x, y + other.y};
    }

    Point operator-(const Point& other) const
    {
        return {x - other.x, y - other.y};
    }

    double operator*(const Point& other) const
    {
        return x * other.y - y * other.x;
    }
};

struct Vertex
{
    Point p;
    int num_polygons;
    ListNodePtr polygons;
};

struct Polygon
{
    int num_vertices;
    int num_traversable;
    double area;
    ListNodePtr vertices;
    // Stores the original polygons.
    // To get the actual polygon, do polygon_unions.find on the polygon you get.
    ListNodePtr polygons;
};

struct SearchNode
{
    // Index of poly.
    int index;
    // Area of the best tentative merge.
    double area;

    // Comparison.
    // Always take the "biggest" search node in a priority queue.
    bool operator<(const SearchNode& other) const
    {
        return area < other.area;
    }

    bool operator>(const SearchNode& other) const
    {
        return area > other.area;
    }
};

// We'll keep all vertices, but we may throw them out in the end if num_polygons
// is 0.
// We'll figure it out once we're finished.
vector<Vertex> mesh_vertices;

// We'll also keep all polygons, but we'll throw them out like above.
vector<Polygon> mesh_polygons;

UnionFind polygon_unions(0);

// Actually returns double the area of the polygon...
// Assume that mesh_vertices is populated and is valid.
double get_area(ListNodePtr vertices)
{
    // first point x second point + second point x third point + ...
    double out = 0;

    ListNodePtr start_vertex = vertices;
    bool is_first = true;

    while (is_first || start_vertex != vertices)
    {
        is_first = false;
        out += mesh_vertices[vertices->val].p *
               mesh_vertices[vertices->next->val].p;
        vertices = vertices->next;
    }

    return out;
}


// taken from structs/mesh.cpp
void read_mesh(istream& infile)
{
    #define fail(message) cerr << message << endl; exit(1);
    string header;
    int version;

    if (!(infile >> header))
    {
        fail("Error reading header");
    }
    if (header != "mesh")
    {
        cerr << "Got header '" << header << "'" << endl;
        fail("Invalid header (expecting 'mesh')");
    }

    if (!(infile >> version))
    {
        fail("Error getting version number");
    }
    if (version != 2)
    {
        cerr << "Got file with version " << version << endl;
        fail("Invalid version (expecting 2)");
    }

    int V, P;
    if (!(infile >> V >> P))
    {
        fail("Error getting V and P");
    }
    if (V < 1)
    {
        cerr << "Got " << V << " vertices" << endl;
        fail("Invalid number of vertices");
    }
    if (P < 1)
    {
        cerr << "Got " << P << " polygons" << endl;
        fail("Invalid number of polygons");
    }

    mesh_vertices.resize(V);
    mesh_polygons.resize(P);
    polygon_unions = UnionFind(P);


    for (int i = 0; i < V; i++)
    {
        Vertex& v = mesh_vertices[i];
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
            cerr << "Got " << neighbours << " neighbours" << endl;
            fail("Invalid number of neighbours around a point");
        }

        v.num_polygons = neighbours;
        // Guaranteed to have 2 or more.
        ListNodePtr cur_node = nullptr;
        for (int j = 0; j < neighbours; j++)
        {
            int polygon_index;
            if (!(infile >> polygon_index))
            {
                fail("Error getting a vertex's neighbouring polygon");
            }
            if (polygon_index >= P)
            {
                cerr << "Got a polygon index of " \
                          << polygon_index << endl;
                fail("Invalid polygon index when getting vertex");
            }

            ListNodePtr new_node = make_node(nullptr, polygon_index);

            if (j == 0)
            {
                cur_node = new_node;
                v.polygons = cur_node;
            }
            else
            {
                cur_node->next = new_node;
                cur_node = new_node;
            }
        }
        cur_node->next = v.polygons;
    }


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
            cerr << "Got " << n << " vertices" << endl;
            fail("Invalid number of vertices in polygon");
        }

        p.num_vertices = n;

        ListNodePtr cur_node = nullptr;
        for (int j = 0; j < n; j++)
        {
            int vertex_index;
            if (!(infile >> vertex_index))
            {
                fail("Error getting a polygon's vertex");
            }
            if (vertex_index >= V)
            {
                cerr << "Got a vertex index of " \
                          << vertex_index << endl;
                fail("Invalid vertex index when getting polygon");
            }
            ListNodePtr new_node = make_node(nullptr, vertex_index);

            if (j == 0)
            {
                cur_node = new_node;
                p.vertices = cur_node;
            }
            else
            {
                cur_node->next = new_node;
                cur_node = new_node;
            }
        }
        cur_node->next = p.vertices;

        // don't worry: the old one is still being pointed to
        cur_node = nullptr;
        p.num_traversable = 0;
        for (int j = 0; j < n; j++)
        {
            int polygon_index;
            if (!(infile >> polygon_index))
            {
                fail("Error getting a polygon's neighbouring polygon");
            }
            if (polygon_index >= P)
            {
                cerr << "Got a polygon index of " \
                          << polygon_index << endl;
                fail("Invalid polygon index when getting polygon");
            }

            if (polygon_index != -1)
            {
                p.num_traversable++;
            }
            ListNodePtr new_node = make_node(nullptr, polygon_index);

            if (j == 0)
            {
                cur_node = new_node;
                p.polygons = cur_node;
            }
            else
            {
                cur_node->next = new_node;
                cur_node = new_node;
            }
        }
        cur_node->next = p.polygons;

        p.area = get_area(p.vertices);
        assert(p.area > 0);
    }

    double temp;
    if (infile >> temp)
    {
        fail("Error parsing mesh (read too much)");
    }
    #undef fail
}

inline bool cw(const Point& a, const Point& b, const Point& c)
{
    return (b - a) * (c - b) < -1e-8;
}

// Can polygon x merge with the polygon adjacent to the edge
// (v->next, v->next->next)?
// (The reason for this is because we don't have back pointers, and we need
// to have the vertex before the edge starts).
// Assume that v and p are "aligned", that is, they have been offset by the
// same amount.
// This also means that the actual polygon used will be p->next->next.
// Also assume that x is a valid non-merged polygon.
bool can_merge(int x, ListNodePtr v, ListNodePtr p)
{
    if (polygon_unions.find(x) != x)
    {
        return false;
    }
    const int merge_index = polygon_unions.find(p->go(2)->val);
    if (merge_index == -1)
    {
        return false;
    }
    const Polygon& to_merge = mesh_polygons[merge_index];
    if (to_merge.num_vertices == 0)
    {
        return false;
    }

    // Define (v->next, v->next->next).
    const int A = v->go(1)->val;
    const int B = v->go(2)->val;

    // We want to find (B, A) inside to_merge's vertices.
    // In fact, we want to find the one BEFORE B. We'll call this merge_end.
    // Assert that we have good data - that is, if B appears, A must be next.
    // Also, we can't iterate for more than to_merge.num_vertices.
    ListNodePtr merge_end_v = to_merge.vertices;
    ListNodePtr merge_end_p = to_merge.polygons;
    int counter;
    counter = 0;
    while (merge_end_v->next->val != B)
    {
        merge_end_v = merge_end_v->next;
        merge_end_p = merge_end_p->next;
        counter++;
        assert(counter <= to_merge.num_vertices);
    }
    // Ensure that A comes after B.
    assert(merge_end_v->go(2)->val == A);
    // Ensure that the neighbouring polygon is x.
    assert(polygon_unions.find(merge_end_p->go(2)->val) == x);

    // The merge will change
    // (v, A, B) to (v, A, [3 after merge_end_v]) and
    // (A, B, [3 after v]) to (merge_end_v, B, [3 after v]).
    // If the new ones are clockwise, we must return false.
    #define P(ptr) mesh_vertices[(ptr)->val].p
    if (cw(P(v), P(v->go(1)), P(merge_end_v->go(3))))
    {
        return false;
    }

    if (cw(P(merge_end_v), P(v->go(2)), P(v->go(3))))
    {
        return false;
    }

    #undef P

    return true;
}

// Assuming can_merge like above, merge the polygons.
void merge(int x, ListNodePtr v, ListNodePtr p)
{
    assert(can_merge(x, v, p));
    // Note that because of the way we're merging,
    // the resulting polygon will NOT always have a valid ListNodePtr, so
    // we need to set it ourself.

    const int merge_index = polygon_unions.find(p->go(2)->val);

    Polygon& to_merge = mesh_polygons[polygon_unions.find(merge_index)];

    const int A = v->go(1)->val;
    const int B = v->go(2)->val;

    ListNodePtr merge_end_v = to_merge.vertices;
    ListNodePtr merge_end_p = to_merge.polygons;
    while (merge_end_v->next->val != B)
    {
        merge_end_v = merge_end_v->next;
        merge_end_p = merge_end_p->next;
    }

    // Our A should point to the thing which their A is pointing to.
    // Their B should point to the thing which our B is pointing to.
    ListNodePtr our_A_v_ptr = v->go(1);
    ListNodePtr our_A_p_ptr = p->go(1);
    ListNodePtr our_B_v_ptr = v->go(2);
    ListNodePtr our_B_p_ptr = p->go(2);

    ListNodePtr their_A_v_ptr = merge_end_v->go(2);
    ListNodePtr their_A_p_ptr = merge_end_p->go(2);
    ListNodePtr their_B_v_ptr = merge_end_v->go(1);
    ListNodePtr their_B_p_ptr = merge_end_p->go(1);

    our_A_v_ptr->next = their_A_v_ptr->next;
    our_A_p_ptr->next = their_A_p_ptr->next;
    their_B_v_ptr->next = our_B_v_ptr->next;
    their_B_p_ptr->next = our_B_p_ptr->next;

    // Set the our lists just in case something goes bad.
    // That is: don't set it to our B.
    Polygon& merged = mesh_polygons[x];
    merged.vertices = our_A_v_ptr;
    merged.polygons = our_A_p_ptr;


    // Merge the numbers.
    merged.num_vertices += to_merge.num_vertices - 2;
    merged.num_traversable += to_merge.num_traversable - 2;
    merged.area += to_merge.area;

    // "Delete" the old one.
    to_merge = {0, 0, 0.0, nullptr, nullptr};

    // We now need to delete these in A and B.
    // A will go like (merge_index, x)
    // B will go like (x, merge_index)
    // We need to set both to just x.
    {
        // For A.
        // Once we find something which points to merge_index, point it to the
        // one after.
        ListNodePtr A_polys = mesh_vertices[A].polygons;
        while (polygon_unions.find(A_polys->next->val) != merge_index)
        {
            A_polys = A_polys->next;
        }
        A_polys->next = A_polys->next->next;
        // Set A to be this just in case.
        mesh_vertices[A].polygons = A_polys;
        mesh_vertices[A].num_polygons--;
    }
    {
        // For B.
        // Once we find something which is x, point it to the
        // one after.
        ListNodePtr B_polys = mesh_vertices[B].polygons;
        while (polygon_unions.find(B_polys->val) != x)
        {
            B_polys = B_polys->next;
            // cerr << "maybe even " << merge_index << endl;
            // cerr << "we want " << x << "but we got" << B_polys->val << endl;
        }
        B_polys->next = B_polys->next->next;
        // Set B to be this just in case.
        mesh_vertices[B].polygons = B_polys;
        mesh_vertices[B].num_polygons--;
    }

    // Do the union-find merge.
    // THIS NEEDS TO BE LAST.
    polygon_unions.merge(x, merge_index);
}

void check_correct()
{
    for (int i = 0; i < (int) mesh_vertices.size(); i++)
    {
        Vertex& v = mesh_vertices[i];
        if (v.num_polygons == 0)
        {
            continue;
        }

        int count = 1;
        ListNodePtr cur_node = v.polygons->next;
        while (cur_node != v.polygons)
        {
            assert(count < v.num_polygons);
            cur_node = cur_node->next;
            count++;
        }
        assert(count == v.num_polygons);
    }

    for (int i = 0; i < (int) mesh_polygons.size(); i++)
    {
        Polygon& p = mesh_polygons[i];
        if (polygon_unions.find(i) != i || p.num_vertices == 0)
        {
            // Has been merged.
            continue;
        }

        {
            #define P(ptr) mesh_vertices[(ptr)->val].p
            int count = 1;

            assert(!cw(P(p.vertices), P(p.vertices->next),
                       P(p.vertices->next->next)));
            can_merge(i, p.vertices, p.polygons);

            ListNodePtr cur_node_v = p.vertices->next;
            ListNodePtr cur_node_p = p.polygons->next;
            while (cur_node_v != p.vertices)
            {
                assert(count < p.num_vertices);
                assert(!cw(P(cur_node_v), P(cur_node_v->next),
                           P(cur_node_v->next->next)));
                can_merge(i, cur_node_v, cur_node_p);

                cur_node_v = cur_node_v->next;
                cur_node_p = cur_node_p->next;
                count++;
            }

            assert(count == p.num_vertices);

            #undef P
        }

        {
            int count = 1;
            ListNodePtr cur_node = p.polygons->next;
            while (cur_node != p.polygons)
            {
                assert(count < p.num_vertices);
                cur_node = cur_node->next;
                count++;
            }
            assert(count == p.num_vertices);
        }
    }
}

void merge_deadend()
{
    bool merged = false;
    do
    {
        merged = false;
        for (int i = 0; i < (int) mesh_polygons.size(); i++)
        {
            Polygon& p = mesh_polygons[i];
            if (polygon_unions.find(i) != i || p.num_vertices == 0)
            {
                // Has been merged.
                continue;
            }
            // We want dead ends here.
            if (p.num_traversable != 1)
            {
                continue;
            }

            // Remember that the polygon we merge with is polygons->go(2).

            {
                const int merge_index = polygon_unions.find(
                    p.polygons->go(2)->val);
                if (merge_index != -1 &&
                    mesh_polygons[merge_index].num_traversable <= 2 &&
                    can_merge(i, p.vertices, p.polygons))
                {
                    merge(i, p.vertices, p.polygons);
                    merged = true;
                    continue;
                }
            }

            ListNodePtr cur_node_v = p.vertices->next;
            ListNodePtr cur_node_p = p.polygons->next;
            while (cur_node_v != p.vertices)
            {
                const int merge_index = polygon_unions.find(
                    cur_node_p->go(2)->val);
                if (merge_index != -1 &&
                    mesh_polygons[merge_index].num_traversable <= 2 &&
                    can_merge(i, cur_node_v, cur_node_p))
                {
                    merge(i, cur_node_v, cur_node_p);
                    merged = true;
                    break;
                }

                cur_node_v = cur_node_v->next;
                cur_node_p = cur_node_p->next;
            }
        }
    } while (merged);
}

void naive_merge(bool keep_deadends = true)
{
    bool merged = false;
    do
    {
        merged = false;
        for (int i = 0; i < (int) mesh_polygons.size(); i++)
        {
            Polygon& p = mesh_polygons[i];
            if (polygon_unions.find(i) != i || p.num_vertices == 0)
            {
                // Has been merged.
                continue;
            }

            if (keep_deadends && p.num_traversable == 1)
            {
                // It's a dead end and we want to keep it.
                continue;
            }

            {
                const int merge_index = polygon_unions.find(
                    p.polygons->go(2)->val);
                if (merge_index != -1 &&
                    (!keep_deadends ||
                     mesh_polygons[merge_index].num_traversable > 1) &&
                    can_merge(i, p.vertices, p.polygons))
                {
                    merge(i, p.vertices, p.polygons);
                    merged = true;
                    // prevents an infinite loop?
                    continue;
                }
            }

            ListNodePtr cur_node_v = p.vertices->next;
            ListNodePtr cur_node_p = p.polygons->next;
            while (cur_node_v != p.vertices)
            {
                const int merge_index = polygon_unions.find(
                    cur_node_p->go(2)->val);
                if (merge_index != -1 &&
                    (!keep_deadends ||
                     mesh_polygons[merge_index].num_traversable > 1) &&
                    can_merge(i, cur_node_v, cur_node_p))
                {
                    merge(i, cur_node_v, cur_node_p);
                    merged = true;
                    // break just in case
                    break;
                }

                cur_node_v = cur_node_v->next;
                cur_node_p = cur_node_p->next;
            }
        }
    } while (merged);
}

void smart_merge(bool keep_deadends = true)
{
    priority_queue<SearchNode> pq;
    // As we aren't going to do pq updates, here's a shoddy workaround.
    vector<double> best_merge(mesh_polygons.size(), -1);

    // Pushes a polygon onto the pq as a node.
    // Also updates best_merge.
    auto push_polygon = [&](int i)
    {
        if (i == -1)
        {
            return;
        }
        Polygon& p = mesh_polygons[i];
        if (p.num_vertices == 0)
        {
            // Has been merged.
            return;
        }

        if (keep_deadends && p.num_traversable == 1)
        {
            // It's a dead end and we don't want to merge it.
            return;
        }

        SearchNode this_node = {i, -1};

        ListNodePtr cur_node_v = p.vertices;
        ListNodePtr cur_node_p = p.polygons;
        bool first = true;
        while (first || cur_node_v != p.vertices)
        {
            first = false;
            const int merge_index = polygon_unions.find(cur_node_p->go(2)->val);
            if (merge_index != -1 &&
                (!keep_deadends ||
                 mesh_polygons[merge_index].num_traversable > 1) &&
                can_merge(i, cur_node_v, cur_node_p))
            {
                this_node.area = max(this_node.area,
                    p.area + mesh_polygons[merge_index].area);
            }

            cur_node_v = cur_node_v->next;
            cur_node_p = cur_node_p->next;
        }

        // Chuck it on the pq... if we found a valid merge.
        if (this_node.area != -1)
        {
            pq.push(this_node);
            best_merge[i] = this_node.area;
        }
        else
        {
            // We need to invalidate this if there isn't a valid merge.
            best_merge[i] = -1;
        }
    };

    for (int i = 0; i < (int) mesh_polygons.size(); i++)
    {
        push_polygon(i);
    }


    while (!pq.empty())
    {
        SearchNode node = pq.top(); pq.pop();
        if (abs(node.area - best_merge[node.index]) > 1e-8)
        {
            // Not the right node.
            continue;
        }
        // We got an actual node!
        const Polygon& p = mesh_polygons[node.index];
        // Do the merge.
        // NOW do the merge.
        // We need to find it again, but that should be fine.
        {
            ListNodePtr cur_node_v = p.vertices;
            ListNodePtr cur_node_p = p.polygons;
            bool first = true;
            bool found = false;
            while (first || cur_node_v != p.vertices)
            {
                first = false;
                const int merge_index = polygon_unions.find(
                    cur_node_p->go(2)->val);
                if (merge_index != -1 &&
                    (!keep_deadends ||
                     mesh_polygons[merge_index].num_traversable > 1) &&
                    abs((p.area + mesh_polygons[merge_index].area)
                        - node.area) < 1e-8 &&
                    can_merge(node.index, cur_node_v, cur_node_p))
                {
                    // Wait - before that, we need to invalidate the thing
                    // we merge with.
                    best_merge[merge_index] = -1;
                    merge(node.index, cur_node_v, cur_node_p);
                    found = true;
                    break;
                }

                cur_node_v = cur_node_v->next;
                cur_node_p = cur_node_p->next;
            }
            assert(found);
        }

        // Update THIS merge.
        push_polygon(node.index);
        // Update the polygons around this merge.

        ListNodePtr cur_node_p = p.polygons;
        bool first = true;
        while (first || cur_node_p != p.polygons)
        {
            first = false;
            push_polygon(cur_node_p->val);
            cur_node_p = cur_node_p->next;
        }
    }
}

void print_mesh(ostream& outfile)
{
    outfile << "mesh\n";
    outfile << "2\n";

    if (pretty)
    {
        outfile << "\n";
    }

    int final_v, final_p;

    vector<int> vertex_mapping;
    vertex_mapping.resize(mesh_vertices.size());
    {
        // We need to create a mapping from old-vertex to new-vertex.
        int next_index = 0;
        for (int i = 0; i < (int) mesh_vertices.size(); i++)
        {
            if (mesh_vertices[i].num_polygons != 0)
            {
                vertex_mapping[i] = next_index;
                next_index++;
            }
            else
            {
                vertex_mapping[i] = INT_MAX;
            }
        }
        final_v = next_index;
    }

    vector<int> polygon_mapping;
    polygon_mapping.resize(mesh_polygons.size());
    {
        // We need to create a mapping from old-vertex to new-vertex.
        int next_index = 0;
        for (int i = 0; i < (int) mesh_polygons.size(); i++)
        {
            if (mesh_polygons[i].num_vertices != 0)
            {
                polygon_mapping[i] = next_index;
                next_index++;
            }
            else
            {
                polygon_mapping[i] = INT_MAX;
            }
        }
        final_p = next_index;
    }

    #define get_v(v) ((v) == -1 ? -1 : vertex_mapping[v]);
    #define get_p(p) ((p) == -1 ? -1 : polygon_mapping[polygon_unions.find(p)]);

    outfile << final_v << " " << final_p << "\n";

    if (pretty)
    {
        outfile << "\n";
    }

    for (int i = 0; i < (int) mesh_vertices.size(); i++)
    {
        Vertex& v = mesh_vertices[i];
        if (v.num_polygons == 0)
        {
            continue;
        }
        outfile << v.p.x << " " << v.p.y << " \t"[pretty];
        outfile << v.num_polygons << " \t"[pretty];

        outfile << get_p(v.polygons->val);
        {
            int count = 1;
            ListNodePtr cur_node = v.polygons->next;
            while (cur_node != v.polygons)
            {
                assert(count < v.num_polygons);
                outfile << " " << get_p(cur_node->val);
                cur_node = cur_node->next;
                count++;
            }
            assert(count == v.num_polygons);
        }
        outfile << "\n";
    }

    if (pretty)
    {
        outfile << "\n";
    }

    int sum_traversable = 0;
    int num_deadends = 0;

    for (int i = 0; i < (int) mesh_polygons.size(); i++)
    {
        Polygon& p = mesh_polygons[i];
        if (p.num_vertices == 0)
        {
            continue;
        }
        if (p.num_traversable == 1)
        {
            num_deadends++;
        }
        sum_traversable += p.num_traversable;
        outfile << p.num_vertices << " \t"[pretty];

        outfile << get_v(p.vertices->val);
        {
            ListNodePtr cur_node = p.vertices->next;
            while (cur_node != p.vertices)
            {
                outfile << " " << get_v(cur_node->val);
                cur_node = cur_node->next;
            }
        }
        outfile << " \t"[pretty];

        outfile << get_p(p.polygons->val);
        {
            ListNodePtr cur_node = p.polygons->next;
            while (cur_node != p.polygons)
            {
                outfile << " " << get_p(cur_node->val);
                cur_node = cur_node->next;
            }
        }
        outfile << "\n";
    }

    // cerr << final_p << ";" << num_deadends << ";" << sum_traversable << endl;

    #undef get_p
    #undef get_v
}

int main(int argc, char* argv[])
{
    if (argc == 2 && string(argv[1]) == "--pretty")
    {
        pretty = true;
    }
    // cerr << "reading in" << endl;
    read_mesh(cin);
    // cerr << "merging dead ends" << endl;
    merge_deadend();
    // cerr << "merging" << endl;
    smart_merge(true);
    // naive_merge(true);
    // cerr << "checking" << endl;
    check_correct();
    // cerr << "outputting" << endl;
    print_mesh(cout);
    delete_nodes();
    return 0;
}
