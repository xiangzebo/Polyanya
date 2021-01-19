#include "HPACluster.h"

#include "ClusterAStar.h"
#include "ClusterNode.h"
#include "graph.h"
#include "graphAbstraction.h"
#include "HPAClusterAbstraction.h"
#include "HPAUtil.h"
#include "IEdgeFactory.h"
#include "map.h"
#include <stdexcept>
#include <sstream>
#include <string>

#include "glUtil.h"

//#ifdef NO_OPENGL
//#include "glut.h"
//#include "gl.h"
//#else
//#ifdef OS_MAC
//#include "GLUT/glut.h"
//#include <OpenGL/gl.h>
//#else
//#include <GL/glut.h>
//#include <GL/gl.h>
//#endif
//#endif

HPACluster::HPACluster(const int _x, const int _y, const int _w, const int _h, 
		AbstractClusterAStar* _alg, HPAClusterAbstraction* map) 
	throw(std::invalid_argument) : AbstractCluster(map)
{
	init(_x, _y, _w, _h, _alg);

	if(!dynamic_cast<HPAClusterAbstraction*>(map))
		throw std::invalid_argument("HPACluster requires an abstraction of"
				"type HPAClusterAbstraction");
}

void 
HPACluster::init(const int _x, const int _y, const int _width, 
		const int _height, AbstractClusterAStar* _alg) 
	throw(std::invalid_argument)
{
	if(_x < 0 || _y < 0)
		throw std::invalid_argument("AbstractCluster::AbstractCluster: "
				"cluster (x,y) coordinates must be >= 0");

	if(_width <= 0 || _height <= 0)
		throw std::invalid_argument("HPACluster::HPACluster: cluster height "
				"and width cannot be <= 0");
	if(_alg == NULL)
		throw std::invalid_argument("HPACluster::HPACluster: search algorithm" 
				"parameter cannot be null");

	startx = _x;
	starty = _y;
	width = _width;
	height = _height;
	alg = _alg;
	verbose = false;
}

HPACluster::~HPACluster()
{
	delete alg;
}

// add all traversable nodes in the cluster area to the cluster 
void 
HPACluster::buildCluster() 
{
	for(int x=this->getHOrigin(); x<getHOrigin()+getWidth(); x++)
		for(int y=this->getVOrigin(); y<getVOrigin()+getHeight(); y++)
		{	
			node* n = map->getNodeFromMap(x,y);
			if(n)
				addNode(map->getNodeFromMap(x,y));
		}
}

// Performs a few extra checks before assigning a node to the cluster.
// First: the node must be within the bounds of the cluster
// Second: the node cannot be assigned to another cluster
void 
HPACluster::addNode(node* _mynode) throw(std::invalid_argument)
{
	ClusterNode* mynode = dynamic_cast<ClusterNode*>(_mynode);
	if(mynode == NULL)
		throw std::invalid_argument(
				"HPACluster::addNode arg is not of type ClusterNode");

	if(nodes[mynode->getUniqueID()] != NULL) // already added
		return;

	int nx = mynode->getLabelL(kFirstData);
	int ny = mynode->getLabelL(kFirstData+1);
	
	if(nx < this->getHOrigin() || nx >= this->getHOrigin() + this->getWidth())
	{
		std::ostringstream oss;
		oss << "HPACluster::addNode arg outside cluster boundary."<<std::endl;
		mynode->print(oss);
		oss << std::endl;
		this->print(oss);
		oss << std::endl;
		throw std::invalid_argument(oss.str().c_str());
	}

	if(ny < this->getVOrigin() || ny >= this->getVOrigin() + this->getHeight())
	{
		std::ostringstream oss;
		oss << "HPACluster::addNode arg outside cluster boundary."<<std::endl;
		mynode->print(oss);
		oss << std::endl;
		this->print(oss);
		oss << std::endl;
		throw std::invalid_argument(oss.str().c_str());
	}
	
	if(mynode->getParentClusterId() != -1)
	{
		std::ostringstream oss;
		oss << "HPACluster::addNode arg already assigned "
			"to cluster "<<mynode->getParentClusterId()<<std::endl;
		mynode->print(oss);
		oss << std::endl;
		print(oss);
		oss << std::endl;
		throw std::invalid_argument(oss.str().c_str());
	}
			
	AbstractCluster::addNode(mynode);
}

