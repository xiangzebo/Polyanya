#include <cstdio>
#include <iomanip>
#include <cstdlib>
#include <queue>
#include <iostream>
#include "Entry.h"
#include <algorithm> // reverse() 

using namespace std;

typedef double dist_t;
int verticesScanned = 0;
int width, height;
const dist_t tangent = 1.4142;
const dist_t straight = 1.0;
const dist_t maxDistance = 999999.0;
const int TOTAL_LANDMARKS = 6;
vector<bool> map;


// used in the heap
struct Node_t
{
	Node_t(int c = 0, dist_t d = 0.0, dist_t sd = 0.0): coords(c), distance(d), supposedDistance(sd)
	{
	}
	
    int coords;
    dist_t distance;
	dist_t supposedDistance;
};

struct Node_info_t
{
	Node_info_t(): distance(maxDistance), precursorCoords(0), isClosed(false)
	{
	}
	dist_t distance;
	int precursorCoords;
	bool isClosed;
};


int linearize(const xyLoc& coords)
{
    return coords.y * width + coords.x;
}


xyLoc linearToCoords(int linear)
{
	xyLoc coords;
	coords.x = linear % width;
	coords.y = linear / width;
	return coords;
}

bool ValidateLoc(const xyLoc& loc)
{
	int locCoords = linearize(loc);
	if (loc.y == -1 || loc.y == height ||
		loc.x == -1 || loc.x == width ||
		map[locCoords] == false)
	{
		return false;
	}

	return true;
}




// ---- START OF DIJKSTRA
// special data structure that makes the heap linear
// does not work with landmarks... - when I make this work - it will be awesome!
class BucketQueue
{
public:
	BucketQueue():size(0), baseDistance(1), currentIndex(0)
	{
		for (int i = 0; i != 3; ++i)
		{
			baseElements[i] = new vector<Node_t>();
			baseIndexes[i] = 0;
		}
	}


	bool empty()
	{
		return size == 0;
	}


	void push(const Node_t & element)
	{
		++size;
		if (element.distance > baseDistance + 1.999)
		{
			pushOrInsert(2, element);
		}
		else if(element.distance > baseDistance + 0.999)
		{
			pushOrInsert(1, element);
		}
		else
		{
			pushOrInsert(0, element);
		}
	}


	Node_t & top()
	{
		//if (empty())
		//	throw std::exception("empty queue!!!");

		return (*baseElements[0])[currentIndex]; // FIXME? in the beginnign there must me at least one element here baseElements[0]
	}


	// pops the element and sets currentIndex to valid value
	void pop()
	{
		//if (empty())
		//	throw std::exception("empty!!!");

		--size;

		++currentIndex; // move to the next thing

		// check if correct
		for (int i=0; i<3; ++i)
			if (currentIndex == baseIndexes[0])
			{
				// baseIndexes[0] = 0;
				currentIndex = 0;
				++baseDistance;

				vector<Node_t>* tmpVecPtr = baseElements[0];

				for (int oldBase = 1; oldBase <= 2; ++oldBase)
				{
					baseElements[oldBase - 1] = baseElements[oldBase];
					baseIndexes[oldBase - 1] = baseIndexes[oldBase];
				}
				baseElements[2] = tmpVecPtr;
				baseIndexes[2] = 0;
			}
			else
				break;


	}

private:
	int size;
	int baseDistance;

	int currentIndex;
	// <baseDistance, baseDistance+1), 
	// <baseDistance+1, baseDistance+2),
	// <baseDistance2, baseDistance+2.41)
	vector<Node_t>* baseElements[3];
	unsigned int baseIndexes[3]; // index in the baseElementsx vector to which the element is to be pushed

	
	

	// arrayIndex \element {0, 1, 2}
	void pushOrInsert(int arrayIndex, const Node_t& element)
	{
		// should I push back?
		if (baseIndexes[arrayIndex] >= baseElements[arrayIndex]->size())
		{
			baseElements[arrayIndex]->push_back(element);
		}
		else
		{
			(*baseElements[arrayIndex])[baseIndexes[arrayIndex]] = element;
		}
		++baseIndexes[arrayIndex];
	}
};

