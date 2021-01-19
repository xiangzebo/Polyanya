#include "EmptyCluster.h"

#include "EmptyClusterAbstraction.h"
#include "IEdgeFactory.h"
#include "MacroEdge.h"
#include "MacroNode.h"

EmptyCluster::EmptyCluster(const int _x, const int _y, 
		EmptyClusterAbstraction* map, bool pr, bool bfr) 
	: AbstractCluster(map)
{
	if(_x < 0 || _y < 0)
		throw std::invalid_argument("EmptyCluster::EmptyCluster: "
				"cluster (x,y) coordinates must be >= 0");

	startx = _x;
	starty = _y;
	height = 1;
	width = 1;
}

EmptyCluster::~EmptyCluster()
{
	graph* absg = map->getAbstractGraph(1);
	while(secondaryEdges.size() > 0)
	{
		edge* e = secondaryEdges.at(0);
		MacroNode* from = dynamic_cast<MacroNode*>(
				absg->getNode(e->getFrom()));
		assert(from);
		MacroNode* to = dynamic_cast<MacroNode*>(
				absg->getNode(e->getTo()));
		assert(to);

		from->removeSecondaryEdge(e->getEdgeNum());
		to->removeSecondaryEdge(e->getEdgeNum());
		secondaryEdges.erase(secondaryEdges.begin());
		delete e;
	}
}

// Starting from a seed location (startx, starty), build a maximally
// sized rectangular room that is free of obstacles.
void 
EmptyCluster::buildCluster()
{
	ClusterNode* n = dynamic_cast<ClusterNode*>(
		map->getNodeFromMap(getHOrigin(), getVOrigin()));

	if(!n || n->getParentClusterId() != -1)
	{
		height = 0;
		width = 0;
		std::cout << "WARNING!! aborting cluster creation; "
			"origin is invalid or already assigned!!"<<std::endl;
		return;
	}
	height=1;
	width=1;

	while(canExtendClearanceSquare())
	{
		height++;
		width++;
	}

	int squareheight = height;
	int squarewidth = width;
	while(canExtendHorizontally())
	{
		width++;
	}
	int maxwidth = width;

	width = squarewidth;
	while(canExtendVertically())
	{
		height++;
	}
	int maxheight = height;

	if(maxheight*squarewidth > squareheight*maxwidth)
	{
		height = maxheight;
		width = squarewidth;
	}
	else
	{
		height = squareheight;
		width = maxwidth;
	}

	// assign nodes to the cluster
	for(int x=getHOrigin(); x<getHOrigin()+width; x++)
		for(int y=getVOrigin(); y<getVOrigin()+height; y++)
			addNode(map->getNodeFromMap(x, y));

}

bool 
EmptyCluster::canExtendClearanceSquare()
{
	int x = getHOrigin() + width;
	int y = getVOrigin();
	for( ; y <= getVOrigin() + height; y++)
	{
		ClusterNode* n = dynamic_cast<ClusterNode*>(
			map->getNodeFromMap(x, y));
		if(!n || n->getParentClusterId() != -1)
			return false;
	}

	x = getHOrigin();
	y = getVOrigin() + height;
	for( ; x <= getHOrigin() + width; x++)
	{
		ClusterNode* n = dynamic_cast<ClusterNode*>(
			map->getNodeFromMap(x, y));
		if(!n || n->getParentClusterId() != -1)
			return false;
	}

	return true;
}

bool 
EmptyCluster::canExtendHorizontally()
{
	int x = getHOrigin() + width;
	for(int y = getVOrigin(); y<getVOrigin() + height; y++)
	{
		ClusterNode* n = dynamic_cast<ClusterNode*>(map->getNodeFromMap(x, y));
		if(!n || n->getParentClusterId() != -1)
			return false;
	}
	return true;
}

bool 
EmptyCluster::canExtendVertically()
{
	int y = getVOrigin() + height;
	for(int x = getHOrigin(); x<getHOrigin() + width; x++)
	{
		ClusterNode* n = dynamic_cast<ClusterNode*>(map->getNodeFromMap(x, y));
		if(!n || n->getParentClusterId() != -1)
			return false;
	}
	return true;

}

