#include "OctileHeuristic.h"
#include "constants.h"
#include "map.h"
#include "graph.h"

#include <stdexcept>

OctileHeuristic::OctileHeuristic()
{
}

OctileHeuristic::~OctileHeuristic()
{
}

// Euclidean distance on octile grids
// author: Nathan Sturtevant
double OctileHeuristic::h(node* a, node* b) const
{
	if(a == NULL || b == NULL) 
		throw std::invalid_argument("OctileHeuristic::h node parameter cannot be null");

	int x1 = a->getLabelL(kFirstData);
	int x2 = b->getLabelL(kFirstData);
	int y1 = a->getLabelL(kFirstData+1);
	int y2 = b->getLabelL(kFirstData+1);
	
	double answer = 0.0;
	const double root2m1 = ROOT_TWO-1;
		if (fabs(x1-x2) < fabs(y1-y2))
			answer = root2m1*fabs(x1-x2)+fabs(y1-y2);
	else
		answer = root2m1*fabs(y1-y2)+fabs(x1-x2);
	return answer;
}
