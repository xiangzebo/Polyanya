#include "JumpPointAbstraction.h"

#include "IEdgeFactory.h"
#include "INodeFactory.h"
#include "OnlineJumpPointLocator.h"
#include "graph.h"
#include "map.h"
#include "OctileHeuristic.h"

#include <cstdlib>
#include <climits>
#include <iostream>
#include <fstream>

JumpPointAbstraction::JumpPointAbstraction(Map* _m, INodeFactory* _nf, 
		IEdgeFactory* _ef, bool _diags, bool _corners, bool _verbose) 
	: mapAbstraction(_m, _diags, _corners), verbose(_verbose)
{
	nf = _nf;
	ef = _ef;

	makeJumpPointGraph();
	//verifyHierarchy();
}

JumpPointAbstraction::JumpPointAbstraction(Map* _m, const char* graphfile, 
		INodeFactory* _nf, IEdgeFactory* _ef, bool _diags, bool _corners, bool _verbose) 
	: mapAbstraction(_m, _diags, _corners), verbose(_verbose)
{
	nf = _nf;
	ef = _ef;

	importGraph(graphfile);
	//verifyHierarchy();
}

JumpPointAbstraction::~JumpPointAbstraction()
{
	delete nf;
	delete ef;
}

mapAbstraction*
JumpPointAbstraction::clone(Map* _m)
{
	return new JumpPointAbstraction(_m, nf->clone(), ef->clone());
}

bool 
JumpPointAbstraction::pathable(node* n, node* m)
{
	return true;
}

void
JumpPointAbstraction::verifyHierarchy()
{
	graph* g = getAbstractGraph(0); 

	for(int i=0; i<g->getNumNodes(); i++)
	{
		node* p = g->getNode(i);

		if(p->getNumOutgoingEdges() != 0)
		{
			repairAbstraction();			
			break;
		}

		if(p->getNumIncomingEdges() != 0)
		{
			repairAbstraction();
			break;
		}
	}
}

void
JumpPointAbstraction::removeNode(node* n)
{
	graph* g = getAbstractGraph(0); 

	edge_iterator it = n->getEdgeIter();
	for(edge* e = n->edgeIterNext(it); e != 0; e = n->edgeIterNext(it))
	{
		g->removeEdge(e);
		delete e;
	}
	g->removeNode(n);
	delete n;
}

void 
JumpPointAbstraction::removeEdge(edge *e, unsigned int absLevel)
{
	if(absLevel != 0)
		return;

	graph* g = getAbstractGraph(0);
	g->removeEdge(e);
	delete e;
}

void 
JumpPointAbstraction::addNode(node *n)
{
	graph* g = getAbstractGraph(0);
	g->addNode(n);
}

void 
JumpPointAbstraction::addEdge(edge *e, unsigned int absLevel)
{
	if(absLevel != 0)
		return;

	graph* g = getAbstractGraph(1); 
	g->addDirectedEdge(e);
}

void 
JumpPointAbstraction::repairAbstraction()
{
	if(verbose)
		std::cout << "repairAbstraction"<<std::endl;

	if(getNumAbstractGraphs() > 1)
		return;

	graph* g = getAbstractGraph(0);
	abstractions.pop_back();
	delete g;

	makeJumpPointGraph();
}

// Finds jump point successors for each neighbour in the grid.
// Searches in the direction of each traversable neighbour.
// If there is no jump point successor in a particular direction,
// the array of neighbours is padded with an edge to the current node itself.
void
JumpPointAbstraction::makeJumpPointGraph()
{
	// compute the set of nodes in the graph
	graph* g = makeMapNodes(this->getMap(), nf);
	abstractions.push_back(g);
	OnlineJumpPointLocator jpl(this); 

	// compute jump point neighbours of each node 
	OctileHeuristic heuristic;
	for(int i=0; i < g->getNumNodes(); i++)
	{
		node* n = g->getNode(i);
		for(int j = 1; j<9; j++)
		{
			// connect n to a jump point in every direction
			int nx = n->getLabelL(kFirstData);
			int ny = n->getLabelL(kFirstData+1);

			node* neighbour = 0;
			switch(j)
			{
				case 1:
					neighbour = jpl.findJumpNode(Jump::N, nx, ny, -1, -1);
					break;
				case 2:
					neighbour = jpl.findJumpNode(Jump::NE, nx, ny, -1, -1);
					break;
				case 3:
					neighbour = jpl.findJumpNode(Jump::E, nx, ny, -1, -1);
					break;
				case 4:
					neighbour = jpl.findJumpNode(Jump::SE, nx, ny, -1, -1);
					break;
				case 5:
					neighbour = jpl.findJumpNode(Jump::S, nx, ny, -1, -1);
					break;
				case 6:
					neighbour = jpl.findJumpNode(Jump::SW, nx, ny, -1, -1);
					break;
				case 7:
					neighbour = jpl.findJumpNode(Jump::W, nx, ny, -1, -1);
					break;
				case 8:
					neighbour = jpl.findJumpNode(Jump::NW, nx, ny, -1, -1);
					break;
			}

			edge* e = 0;
			if(neighbour) 
			{
				e = new edge(n->getNum(), neighbour->getNum(),
						heuristic.h(n, neighbour));
			}
			else
			{
				// pad out the array when a neighbour doesn't exist
				// (so we can achieve constant-time lookup of any neighbour) 
				e = new edge(n->getNum(), n->getNum(), 0); 
			}

			g->addDirectedEdge(e);
		}
	}
}

