#include "polymap.h"
#include <map>
#include <iomanip>

#define FORMAT_VERSION 2


using namespace std;
using namespace GEOM_FADE2D;

typedef vector<Point2> Polygon;
Zone2 *traversable;
vector<Point2*> vertices;
vector<Triangle2*> triangles;
// Fade2D doesn't have a nice "is this triangle in this zone?" function so we
// will have to make our own. Additionally, Fade2D has its own point index
// feature, but it doesn't have a triangle index feature.
// This map will satisfy both of these uses.
map<Triangle2*, int> triangle_to_index;

Fade_2D dt;

void init()
{
    traversable = fadeutils::create_traversable_zone(cin, dt);

    // Initialise vertices;
    dt.getVertexPointers(vertices);
    for (int i = 0; i < (int)vertices.size(); i++)
    {
        vertices[i]->setCustomIndex(i);
    }
    // Initialise triangles.
    traversable->getTriangles(triangles);
    for (int i = 0; i < (int)triangles.size(); i++)
    {
        triangle_to_index[triangles[i]] = i;
    }
}

void print_header()
{
    cout << "mesh" << endl;
    cout << FORMAT_VERSION << endl;
    // Assume that all the vertices in the triangulation are interesting.
    cout << vertices.size() << " " << triangles.size() << endl;
}

void print_vertices()
{
    cout << fixed << setprecision(10);
    for (auto vertex : vertices)
    {
        double x, y;
        vertex->xy(x, y);
        if (x == (int) x)
        {
            cout << (int) x;
        }
        else
        {
            cout << x;
        }
        cout << " ";
        if (y == (int) y)
        {
            cout << (int) y;
        }
        else
        {
            cout << y;
        }

        vector<int> neighbours;
        typedef TriangleAroundVertexIterator TAVI;
        TAVI start(vertex);
        bool first = true;
        for (TAVI it(start); (it != start) || first; ++it)
        {
            first = false;
            Triangle2 *cur_triangle = *it;

            // triangle not in traversable area
            if (cur_triangle == NULL ||
                triangle_to_index.count(cur_triangle) == 0)
            {
                if (neighbours.empty() || neighbours.back() != -1)
                {
                    neighbours.push_back(-1);
                }
            }
            else
            {
                neighbours.push_back(triangle_to_index[cur_triangle]);
            }
        }

        // Most -1s should be removed already, but it is possible that
        // the first AND last element are -1s.
        assert(!neighbours.empty());
        if (neighbours.front() == -1 && neighbours.back() == -1)
        {
            neighbours.pop_back();
        }

        cout << " " << neighbours.size();

        for (auto index : neighbours)
        {
            cout << " " << index;
        }
        cout << endl;
    }
}

void print_polys()
{
    for (auto triangle : triangles)
    {
        cout << 3;
        // Vertices.
        // Go 0 1 2.
        const int vertex_index[] = {0, 1, 2};
        for (int i : vertex_index)
        {
            cout << " " << triangle->getCorner(i)->getCustomIndex();
        }

        // Triangles.
        // Go 2 0 1 in version 1, 1 2 0 in version 2.
        const int triangle_index[] =
        #if FORMAT_VERSION == 1
            {2, 0, 1}
        #else
            {1, 2, 0}
        #endif
        ;
        for (int i : triangle_index)
        {
            cout << " ";
            Triangle2 *cur_triangle = triangle->getOppositeTriangle(i);
            if (cur_triangle == NULL ||
                triangle_to_index.count(cur_triangle) == 0)
            {
                cout << -1;
            }
            else
            {
                cout << triangle_to_index[cur_triangle];
            }
        }
        cout << endl;
    }
}

int main()
{
    init();
    print_header();
    print_vertices();
    print_polys();
    return 0;
}