// Connects a new parent node with every other other parent node in the cluster by using A*. 
// Each successful search results in a new edge being added to the abstract graph.
void 
HPACluster::connectParent(node* absStart) throw(std::invalid_argument)
{	
	if(absStart == 0)
		throw std::invalid_argument("HPACluster::connectParent null argument");

	nodesExpanded = nodesGenerated = nodesTouched = 0;
	for(HPAUtil::nodeTable::iterator nodeIter = parents.begin(); 
		nodeIter != parents.end(); 
		nodeIter++)
	{
		node* absGoal = (*nodeIter).second;
		graph* absg = map->getAbstractGraph(1);
		
		alg->markForVis = false;
		alg->setCorridorNodes(&nodes);

		// get low-level nodes
		node* from = map->getNodeFromMap(
				absStart->getLabelL(kFirstData),
				absStart->getLabelL(kFirstData+1)); 
		node* to = map->getNodeFromMap(
				absGoal->getLabelL(kFirstData),
				absGoal->getLabelL(kFirstData+1)); 

		path* solution = alg->getPath(map, from, to);
		if(solution != 0)
		{
			double dist = to->getLabelF(kTemporaryLabel); // == g(to)
			edge* e = new edge(absStart->getNum(), absGoal->getNum(), dist);
			absg->addEdge(e);
			map->addPathToCache(e, solution);				
		}

		// record some metrics about the operation
		// NB: searchTime is measured by AbstractCluster::addParent (which calls
		// this function)
		nodesExpanded += alg->getNodesExpanded();
		nodesGenerated += alg->getNodesGenerated();
		nodesTouched += alg->getNodesTouched();
	}
}

void 
HPACluster::buildEntrances() 
{
	if(getVerbose())
	{
		std::cout << "buildEntrances; ";
		print(std::cout);
		std::cout << std::endl;
	}

	buildHorizontalEntrances();
	buildVerticalEntrances();
//	if(map->getAllowDiagonals())
//		buildDiagonalEntrances();
}

// Each cluster only considers veritcal entrances along the length of its eastern border. 
// A naive method would duplicate the creation of some entrances 
void 
HPACluster::buildVerticalEntrances()
{	
	int mapwidth = map->getMap()->getMapWidth();
	int x = this->getHOrigin()+this->getWidth();
	if(x == mapwidth)
		return; 

	// scan for vertical entrances along the eastern border 
	int y = this->getVOrigin();
	while(y < this->getVOrigin()+this->getHeight())
	{
		int length = findVerticalEntranceLength(x,y);
	
		// build transition points; long entrances have 2, short entrances have 1.
		if(length == 0)
			y++;
		else
		{
			processVerticalEntrance(x, y, length);
			y += length;
		}
	}
}

void 
HPACluster::processVerticalEntrance(int x, int y, int length)
{
	const double entrance_weight = 1.0;
	if(length >= HPAUtil::MAX_SINGLE_TRANSITION_ENTRANCE_SIZE) 
	{
		// place one transition point at each end of the entrance area
		// NB: (x,y) is inside eastern neighbour
		node* endpoint1 = map->getNodeFromMap(x, y); 
		node* endpoint2 = map->getNodeFromMap(x-1, y);
		this->addTransition(endpoint1, endpoint2, entrance_weight);

		endpoint1 = map->getNodeFromMap(x, y+length-1); 
		endpoint2 = map->getNodeFromMap(x-1, y+length-1);
		this->addTransition(endpoint1, endpoint2, entrance_weight);			
	}
	else
	{
		// place a transition point in the middle of the entrance area 
		int ty = y + (length/2);
		int tx = x;
		node* endpoint1 = map->getNodeFromMap(tx, ty); 
		node* endpoint2 = map->getNodeFromMap(tx-1, ty);
		this->addTransition(endpoint1, endpoint2, entrance_weight);
	}
}

int 
HPACluster::findVerticalEntranceLength(int x, int y)
{
	int length = 0;
	while(y < this->getVOrigin()+this->getHeight())
	{
		if(map->getNodeFromMap(x, y) == NULL || map->getNodeFromMap(x-1, y) == NULL)
			break;
		y++;
		length++;
	}
	
	return length;
}

void 
HPACluster::buildHorizontalEntrances()
{
	int mapheight = map->getMap()->getMapHeight();
	int y = this->getVOrigin()+this->getHeight();
	if(y == mapheight)
		return; 

	// scan for vertical entrances along the eastern border 
	int x = this->getHOrigin();
	while(x < this->getHOrigin()+this->getWidth())
	{
		int length = findHorizontalEntranceLength(x,y);
	
		// build transition points; long entrances have 2, others have 1.
		if(length == 0)
			x++;
		else
		{
			processHorizontalEntrance(x, y, length);
			x += length;
		}
	}
}