// Every node from the perimeter, which has a neighbour in an adjacent cluster,
// is added to the abstract graph. 
void 
EmptyCluster::frameCluster()
{
        if(getVerbose())
                std::cout << "frameCluster "<<this->getId()<<std::endl;

		// special case for when the cluster is size 1x1
		if(width == 1 && height == 1)
		{
				MacroNode* n = dynamic_cast<MacroNode*>(
						map->getNodeFromMap(startx, starty));
				assert(n);
				if(!perimeterReduction || 
						(isIncidentWithInterEdge(n) && perimeterReduction))
				{
					addParent(n);
				}
				return;
		}

		graph* absg = map->getAbstractGraph(1);
        MacroNode* first = 0;
        MacroNode* last = 0;
		
        // add nodes along left border
        int x = this->getHOrigin();
        int y = this->getVOrigin();
        for( ; y<this->getVOrigin()+this->getHeight(); y++)
        {
			MacroNode* n = dynamic_cast<MacroNode*>(map->getNodeFromMap(x,y));
			assert(n);
			if(!perimeterReduction || 
					(isIncidentWithInterEdge(n) && perimeterReduction))
			{
				addParent(n);
				if(last)
				{
					if(n->getUniqueID() != last->getUniqueID() &&
					     ((isIncidentWithInterEdge(last) && 
						   perimeterReduction) || !perimeterReduction))
					{
						addSingleMacroEdge(
								absg->getNode(n->getLabelL(kParent)), 
								absg->getNode(last->getLabelL(kParent)), 
								map->h(n, last), absg);
						last = n;
					}
				}
				else
				{
					first = n;
					last = n;
				}
			}
        }

        // add nodes along bottom border
        y = this->getVOrigin()+this->getHeight()-1;
        for(x=this->getHOrigin()+1; x<this->getHOrigin()+this->getWidth(); x++)
        {
			MacroNode* n = dynamic_cast<MacroNode*>(map->getNodeFromMap(x,y));
			assert(n);
			if(!perimeterReduction || 
					(isIncidentWithInterEdge(n) && perimeterReduction))
			{
				addParent(n);
				if(last)
				{
					if(n->getUniqueID() != last->getUniqueID() &&
					     ((isIncidentWithInterEdge(last) && 
						   perimeterReduction) || !perimeterReduction))
					{
						addSingleMacroEdge(
								absg->getNode(n->getLabelL(kParent)), 
								absg->getNode(last->getLabelL(kParent)), 
								map->h(n, last), absg);
						last = n;
					}
				}
				else
				{
					first = n;
					last = n;
				}
			}
        }

        // don't try to add edges twice if the cluster has dimension = 1
        if(getWidth() > 1 && getHeight() > 1)
        {
			// add nodes along right border
			x = this->getHOrigin()+this->getWidth()-1;
			for(y=this->getVOrigin()+this->getHeight()-2; 
					y>=this->getVOrigin(); y--)
			{
				MacroNode* n = dynamic_cast<MacroNode*>(map->getNodeFromMap(x,y));
				assert(n);
				if(!perimeterReduction || 
						(isIncidentWithInterEdge(n) && perimeterReduction))
				{
					addParent(n);
					if(last)
					{
						if(n->getUniqueID() != last->getUniqueID() &&
							 ((isIncidentWithInterEdge(last) && 
							   perimeterReduction) || !perimeterReduction))
						{
							addSingleMacroEdge(
								absg->getNode(n->getLabelL(kParent)), 
								absg->getNode(last->getLabelL(kParent)), 
								map->h(n, last), absg);
							last = n;
						}
					}
					else
					{
						first = n;
						last = n;
					}
				}
			}

			// add nodes along top border
			y = this->getVOrigin();
			for(x=this->getHOrigin()+this->getWidth()-2; 
					x>=this->getHOrigin()+1; x--)
			{
				MacroNode* n = dynamic_cast<MacroNode*>(map->getNodeFromMap(x,y));
				assert(n);
				if(!perimeterReduction || 
						(isIncidentWithInterEdge(n) && perimeterReduction))
				{
					addParent(n);
					if(last)
					{
						if(n->getUniqueID() != last->getUniqueID() &&
							 ((isIncidentWithInterEdge(last) && 
							   perimeterReduction) || !perimeterReduction))
						{
							addSingleMacroEdge(
								absg->getNode(n->getLabelL(kParent)), 
								absg->getNode(last->getLabelL(kParent)), 
								map->h(n, last), absg);
							last = n;
						}
					}
					else
					{
						first = n;
						last = n;
					}
				}
			}

			// connect the first and last nodes to be considered
			if(first && last && first->getUniqueID() != last->getUniqueID())
			{
				addSingleMacroEdge(
						absg->getNode(first->getLabelL(kParent)), 
						absg->getNode(last->getLabelL(kParent)), 
						map->h(first, last), absg);
			}
        }

}

