#include "HPAUtil.h"

#include "constants.h"
#include "graph.h"
#include "map.h"

double HPAUtil::h(node* a, node* b) throw(std::invalid_argument)
{
	if(a == NULL || b == NULL) 
		throw std::invalid_argument("null node");

	int x1 = a->getLabelL(kFirstData);
	int x2 = b->getLabelL(kFirstData);
	int y1 = a->getLabelL(kFirstData+1);
	int y2 = b->getLabelL(kFirstData+1);
	
	double answer = 0.0;
	const double root2m1 = ROOT_TWO-1;//sqrt(2.0)-1;
		if (fabs(x1-x2) < fabs(y1-y2))
			answer = root2m1*fabs(x1-x2)+fabs(y1-y2);
	else
		answer = root2m1*fabs(y1-y2)+fabs(x1-x2);
	return answer;
}
