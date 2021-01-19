#include "RecursiveJumpPointExpansionPolicy.h"

#include "JumpInfo.h"
#include "JumpPointLocator.h"
#include "fpUtil.h"
#include "Heuristic.h"
#include "mapAbstraction.h"
#include "ProblemInstance.h"

#include <cfloat>
#include <climits>

RecursiveJumpPointExpansionPolicy::RecursiveJumpPointExpansionPolicy(
		JumpPointLocator* _jpl, unsigned int _maxdepth) 
	: ExpansionPolicy(), neighbourIndex(0), jpl(_jpl), MAX_DEPTH(_maxdepth) 

{
}

RecursiveJumpPointExpansionPolicy::~RecursiveJumpPointExpansionPolicy()
{
	for(unsigned int i=0; i < neighbours.size(); i++)
	{
		delete neighbours.at(i);
	}
	neighbours.clear();
	directions.clear();
	delete jpl;
}

void 
RecursiveJumpPointExpansionPolicy::setProblemInstance(ProblemInstance* p)
{
	int graphSize = p->getMap()->getAbstractGraph(0)->getNumNodes();
	directions.resize(graphSize);

	ExpansionPolicy::setProblemInstance(p);
	directions.at(p->getStartNode()->getNum()) = Jump::NONE;
}

void 
RecursiveJumpPointExpansionPolicy::expand(node* t) throw(std::logic_error)
{
	// clear any data from previous expansion operations
	for(unsigned int i=0; i < neighbours.size(); i++)
	{
		delete neighbours.at(i);
	}
	neighbours.clear();
	neighbourIndex = 0;

	// assign a new current node and identify its neighbours
	ExpansionPolicy::expand(t);
	computeNeighbourSet();

}

