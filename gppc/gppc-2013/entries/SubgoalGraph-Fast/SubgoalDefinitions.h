#ifndef SUBGOAL_DEFINITIONS_H
#define SUBGOAL_DEFINITIONS_H

#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <limits>

#define USE_SUBGOALS
#define USE_STACK
//#define USE_BOOL
#define USE_CLEARANCES
#define REPLACE_POPPED
#define PRUNE_GRAPH
//#define KEEP_LOCAL_EDGES	// Only partially implemented. Does not yet work with pairwise distances

//#define SUBGOAL_LIMIT	// 16bit = 65536 subgoals at most. If we have more subgoals, default to using buckets
#define PAIRWISE_DISTANCES

// Memory limit in bytes, time limit in seconds (time limit not implemented yet, doesn't seem to be necessary)
#define MEMORY_LIMIT 50000000
#define TIME_LIMIT 1800

//#define SG_STATISTICS
//#ifdef SG_STATISTICS_PER_SEARCH
//#define TIMER_USE_CYCLE_COUNTER

//#define CARD_COST 70
//#define DIAG_COST 99
//#define DIAG_DIFF 29
#define CARD_COST 2378
#define DIAG_COST 3363
#define DIAG_DIFF 985
typedef uint32_t cost;
#define INFINITE_COST std::numeric_limits<cost>::max()

typedef uint32_t mapLoc;
typedef uint32_t direction;

#ifdef SUBGOAL_LIMIT
	typedef uint16_t subgoalId;
	#define CLEARANCE_MASK 0x0000000F
	#define CLEARANCE_LIMIT 15
	static int clearanceShift[24] = {0, 0, 4, 0, 8, 0, 12, 0, 0, 0, 4, 0, 8, 0, 12, 0, 0, 0, 4, 0, 8, 0, 12, 0};
#else
	typedef uint32_t subgoalId;
	#define CLEARANCE_MASK 0x000000FF
	#define CLEARANCE_LIMIT 255
	static int clearanceShift[24] = {0, 0, 8, 0, 16, 0, 24, 0, 0, 0, 8, 0, 16, 0, 24, 0, 0, 0, 8, 0, 16, 0, 24, 0};
#endif

//enum direction {N=0, NE=1, E=2, SE=3, S=4, SW=5, W=6, NW=7};

#define MAX_SEARCH 50000
//#define MAX_SEARCH std::numeric_limits<uint16_t>::max()

//#ifdef CLEARANCE_7_BIT
//#define CLEARANCE_MASK 0x0000007F
//#define CLEARANCE_LIMIT 127
//static int clearanceShift[24] = {0, 0, 7, 0, 14, 0, 21, 0, 0, 0, 7, 0, 14, 0, 21, 0, 0, 0, 7, 0, 14, 0, 21, 0};

//#ifdef CLEARANCE_8_BIT

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