void 
EmptyCluster::buildEntrances()
{
	if(getVerbose())
	{
		std::cout << "EmptyCluster::buildEntrances. Cluster origin: "
			<<getHOrigin()<<", "<<getVOrigin()<<std::endl;
	}

	// identfy perimeter nodes
	frameCluster();

	// connect perimeter nodes with neighbours from adjacent clusters
	addInterEdges();

	// add interior macro edges
	if(map->getAllowDiagonals())
	   addMacroEdges();
	else
		addCardinalMacroEdges();
}

// Not implemented. See Issue 34 at http://ddh.googlecode.com
void 
EmptyCluster::connectParent(node* n) 
	throw(std::invalid_argument)
{
}

// returns true if node n_ has a neighbour with a different parent cluster. 
bool 
EmptyCluster::isIncidentWithInterEdge(node* n_)
{
	bool retVal = false;
	MacroNode* n = dynamic_cast<MacroNode*>(n_);
	assert(n);

	int nx = n->getLabelL(kFirstData);
	int ny = n->getLabelL(kFirstData+1);
	for(int nbx = nx-1; nbx < nx+2; nbx++)
		for(int nby = ny-1; nby < ny+2; nby++)
		{
			MacroNode* nb = dynamic_cast<MacroNode*>(
					map->getNodeFromMap(nbx, nby));
			if(!map->getAllowDiagonals() && nbx != nx && nby != ny)
				continue;
			else if(nb && 
					nb->getParentClusterId() != n->getParentClusterId())
			{
				retVal = true;
			}
		}
	
	return retVal;
}

void 
EmptyCluster::addSingleMacroEdge(node* from_, node* to_, double weight, 
		graph* absg, bool secondaryEdge)
{
	assert(from_ && to_);
	assert(from_->getUniqueID() != to_->getUniqueID());

	MacroNode* from = dynamic_cast<MacroNode*>(from_);
	MacroNode* to = dynamic_cast<MacroNode*>(to_);

	assert(from && to);
	if(from->getParentClusterId() != to->getParentClusterId())
	{
		std::cerr << "warning: adding macro edge between nodes from different"
			" clusters" << std::endl;
	}

	edge* e = absg->findEdge(from->getNum(), to->getNum());
	if(e == 0) 
	{
		if(secondaryEdge && bfReduction)
		{
			e = findSecondaryEdge(from->getNum(), to->getNum());
			if(e == 0)
			{
				e = map->getEdgeFactory()->newEdge(from->getNum(), to->getNum(), weight);
				from->addSecondaryEdge(e);
				to->addSecondaryEdge(e);
				secondaryEdges.push_back(e);
				macro++;
			}
		}
		else
		{
			e = map->getEdgeFactory()->newEdge(from->getNum(), to->getNum(), weight);
			absg->addEdge(e);
			macro++;
		}

		if(e && getVerbose())
		{
			std::cout << "added";
			if(secondaryEdge)
				std::cout << " secondary ";
			std::cout << " macro edge: [ "<<from->getNum()<<" ("<<from->getLabelL(kFirstData)<<", ";
			std::cout <<from->getLabelL(kFirstData+1)<<") <-> "<<to->getNum()<<" (";
			std::cout << to->getLabelL(kFirstData) << ", ";
			std::cout << to->getLabelL(kFirstData+1);
			std::cout <<") wt: "<<weight<< " ] "<<std::endl;
		}
	}
}