// Computes the set of jump point neighbours for the target node which is being
// expanded. If the target node has only a single successor, we do not generate
// it. Instead, we immediately jump again in a recursive fashion, each time
// stepping in the direction of the single successor until we identify a jump
// point with a branching factor >= 2. When we find such a jump point, we
// generate it as a successor of the target node.
// 
// NB: it is possible that after some number of recursive steps we eventually
// reach a node that has zero jump point successors. In this case we conclude
// that the target node is sterile -- i.e. it cannot appear on the optimal path
// to the goal.
//
void 
RecursiveJumpPointExpansionPolicy::computeNeighbourSet()
{
	mapAbstraction* map = problem->getMap();
	int x = target->getLabelL(kFirstData);
	int y = target->getLabelL(kFirstData+1);

	// Retrieve the direction of travel used to reach the current node.
	Jump::Direction which = getDirection(this->target); 
	if(verbose)
	{
		std::cout << "computing jump points. lastdir="<<Jump::toString(which)
			<< std::endl;
	}

	// Look for jump point successors in the same direction
	switch(which)
	{
		case Jump::S:
		{
			JumpInfo result;
			findJumpNode(which, result);
			if(result.nodecount() > 0)
				addNeighbour(result);

			// add SE neighbour only if E neighbour is null 
			if(!map->getNodeFromMap(x+1, y))
			{
				which = Jump::SE;
				findJumpNode(which, result); 
				if(result.nodecount() > 0)
					addNeighbour(result);
			}
			// add SW neighbour only if W neighbour is null
			if(!map->getNodeFromMap(x-1, y))
			{
				which = Jump::SW;	
				findJumpNode(which, result); 
				if(result.nodecount() > 0)
					addNeighbour(result);
			}
			break;
		}

		case Jump::SW:
		{
			JumpInfo result;
			findJumpNode(which, result);
			if(result.nodecount() > 0)
				addNeighbour(result);

			which = Jump::S;
			findJumpNode(which, result);
			if(result.nodecount() > 0)
				addNeighbour(result);

			which = Jump::W;
			findJumpNode(which, result);
			if(result.nodecount() > 0)
				addNeighbour(result);

			// add NW neighbour only if N neighbour is null
			if(!map->getNodeFromMap(x, y-1))
			{
				which = Jump::NW;
				findJumpNode(which, result); 
				if(result.nodecount() > 0)
					addNeighbour(result);
			}

			// add SE neighbour only if E neighbour is null
			if(!map->getNodeFromMap(x+1, y))
			{
				which = Jump::SE;
				findJumpNode(which, result);
				if(result.nodecount() > 0)
					addNeighbour(result);
			}
			break;
		}

		case Jump::W:
		{
			JumpInfo result;
			findJumpNode(which, result);
			if(result.nodecount() > 0)
				addNeighbour(result);

		 	// add NW neighbour only if N neighbour is null	
			if(!map->getNodeFromMap(x, y-1))
			{
				which = Jump::NW;
				findJumpNode(which, result);
				if(result.nodecount() > 0)
					addNeighbour(result);
			}
			// add SW neighbour only if S neighbour is null
			if(!map->getNodeFromMap(x, y+1))
			{
				which = Jump::SW;
				findJumpNode(which, result); 
				if(result.nodecount() > 0)
					addNeighbour(result);
			}
			break;
		}

		case Jump::NW:
		{
			JumpInfo result;
			findJumpNode(which, result); 
			if(result.nodecount() > 0)
				addNeighbour(result);

			which = Jump::N;
			findJumpNode(which, result);
			if(result.nodecount() > 0)
				addNeighbour(result);

			which = Jump::W;
			findJumpNode(which, result);
			if(result.nodecount() > 0)
				addNeighbour(result);

			// add SW neighbour only if S neighbour is null
			if(!map->getNodeFromMap(x, y+1))
			{
				which = Jump::SW;
				findJumpNode(which, result); 
				if(result.nodecount() > 0)
					addNeighbour(result);
			}

			// add NE neighbour only if E neighbour is null
			if(!map->getNodeFromMap(x+1, y))
			{
				which = Jump::NE;
				findJumpNode(which, result); 
				if(result.nodecount() > 0)
					addNeighbour(result);
			}
			break;
		}

		case Jump::N:
		{
			JumpInfo result;
			findJumpNode(which, result);
			if(result.nodecount() > 0)
				addNeighbour(result);

			// add NE neighbour only if E neighbour is null 
			if(!map->getNodeFromMap(x+1, y))
			{
				which = Jump::NE;
				findJumpNode(which, result); 
				if(result.nodecount() > 0)
					addNeighbour(result);
			}
			// add NW neighbour only if W neighbour is null
			if(!map->getNodeFromMap(x-1, y))
			{
				which = Jump::NW;
				findJumpNode(which, result);
				if(result.nodecount() > 0)
					addNeighbour(result);
			}
			break;
		}

		case Jump::NE:
		{
			JumpInfo result;
			findJumpNode(which, result); 
			if(result.nodecount() > 0)
				addNeighbour(result);

			which = Jump::N;
			findJumpNode(which, result);
			if(result.nodecount() > 0)
				addNeighbour(result);

			which = Jump::E;
			findJumpNode(which, result);
			if(result.nodecount() > 0)
				addNeighbour(result);

			// add SE neighbour only if S neighbour is null
			if(!map->getNodeFromMap(x, y+1))
			{
				which = Jump::SE;
				findJumpNode(which, result);
				if(result.nodecount() > 0)
					addNeighbour(result);
			}

			// add NW neighbour only if W neighbour is null
			if(!map->getNodeFromMap(x-1, y))
			{
				which = Jump::NW;
				findJumpNode(which, result);
				if(result.nodecount() > 0)
					addNeighbour(result);
			}
			break;
		}

		case Jump::E:
		{
			JumpInfo result;
			findJumpNode(which, result);
			if(result.nodecount() > 0)
				addNeighbour(result);

		 	// add NE neighbour only if N neighbour is null 
			if(!map->getNodeFromMap(x, y-1))
			{
				which = Jump::NE;
				findJumpNode(which, result);
				if(result.nodecount() > 0)
					addNeighbour(result);
			}
			// add SE neighbour only if S neighbour is null
			if(!map->getNodeFromMap(x, y+1))
			{
				which = Jump::SE;
				findJumpNode(which, result);
				if(result.nodecount() > 0)
					addNeighbour(result);
			}
			break;
		}

		case Jump::SE:
		{
			JumpInfo result;
			findJumpNode(which, result);
			if(result.nodecount() > 0)
				addNeighbour(result);

			which = Jump::S;
			findJumpNode(which, result);
			if(result.nodecount() > 0)
				addNeighbour(result);

			which = Jump::E;
			findJumpNode(which, result);
			if(result.nodecount() > 0)
				addNeighbour(result);

			// add NE neighbour only if N neighbour is null
			if(!map->getNodeFromMap(x, y-1))
			{
				which = Jump::NE;
				findJumpNode(which, result); 
				if(result.nodecount() > 0)
					addNeighbour(result);
			}

			// add SW neighbour only if W neighbour is null
			if(!map->getNodeFromMap(x-1, y))
			{
				which = Jump::SW;
				findJumpNode(which, result);
				if(result.nodecount() > 0)
					addNeighbour(result);
			}
			break;
		}
		case Jump::NONE:
		{
			// when a node has no parent (usually only the start node)
			// we generate jump point successors in every traversable direction
			which = Jump::N;
			JumpInfo result;
			findJumpNode(which, result);
			if(result.nodecount() > 0)
				addNeighbour(result);

			which = Jump::S;
			findJumpNode(which, result);
			if(result.nodecount() > 0)
				addNeighbour(result);

			which = Jump::E;
			findJumpNode(which, result);
			if(result.nodecount() > 0)
				addNeighbour(result);

			which = Jump::W;
			findJumpNode(which, result);
			if(result.nodecount() > 0)
				addNeighbour(result);

			which = Jump::NE;
			findJumpNode(which, result);
			if(result.nodecount() > 0)
				addNeighbour(result);

			which = Jump::NW;
			findJumpNode(which, result);
			if(result.nodecount() > 0)
				addNeighbour(result);

			which = Jump::SE;
			findJumpNode(which, result);
			if(result.nodecount() > 0)
				addNeighbour(result);

			which = Jump::SW;
			findJumpNode(which, result);
			if(result.nodecount() > 0)
				addNeighbour(result);
		}
	}
}

