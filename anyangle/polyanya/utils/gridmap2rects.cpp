/*
Converts a gridmap into a mesh made of big rectangles.
Uses "clearance" values, very very similar to
http://harabor.net/data/papers/harabor-botea-cig08.pdf.

Firstly - all orientation is based on
Clearance is biggest square you can make with bottom-right corner at that cell.
Area is the total area of the square, plus if you extend the square right/left.
When we take a rectangle, mark all of the squares of that rectangle
non-traversable and with the rectangle ID. Store that rectangle somewhere as
well.
*/
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cassert>
#include <iomanip>
#include <queue>
#include <algorithm>

using namespace std;

typedef vector<bool> vbool;
typedef vector<int> vint;


// Everything here is [y][x]!
vector<vbool> map_traversable;

// Length of longest line starting here going up.
vector<vint> clear_above;
vector<vint> clear_left;

struct Rect
{
    int width, height;
    long long h;

    // Comparison.
    // Always take the one with highest h.
    bool operator<(const Rect& other) const
    {
        return h < other.h;
    }

    bool operator>(const Rect& other) const
    {
        return h > other.h;
    }
};

struct SearchNode
{
    int y, x; // !!!
    long long h;

    // Comparison.
    // Always take the one with highest h.
    bool operator<(const SearchNode& other) const
    {
        return h < other.h;
    }

    bool operator>(const SearchNode& other) const
    {
        return h > other.h;
    }
};

typedef vector<Rect> vrect;

vector<vrect> grid_rectangles;
vector<vint> rectangle_id;
int cur_rect_id = 0;

struct FinalRect
{
    int y, x; // y, x of TOP-LEFT CORNER
    int width, height;
};

struct Vertex
{
    int y, x;

    Vertex operator+(const Vertex& other) const
    {
        return {y + other.y, x + other.x};
    }

    Vertex operator-(const Vertex& other) const
    {
        return {y - other.y, x - other.x};
    }
};

vector<FinalRect> final_rectangles;

// [0][0] is top-left corner of map, [height][width] is bottom-right
vector<vint> vertex_id;
vector<Vertex> final_vertices;
int cur_vertex_id = 0;

int map_width;
int map_height;


long long get_heuristic(int width, int height)
{
    long long out = min(width, height);
    out *= width;
    out *= height;
    return out;
}

void fail(string msg)
{
    cerr << msg << endl;
    exit(1);
}

void read_map(istream& infile)
{
    // Most of this code is from dharabor's warthog.
    // read in the whole map. ensure that it is valid.
    unordered_map<string, string> header;

    // header
    for (int i = 0; i < 3; i++)
    {
        string hfield, hvalue;
        if (infile >> hfield)
        {
            if (infile >> hvalue)
            {
                header[hfield] = hvalue;
            }
            else
            {
                fail("err; map has bad header");
            }
        }
        else
        {
            fail("err; map has bad header");
        }
    }

    if (header["type"] != "octile")
    {
        fail("err; map type is not octile");
    }

    // we'll assume that the width and height are less than INT_MAX
    map_width = atoi(header["width"].c_str());
    map_height = atoi(header["height"].c_str());

    if (map_width == 0 || map_height == 0)
    {
        fail("err; map has bad dimensions");
    }

    // we now expect "map"
    string temp_str;
    infile >> temp_str;
    if (temp_str != "map")
    {
        fail("err; map does not have 'map' keyword");
    }


    // basic checks passed. initialse the map
    map_traversable = vector<vbool>(map_height, vbool(map_width));
    clear_above = vector<vint>(map_height, vint(map_width, 0));
    clear_left = vector<vint>(map_height, vint(map_width, 0));
    rectangle_id = vector<vint>(map_height, vint(map_width, -1));
    vertex_id = vector<vint>(map_height+1, vint(map_width+1, -1));
    grid_rectangles = vector<vrect>(map_height, vrect(map_width));
    // so to get (x, y), do map_traversable[y][x]
    // 0 is nontraversable, 1 is traversable

    // read in map_data
    int cur_y = 0;
    int cur_x = 0;

    char c;
    while (infile.get(c))
    {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
        {
            // whitespace.
            // cannot put in the switch statement below as we need to check
            // "too many chars" before anything else
            continue;
        }

        if (cur_y == map_height)
        {
            fail("err; map has too many characters");
        }

        switch (c)
        {
            case 'S':
            case 'W':
            case 'T':
            case '@':
            case 'O':
                // obstacle
                map_traversable[cur_y][cur_x] = 0;
                break;
            default:
                // traversable
                map_traversable[cur_y][cur_x] = 1;
                break;
        }

        cur_x++;
        if (cur_x == map_width)
        {
            cur_x = 0;
            cur_y++;
        }
    }

    if (cur_y != map_height || cur_x != 0)
    {
        fail("err; map has too few characters");
    }
}