// Connects abstract nodes in this cluster to any abstract neighbours in 
// immediately adjacent clusters. Following the terminology in 
// [Botea et al 2004], each such edge is referred to as an 
// inter(cluster) edge.
void
EmptyCluster::addInterEdges()
{
	
	for(HPAUtil::nodeTable::iterator it = 	parents.begin(); 
			it != parents.end(); it++)
	{
		ClusterNode* parent = dynamic_cast<ClusterNode*>((*it).second);
		assert(parent);

		ClusterNode *n = dynamic_cast<ClusterNode*>(
						map->getNodeFromMap(
								parent->getLabelL(kFirstData),
								parent->getLabelL(kFirstData+1)));
		assert(n->getLabelL(kParent) == parent->getNum());

		graph* g = map->getAbstractGraph(0);
		graph* absg = map->getAbstractGraph(1);

		edge_iterator edge_it = n->getEdgeIter();
		for(edge* e = n->edgeIterNext(edge_it); e != 0; e = n->edgeIterNext(edge_it))
		{
			double edgeweight = e->getWeight();
			ClusterNode* nb = dynamic_cast<ClusterNode*>(
							g->getNode(
									e->getFrom()== n->getNum()?e->getTo():e->getFrom()));
			assert(nb);

			if(nb->getParentClusterId() != -1 && nb->getLabelL(kParent) != -1)
			{
				ClusterNode* parent2 = dynamic_cast<ClusterNode*>(
						absg->getNode(nb->getLabelL(kParent))); 
				assert(parent2);
						
				edge* absedge = absg->findEdge(parent->getNum(), parent2->getNum());
				if(!absedge)
				{
					absedge = map->getEdgeFactory()->newEdge(parent->getNum(),
									parent2->getNum(), edgeweight);
					absg->addEdge(absedge);

					if(getVerbose())
					{
							std::cout << "EmptyCluster::addInterEdges connecting: ";
							parent->print(std::cout);
							parent2->print(std::cout);
							std::cout << " cost: "<<edgeweight<<std::endl;
					}
				}
			}       
		}
	}
}

// use this when working with 8-connected grid maps
void EmptyCluster::addMacroEdges()
{
	if(getVerbose())
	{
		std::cout << "adding macro edges for cluster "<<getId()<<" origin ";
		std::cout <<"("<< getHOrigin()<<", "<<getVOrigin()<<")";
		std::cout <<" diagonal edges allowed? "<<map->getAllowDiagonals()<< " "<<std::endl;
	}

	if(this->getHeight() > 1 && this->getWidth() > 1)
	{
		addDiagonalMacroEdges();
		addDiagonalFanMacroEdges();
	}

	if(getVerbose())
		std::cout << macro << " macro edges added for cluster "<<getId()<<std::endl;
}

