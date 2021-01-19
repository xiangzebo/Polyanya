#ifndef SUBGOAL_DEFINITIONS_H
#define SUBGOAL_DEFINITIONS_H

#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <limits>

#ifndef NO_HOG
	#ifndef SG_RUNNING_IN_HOG
		#define SG_RUNNING_IN_HOG
	#endif
#endif

#define USE_SUBGOALS
#define USE_STACK
#define USE_CLEARANCES
#define REPLACE_POPPED
#define PRUNE_GRAPH

#define EXTRA_EDGES						// Allows the addition of more edges during pruning, to prune more nodes at a time

#define MAX_LEVEL 10000				// If defined, (MAX_LEVEL - 1) overrides the parameter for the number of prunings2

// The following does not work with save/load
//#define SUCCESSOR_PRUNING				// When expanding a state during search, ignore the successors if their direction would
										// result in a suboptimal path (taking into account the parent and surrounding obstacles)

#ifdef SUCCESSOR_PRUNING
//#define PRUNE_WHILE_CONNECTING			// Same idea as above, but also do this procedure when going back from the goal and adding necessary edges (requires SUCCESSOR_PRUNING)
#endif

//#define EARLY_TERMINATE					// When adding edges to the graph, store the costs of the shortest paths to the goal.
										// When a node whose exact distance-to-go is expanded, note the cost of the shortest path to the goal that goes through this node
										// Never add/update a node with a current f-value higher than the minimum known solution cost
										// Terminate search when the minimum f-value in OPEN is no-larger than the minimum known solution cost

//#define DISCARD_FIRST_LEVEL_LOCAL_EDGES	// Usually, most of the edges are between first level subgoals. Discard those to gain speed/memory while losing optimality guarantee
										// After search, see if we can reconstruct some of these edges to find a path shorter than the one found by the search
										// If no path is found, reconstruct all the relevant first level local edges, until a solution is found, or determine that there is no path


//#define SG_STATISTICS
//#define SG_STATISTICS_PER_SEARCH
//#define SG_ASSERTIONS
//#define SG_VERBOSE

#ifdef SG_RUNNING_IN_HOG
	//#define DISPLAY_EXTRA_EDGES
#endif

//#define CARD_COST 70
//#define DIAG_COST 99
//#define DIAG_DIFF 29
#define CARD_COST 2378
#define DIAG_COST 3363
#define DIAG_DIFF 985
typedef uint32_t cost;
#define INFINITE_COST std::numeric_limits<cost>::max()
#define INVALID_XYLOC xyLoc(10000,10000)

typedef uint32_t mapLoc;
typedef uint32_t direction;
typedef uint16_t level;

typedef uint8_t edgeDirection;
typedef uint8_t neighborhoodInfo;

typedef uint32_t subgoalId;

#define CLEARANCE_MASK 0x000000FF
#define CLEARANCE_LIMIT 255
static int clearanceShift[24] = {0, 0, 8, 0, 16, 0, 24, 0, 0, 0, 8, 0, 16, 0, 24, 0, 0, 0, 8, 0, 16, 0, 24, 0};

//enum direction {N=0, NE=1, E=2, SE=3, S=4, SW=5, W=6, NW=7};

#define MAX_SEARCH 50000
//#define MAX_SEARCH std::numeric_limits<uint16_t>::max()

#ifndef SG_RUNNING_IN_HOG
struct xyLoc {
public:
	xyLoc() {}
	xyLoc(uint16_t _x, uint16_t _y) :x(_x), y(_y) {}
	uint16_t x;
	uint16_t y;
};
#endif

struct heapElement
{
	subgoalId sg;
	cost fVal;
	heapElement(subgoalId _sg, cost _fVal) : sg(_sg), fVal(_fVal) {}
};
struct heapElementComp
{
    bool operator()(heapElement const& a, heapElement const& b) const {
        return a.fVal > b.fVal;
    	//return b.fVal < a.fVal?true:((b.fVal == a.fVal) && (gCost[b.sg] > gCost[a.sg]));
    }
};

// Memory Calculation Macros
#define BITS_PER_CELL (2 + (sizeof(subgoalId)*8))
// (1 + 1 + subgoalId)
// traversable, subgoal, cellInfo

#define BITS_PER_SUBGOAL_STORED 49
// (32 + 16 + 1) 
// location + nNeighbor + pruned

#define BITS_PER_SUBGOAL_ONLINE 146
// (32 + 32 + 16 + 1 + 32 + 1 + 32)
// gCost + parent + search counter + open + pruned +  extraEdge + hasExtraEdge + neighborPtr

#define BITS_PER_EDGE (sizeof(subgoalId)*8)

#endif
