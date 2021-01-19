#include "EmptyClusterInsertionPolicy.h"

#include "EmptyCluster.h"
#include "EmptyClusterAbstraction.h"
#include "IEdgeFactory.h"
#include "MacroEdge.h"
#include "MacroNode.h"
#include "timer.h"

EmptyClusterInsertionPolicy::EmptyClusterInsertionPolicy(
	EmptyClusterAbstraction* _map) : InsertionPolicy(), map(_map)
{
}

EmptyClusterInsertionPolicy::~EmptyClusterInsertionPolicy()
{
}


node* 
EmptyClusterInsertionPolicy::insert(node* _n) 
	throw(std::invalid_argument)
{
	resetMetrics();
	MacroNode* retVal = 0;

	Timer t;
	t.startTimer();

	MacroNode* n = dynamic_cast<MacroNode*>(_n);
	if(n->getLabelL(kParent) == -1)
	{
		EmptyCluster* nCluster = dynamic_cast<EmptyCluster*>(
				map->getCluster(n->getParentClusterId()));

		nCluster->addParent(n);
		graph* absg = map->getAbstractGraph(1);
		retVal = dynamic_cast<MacroNode*>(
				absg->getNode(n->getLabelL(kParent)));

		if(map->getAllowDiagonals())
		{
			if(nCluster->getHeight() == 1 || nCluster->getWidth() == 1)
				cardinalConnect(retVal);
			else
				connect(retVal);
		}
		else
			cardinalConnect(retVal);

		connectInserted(retVal);
		addNode(retVal);
	}
	else
	{
		retVal = dynamic_cast<MacroNode*>(
				map->getAbstractGraph(1)->getNode(
				n->getLabelL(kParent)));
	}

	searchTime = t.endTimer();
	return retVal;
}

void 
EmptyClusterInsertionPolicy::remove(node* _n) 
	throw(std::runtime_error)
{
	if(InsertionPolicy::removeNode(_n))
	{
		// then remove it from the abstract graph and its parent cluster
		graph* g = map->getAbstractGraph(1);
		MacroNode* n = dynamic_cast<MacroNode*>(g->getNode(_n->getNum()));

		if(n)
		{               
			edge_iterator ei = n->getEdgeIter();
			edge* e = n->edgeIterNext(ei);
			while(e)
			{
					g->removeEdge(e);
					delete e;
					ei = n->getEdgeIter();
					e = n->edgeIterNext(ei);
			}

			AbstractCluster* parentCluster = dynamic_cast<GenericClusterAbstraction*>(
					map)->getCluster(n->getParentClusterId());
			parentCluster->removeParent(n);
			g->removeNode(n->getNum()); 

			node* originalN = map->getNodeFromMap(n->getLabelL(kFirstData), 
							n->getLabelL(kFirstData+1));
			originalN->setLabelL(kParent, -1);
			delete n;
			n = 0;
		}
	}
}