// Use this when working with 4-connected grid maps
void EmptyCluster::addCardinalMacroEdges()
{
//	this->print(std::cout);
//	std::cout << std::endl;
	if(getVerbose())
	{
		std::cout << "adding cardinal macro edges for cluster "<<getId()<<" origin ";
		std::cout <<"("<< getHOrigin()<<", "<<getVOrigin()<<")";
		std::cout <<" diagonal edges allowed? "<<map->getAllowDiagonals()<< " "<<std::endl;
	}
	graph* absg = map->getAbstractGraph(1);
	macro = 0;

	// connect nodes on directly opposite sides of the perimeter
	if(this->getWidth() > 1)
	{
		int lx = this->getHOrigin();
		int rx = lx + this->getWidth()-1;
		for(int y=this->getVOrigin(); y<this->getVOrigin()+this->getHeight(); y++)
		{
			node *left = absg->getNode(
					map->getNodeFromMap(lx, y)->getLabelL(kParent));
			node *right = absg->getNode(
					map->getNodeFromMap(rx, y)->getLabelL(kParent));
			if(left && right)
				addSingleMacroEdge(left, right, map->h(left, right), absg, true);
			else if(left && !right)
			{	
				right = nextNodeInColumn(rx, y, false);
				if(right)
					addSingleMacroEdge(left, right, map->h(left, right), absg, true);
				right = nextNodeInColumn(rx, y, true);
				if(right)
					addSingleMacroEdge(left, right, map->h(left, right), absg, true);
			}
			else if(!left && right)
			{
				left = nextNodeInColumn(lx, y, false);
				if(left)
					addSingleMacroEdge(left, right, map->h(left, right), absg, true);
				left = nextNodeInColumn(lx, y, true);
				if(left)
					addSingleMacroEdge(left, right, map->h(left, right), absg, true);
			}
		}
	}

	if(this->getHeight() > 1)
	{
		int ty = this->getVOrigin();
		int by = this->getVOrigin()+this->getHeight()-1;
		for(int x=this->getHOrigin(); x<this->getHOrigin()+this->getWidth(); x++)
		{
			node *top = absg->getNode(
					map->getNodeFromMap(x, ty)->getLabelL(kParent));
			node *bottom = absg->getNode(
					map->getNodeFromMap(x, by)->getLabelL(kParent));
			if(top && bottom)
				addSingleMacroEdge(top, bottom, map->h(top, bottom), absg, true);
			else if(top && !bottom)
			{
				bottom = nextNodeInRow(x, by, false);
				if(bottom)
					addSingleMacroEdge(top, bottom, map->h(top, bottom), absg, true);
				bottom = nextNodeInRow(x, by, true);
				if(bottom)
					addSingleMacroEdge(top, bottom, map->h(top, bottom), absg, true);
			}
			else if(!top && bottom)
			{
				top = nextNodeInRow(x, ty, false);
				if(top)
					addSingleMacroEdge(top, bottom, map->h(top, bottom), absg, true);
				top = nextNodeInRow(x, ty, true);
				if(top)
					addSingleMacroEdge(top, bottom, map->h(top, bottom), absg, true);
			}
		}
	}

	if(getVerbose())
		std::cout << macro << " macro edges added for cluster "<<getId()<<std::endl;
}