bool Dijkstra(xyLoc s, std::vector<Node_info_t>& outputMap)
{
	int from = linearize(s);

	outputMap.resize(width * height, Node_info_t());
    //priority_queue<Node_t, vector<Node_t>, Comp> Q;
	BucketQueue Q;
    
    Node_t fromV(from, 0.0);
    Q.push(fromV);
	outputMap[from].distance = 0.0;
	outputMap[from].precursorCoords = 0;

	const int neighborsCount = 8;

	// check out all the neighbors && push them
	// starting with the one up clockwise
	pair<int, int> neighbors[] = { // [y, x]
		make_pair(-1, 0),
		make_pair(-1, 1),
		make_pair(0, 1),
		make_pair(1, 1),
		make_pair(1, 0),
		make_pair(1, -1),
		make_pair(0, -1),
		make_pair(-1, -1),
	};

	dist_t neighborDistances[] = {
		straight,
		tangent,
		straight,
		tangent,
		straight,
		tangent,
		straight,
		tangent
	};

    while(!Q.empty()) 
    {
        // select vertex with the shortest distance
        Node_t vertex = Q.top();
        Q.pop();

		outputMap[vertex.coords].isClosed = true;
	
		// check all the neighbors
		for (int i = 0; i < neighborsCount; ++i)
		{

			xyLoc vc = linearToCoords(vertex.coords);

			xyLoc neighborC;
			neighborC.x = vc.x + neighbors[i].first;
			neighborC.y = vc.y + neighbors[i].second;
			
			int neighborCoords = linearize(neighborC);

			if (!ValidateLoc(neighborC) || outputMap[neighborCoords].isClosed == true)
			{
				continue;
			}

			
			// both adjacent cardinal directions must be unblocked
			int previous = (i + neighborsCount - 1) % neighborsCount;
			xyLoc previousNeighborC;
			previousNeighborC.x = vc.x + neighbors[previous].first;
			previousNeighborC.y = vc.y + neighbors[previous].second;
			//cout << "previousNeighbor x="<<previousNeighborC.x<<"y="<<previousNeighborC.y << endl;

			int next = (i + 1) % neighborsCount;
			xyLoc nextNeighborC;
			nextNeighborC.x = vc.x + neighbors[next].first;
			nextNeighborC.y = vc.y + neighbors[next].second;
			if (i % 2 == 1 && (!ValidateLoc(previousNeighborC) || !ValidateLoc(nextNeighborC)) )
			{
				continue;
			}


			
			// relax if necessary
			// s....u.v...t
			// if d(su)+ d(uv) < d(sv) 
			dist_t lengthThroughV = outputMap[vertex.coords].distance + neighborDistances[i];
			if (lengthThroughV < outputMap[neighborCoords].distance)
			{	
				Q.push(Node_t(neighborCoords, lengthThroughV));
				outputMap[neighborCoords].precursorCoords = vertex.coords;
				outputMap[neighborCoords].distance = vertex.distance + neighborDistances[i];
			}
		}
    }

	return true;
}

//-------- END OF DIJKSTRA
//
//
//void LoadMap(const char *fname, std::vector<bool> &map, int &width, int &height)
//{
//	FILE *f;
//	f = fopen(fname, "r");
//	if (f)
//    {
//		fscanf(f, "type octile\nheight %d\nwidth %d\nmap\n", &height, &width);
//		map.resize(height*width);
//		for (int y = 0; y < height; y++)
//		{
//			for (int x = 0; x < width; x++)
//			{
//				char c;
//				do {
//					fscanf(f, "%c", &c);
//				} while (isspace(c));
//				map[y*width+x] = (c == '.' || c == 'G' || c == 'S');
//			}
//		}
//		fclose(f);
//    }
//}



//#pragma region aaaa
//aassd
//#pragma endregion aaaa

//-------- START OF A*
dist_t getGreatestLowerBound(int vertex, int goal, vector<Node_info_t>* landmarksOutputBitmap)
{
	dist_t lowerBound = 0.0;

	for (int i = 0; i!= TOTAL_LANDMARKS; ++i)
	{
		dist_t landmarkVertexDistance = landmarksOutputBitmap[i][vertex].distance;
		dist_t landmarkGoalDistance = landmarksOutputBitmap[i][goal].distance;

		dist_t delta;
		if (landmarkGoalDistance > landmarkVertexDistance)
		{
			delta = landmarkGoalDistance - landmarkVertexDistance;
		} 
		else
		{
			delta = landmarkVertexDistance - landmarkGoalDistance;
		}
		if (lowerBound < delta)
		{
			lowerBound = delta;
		}
	}

	return lowerBound;
}

