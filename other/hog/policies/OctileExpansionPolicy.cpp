#include "OctileExpansionPolicy.h"

#include "mapAbstraction.h"
#include "OctileHeuristic.h"
#include "ProblemInstance.h"

OctileExpansionPolicy::OctileExpansionPolicy()
	: GridMapExpansionPolicy(8)
{
}

OctileExpansionPolicy::~OctileExpansionPolicy()
{

}

node* OctileExpansionPolicy::n()
{
	node* n = 0;
	mapAbstraction* map = problem->getMap();
	switch(which)
	{
		case 0: // n
			n = map->getNodeFromMap(
					target->getLabelL(kFirstData),
					target->getLabelL(kFirstData+1)-1);
			break;

		case 1: // e
			n = map->getNodeFromMap(
					target->getLabelL(kFirstData)+1,
					target->getLabelL(kFirstData+1));
			break;
		
		case 2: // s
			n = map->getNodeFromMap(
					target->getLabelL(kFirstData),
					target->getLabelL(kFirstData+1)+1);
			break;

		case 3: // w
			n = map->getNodeFromMap(
					target->getLabelL(kFirstData)-1,
					target->getLabelL(kFirstData+1));
			break;

		case 4: // nw
		{
			int tx = target->getLabelL(kFirstData);
			int ty = target->getLabelL(kFirstData+1);
			n = map->getNodeFromMap(tx-1, ty-1);
			if( ! map->getCutCorners() )
			{
				if(!map->getNodeFromMap(tx-1, ty) || 
				   !map->getNodeFromMap(tx, ty-1))
				{
					n = 0;
				}
			}
			break;
		}

		case 5: // ne
		{
			int tx = target->getLabelL(kFirstData);
			int ty = target->getLabelL(kFirstData+1);
			n = map->getNodeFromMap(tx+1, ty-1);
			if( ! map->getCutCorners() )
			{
				if(!map->getNodeFromMap(tx+1, ty) || 
				   !map->getNodeFromMap(tx, ty-1))
				{
					n = 0;
				}
			}
			break;
		}

		case 6: // se
		{
			int tx = target->getLabelL(kFirstData);
			int ty = target->getLabelL(kFirstData+1);
			n = map->getNodeFromMap(tx+1, ty+1);
			if( ! map->getCutCorners() )
			{
				if(!map->getNodeFromMap(tx+1, ty) || 
				   !map->getNodeFromMap(tx, ty+1))
				{
					n = 0;
				}
			}
			break;
		}

		case 7: // sw
		{
			int tx = target->getLabelL(kFirstData);
			int ty = target->getLabelL(kFirstData+1);
			n = map->getNodeFromMap(tx-1, ty+1);
			if( ! map->getCutCorners() )
			{
				if(!map->getNodeFromMap(tx-1, ty) || 
				   !map->getNodeFromMap(tx, ty+1))
				{
					n = 0;
				}
			}
			break;
		}
	}
	return n;
}

void OctileExpansionPolicy::label_n()
{

	node* tmp = this->n();
	if(tmp)
		tmp->backpointer = this->target;
}

double OctileExpansionPolicy::cost_to_n()
{
	node* tmp = this->n();
	if(tmp)
	{
		int absx = abs(tmp->getLabelL(kFirstData) - target->getLabelL(kFirstData));
		int absy = abs(tmp->getLabelL(kFirstData+1) - target->getLabelL(kFirstData+1));
		if(absx+absy == 1)
		{
			return 1;
		}
		if(absx+absy == 2)
		{
			return ROOT_TWO;
		}
		std::cerr << "OctileExpansionPolicy::cost_to_n current node has non-adjacent neighbour??!? \n";
	}
	return 0;
}