void
JumpPointAbstraction::importGraph(const char* filename)
{
	// compute the set of nodes in the graph
	graph* g = makeMapNodes(this->getMap(), nf);
	abstractions.push_back(g);

	std::ifstream fin(filename);
	std::string nextline;
	int line_num=1;

	getline(fin, nextline);
	int begin = nextline.find(std::string("nodes="));
	int end = nextline.find(std::string(" "), begin);
	int intVal = atoi(nextline.substr(begin+6, end).c_str());
	if(intVal==0 || intVal == INT_MAX || intVal == INT_MIN)
	{
		std::cout << "Error importing graph: broken header on line."
			<< line_num << std::endl;
		exit(1);
	}

	line_num++;
	nextline.clear();
	getline(fin, nextline);
	while(fin.good())
	{
		begin = 0;
		end = nextline.find("[")-1;

		if(end <= begin)
		{
			// line doesn't begin with a node identifier
			std::cout << "Error importing graph: cannot find nodeid "
				"on line " << line_num << std::endl;
			exit(1);
		}

		// read nodeId
		intVal = atoi(nextline.substr(begin, end-begin).c_str());
		if((intVal == 0 && nextline.at(begin) != '0') || intVal == INT_MAX || intVal == INT_MIN)
		{
			// record doesn't have an integral node identifier 
			std::cout << "Error importing graph: invalid nodeid "
				"on line " << line_num << std::endl;
			exit(1);
		}
		int nodeId = intVal;

		// read associated edge records
		while(1)
		{
			// identify the range of the substring containing the edge record
			end = nextline.find("]", begin);
			begin = nextline.find("[", begin);

			if(begin ==-1 && end == -1)
			{
				// node has no (more) outgoing edges
				break;
			}

			if(begin==-1 || end == -1)
			{
				// invalid edge record
				std::cout << "Error importing graph: invalid edge record on"
					" line " << line_num << std::endl;
				begin = nextline.find(" ", begin+1);
				exit(1);
			}

			// adjust begin offset to exclude delimiter char '[' 
			begin++;

			// read neighbour id
			int commapos = nextline.find(",", begin);
			intVal = atoi(nextline.substr(begin, (commapos-begin)).c_str());
			if((intVal==0 && nextline.at(begin) != '0')  // do not fail if id is actually 0
					|| intVal == INT_MAX || intVal == INT_MIN)
			{
				std::cout << "Error importing graph: cannot read neighbourId on "
					"line " << line_num << ": val=" 
					<< nextline.substr(begin, (commapos-begin)).c_str() << std::endl;
				exit(1);
			}
			int neighbourId = intVal;
			begin = commapos+1;
			assert(begin < end);

			// read edge cost
			double doubleVal = atof(nextline.substr(begin, end-begin).c_str());
			if((doubleVal == 0 && nextline.at(begin) != '0') || doubleVal == HUGE_VAL)
			{
				std::cout << "Error importing graph: cannot read edge cost on "
					" line " << line_num << ": neighbourId=" << neighbourId << " val="
				   	<< nextline.substr(begin, end-begin).c_str() << std::endl;
				exit(1);
			}

			edge* e = new edge(nodeId, neighbourId, doubleVal);
			int numEdges = g->getNumEdges();
			g->addDirectedEdge(e);
			assert(g->getNumEdges() > numEdges);

			begin = end+1; // place begin after end-of-record delimiter '['
		}
		line_num++;
		nextline.clear();
		getline(fin, nextline);
	}
	fin.close();
}