node* 
RecursiveJumpPointExpansionPolicy::first()
{
	neighbourIndex = 0;
	return n();
}

node* 
RecursiveJumpPointExpansionPolicy::next()
{
	if(neighbourIndex < neighbours.size())
	{
		neighbourIndex++;
	}
	return n();
}

node* 
RecursiveJumpPointExpansionPolicy::n()
{
	node* n_ = 0;
	if(neighbourIndex < neighbours.size())
	{
		JumpInfo* info = neighbours.at(neighbourIndex);
		n_ = info->getNode(info->nodecount() - 1);
		assert(n_);
	}
	else
	{
		n_ = 0;
	}
	return n_;
}

double 
RecursiveJumpPointExpansionPolicy::cost_to_n()
{
	double retVal = DBL_MAX;
	if(neighbourIndex < neighbours.size())
	{
		JumpInfo* info = neighbours.at(neighbourIndex);
		retVal = info->totalCost();
	}
	return retVal;
}

bool 
RecursiveJumpPointExpansionPolicy::hasNext()
{
	if(neighbourIndex+1 < neighbours.size())
		return true;
	return false;
}

// Find the next jump point reachable from (x, y) in direction d. This 
// operation involves running a depth-limited search with ::target as the root.
// At each step, we identify child nodes by searching for jump points in the 
// direction of each forced and natural neighbour.
// The search follows each branch down to some maximum depth, trying to
// determine if the subtree leads to a dead-end or to a branching node which
// has > 1 jump point successors.
// If we reach a dead-end, the branch is marked sterile as it leads nowhere.
// If we reach a branching node, or hit the depth limit, the branch is kept.
// The chosen successor of (x, y) is the first branching node encountered when
// walking the generated search tree.
//
// TODO: 
// 1. Generating a search tree here is less efficient than searching in
// a graph. For example: a node may appear multiple times with different
// costs. Also, we rely on MAX_DEPTH to break cycles contained in the map.
// 2. Could identify the jump point successor during search instead of walking
// the tree at the end as is currently done.
//
// @param dir: the direction to jump in, relative to ::target
// @param out: output parameter containing a description of the branching jump 
// node successor of ::target (if any such node was found).
//
void
RecursiveJumpPointExpansionPolicy::findJumpNode(
		Jump::Direction dir, JumpInfo& out)
{
	out.clear();
	if(verbose)
	{
		std::cout << "jumping dir=" << Jump::toString(dir);
	}

	// jump in the given direction
	node* from = this->target;
	node* goal = problem->getGoalNode();
	int fromx = from->getLabelL(kFirstData);
	int fromy = from->getLabelL(kFirstData+1);
	int goalx = goal->getLabelL(kFirstData);
	int goaly = goal->getLabelL(kFirstData+1);
	node* succ = jpl->findJumpNode(dir, fromx, fromy, goalx, goaly);

	if(!succ)
	{
		// terminate; no jump point in direction @param dir
		out.clear();
		if(verbose)
		{
			std::cout << " succ: none" << std::endl;
		}
		return;
	}
	else
	{
		// record some details about this latest jump
		out.addJump(succ, dir, problem->getHeuristic()->h(from, succ));
		if(verbose)
		{
			std::cout << " succ: (" << succ->getLabelL(kFirstData) <<", "
				<< succ->getLabelL(kFirstData+1) << ")" << std::endl;
		}
	}

	// determine if succ is a branching node or if it can be jumped over
	JumpInfo* branch = findBranchingNode(succ, dir, 0);

	if(!branch)
	{
		// all nodes after succ lead to dead-ends; don't bother to generate it
		out.clear();
		return;
	}

	while(branch)
	{
		// store the jump points to the last branching node. 
		// NB: start at index 1 to avoid duplicates b/w out and branch
		for(unsigned int i=1; i < branch->nodecount(); i++)
		{
			out.addJump( branch->getNode(i), branch->getDirection(i),
						 branch->getCost(i) );
		}

		// stop jumping if any of the following conditions are met:
		// 	1. maxdepth == 0 (can't go any further)
		// 	2. we found the last branching node before hitting the depth limit
		// 	3. the last branching is the goal node
		int idx_last = branch->nodecount() - 1;
		node* last_node = branch->getNode(idx_last);
		Jump::Direction last_dir = branch->getDirection(idx_last);
		if(		this->MAX_DEPTH == 0 ||
				branch->edgecount() != this->MAX_DEPTH || 
				&*(last_node) == &*(problem->getGoalNode())	)
		{
			break;
		}

		delete branch;
		branch = findBranchingNode(last_node, last_dir, 0);	
	}

	delete branch;
}

