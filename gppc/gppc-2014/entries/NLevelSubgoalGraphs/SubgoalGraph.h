#ifndef SUBGOAL_GRAPH_H
#define SUBGOAL_GRAPH_H

#include "SubgoalDefinitions.h"

#include <deque>
#include <queue>
#include <vector>
//#include <ext/hash_map>
#include "Timer.h"
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <assert.h>
//#include <stdio.h>
//#include <stdlib.h>

#ifdef SG_RUNNING_IN_HOG
#include "Map2DEnvironment.h"
#include "FPUtil.h"
#endif


static std::vector<subgoalId> defaultAbstractPath;	// For default parameters
static std::vector<xyLoc> defaultXYPath;
static std::vector<mapLoc> defaultPath;	

class SubgoalGraph
{
public:

	SubgoalGraph(std::vector<bool> &bits, int width, int height, int nPrunings = 1000010000);
	void LoadMap(std::vector<bool> &bits, int width, int height);
	SubgoalGraph(const char *filename);	// Read a saved graph from the file
	~SubgoalGraph();


#ifdef SG_RUNNING_IN_HOG
	SubgoalGraph(Map* map, int nPrunings = 1);
	void LoadMap(Map* map);
	void OpenGLDraw(const MapEnvironment *env);
#endif
	
	void InitializeValues();
	void CreateGraph(int nPrunings = 1);
	
	/// Main preprocessing functions
	void SetDirections();		// Initialize the deltaMapLoc array (after we know the map dimensions)
	void IdentifySubgoals();	// Place a subgoal at every corner
	void ComputeClearances();	// Compute clearances (from an obstacle or subgoal) for the 4 cardinal directions 
	void LinkSubgoals();		// Add edges so that every subgoal is optimally reachable from each other in the graph
	bool PruneSubgoals();		// Prune subgoals that are unnecessary for global travel (make them local subgoals)
	void FinalizeGraph();		// Now that the number of edges are fixed, move them from vector to array to save memory
	void MemoryAnalysis();	// Decide if we have extra space for pairwise distances
															// (or if we even have space for the actual subgoals)
															// OUTDATED
	void SaveGraph(const char *filename);	// Save all the relevant data to the provided file
	void LoadGraph(const char *filename);	// Load all the relevant data from the provided file

	void PrintDimacs(const char *filename = "DIMACS");
	
	/// Functions for finding low-level paths / areas
	void GetMoveDirectionDetails	// Decide how many diagonal and cardinal steps to make and in which direction
		(xyLoc & from, xyLoc & to, direction & c, direction & d, int & nTotalMoves, int & nDiagMoves);
	bool IsHReachable		// Check if two locations are h-reachable, save the path to the path
		(xyLoc & from, xyLoc & to, std::vector<mapLoc> & path = defaultPath, bool append = false);
	bool IsQuickHReachable	// Quickly check for a path between the two locations (no guarantee of finding an existing path)
		(xyLoc & from, xyLoc & to, std::vector<mapLoc> & path = defaultPath, bool append = false);
	bool IsLookaheadHReachable	// Similar to QuickHReachable, better chance of finding a path (some extra cost)
		(xyLoc & from, xyLoc & to, std::vector<mapLoc> & path = defaultPath, bool append = false);
	void GetDirectHReachableSubgoals	// Given a location, find all the subgoals it should be connected to
		(xyLoc & from, std::vector<subgoalId> & subgoals);
		
	/// Functions related to pruning
	cost CostOtherPath	// Look for the best path between sg1 and sg2, that does not go through sg
		(subgoalId & sg, subgoalId & sg1, subgoalId & sg2, cost limit = INFINITE_COST);
	bool IsNecessaryToConnect	// Check if we can find the optimal path between sg1 and sg2 without using sg
		(subgoalId sg, subgoalId sg1, subgoalId sg2);
	void AddEdge(subgoalId sg1, subgoalId sg2);	// Add the edge sg1->sg2 to the neighborhood vector (also to the edge vector if sg2 is not pruned)	
	void RemoveEdge(subgoalId sg1, subgoalId sg2);	// Remove the edge sg1->sg2 from the edge vector
	void PruneSubgoal(subgoalId sg);	// Mark the subgoal as local and make the relevant edge changes