void 
EmptyClusterInsertionPolicy::connect(MacroNode* absNode)
{
	EmptyCluster* nCluster = dynamic_cast<EmptyCluster*>(
			map->getCluster(absNode->getParentClusterId()));
	graph* absg = map->getAbstractGraph(1);

	const int x = absNode->getLabelL(kFirstData);
	const int y = absNode->getLabelL(kFirstData+1);
	if(getVerbose())
			std::cout << "inserting node ("<<x<<", "<<y<<") "
				"into abstract graph"<<std::endl;

	if(getVerbose())
			std::cout << "connecting to nodes along the "
				"top of the cluster"<<std::endl;

	// connect to nodes along top perimeter of cluster
	int maxDiagSteps = y - nCluster->getVOrigin();
	int minx = (x-maxDiagSteps)<nCluster->getHOrigin()?
		nCluster->getHOrigin():(x-maxDiagSteps);
	int maxx = (x+maxDiagSteps)>(nCluster->getHOrigin()+nCluster->getWidth()-1)?
			(nCluster->getHOrigin()+nCluster->getWidth()-1):(x+maxDiagSteps);

	MacroNode* absNeighbour = 0;
	int ny = nCluster->getVOrigin();
	int nx = minx+1;
	for( ; nx <= maxx-1; nx++)
	{
		assert(map->getNodeFromMap(nx, ny));
		absNeighbour = static_cast<MacroNode*>(
						absg->getNode(
						map->getNodeFromMap(nx, ny)->getLabelL(kParent)));
		if(absNeighbour && &*absNeighbour != &*absNode)
				addMacroEdge(absNode, absNeighbour);
	}

	// try to connect to nearest entrance along the top border not in the fan 
	// of absNode
	if(nCluster->getHeight() > 1 && nCluster->getWidth() > 1)
	{
		if(maxDiagSteps == 0)
			absNeighbour = nCluster->nextNodeInRow(minx-1, ny, false);
		else
			absNeighbour = nCluster->nextNodeInRow(minx, ny, false);

		if(absNeighbour)
			addMacroEdge(absNode, absNeighbour);

		if(maxDiagSteps == 0)
			absNeighbour = nCluster->nextNodeInRow(maxx+1, ny, true);
		else
			absNeighbour = nCluster->nextNodeInRow(maxx, ny, true);

		if(absNeighbour)
				addMacroEdge(absNode, absNeighbour);
	}

	if(getVerbose())
			std::cout << "connecting to nodes along the bottom of "
				"the cluster"<<std::endl;

	// connect to nodes along the bottom perimeter of cluster
	maxDiagSteps = (nCluster->getVOrigin()+nCluster->getHeight()-1) - y;
	minx = (x-maxDiagSteps)<nCluster->getHOrigin()?
		nCluster->getHOrigin():(x-maxDiagSteps);
	maxx = (x+maxDiagSteps)>(nCluster->getHOrigin()+nCluster->getWidth()-1)?
			(nCluster->getHOrigin()+nCluster->getWidth()-1):(x+maxDiagSteps);

	nx = minx+1;
	ny = nCluster->getVOrigin()+nCluster->getHeight()-1;
	for( ; nx <= maxx-1; nx++)
	{
		assert(map->getNodeFromMap(nx, ny));
		absNeighbour = static_cast<MacroNode*>(
						absg->getNode(
						map->getNodeFromMap(nx, ny)->getLabelL(kParent)));
		if(absNeighbour && &*absNeighbour != &*absNode)
			addMacroEdge(absNode, absNeighbour);
	}

	// try to connect to nearest entrance along the bottom border not in the 
	// fan of absNode
	if(nCluster->getHeight() > 1 && nCluster->getWidth() > 1)
	{
		if(maxDiagSteps == 0)
			absNeighbour = nCluster->nextNodeInRow(minx-1, ny, false);
		else
			absNeighbour = nCluster->nextNodeInRow(minx, ny, false);

		if(absNeighbour)
			addMacroEdge(absNode, absNeighbour);

		if(maxDiagSteps == 0)
			absNeighbour = nCluster->nextNodeInRow(maxx+1, ny, true);
		else
			absNeighbour = nCluster->nextNodeInRow(maxx, ny, true);

		if(absNeighbour)
			addMacroEdge(absNode, absNeighbour);

	}
	
	if(getVerbose())
			std::cout << "connecting to nodes along the left of the "
			"cluster"<<std::endl;

	// connect to nodes along the left perimeter of cluster
	maxDiagSteps = x - nCluster->getHOrigin();       
	int miny = (y-maxDiagSteps)<nCluster->getVOrigin()?
		nCluster->getVOrigin():(y-maxDiagSteps);
	int maxy = (y+maxDiagSteps)>(nCluster->getVOrigin()+nCluster->getHeight()-1)?
			(nCluster->getVOrigin()+nCluster->getHeight()-1):(y+maxDiagSteps);

	nx = nCluster->getHOrigin();
	ny = miny+1;
	for( ; ny <= maxy-1; ny++)
	{
		assert(map->getNodeFromMap(nx, ny));
		absNeighbour = static_cast<MacroNode*>(
						absg->getNode(
						map->getNodeFromMap(nx, ny)->getLabelL(kParent)));
		if(absNeighbour && &*absNeighbour != &*absNode)
			addMacroEdge(absNode, absNeighbour);
	}

	// connect to nearest nodes outside fan area (if necessary)
	if(nCluster->getHeight() > 1 && nCluster->getWidth() > 1)
	{
		if(maxDiagSteps == 0)
			absNeighbour = nCluster->nextNodeInColumn(nx, miny-1, false);
		else
			absNeighbour = nCluster->nextNodeInColumn(nx, miny, false);

		if(absNeighbour)
			addMacroEdge(absNode, absNeighbour);

		if(maxDiagSteps == 0)
			absNeighbour = nCluster->nextNodeInColumn(nx, maxy+1, true);
		else
			absNeighbour = nCluster->nextNodeInColumn(nx, maxy, true);

		if(absNeighbour)
			addMacroEdge(absNode, absNeighbour);
	}

	if(getVerbose())
			std::cout << "connecting to nodes along the right of the "
			"cluster"<<std::endl;

	// connect to nodes along the right perimeter of cluster
	maxDiagSteps = (nCluster->getHOrigin()+nCluster->getWidth()-1) - x;
	miny = (y-maxDiagSteps)<nCluster->getVOrigin()?
		nCluster->getVOrigin():(y-maxDiagSteps);
	maxy = (y+maxDiagSteps)>(nCluster->getVOrigin()+nCluster->getHeight()-1)?
			(nCluster->getVOrigin()+nCluster->getHeight()-1):(y+maxDiagSteps);

	nx = nCluster->getHOrigin()+nCluster->getWidth()-1;
	ny = miny+1;
	for( ; ny <= maxy-1; ny++)
	{
		assert(map->getNodeFromMap(nx, ny));
		absNeighbour = static_cast<MacroNode*>(
						absg->getNode(
						map->getNodeFromMap(nx, ny)->getLabelL(kParent)));
		if(absNeighbour && &*absNeighbour != &*absNode)
			addMacroEdge(absNode, absNeighbour);
	}

	// connect to nearest nodes outside fan area (if necessary)
	if(nCluster->getHeight() > 1 && nCluster->getWidth() > 1)
	{
		if(maxDiagSteps == 0)
			absNeighbour = nCluster->nextNodeInColumn(nx, miny-1, false);
		else
			absNeighbour = nCluster->nextNodeInColumn(nx, miny, false);

		if(absNeighbour)
			addMacroEdge(absNode, absNeighbour);

		if(maxDiagSteps == 0)
			absNeighbour = nCluster->nextNodeInColumn(nx, maxy+1, true);
		else
			absNeighbour = nCluster->nextNodeInColumn(nx, maxy, true);

		if(absNeighbour)
			addMacroEdge(absNode, absNeighbour);
	}
}