class Comp
{
public:
    bool operator() (const Node_t& a, const Node_t& b)
    {
        return a.supposedDistance > b.supposedDistance; 
		//return a.distance > b.distance;
    }
};




// FIXME - store path as xyloc, not as a single int...
bool GetPath(void* data, xyLoc s, xyLoc g, std::vector<xyLoc> &path)
{
	vector<Node_info_t> * landmarksOutputBitmap = (vector<Node_info_t> *) data;
	int from = linearize(s);
	int to = linearize(g);


	vector<Node_info_t> outputMap(width * height, Node_info_t());
    priority_queue<Node_t, vector<Node_t>, Comp> Q;
	//BucketQueue Q;
    
    Node_t fromV(from, 0.0);
    Q.push(fromV);
	outputMap[from].distance = 0.0;
	outputMap[from].precursorCoords = 0;

	const int neighborsCount = 8;

	// check out all the neighbors && push them
	// starting with the one up clockwise
	pair<int, int> neighbors[] = { // [y, x]
		make_pair(-1, 0),
		make_pair(-1, 1),
		make_pair(0, 1),
		make_pair(1, 1),
		make_pair(1, 0),
		make_pair(1, -1),
		make_pair(0, -1),
		make_pair(-1, -1),
	};

	dist_t neighborDistances[] = {
		straight,
		tangent,
		straight,
		tangent,
		straight,
		tangent,
		straight,
		tangent
	};

    while(!Q.empty()) 
    {
        // select vertex with the shortest distance
        Node_t vertex = Q.top();
        Q.pop();

		outputMap[vertex.coords].isClosed = true;
		if (vertex.coords == to)
		{
			break;
		}

		// check all the neighbors
		for (int i = 0; i < neighborsCount; ++i)
		{
			xyLoc vc = linearToCoords(vertex.coords);

			//cout << "vertex x="<<vc.x<<"y="<<vc.y << endl;
			xyLoc neighborC;
			neighborC.x = vc.x + neighbors[i].first;
			neighborC.y = vc.y + neighbors[i].second;
			
			int neighborCoords = linearize(neighborC);

			if (!ValidateLoc(neighborC) || outputMap[neighborCoords].isClosed == true)
			{
				continue;
			}

			
			// both adjacent cardinal directions must be unblocked
			int previous = (i + neighborsCount - 1) % neighborsCount;
			xyLoc previousNeighborC;
			previousNeighborC.x = vc.x + neighbors[previous].first;
			previousNeighborC.y = vc.y + neighbors[previous].second;
			//cout << "previousNeighbor x="<<previousNeighborC.x<<"y="<<previousNeighborC.y << endl;

			int next = (i + 1) % neighborsCount;
			xyLoc nextNeighborC;
			nextNeighborC.x = vc.x + neighbors[next].first;
			nextNeighborC.y = vc.y + neighbors[next].second;
			if (i % 2 == 1 && (!ValidateLoc(previousNeighborC) || !ValidateLoc(nextNeighborC)) )
			{
				continue;
			}


			// relax if necessary
			// s....u.v...t
			// if d(su)+ d(uv) < d(sv) 
			dist_t greatestLowerBound = getGreatestLowerBound(vertex.coords, to, landmarksOutputBitmap);
			dist_t lengthThroughV = outputMap[vertex.coords].distance + neighborDistances[i];
			if (lengthThroughV < outputMap[neighborCoords].distance)
			{	
				Q.push(Node_t(neighborCoords, lengthThroughV, lengthThroughV + greatestLowerBound)); 
				//Q.push(Node_t(neighborCoords, lengthThroughV, lengthThroughV)); 
				++verticesScanned;

				outputMap[neighborCoords].precursorCoords = vertex.coords;
				outputMap[neighborCoords].distance = vertex.distance + neighborDistances[i];
			}
		}
    }


    //cout << "qqvzidalenost " << outputMap[to].distance << endl;
	if (outputMap[to].isClosed == true)
	{
		//cout << "pocet skenovanych vrcholov " << verticesScanned << endl;
		//cout << "je to zavrety vrchol!" << endl;
		//vector<xyLoc> tmppath;
		int v = to;
		while (true)
		{
			//cout << "iterujem, som na " << v << endl;
			xyLoc loc = linearToCoords(v);
			path.push_back(loc);

			if (outputMap[v].precursorCoords != 0)
			{
				v = outputMap[v].precursorCoords;
			} else 
			{
				break;
			}

		}
		
		std::reverse(path.begin(), path.end());

	}

	return true;
}
//-------- END OF A*