	/// Functions for managing clearances
	void SetClearance(mapLoc loc, direction d, int clearance)
		{cellInfo[loc] = (cellInfo[loc] & ~(CLEARANCE_MASK << clearanceShift[d])) | (((clearance <= CLEARANCE_LIMIT)?clearance:0) << clearanceShift[d]);}
	int GetClearance(mapLoc loc, direction d)
		{return (cellInfo[loc] >> clearanceShift[d]) & CLEARANCE_MASK;}
	int GetTotalClearance(mapLoc loc, direction d)
	{
		int totalClearance = 0;
		while (GetClearance(loc,d) == 0)
		{
			loc += deltaMapLoc[d]*(CLEARANCE_LIMIT);
			totalClearance += (CLEARANCE_LIMIT);
		}
		return totalClearance + GetClearance(loc,d);
	}

	/// Heuristic functions
	cost HCost(xyLoc l1, xyLoc l2)	// The octile distance between l1 and l2
		{int dx = (l1.x>l2.x)?(l1.x-l2.x):(l2.x-l1.x);	int dy = (l1.y>l2.y)?(l1.y-l2.y):(l2.y-l1.y);	
		return (dx>dy)?(dx*CARD_COST + dy*DIAG_DIFF):(dy*CARD_COST + dx*DIAG_DIFF);}
	cost HCost(subgoalId sg1, subgoalId sg2) {return HCost(location[sg1], location[sg2]);}
	
	/// Functions for managing the heap
	inline void AddToOpen(subgoalId sg, cost fVal);
	inline void PopMin();
#ifdef REPLACE_POPPED
	void PopReplacableTop(); // Force the heap to remove the top element, without waiting for a replacement
#endif
	inline heapElement GetMin();
	inline void PercolateUp(int index);
	inline void PercolateDown(int index);
	/* The stack and heap work together as follows:
	 * If an element is expanded from the heap, it is not immediately removed.
	 * All its successors with the same fVal are inserted into the stack and
	 * all its successors with a higher fVal are inserted into the heap
	 * If at the end of the expansion the stack is empty, only then we remove the element from the heap.
	 * Essentially, as long as all its same-fVal successors are around, the original state stays in the heap,
	 * mainly as a representative of the stack (for instance, theHeap[0].fVal is used for termination)
	 */

	/// Conversion functions
	/* When generating our version of the map, we add a padding. Any xyLoc refers
	 * to a location in the original map, any mapLoc refers to the index of a cell
	 * in our padded and linearized map. The following functions do the conversion
	 * (height and width take the padding into account, that is, height = 2 + original height)
	 */
	xyLoc ToXYLoc(mapLoc loc) {return xyLoc((loc%width)-1, (loc/width)-1);}
	mapLoc ToMapLoc(xyLoc loc) {return (loc.y+1)*width + loc.x + 1;}
	subgoalId ToSubgoalId(mapLoc loc) {return cellInfo[loc];}
	void ToXYLocPath	// Convert a mapLocPath to xyLocPath
		(std::vector<mapLoc> & mapLocPath, std::vector<xyLoc> & xyLocPath);
	
	/// Functions for doing an A* search, using the subgoal graph
	void ResetSearch();	// Reset the search counter and the 'generated' values
	void ConnectStartAndGoalToGraph	// As preparation for a search over the subgoal graph
		(xyLoc & start, xyLoc & goal, subgoalId & sgStart, subgoalId & sgGoal, std::vector<subgoalId> & startSubgoals, std::vector<subgoalId> & goalSubgoals);
	cost SubgoalAStarSearchSimple	// Do a search over the subgoal graph, between two subgoals (used during pruning)
		(subgoalId & start, subgoalId & goal, cost limit = INFINITE_COST, std::vector<subgoalId> & abstractPath = defaultAbstractPath);
	cost SubgoalAStarSearch	// Do a search over the subgoal graph, between two subgoals (used online)
		(subgoalId & start, subgoalId & goal, std::vector<subgoalId> & abstractPath = defaultAbstractPath);
	//cost TryLocalPair	// Try to find a local subgoal pair connecting start and goal (not guaranteed)
	//	(subgoalId & sgStart, subgoalId & sgGoal, std::vector<subgoalId> & startDirectHReachableSubgoals, std::vector<subgoalId> & goalDirectHReachableSubgoals, std::vector<subgoalId> & localPath);
	
	/// Main functions for getting a low level path between two locations
	cost FindPath	// Either use search or lookup, convert abstract path to low level path
		(xyLoc & start, xyLoc & goal, std::vector<xyLoc> & thePath = defaultXYPath);
	cost GetPath(xyLoc & start, xyLoc & goal, std::vector<xyLoc> & thePath = defaultXYPath);
	level GetGraphLevel(){return graphLevel;}
	
	/// Statistics/output functions
	double directHReachableTime, initializeTime, searchTime, finalizeTime;
	int nTotalExpansions, nTotalTouched, nTotalDirectHReachable, nTotalDirectHReachableCalls, nTotalExtraEdges, nInstances;
	std::ofstream detailed, brief;