void 
EmptyClusterInsertionPolicy::cardinalConnect(MacroNode* absNode)
{
        graph* absg = map->getAbstractGraph(1);
        EmptyCluster* nodeCluster = map->getCluster(absNode->getParentClusterId());

        int x = absNode->getLabelL(kFirstData);
        int y = absNode->getLabelL(kFirstData+1);

        if(getVerbose())
                std::cout << "inserting node ("<<x<<", "<<y<<") into abstract "
				"graph"<<std::endl;

        // connect to neighbour above
        int ny = nodeCluster->getVOrigin();
        int nx = x;
        MacroNode* absNeighbour = static_cast<MacroNode*>(
                        absg->getNode(
						map->getNodeFromMap(nx, ny)->getLabelL(kParent)));
        if(absNeighbour == 0 || absNeighbour->getNum() == absNode->getNum())
        {
			absNeighbour = nodeCluster->nextNodeInRow(nx+1, ny, true);
			if(absNeighbour)
				addMacroEdge(absNode, absNeighbour);

			absNeighbour =  nodeCluster->nextNodeInRow(nx-1, ny, false);
			if(absNeighbour)
				addMacroEdge(absNode, absNeighbour);
        }
        else
                addMacroEdge(absNode, absNeighbour);

        // connect to neighbour below
        ny = nodeCluster->getVOrigin()+nodeCluster->getHeight()-1;
        nx = x;
        absNeighbour = static_cast<MacroNode*>(
                        absg->getNode(
						map->getNodeFromMap(nx, ny)->getLabelL(kParent)));
        if(absNeighbour == 0 || absNeighbour->getNum() == absNode->getNum())
        {
                absNeighbour = nodeCluster->nextNodeInRow(nx+1, ny, true);
                if(absNeighbour)
                        addMacroEdge(absNode, absNeighbour);

                absNeighbour =  nodeCluster->nextNodeInRow(nx-1, ny, false);
                if(absNeighbour)
                        addMacroEdge(absNode, absNeighbour);
        }
        else
                addMacroEdge(absNode, absNeighbour);
                

        // connect to neighbour to the left
        ny = y; 
        nx = nodeCluster->getHOrigin();
        absNeighbour = static_cast<MacroNode*>(
                        absg->getNode(
						map->getNodeFromMap(nx, ny)->getLabelL(kParent)));
        if(absNeighbour == 0 || absNeighbour->getNum() == absNode->getNum())
        {
                absNeighbour = nodeCluster->nextNodeInColumn(nx, ny+1, true);
                if(absNeighbour)
                        addMacroEdge(absNode, absNeighbour);

                absNeighbour =  nodeCluster->nextNodeInColumn(nx, ny-1, false);
                if(absNeighbour)
                        addMacroEdge(absNode, absNeighbour);
        }
        else
                addMacroEdge(absNode, absNeighbour);


        // connect to neighbour to the right
        ny = y; 
        nx = nodeCluster->getHOrigin()+nodeCluster->getWidth()-1;
        absNeighbour = static_cast<MacroNode*>(
                        absg->getNode(map->getNodeFromMap(nx, ny)->getLabelL(kParent)));
        if(absNeighbour == 0 || absNeighbour->getNum() == absNode->getNum())
        {
                absNeighbour = nodeCluster->nextNodeInColumn(nx, ny+1, true);
                if(absNeighbour)
                        addMacroEdge(absNode, absNeighbour);

                absNeighbour =  nodeCluster->nextNodeInColumn(nx, ny-1, false);
                if(absNeighbour)
                        addMacroEdge(absNode, absNeighbour);
        }
        else
                addMacroEdge(absNode, absNeighbour);
}

