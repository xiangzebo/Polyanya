#include "TileExpansionPolicy.h"
#include "ManhattanHeuristic.h"
#include "mapAbstraction.h"
#include "ProblemInstance.h"

TileExpansionPolicy::TileExpansionPolicy() 
	: GridMapExpansionPolicy(4)
{
}

TileExpansionPolicy::~TileExpansionPolicy()
{
}

node* TileExpansionPolicy::n()
{
	node* n = 0;
	mapAbstraction* map = problem->getMap();
	switch(which)
	{
		case 0:
			n = map->getNodeFromMap(
					target->getLabelL(kFirstData),
					target->getLabelL(kFirstData+1)-1);
			break;

		case 1:
			n = map->getNodeFromMap(
					target->getLabelL(kFirstData)+1,
					target->getLabelL(kFirstData+1));
			break;
		
		case 2:
			n = map->getNodeFromMap(
					target->getLabelL(kFirstData),
					target->getLabelL(kFirstData+1)+1);
			break;
		case 3:
			n = map->getNodeFromMap(
					target->getLabelL(kFirstData)-1,
					target->getLabelL(kFirstData+1));
			break;
	}
	return n;
}


void TileExpansionPolicy::label_n()
{

	node* tmp = this->n();
	if(tmp)
		tmp->backpointer = this->target;
}

double TileExpansionPolicy::cost_to_n()
{
	node* tmp = this->n();
	problem->getHeuristic()->h(this->target, tmp);
}