	void OpenOutputFiles(const char * _detailed = "", const char * _brief = "")
	{
		if (_detailed != "")
			detailed.open(_detailed);
		if (_brief != "")
		{
			brief.open(_brief, std::ios::app);
		}
	}
	void ReportIntStatistic(const char * description, int val)
	{
		if (detailed.is_open())	detailed<<description<<": "<<val<<std::endl;
		if (brief.is_open())		brief<<val<<"\t";
		#ifdef SG_VERBOSE
			std::cout<<description<<": "<<val<<std::endl;
		#endif
	}
	void ReportDoubleStatistic(const char * description, double val)
	{
		if (detailed.is_open())	detailed<<description<<": "<<val<<std::endl;
		if (brief.is_open())		brief<<val<<"\t";
		#ifdef SG_VERBOSE
			std::cout<<description<<": "<<val<<std::endl;
		#endif
	}
	void CloseOutputFiles()
	{
		if (detailed.is_open())	detailed.close();
		if (brief.is_open()){brief<<std::endl;	brief.close();}
	}
	void SearchStatistics()
	{
		#ifdef SG_STATISTICS
		ReportDoubleStatistic("Average number of direct-h-reachable subgoals", ((double) nTotalDirectHReachable)/nTotalDirectHReachableCalls);

		ReportIntStatistic("Number of instances", nInstances);

		ReportIntStatistic("Total number of expansions", nTotalExpansions);
		ReportIntStatistic("Total number of successors processed", nTotalTouched);
		ReportIntStatistic("Total number of extra edges", nTotalExtraEdges);
		ReportDoubleStatistic("Total initialize time (ms)", initializeTime*1000.0);
		ReportDoubleStatistic("Total search time (ms)", searchTime*1000.0);
		ReportDoubleStatistic("Total finalize time (ms)", finalizeTime*1000.0);
		ReportDoubleStatistic("Total solution time (ms)", (initializeTime + searchTime + finalizeTime)*1000.0);

		ReportDoubleStatistic("Average number of expansions", ((double)nTotalExpansions)/nInstances);
		ReportDoubleStatistic("Average number of successors processed", ((double)nTotalTouched)/nInstances);
		ReportDoubleStatistic("Average number of extra edges", ((double)nTotalExtraEdges)/nInstances);
		ReportDoubleStatistic("Average initialize time (ms)", (initializeTime*1000.0)/nInstances);
		ReportDoubleStatistic("Average search time (ms)", (searchTime*1000.0)/nInstances);
		ReportDoubleStatistic("Average finalize time (ms)", (finalizeTime*1000.0)/nInstances);
		ReportDoubleStatistic("Average solution time (ms)", ((directHReachableTime + initializeTime + searchTime + finalizeTime)*1000.0)/nInstances);

		#endif
	}
	void PrintNLevelGraphStatistics();

	void GetHeapCapacity()
	{
		#ifdef SG_STATISTICS
			#ifdef SG_VERBOSE
				std::cout<<"The heap capacity is: "<<theHeap.capacity()<<", which is about "<<(theHeap.capacity()*sizeof(heapElement))/1024<<"kb."<<std::endl;
			#endif
		#endif
	}

	/// Auxiliary (mostly inline) functions:
	bool IsTraversable(mapLoc loc){return traversable[(loc >> 3)] & (1 << (loc & 7));}
	void SetTraversable(mapLoc loc){traversable[loc >> 3] |= (1 << (loc & 7));}

	bool IsSubgoal(mapLoc loc){return subgoal[(loc >> 3)] & (1 << (loc & 7));}
	void SetSubgoal(mapLoc loc){subgoal[loc >> 3] |= (1 << (loc & 7));}

	bool IsPruned(subgoalId sg){return sgLevel[sg] < graphLevel;}
	void SetPruned(subgoalId sg){if(sgLevel[sg] == graphLevel) sgLevel[sg]--;}

	bool HasExtraEdge(subgoalId sg){return hasExtraEdge[(sg >> 3)] & (1 << (sg & 7));}
	void SetExtraEdgeFlag(subgoalId sg){hasExtraEdge[sg >> 3] |= (1 << (sg & 7));}
	void RemoveExtraEdgeFlag(subgoalId sg){hasExtraEdge[sg >> 3] &= ~(1 << (sg & 7));}