int get_clear_above(int y, int x)
{
    if (x < 0 || y < 0)
    {
        return 0;
    }
    assert(y < map_height);
    assert(x < map_width);
    if (!map_traversable[y][x])
    {
        return clear_above[y][x] = 0;
    }
    if (clear_above[y][x])
    {
        return clear_above[y][x];
    }
    return clear_above[y][x] = get_clear_above(y-1, x) + 1;
}

int get_clear_above_lazy(int y, int x)
{
    int out = 0;
    while (y >= 0 && map_traversable[y][x])
    {
        out++;
        y--;
    }
    return out;
}

int get_clear_left(int y, int x)
{
    if (x < 0 || y < 0)
    {
        return 0;
    }
    assert(y < map_height);
    assert(x < map_width);
    if (!map_traversable[y][x])
    {
        return clear_left[y][x] = 0;
    }
    if (clear_left[y][x])
    {
        return clear_left[y][x];
    }
    return clear_left[y][x] = get_clear_left(y, x-1) + 1;
}

int get_clear_left_lazy(int y, int x)
{
    int out = 0;
    while (x >= 0 && map_traversable[y][x])
    {
        out++;
        x--;
    }
    return out;
}

void calculate_clearance(int bottom_y, int bottom_x)
{
    // Bottom up DP.
    // Invalidate our cache and run get_clearance.
    // Go [bottom_x+1, end) for y from [0, bottom_y+1)
    // and then go [0, end) for y from [bottom_y+1, end)
    for (int y = 0; y < bottom_y+1; y++)
    {
        for (int x = bottom_x; x < map_width; x++)
        {
            clear_above[y][x] = 0;
            clear_left[y][x] = 0;
            get_clear_above(y, x);
            get_clear_left(y, x);
        }
    }
    for (int y = bottom_y+1; y < map_height; y++)
    {
        for (int x = 0; x < map_width; x++)
        {
            clear_above[y][x] = 0;
            clear_left[y][x] = 0;
            get_clear_above(y, x);
            get_clear_left(y, x);
        }
    }
}

Rect get_best_rect(int y, int x)
{
    assert(y >= 0);
    assert(x >= 0);
    assert(y < map_height);
    assert(x < map_width);
    Rect out = {0, 0, 0};
    if (!map_traversable[y][x])
    {
        return out;
    }
    // Try every width, figure out height.
    // For width from 1 to clear_left[y][x],
    // take the min of this one and the one we just took.
    {
        int height = clear_above[y][x]; // The first height.
        for (int width = 1; width <= clear_left[y][x]; width++)
        {
            height = min(height, clear_above[y][x-width+1]);
            const long long h = get_heuristic(width, height);
            if (h > out.h)
            {
                out = {width, height, h};
            }
        }
    }
    // Try every height, figure out width.
    {
        int width = clear_left[y][x]; // The first width.
        for (int height = 1; height <= clear_above[y][x]; height++)
        {
            width = min(width, clear_left[y-height+1][x]);
            const long long h = get_heuristic(width, height);
            if (h > out.h)
            {
                out = {width, height, h};
            }
        }
    }
    return out;
}

Rect get_best_rect_lazy(int y, int x)
{
    Rect out = {0, 0, 0};
    if (!map_traversable[y][x])
    {
        return out;
    }
    // Try every width, figure out height.
    // For width from 1 to clear_left[y][x],
    // take the min of this one and the one we just took.
    {
        int height = get_clear_above_lazy(y, x); // The first height.
        const int max_width = get_clear_left_lazy(y, x);
        for (int width = 1; width <= max_width; width++)
        {
            height = min(height, get_clear_above_lazy(y, x-width+1));
            const long long h = get_heuristic(width, height);
            if (h > out.h)
            {
                out = {width, height, h};
            }
        }
    }
    // Try every height, figure out width.
    {
        int width = get_clear_left_lazy(y, x); // The first width.
        const int max_height = get_clear_above_lazy(y, x);
        for (int height = 1; height <= max_height; height++)
        {
            width = min(width, get_clear_left_lazy(y-height+1, x));
            const long long h = get_heuristic(width, height);
            if (h > out.h)
            {
                out = {width, height, h};
            }
        }
    }
    return out;
}