void
EmptyCluster::addDiagonalMacroEdges()
{
	graph* absg = map->getAbstractGraph(1);
	macro = 0;

	// first, add diagonal edges between nodes on orthogonal sides of the 
	// cluster
	int max = this->getHeight() > this->getWidth()?
		this->getWidth():this->getHeight();
	for(int offset=1; offset<max; offset++)
	{
		// connect left and bottom sides of cluster
		int fx = this->getHOrigin()+offset;
		int fy = this->getVOrigin();
		int sx = this->getHOrigin();
		int sy = this->getVOrigin()+offset;
		node *first = absg->getNode(
				map->getNodeFromMap(fx, fy)->getLabelL(kParent));
		node *second = absg->getNode(
				map->getNodeFromMap(sx, sy)->getLabelL(kParent));
		if(first && second)
			addSingleMacroEdge(first, second, map->h(first, second), absg);
		else if(!first && second)
		{
			first = nextNodeInRow(fx, fy, true);
			if(first)
				addSingleMacroEdge(first, second, map->h(first, second), absg);
		}
		else if(!second && first)
		{
			second = nextNodeInColumn(sx, sy, true);
			if(second)
				addSingleMacroEdge(first, second, map->h(first, second), absg);
		}

		// connect top and right sides of cluster
		fx = this->getHOrigin()+this->getWidth()-1-offset;
		sx = this->getHOrigin()+this->getWidth()-1;
		first = absg->getNode(
				map->getNodeFromMap(fx, fy)->getLabelL(kParent));
		second = absg->getNode(
				map->getNodeFromMap(sx, sy)->getLabelL(kParent));
		if(first && second)
			addSingleMacroEdge(first, second, map->h(first, second), absg);
		else if(!first && second)
		{
			first = nextNodeInRow(fx, fy, false);
			if(first)
				addSingleMacroEdge(first, second, map->h(first, second), absg);
		}
		else if(first && !second)
		{
			second = nextNodeInColumn(sx, sy, true);
			if(second)
				addSingleMacroEdge(first, second, map->h(first, second), absg);
		}

		// connect left and bottom sides of cluster
		fx = this->getHOrigin()+offset;
		fy = this->getVOrigin()+getHeight()-1;
		sx = this->getHOrigin();
		sy = this->getVOrigin()+this->getHeight()-1-offset;
		first = absg->getNode(
				map->getNodeFromMap(fx, fy)->getLabelL(kParent));
		second = absg->getNode(
				map->getNodeFromMap(sx, sy)->getLabelL(kParent));
		if(first && second)
			addSingleMacroEdge(first, second, map->h(first, second), absg);
		else if(!first && second)
		{
			first = nextNodeInRow(fx, fy, true);
			if(first)
				addSingleMacroEdge(first, second, map->h(first, second), absg);
		}
		else if(first && !second)
		{
			second = nextNodeInColumn(sx, sy, false);
			if(second)
				addSingleMacroEdge(first, second, map->h(first, second), absg);
		}

		// connect bottom and right sides sides of cluster
		fx = this->getHOrigin()+this->getWidth()-1;
		fy = this->getVOrigin()+getHeight()-1-offset;
		sx = this->getHOrigin()+this->getWidth()-1-offset;
		sy = this->getVOrigin()+this->getHeight()-1;
		first = absg->getNode(
				map->getNodeFromMap(fx, fy)->getLabelL(kParent));
		second = absg->getNode(
				map->getNodeFromMap(sx, sy)->getLabelL(kParent));
		if(first && second) 
			addSingleMacroEdge(first, second, map->h(first, second), absg);
		else if(!first && second)
		{
			first = nextNodeInColumn(fx, fy, false);
			if(first)
				addSingleMacroEdge(first, second, map->h(first, second), absg);
		}
		else if(first && !second)
		{
			second = nextNodeInRow(sx, sy, false);
			if(second)
				addSingleMacroEdge(first, second, map->h(first, second), absg);
		}
	}

}