// Performs a limited depth-first search for jump points with a branching
// factor > 1. 
//
// NB: If the depth limit is reached, the node at maximum depth is taken to
// be a branching node. Further, the goal node is always considered a
// branching node (we do not want to jump past it. ever.)
//
// @param from: the jump point at the root of the search. 
// @param last_dir: the last direction of travel taken to reach @param from
// @param depth: the maximum depth to recurse to. 
//
// @return a JumpInfo description of any branching nodes found and null if no
// such node could be found.
JumpInfo*
RecursiveJumpPointExpansionPolicy::findBranchingNode(
		node* from, Jump::Direction last_dir, unsigned int depth)
{
	if(!from)
		return 0;

	if(verbose)
	{
		std::cout << "branching (" << from->getLabelL(kFirstData) <<", "
			<< from->getLabelL(kFirstData+1) << ") last_dir: " << Jump::toString(last_dir) 
			<< " depth: "<< depth << std::endl;
	}

	// @param from is designated a branching node if we reach
	// maximum recursion depth
	if(depth >= this->MAX_DEPTH)
	{
		JumpInfo* branch = new JumpInfo();
		branch->addJump(from, last_dir, 0);
		return branch;
	}
	
	// @param from is designated a branching node if its identity is the goal
	if(&*from == &*(problem->getGoalNode()))
	{
		JumpInfo* branch = new JumpInfo();
		branch->addJump(from, last_dir, 0);
		return branch;
	}

	int fromx = from->getLabelL(kFirstData);
	int fromy = from->getLabelL(kFirstData+1);
	int goalx = problem->getGoalNode()->getLabelL(kFirstData);
	int goaly = problem->getGoalNode()->getLabelL(kFirstData+1);

	std::vector<JumpInfo*> branches;
	int nei_dirs = jpl->computeSuccessors(last_dir, fromx, fromy);
	for(int nextdir = 1; nextdir < 256; nextdir = nextdir << 1)
	{
		if(branches.size() > 1)
			break; // early termination: branching node identified

		// recurse only in the direction of forced and natural neighbours
		if(!(nextdir & nei_dirs)) 
			continue;

		node* succ = jpl->findJumpNode((Jump::Direction)nextdir, 
				fromx, fromy, goalx, goaly);
		if(succ)
		{
			JumpInfo* branch = findBranchingNode(succ, 
					(Jump::Direction)nextdir, depth+1);
			if(branch)
			{
				// don't forget the jumpcost to reach succ
				branch->setCost(0, problem->getHeuristic()->h(from, succ));
				branches.push_back(branch);
			}
		}
	}

	JumpInfo* retVal = 0;
	switch(branches.size())
	{
		case 0:
			break;

		// @param from is an intermediate node on the way to a branching node
		case 1:
		{
			retVal = new JumpInfo();
			retVal->addJump(from, last_dir, 0);

			// append all the jumps we took while recursing
			JumpInfo* branch = branches.back();
			for(unsigned int i=0; i < branch->nodecount(); i++)
			{
				retVal->addJump(
						branch->getNode(i), 
						branch->getDirection(i), 
						branch->getCost(i)	);
			}
			break;
		}

		// @param from is a braching node 
		default:
			retVal = new JumpInfo();
			retVal->addJump(from, last_dir, 0);
			break;
	}
	
	// cleanup explored branches
	while(branches.size() > 0)
	{
		delete branches.back();
		branches.pop_back();
	}
	return retVal;
}