void calculate_rectangles(int bottom_y, int bottom_x)
{
    // Assume calculate_clearance was called before.
    for (int y = 0; y < bottom_y+1; y++)
    {
        for (int x = bottom_x; x < map_width; x++)
        {
            grid_rectangles[y][x] = get_best_rect(y, x);
        }
    }
    for (int y = bottom_y+1; y < map_height; y++)
    {
        for (int x = 0; x < map_width; x++)
        {
            grid_rectangles[y][x] = get_best_rect(y, x);
        }
    }
}

void make_rectangles()
{
    // Gets the best rectangle and takes that.
    // Repeat until there are no more rectangles.
    priority_queue<SearchNode> pq;
    calculate_clearance(-1, -1);
    calculate_rectangles(-1, -1);
    for (int y = 0; y < map_height; y++)
    {
        for (int x = 0; x < map_width; x++)
        {
            const Rect& r = grid_rectangles[y][x];
            if (r.h > 0)
            {
                pq.push({y, x, r.h});
            }
        }
    }

    while (!pq.empty())
    {
        SearchNode node = pq.top(); pq.pop();
        const Rect r = get_best_rect_lazy(node.y, node.x);
        if (node.h != r.h)
        {
            // Not the right node.
            // Push it on so we can get to it later if r.h isn't 0.
            if (r.h != 0)
            {
                pq.push({node.y, node.x, r.h});
            }
            continue;
        }
        // Use r.
        // Set all those rectangle ids, and set non-traversable.
        // Also invalidate the rectangles.
        for (int y = node.y; y > node.y - r.height; y--)
        {
            for (int x = node.x; x > node.x - r.width; x--)
            {
                rectangle_id[y][x] = cur_rect_id;
                map_traversable[y][x] = false;
            }
        }
        {
            const int max_y = node.y + 1;
            const int max_x = node.x + 1;
            const int min_y = max_y - r.height;
            const int min_x = max_x - r.width;
            // Set vertices.
            const Vertex corners[] = {
                {min_y, min_x},
                {max_y, min_x},
                {max_y, max_x},
                {min_y, max_x}
            };
            for (int i = 0; i < 4; i++)
            {
                const Vertex& p = corners[i];
                int& id_ref = vertex_id[p.y][p.x];
                if (id_ref != -1)
                {
                    continue;
                }
                id_ref = cur_vertex_id;
                final_vertices.push_back(p);
                cur_vertex_id++;
            }
            // Push final rectangle.
            final_rectangles.push_back({min_y, min_x, r.width, r.height});
        }
        cur_rect_id++;
    }
}

void print_mesh_vertices()
{
    vector<int> temp;
    temp.resize(4);
    // For each vertex, print it out.
    for (Vertex& v : final_vertices)
    {
        cout << v.x << " " << v.y;
        // Now we get its neighbours.
        // Remember that Vertices are {y, x}!
        static const Vertex deltas[] = {
            {-1, -1},
            { 0, -1},
            { 0,  0},
            {-1,  0}
        };

        // Append all, then cull after.
        for (int i = 0; i < 4; i++)
        {
            Vertex grid_loc = v + deltas[i];
            if (grid_loc.x < 0 || grid_loc.x >= map_width ||
                grid_loc.y < 0 || grid_loc.y >= map_height)
            {
                temp[i] = -1;
            }
            else
            {
                temp[i] = rectangle_id[grid_loc.y][grid_loc.x];
            }
        }

        // Cull.
        vector<int> out;
        {
            int last = temp[3];
            for (int i = 0; i < 4; i++)
            {
                const int cur = temp[i];
                if (cur != last)
                {
                    out.push_back(cur);
                }
                last = cur;
            }
        }

        // Print.
        cout << " " << out.size();
        for (int poly : out)
        {
            cout << " " << poly;
        }
        cout << "\n";
    }
}