	bool IsOpen(subgoalId sg){return open[(sg >> 3)] & (1 << (sg & 7));}
	void SetOpen(subgoalId sg){open[sg >> 3] |= (1 << (sg & 7));}
	void SetClosed(subgoalId sg){open[sg >> 3] &= ~(1 << (sg & 7));}

private:
	/// Variables for controling the behaviour of the subgoal graph

	unsigned int height, width, mapSize, nSubgoals, nGlobalSubgoals, nLocalSubgoals;
	/* A note about directions:
	 * There are 8 directions. We label them 0-7 as follows: N = 0, NE, E, SE, S, SW, W, NW = 7
	 * Even numbers are cardinal directions and odd numbers are diagonal directions.
	 * For a diagonal direction d, its associated cardinal directions are d+1 and d-1
	 */
	int deltaMapLoc[24];	/* Given a mapLoc loc and a direction d, this array can be used to determine the
							 * loc' we end up at if we follow d from loc (loc' = loc + deltaMapLoc[d]). Note that,
							 * even there is only 8 directions, we keep 3 copies for each direction for a total of
							 * 24 values. This is to minimize the number of mod operations. This array should be
							 * initialized after we know the width and height of the (padded) map.
							 */

	/// Information about each location on the map						 
	char* traversable;		// 1: Traversable, 0: Not traversable (size = mapSize)
	char* subgoal;			// 1: Subgoal, 0: Not a subgoal (size = mapSize)
	subgoalId* cellInfo;	/* For subgoals, keep the subgoalId to directly link a cell to the subgoal graph;
							 * For traversable cells, keep the obstacle-subgoal clearances for the four
							 * cardinal directions (to speed up the linking to nearby subgoals)
							 * Currently not used for obstacles (maybe use it to label different chunks of obstacles?)
							 * (size = mapSize)
							 */

	/// Information about each subgoal
	xyLoc* location;		// Keep the location of the subgoal (size = nSubgoals)
	//char* pruned;			// Keep track of whether a subgoal is pruned or not (size = nSubgoals)
	level* sgLevel;			// Keep track of the level the subgoal is in, makes 'pruned' obsolete (size = nSubgoals)
	level graphLevel;		/* Keep track of the graph's highest level:
							 * 0 => no subgoals, only grid
							 * 1 => simple subgoal graph
							 * 2 => two-level subgoal gaph
							 * n > 2 => n-level subgoal graph
							 */

	/// Variables for storing edges
	bool finalized;	// Whether to use the vectors or edges to lookup neighbors
	std::vector<std::vector<subgoalId> > edgeVector;	// For storing the edges that will be used for search (size = nSubgoals)
	std::vector<std::vector<subgoalId> > neighborhoodVector;	// For storing the subgoals that should be considered to see if the subgoal can be pruned

	subgoalId** neighbors;	// Once the subgoal graph is pruned and the number of edges are fixed, use this instead of vectors (for memory)
	uint16_t* nNeighbors;	// Keep the number of neighbors for each subgoal
	
	subgoalId** localNeighbors;	// (NOT USED) Once the subgoal graph is pruned and the number of edges are fixed, use this instead of vectors (for memory)
	uint16_t* nLocalNeighbors;	// Keep the number of local neighbors for each subgoal
	char* hasExtraEdge;		/* Even after the graph is finalized, the number of neighbors of a subgoal
							 * can change when a new start and goal state are linked to the graph.
							 * However, handled carefully, the change in this number can be limited
							 * to be at most 1. Therefore, we allocate space for an extra neighbor
							 * for each subgoal and keep track of whether there is an extra neighbor
							 * with this boolean vector
							 */

	/// Search related variables
	/* Instead of keeping the list of generated states for each search and use hash tables
	 * to access that information, preallocate the memory for search info for each subgoal
	 * and directly access the info with the subgoalId. This is manageable since we only keep
	 * the information for subgoals, not any arbitrary cell in the map.
	 */
	uint16_t* generated;	// Keep track of the last time a subgoal has been generated
	uint16_t search;	/* The current search number. When a search terminates, only increment this
						 * to reset the previous search. search == generated[subgoalX] iff
						 * subgoalX is generated for the current search
						 */
	cost* gCost;		// g-cost of each subgoal
	subgoalId* parent;	// Parent of each subgoal
	char* open;			// 0: In closed list; 1: In open list. (generated[x] < search: x not yet generated)
	cost* distanceToGoal;

	std::vector<std::vector<subgoalId> > buckets;
	std::vector<std::vector<subgoalId> > bucketsDisplay;
#ifdef REPLACE_POPPED
	bool canReplaceTop;
#endif
	std::vector<heapElement> theHeap;
	std::vector<heapElement> theStack;
};

#endif