// Updates the direction and parent labels the current neighbour 
// (as returned by ::n()). This operation is called by an external search 
// algorithm when a new best path to node n is found with parent(n) = ::target.
// NB: Direction tracking is necessary to support recursive jumping.
void
RecursiveJumpPointExpansionPolicy::label_n()
{
	assert(neighbourIndex < neighbours.size());
	JumpInfo* info = neighbours.at(neighbourIndex);
	node* n_ = info->getNode(info->nodecount() - 1);
	Jump::Direction last_dir = info->getDirection(info->nodecount() - 1);

	directions.at(n_->getNum()) = last_dir;
	n_->backpointer = this->target;
}

// Returns the travel direction used to reach a given node n. If
// the node has not previously been encountered (e.g. the start node)
// Jump::NONE is is returned.
//
// NB: Unlike non-recursive jumping, there is no guarantee that p(n) can
// be reached by travelling in the opposite direction to the return value.
Jump::Direction 
RecursiveJumpPointExpansionPolicy::getDirection(node* n_)
{
	assert(n_->getNum() < directions.size());
	return directions.at(n_->getNum());
}

void
RecursiveJumpPointExpansionPolicy::addNeighbour(JumpInfo& out)
{
	// NB: no need to track best parent; let the search do that and change
	// direction labels only on ::label_n()
	JumpInfo* myparams = new JumpInfo(out);
	neighbours.push_back(myparams);
}

// get the JumpInfo entry for the node being returned by ::n()
JumpInfo* 
RecursiveJumpPointExpansionPolicy::getJumpInfo()
{
	if(neighbourIndex < neighbours.size())
		return neighbours.at(neighbourIndex);
	return 0;
}