void print_mesh_polygons()
{
    for (FinalRect& r : final_rectangles)
    {
        /*
        Iterate over vertices which lie on the rectangle in this order:

        16 15 14 13
        01       12
        02       11
        03       10
        04       09
        05 06 07 08
        */

        assert(r.width  >= 1);
        assert(r.height >= 1);

        vector<int> vertices;
        vector<int> polygons;

        auto push_vertex = [&](int y, int x, int dy, int dx)
        {
            // Assume that the coordianates we get are always valid.
            const int vertex = vertex_id[y][x];
            if (vertex == -1)
            {
                return;
            }
            vertices.push_back(vertex);
            // Use dy and dx to get the grid location of the neighbours.
            const int grid_loc_y = y + dy;
            const int grid_loc_x = x + dx;
            if (grid_loc_x < 0 || grid_loc_x >= map_width ||
                grid_loc_y < 0 || grid_loc_y >= map_height)
            {
                polygons.push_back(-1);
            }
            else
            {
                polygons.push_back(rectangle_id[grid_loc_y][grid_loc_x]);
            }
        };

        // Go through "01-05".
        {
            const int x = r.x;
            for (int y = r.y + 1; y <= r.y + r.height; y++)
            {
                // dy = -1, dx = -1
                push_vertex(y, x, -1, -1);
            }
        }

        // Go through "06-08".
        {
            const int y = r.y + r.height;
            for (int x = r.x + 1; x <= r.x + r.width; x++)
            {
                // dy = 0, dx = -1
                push_vertex(y, x, 0, -1);
            }
        }

        // Go through "09-13".
        {
            const int x = r.x + r.width;
            for (int y = r.y + r.height - 1; y >= r.y; y--)
            {
                // dy = 0, dx = 0
                push_vertex(y, x, 0, 0);
            }
        }

        // Go through "14-16".
        {
            const int y = r.y;
            for (int x = r.x + r.width - 1; x >= r.x; x--)
            {
                // dy = -1, dx = 0
                push_vertex(y, x, -1, 0);
            }
        }

        // Reverse because orientations are mixed up
        reverse(vertices.begin(), vertices.end());
        reverse(polygons.begin(), polygons.end());
        // and fix up the broken polygons
        rotate(polygons.begin(), polygons.end()-1, polygons.end());

        cout << vertices.size();

        for (int v : vertices)
        {
            cout << " " << v;
        }

        for (int p : polygons)
        {
            cout << " " << p;
        }
        cout << "\n";
    }
}

void print_clearance()
{
    cout << "above" << endl;
    for (auto& x : clear_above)
    {
        for (auto y : x)
        {
            if (y)
            {
                cout << setfill(' ') << setw(3) << y;
            }
            else
            {
                cout << "   ";
            }
        }
        cout << "\n";
    }

    cout << endl;
    cout << "left" << endl;
    for (auto& x : clear_left)
    {
        for (auto y : x)
        {
            if (y)
            {
                cout << setfill(' ') << setw(3) << y;
            }
            else
            {
                cout << "   ";
            }
        }
        cout << "\n";
    }
}

void print_clearance_lazy()
{
    cout << "above" << endl;
    for (int y = 0; y < map_height; y++)
    {
        for (int x = 0; x < map_width; x++)
        {
            const int clearance = get_clear_above_lazy(y, x);
            if (clearance)
            {
                cout << setfill(' ') << setw(3) << clearance;
            }
            else
            {
                cout << "   ";
            }
        }
        cout << "\n";
    }

    cout << endl;
    cout << "left" << endl;
    for (int y = 0; y < map_height; y++)
    {
        for (int x = 0; x < map_width; x++)
        {
            const int clearance = get_clear_left_lazy(y, x);
            if (clearance)
            {
                cout << setfill(' ') << setw(3) << clearance;
            }
            else
            {
                cout << "   ";
            }
        }
        cout << "\n";
    }
}

void print_rects()
{
    for (auto& x : final_rectangles)
    {
        cout << "(" << x.x << ", " << x.y << "), "
             << "w=" << x.width << ", h=" << x.height << endl;
    }
}

void print_heuristic()
{
    for (auto& x : grid_rectangles)
    {
        for (auto y : x)
        {
            if (y.h)
            {
                cout << setfill(' ') << setw(4) << y.h;
                cout << " ";
            }
            else
            {
                cout << "     ";
            }
        }
        cout << "\n";
    }
}

void print_ids()
{
    for (auto& x : rectangle_id)
    {
        for (auto y : x)
        {
            if (y != -1)
            {
                cout << setfill(' ') << setw(3) << y;
            }
            else
            {
                cout << "   ";
            }
        }
        cout << "\n";
    }
}

void print_traversable()
{
    for (auto& x : map_traversable)
    {
        for (auto y : x)
        {
            cout << "@."[y];
        }
        cout << "\n";
    }
}

int main()
{
    read_map(cin);
    // calculate_clearance(-1, -1);
    // calculate_rectangles(-1, -1);
    // print_clearance();
    // print_rects();
    // print_traversable();
    // print_heuristic();
    make_rectangles();
    // print_rects();
    // print_ids();
    cout << "mesh" << endl;
    cout << 2 << endl;
    cout << cur_vertex_id << " " << cur_rect_id << endl;
    print_mesh_vertices();
    print_mesh_polygons();
    // print_ids();

    return 0;
}