void
EmptyCluster::addDiagonalFanMacroEdges()
{
	graph* absg = map->getAbstractGraph(1);
	macro = 0;


	// add edges connecting nodes on the top side to nodes on the bottom 
	// side of the cluster
	int max = this->getHeight() > this->getWidth()?
		this->getWidth():this->getHeight();
	max--;  // max # diagonal steps we can take in crossing this cluster
	if(getVerbose())
	{
		std::cout << "adding fan edges between top and bottom"<<std::endl;
		std::cout << "max diagonal steps: "<<max<<std::endl;
	}
	for(int fx=this->getHOrigin(); 
			fx<this->getHOrigin()+this->getWidth();
			fx++)
	{
		int fy = this->getVOrigin();
		node* first = absg->getNode(
				map->getNodeFromMap(fx, fy)->getLabelL(kParent));

		if(!first) continue;

		// nodes in the fan are in the range [minx, maxx] 
		int minx = (fx-max)<this->getHOrigin()?
			(this->getHOrigin()):(fx-max); 
		int maxx = (fx+max)>(this->getHOrigin()+this->getWidth()-1)?
			(this->getHOrigin()+this->getWidth()-1):(fx+max);

		int sx = minx+1;
		int sy = this->getVOrigin()+this->getHeight()-1;
		node* second = 0;
		for( ; sx <= maxx-1; sx++)
		{
			second = absg->getNode(
				map->getNodeFromMap(sx, sy)->getLabelL(kParent));

			if(!second) continue;
			addSingleMacroEdge(first, second, map->h(first, second), absg, true);
		}
		
		// check if nodes exist at fan edges; if not, extend the fan area until we hit something
		second = nextNodeInRow(minx, sy, false);
		if(second)
			addSingleMacroEdge(first, second, map->h(first, second), absg, true);

		second = nextNodeInRow(maxx, sy, true);
		if(second)
			addSingleMacroEdge(first, second, map->h(first, second), absg, true);
	}

	// when using perimeter reduction we also need to connect bottom to top
	// in this case we only try to connect neighbours at the edge of the fan
	if(getVerbose())
	{
		std::cout << "adding fan edges between bottom and top"<<std::endl;
		std::cout << "max diagonal steps: "<<max<<std::endl;
	}
	for(int fx=this->getHOrigin();
			fx<this->getHOrigin()+this->getWidth();
			fx++)
	{
		int fy = this->getVOrigin()+this->getHeight()-1;
		node* first = absg->getNode(
				map->getNodeFromMap(fx, fy)->getLabelL(kParent));

		if(!first) continue;

		// nodes in the fan are in the range [minx, maxx] 
		int minx = (fx-max)<this->getHOrigin()?
			(this->getHOrigin()):(fx-max); 
		int maxx = (fx+max)>(this->getHOrigin()+this->getWidth()-1)?
			(this->getHOrigin()+this->getWidth()-1):(fx+max);

		int sy = this->getVOrigin();
		node* second = 0;
		
		// check if nodes exist at fan edges; if not, extend the fan area until we hit something
		second = nextNodeInRow(minx, sy, false);
		if(second)
			addSingleMacroEdge(first, second, map->h(first, second), absg, true);

		second = nextNodeInRow(maxx, sy, true);
		if(second)
			addSingleMacroEdge(first, second, map->h(first, second), absg, true);
	}

	if(getVerbose())
	{
		std::cout << "adding fan edges between left and right"<<std::endl;
	}
	// add edges connecting nodes on the left side to nodes on the right 
	// side of the cluster
	for(int fy=this->getVOrigin();
			fy<this->getVOrigin()+this->getHeight();
			fy++)
	{
		int fx = this->getHOrigin();
		node* first = absg->getNode(
				map->getNodeFromMap(fx, fy)->getLabelL(kParent));

		if(!first) continue;

		int miny = (fy-max)<this->getVOrigin()?
			(this->getVOrigin()):(fy-max); 
		int maxy = (fy+max)>(this->getVOrigin()+this->getHeight()-1)?
			(this->getVOrigin()+this->getHeight()-1):(fy+max);

		int sx = this->getHOrigin()+this->getWidth()-1;
		int sy = miny;
		sy = miny+1;
		node* second = 0;
		for( ; sy <= maxy-1; sy++) 
		{
			second = absg->getNode(
				map->getNodeFromMap(sx, sy)->getLabelL(kParent));

			if(!second) continue;
			addSingleMacroEdge(first, second, map->h(first, second), absg, true);
		}
		
		// check if nodes exist at fan edges; if not, extend the fan area until we hit something
		second = nextNodeInColumn(sx, miny, false);
		if(second)
			addSingleMacroEdge(first, second, map->h(first, second), absg, true);

		second = nextNodeInColumn(sx, maxy, true);
		if(second)
			addSingleMacroEdge(first, second, map->h(first, second), absg, true);
	}

	if(getVerbose())
	{
		std::cout << "adding fan edges between right and left"<<std::endl;
	}
	// add edges connecting nodes on the left side to nodes on the right 
	// side of the cluster
	for(int fy=this->getVOrigin();
			fy<this->getVOrigin()+this->getHeight();
			fy++)
	{
		int fx = this->getHOrigin()+this->getWidth()-1;
		node* first = absg->getNode(
				map->getNodeFromMap(fx, fy)->getLabelL(kParent));

		if(!first) continue;

		int miny = (fy-max)<this->getVOrigin()?
			(this->getVOrigin()):(fy-max); 
		int maxy = (fy+max)>(this->getVOrigin()+this->getHeight()-1)?
			(this->getVOrigin()+this->getHeight()-1):(fy+max);

		int sx = this->getHOrigin();
		node* second = 0;
		
		// check if nodes exist at fan edges; if not, extend the fan area until we hit something
		second = nextNodeInColumn(sx, miny, false);
		if(second)
			addSingleMacroEdge(first, second, map->h(first, second), absg, true);

		second = nextNodeInColumn(sx, maxy, true);
		if(second)
			addSingleMacroEdge(first, second, map->h(first, second), absg, true);
	}
}

