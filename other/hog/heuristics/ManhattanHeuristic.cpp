#include "ManhattanHeuristic.h"

#include "constants.h"
#include <stdexcept>

ManhattanHeuristic::ManhattanHeuristic()
{
}

ManhattanHeuristic::~ManhattanHeuristic()
{
}

double ManhattanHeuristic::h(node* a, node* b) const
{
	if(a == NULL || b == NULL) 
		throw std::invalid_argument("ManhattanHeuristic::h null node");

	double answer = 0.0;

	int ax = a->getLabelL(kFirstData);
	int ay = a->getLabelL(kFirstData+1);
	int bx = b->getLabelL(kFirstData);
	int by = b->getLabelL(kFirstData+1);
	//std::cout << "from: "<<ax<<","<<ay<<") to: ("<<bx<<","<<by<<") ";

	int deltax = ax - bx;
	if(deltax < 0) deltax *=-1;

	int deltay = ay - by;
	if(deltay < 0) deltay *=-1;

	//std::cout << "deltax: "<<deltax<<" deltay: "<<deltay<<std::endl;
	answer = deltax + deltay;
	return answer;
}