void 
EmptyClusterInsertionPolicy::addMacroEdge(
		MacroNode* absNode, MacroNode* absNeighbour)
{
//	assert(map->getNodeFromMap(absNode->getLabelL(kFirstData),
//			absNode->getLabelL(kFirstData+1))->getLabelL(kParent) != -1);
//	assert(map->getNodeFromMap(absNeighbour->getLabelL(kFirstData), 
//			absNeighbour->getLabelL(kFirstData+1))->getLabelL(kParent) != -1);

	if(absNode->getUniqueID() == absNeighbour->getUniqueID())
		return;

	graph* absg = map->getAbstractGraph(1);

	assert((int)absNeighbour->getNum() < absg->getNumNodes());
	assert((int)absNode->getNum() < absg->getNumNodes());

	edge* e = map->getEdgeFactory()->newEdge(absNode->getNum(), absNeighbour->getNum(), 
			map->h(absNode, absNeighbour));
	absg->addEdge(e);

	if(getVerbose())
	{
		std::cout << "absNeighbour ("<<absNeighbour->getLabelL(kFirstData)<<", "
			<<absNeighbour->getLabelL(kFirstData+1)<<") weight: "
			<<e->getWeight() <<std::endl;
	}
}
	
void 
EmptyClusterInsertionPolicy::connectInserted(MacroNode* n)
{
	// TODO: only add edges if node is in the interior (and not the
	// perimeter) of the cluster. At the moment, we add edges unnecessarily
	// when this is not the case.

	if(getVerbose())
			std::cout << "connecting to other inserted nodes from the "
				"interior of the cluster"<<std::endl;

	for(int i=0; i < getNumInserted(); i++)
	{
		MacroNode* neighbour = dynamic_cast<MacroNode*>(
				getInsertedAtIndex(i));

		if(n->getParentClusterId() == neighbour->getParentClusterId())
			addMacroEdge(n, neighbour);
	}
}