// Returns the next parent node in a given row. The value of leftToRight 
// (true or false) determines if the search, which starts at (x, y), runs 
// to the left of that position or the right.
MacroNode*
EmptyCluster::nextNodeInRow(int x, int y, bool leftToRight)
{
	MacroNode* retVal = 0;
	graph* absg = map->getAbstractGraph(1);
	if(leftToRight)
	{
		for( ; x < (this->getHOrigin() + this->getWidth()) ; x++)
		{
			int nodeId = map->getNodeFromMap(x, y)->getLabelL(kParent);
			if(nodeId != -1)
			{
				retVal = static_cast<MacroNode*>(absg->getNode(nodeId));
				break;
			}
		}
	}
	else
	{
		for( ; x >= this->getHOrigin(); x--)
		{
			int nodeId = map->getNodeFromMap(x, y)->getLabelL(kParent);
			if(nodeId != -1)
			{
				retVal = static_cast<MacroNode*>(absg->getNode(nodeId));
				break;
			}
		}
	}

	return retVal;
}

// Returns the next parent node in a given column. The value of topToBottom
// (true or false) determines if the search, which starts at (x, y), runs 
// toward the top of the map or the bottom.
MacroNode*
EmptyCluster::nextNodeInColumn(int x, int y, bool topToBottom)
{
	MacroNode* retVal = 0;
	graph* absg = map->getAbstractGraph(1);

	if(topToBottom)
	{
		for( ; y < (this->getVOrigin() + this->getHeight()); y++)
		{
			int nodeId = map->getNodeFromMap(x, y)->getLabelL(kParent);
			if(nodeId != -1)
			{
				retVal = static_cast<MacroNode*>(absg->getNode(nodeId));
				break;
			}
		}
	}
	else
	{
		for( ; y >= this->getVOrigin(); y--)
		{
			int nodeId = map->getNodeFromMap(x, y)->getLabelL(kParent);
			if(nodeId != -1)
			{
				retVal = static_cast<MacroNode*>(absg->getNode(nodeId));
				break;
			}
		}
	}

	return retVal;
}


edge* 
EmptyCluster::findSecondaryEdge(unsigned int fromId, unsigned int toId)
{
	edge* retVal = 0;
	for(unsigned int i=0; i<secondaryEdges.size(); i++)
	{
		edge* e = secondaryEdges.at(i);
		if((e->getFrom() == fromId && e->getTo() == toId) ||
			(e->getTo() == fromId && e->getFrom() == toId))
		{
			retVal = e;
			break;
		}
	}
	return retVal;
}

void 
EmptyCluster::openGLDraw()
{
	Map* themap = map->getMap();
	GLdouble glx, gly, glz;  
	GLdouble glHeight, glWidth;
	GLdouble xx, yy, zz,rr;

	// calc position of cluster origin in OpenGL coordinate space
	themap->getOpenGLCoord(getHOrigin(), getVOrigin(), xx, yy, zz, rr);
	glx = xx;
	gly = yy;
	glz = zz-rr*0.5;

	// calculate cluster height and width in OpenGL coordinate space
	themap->getOpenGLCoord(getHOrigin()+width-1, getVOrigin(), xx, yy, zz, rr);
	glWidth = xx - glx;
	themap->getOpenGLCoord(getHOrigin(), getVOrigin()+height-1, xx, yy, zz, rr);
	glHeight = yy - gly;

	glx-=rr;
	gly-=rr;
	glHeight+=2*rr;
	glWidth+=2*rr;
	glColor3f(0.2, 0.6, 0.2);
	glLineWidth(3.0f);
	glBegin(GL_LINE_STRIP);
	glVertex3f(glx, gly, glz);
	glVertex3f(glx+glWidth, gly, glz);
	glVertex3f(glx+glWidth, gly+glHeight, glz);
	glVertex3f(glx, gly+glHeight, glz);
	glVertex3f(glx, gly, glz);
	glEnd();
	glLineWidth(1.0f);
}
