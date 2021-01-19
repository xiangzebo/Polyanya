#include "polymap.h"

using namespace std;
using namespace GEOM_FADE2D;

typedef vector<Point2> Polygon;

// Retrieves the triangles of pZone and highlights them in the triangulation.
void highlightTriangles(Fade_2D& dt,Zone2* pZone, const string& filename)
{
    Color colorBlue(0, 0, 1, 2, true); // The true parameter means 'fill'
    vector<Segment2> l;
    // Add your segments here, like
    // l.push_back({{20, 46}, {15, 35}});

    // 1) Show the full triangulation
    Visualizer2 vis(filename);

    vis.addObject(l, colorBlue);

    // Add your points here, like
    // vis.addObject({1, 12}, colorBlue);

    dt.show(&vis);

    // 2) Fetch the triangles from the zone
    vector<Triangle2*> vZoneT;
    pZone->getTriangles(vZoneT);

    // 3) Highlight them in red
    Color colorRed(1, 0, 0, 0.01, false); // The true parameter means 'fill'
    for(size_t i=0; i<vZoneT.size(); i++)
    {
        vis.addObject(*vZoneT[i], colorRed);
    }
    vis.writeFile();
}

void print_points(const vector<Polygon> &polygons)
{
    for (auto poly : polygons)
    {
        cout << poly.size() << " ";
        for (auto p : poly)
        {
            double x, y;
            p.xy(x, y);
            cout << x << " " << y << " ";
        }
        cout << endl;
    }
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        cerr << "usage: " << argv[0] << " <file>" << endl;
        return 1;
    }
    Fade_2D dt;
    string filename = argv[1];
    ifstream mapfile(filename);
    if (!mapfile.is_open())
    {
        cerr << "Unable to open file" << endl;
        return 1;
    }
    Zone2 *traversable = fadeutils::create_traversable_zone(mapfile, dt);
    highlightTriangles(dt, traversable, filename + "-traversable.ps");

    vector<Triangle2*> triangles;
    traversable->getTriangles(triangles);
    cout << "Traversable triangles: " << triangles.size() << endl;
    cout << "Total triangles: " << dt.numberOfTriangles() << endl;
    return 0;
}