void PreprocessMap(std::vector<bool> &bits, int width, int height, const char *filename)
{
	return;
}


void *PrepareForSearch(std::vector<bool> &bits, int w, int h, const char *filename)
{
	map = bits;
	width = w;
	height = h;

	// I did some quick dijkstra in this function since it really takes VERY little time (just some milliseconds)
	// if this should be a reason for disqualification, I do apologize and I may reprogram in into the PreprocessMap...
	// this whole function calls dijkstra 6times on the map
	
	// pick TOTAL_LANDMARKS/2 random points
	int attempts = 100;

	int pos = 0;
	int landmarks[TOTAL_LANDMARKS];

	int randomVertex;
	for(int i = 0; i < TOTAL_LANDMARKS/2; ++i)
	{
		randomVertex = (rand() % (width * height));
		//cout << randomVertex << endl;

		while (!map[randomVertex])
		{
			randomVertex = (randomVertex+1) % (width*height);
		}
		landmarks[pos++] = randomVertex;
		// cout << "nas pokus " << p << endl;
	}

	// cout << "zbyvalo pokusov" << attempts << endl;

	vector<Node_info_t>* landmarkOutputBitmaps = new vector<Node_info_t>[TOTAL_LANDMARKS];
	for (int i=0; i != TOTAL_LANDMARKS; ++i)
	{
		Dijkstra(linearToCoords(landmarks[i]), landmarkOutputBitmaps[i]);

		if (i < TOTAL_LANDMARKS/2) 
		{
			// find most distant points
			int mostDistantVertex = 0;
			dist_t mostDistantDistance = 0;
			for (int tmp=0; tmp!= TOTAL_LANDMARKS; ++tmp)
				//cout << "size "<<landmarkOutputBitmaps[i].size() << endl;
			for (unsigned int j=0; j != landmarkOutputBitmaps[i].size(); ++j)
			{
				//cout <<"dist"<<landmarkOutputBitmaps[i][j].distance << 
				//	"j"<<j<<	endl;
				if (landmarkOutputBitmaps[i][j].distance > mostDistantDistance && landmarkOutputBitmaps[i][j].distance+1 < maxDistance)
				{
					mostDistantVertex = j;
					mostDistantDistance = landmarkOutputBitmaps[i][j].distance;
					//cout <<"MDD " << mostDistantDistance << endl;
				}
			}
			landmarks[TOTAL_LANDMARKS/2 + i] = mostDistantVertex;
		}
	}

	return landmarkOutputBitmaps;
}

const char *GetName() 
{
	return "NovellA*";
}


struct HelpMap {
	HelpMap(const char *m, int a, int b, int c, int d): map(m), xfrom(a), yfrom(b),xto(c), yto(d)
	{
	}
	const char * map;
	int xfrom;
	int yfrom;
	int xto;
	int yto;
};
////HelpMap m("mapa.map", 1,1,7,1);
////HelpMap m("Aftershock.map", 163,428,170,427);
////HelpMap m("Aftershock.map", 1,130,9,113); //20.3136
//HelpMap m("Aftershock.map", 442,8,503,495); //726.247
////HelpMap m("Aftershock.map", 490, 264, 488, 260); //4.82843
//int main()
//{
//	LoadMap(m.map, map, width, height);
//
//	vector<Node_info_t> outputMap;
//	void * reference = PrepareForSearch(map, width, height, "foofilename");
//
//	vector<xyLoc> thePath;
//	for(int i=0; i != 1; ++i)
//	{
//		xyLoc s, g;
//		s.x = m.xfrom;
//		s.y = m.yfrom;
//		g.x = m.xto;
//		g.y = m.yto;
//
//		bool done = GetPath(reference, s, g, thePath);
//	}
//
//	cout << "velkost thePath (ideal length=726.247)" << thePath.size() << endl;
//	for (int i=0; i != thePath.size(); ++i)
//	{
//		//cout << "x=" <<thePath[i].x<<" y="<<thePath[i].y << endl;
//	}
//
//
//	//for (int i= linearize(s); != linearize(g))
//
//	system("pause");
//    return 0;
//}
//
//