void 
HPACluster::processHorizontalEntrance(int x, int y, int length)
{
	const double entrance_weight = 1.0;
	if(length >= HPAUtil::MAX_SINGLE_TRANSITION_ENTRANCE_SIZE) 
	{
		// place one transition point at each end of the entrance area
		// NB: (x,y) is inside eastern neighbour
		node* endpoint1 = map->getNodeFromMap(x, y); 
		node* endpoint2 = map->getNodeFromMap(x, y-1);
		this->addTransition(endpoint1, endpoint2, entrance_weight);

		endpoint1 = map->getNodeFromMap(x+length-1, y); 
		endpoint2 = map->getNodeFromMap(x+length-1, y-1);
		this->addTransition(endpoint1, endpoint2, entrance_weight);			
	}
	else
	{
		// place a transition point in the middle of the entrance area 
		int tx = x + (length/2);
		int ty = y;
		node* endpoint1 = map->getNodeFromMap(tx, ty); 
		node* endpoint2 = map->getNodeFromMap(tx, ty-1);
		this->addTransition(endpoint1, endpoint2, entrance_weight);
	}
}

int 
HPACluster::findHorizontalEntranceLength(int x, int y)
{
	int length = 0;
	while(x < this->getHOrigin()+this->getWidth())
	{
		if(map->getNodeFromMap(x, y) == NULL || map->getNodeFromMap(x, y-1) == NULL)
			break;
		x++;
		length++;
	}
	
	return length;
}

// look for entrances between diagonally adjacent clusters
void 
HPACluster::buildDiagonalEntrances()
{
	if(getVerbose())
		std::cout << "buildDiagonalEntrances"<<std::endl;

	int y = this->getVOrigin();
	int x = this->getHOrigin();
	const double entrance_weight = 1.0;

	// look for diagonal entrances in the top-left corner of the cluster
	node* endpoint1 = map->getNodeFromMap(x, y);
	node* endpoint2 = map->getNodeFromMap(x-1, y-1);
	if(endpoint1 && endpoint2)
	{
		addTransition(endpoint1, endpoint2, entrance_weight);
	}

	// look for diagonal entrances in the top-right corner of the cluster
	x = this->getHOrigin() + this->getWidth()-1;
	endpoint1 = map->getNodeFromMap(x, y);
	endpoint2 = map->getNodeFromMap(x+1, y-1);
	if(endpoint1 && endpoint2)
	{
		addTransition(endpoint1, endpoint2, entrance_weight);
	}

	// look for diagonal entrances in the bottom-right corner of the cluster
	y = this->getVOrigin() + this->getHeight() - 1;
	endpoint1 = map->getNodeFromMap(x,y);
	endpoint2 = map->getNodeFromMap(x+1, y+1);
	if(endpoint1 && endpoint2)
	{
		addTransition(endpoint1, endpoint2, entrance_weight);
	}

	// look for diagonal entrances in the bottom-left corner of the cluster
	x = this->getHOrigin();
	endpoint1 = map->getNodeFromMap(x,y);
	endpoint2 = map->getNodeFromMap(x-1, y+1);
	if(endpoint1 && endpoint2)
	{
		addTransition(endpoint1, endpoint2, entrance_weight);
	}
}

void
HPACluster::print(std::ostream& out)
{
	AbstractCluster::print(out);
	out << std::endl;
	out << "Origin: ("<<getHOrigin()<<", "<<getVOrigin()<<") ";
	out << "height: "<<getHeight()<<" width: "<<getWidth();
}


void 
HPACluster::openGLDraw()
{
	if(parents.size() == 0)
		return;

	Map* themap = map->getMap();
	GLdouble xx, yy, zz,rr;
	glLineWidth(3.0f);
	glColor3f(0.2, 0.6, 0.2);

	glBegin(GL_LINE_STRIP);

	themap->getOpenGLCoord(0, 0, xx, yy, zz, rr);
	double offset = xx;
	themap->getOpenGLCoord(1, 0, xx, yy, zz, rr);
	offset = (xx-offset)*0.5;


	themap->getOpenGLCoord(this->getHOrigin(), this->getVOrigin(), xx, yy, zz, rr);
	glVertex3f(xx-offset, yy-offset, zz-rr*0.5);

	themap->getOpenGLCoord(this->getHOrigin()+width-1, this->getVOrigin(), 
			xx, yy, zz, rr);
	glVertex3f(xx+offset, yy-offset, zz-rr*0.5);

	themap->getOpenGLCoord(this->getHOrigin()+width-1, 
			this->getVOrigin()+height-1, xx, yy, zz, rr);
	glVertex3f(xx+offset, yy+offset, zz-rr*0.5);

	themap->getOpenGLCoord(this->getHOrigin(), this->getVOrigin()+height-1, 
			xx, yy, zz, rr);
	glVertex3f(xx-offset, yy+offset, zz-rr*0.5);

	themap->getOpenGLCoord(this->getHOrigin(), this->getVOrigin(), xx, yy, zz, rr);
	glVertex3f(xx-offset, yy-offset, zz-rr*0.5);

	glEnd();

	glLineWidth(1.0f);
}

