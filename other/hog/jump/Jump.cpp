#include "Jump.h"

#include "constants.h"
#include "graph.h"

#include <cstdlib>

// Find the direction of travel from a node to its jump point successor.
// Note that jump point successors can only be located in a straight line
// relative to their parent. Otherwise Jump::NONE is returned.
//
// @param cur: the current node
// @param succ: the jump point successor of cur
// @return the direction of travel from cur to succ. 
//
Jump::Direction 
Jump::computeDirection(node* cur, node* succ)
{
	if(cur == 0 || succ == 0)
	{
		return Jump::NONE;
	}

	int x = cur->getLabelL(kFirstData);
	int y = cur->getLabelL(kFirstData+1);
	int x2 = succ->getLabelL(kFirstData);
	int y2 = succ->getLabelL(kFirstData+1);
	return computeDirection(x, y, x2, y2);
}

Jump::Direction
Jump::computeDirection(int x, int y, int x2, int y2)
{
	// Check if the nodes are in a straight line relative to each other
	if(x != x2 && y != y2 && abs(x - x2) != abs(y - y2))
	{
		return Jump::NONE;
	}

	Jump::Direction dir = Jump::NONE;
	if(y2 == y)
	{
		if(x2 > x)
			dir = Jump::E;
		else
			dir = Jump::W;
	}
	else if(y2 < y)
	{
		if(x2 == x)
			dir = Jump::N;
		else if(x2 < x)
			dir = Jump::NW;
		else // x2 > x
			dir = Jump::NE;
	}
	else // y2 > y 
	{
		if(x2 == x)
			dir = Jump::S;
		else if(x2 < x)
			dir = Jump::SW;
		else // x2 > x
			dir = Jump::SE;
	}

	return dir;
}
std::string 
Jump::toString(Jump::Direction d)
{
	std::string retVal;
	switch(d)
	{
		case Jump::N:
			retVal.append("N");
			break;
		case Jump::S:
			retVal.append("S");
			break;
		case Jump::E:
			retVal.append("E");
			break;
		case Jump::W:
			retVal.append("W");
			break;
		case Jump::NE:
			retVal.append("NE");
			break;
		case Jump::SE:
			retVal.append("SE");
			break;
		case Jump::SW:
			retVal.append("SW");
			break;
		case Jump::NW:
			retVal.append("NW");
			break;
		case Jump::NONE:
			retVal.append("NONE");
			break;
	}
	return retVal;
}
bool Jump::hog_asserts_enabled()
{
	bool debug=false; 
	assert(debug = true);
   	return debug;
}
