#include "SubgoalGraph.h"

void SubgoalGraph::InitializeValues()
{
	search = MAX_SEARCH;
	graphLevel = 0;
	finalized = false;

	traversable = NULL;
	subgoal = NULL;
	cellInfo = NULL;

	location = NULL;
	sgLevel = NULL;
	neighbors = NULL;
	nNeighbors = NULL;
	localNeighbors = NULL;
	nLocalNeighbors = NULL;
	hasExtraEdge = NULL;

	generated = NULL;
	gCost = NULL;
	parent = NULL;
	open = NULL;
	
	#ifdef SUCCESSOR_PRUNING
		surroundingObstacles = NULL;
		incomingDirection = NULL;
		edgeDirections = NULL;
	#endif
}
void SubgoalGraph::CreateGraph(int nPrunings)
{
	SetDirections();

	#ifdef SUCCESSOR_PRUNING
		CreateExpansionPruningMap();
	#endif
	
	#ifdef SG_STATISTICS
		Timer t;
		t.StartTimer();
	#endif

	IdentifySubgoals();
	ComputeClearances();
	LinkSubgoals();

	#ifdef SG_STATISTICS
		t.EndTimer();
		ReportDoubleStatistic("Graph built in (ms)", t.GetElapsedTime()*1000);
		t.StartTimer();
	#endif

	while (nPrunings > 0 && PruneSubgoals())
	{
		//#ifdef SG_STATISTICS
		//	PrintNLevelGraphStatistics();
		//#endif
		nPrunings--;
	}
	#ifdef SG_STATISTICS
		t.EndTimer();
		ReportDoubleStatistic("Graph pruned in (ms)", t.GetElapsedTime()*1000);
		PrintNLevelGraphStatistics();
	#endif
	FinalizeGraph();
	MemoryAnalysis();
}

#ifdef SG_RUNNING_IN_HOG
SubgoalGraph::SubgoalGraph(Map* map, int nPrunings)
{
	InitializeValues();
//	OpenOutputFiles(detailedStatistics, briefStatistics);
	LoadMap(map);
	CreateGraph(nPrunings);
}
#endif

SubgoalGraph::SubgoalGraph(std::vector<bool> &bits, int width, int height, int nPrunings)
{
	InitializeValues();
	//OpenOutputFiles(detailedStatistics, briefStatistics);
	LoadMap(bits, width, height);

#ifdef MAX_LEVEL
	nPrunings = MAX_LEVEL-1;
#endif
	CreateGraph(nPrunings);
}

#ifdef SG_RUNNING_IN_HOG
void SubgoalGraph::LoadMap(Map* map)
{
	height = map->GetMapHeight() + 2;	// Add the padding
	width = map->GetMapWidth() + 2;
	mapSize = height*width;

	unsigned int reducedMapSize = (mapSize + 7)/8;
	traversable = new char[reducedMapSize];
	for (unsigned int i = 0; i < reducedMapSize; i++)
		traversable[i] = 0;

	int nTraversable = 0;

	for (unsigned int x = 0; x < width-2; x++){
		for (unsigned int y = 0; y < height-2; y++){
			if(map->GetTerrainType(x,y) == kGround)
			{
				SetTraversable(ToMapLoc(xyLoc(x,y)));
				nTraversable++;
			}
		}
	}

	#ifdef SG_STATISTICS
		ReportIntStatistic("Height", height);
		ReportIntStatistic("Width", width);
		ReportIntStatistic("Map size", mapSize);
		ReportIntStatistic("Traversable cells", nTraversable);
	#endif
}
#endif

void SubgoalGraph::LoadMap(std::vector<bool> &bits, int _width, int _height)
{
	height = _height + 2;	// Add the padding
	width = _width + 2;
	mapSize = height*width;

	unsigned int reducedMapSize = (mapSize + 7)/8;
	traversable = new char[reducedMapSize];
	for (unsigned int i = 0; i < reducedMapSize; i++)
		traversable[i] = 0;

	int nTraversable = 0;

	for (unsigned int x = 0; x < width-2; x++){
		for (unsigned int y = 0; y < height-2; y++){
			if(bits[y*_width+x])
			{
				SetTraversable(ToMapLoc(xyLoc(x,y)));
				nTraversable++;
			}
		}
	}

	#ifdef SG_STATISTICS
		ReportIntStatistic("Height", height);
		ReportIntStatistic("Width", width);
		ReportIntStatistic("Map size", mapSize);
		ReportIntStatistic("Traversable cells", nTraversable);
	#endif
}

SubgoalGraph::SubgoalGraph(const char *filename)
{
	InitializeValues();
	LoadGraph(filename);
}
SubgoalGraph::~SubgoalGraph()
{
	if(neighbors)
	{
		for (subgoalId sg = 0; sg < nSubgoals; sg++)
		{
			if(neighbors[sg])
				delete [] neighbors[sg];
		}
	}
	
	if(localNeighbors)
	{
		for (subgoalId sg = 0; sg < nLocalSubgoals; sg++)
		{
			if(localNeighbors[sg])
				delete [] localNeighbors[sg];
		}
	}

	if(traversable)	delete [] traversable;
	if(subgoal)		delete [] subgoal;
	if(cellInfo)	delete [] cellInfo;

	if(location)	delete [] location;
	if(sgLevel)		delete [] sgLevel;
	if(neighbors)	delete [] neighbors;
	if(nNeighbors)	delete [] nNeighbors;
	if(localNeighbors)	delete [] localNeighbors;
	if(nLocalNeighbors)	delete [] nLocalNeighbors;
	if(hasExtraEdge)delete [] hasExtraEdge;

	if(generated)	delete [] generated;
	if(gCost)		delete [] gCost;
	if(distanceToGoal) delete [] distanceToGoal;
	if(parent)		delete [] parent;
	if(open)		delete [] open;


	#ifdef SUCCESSOR_PRUNING
	if(surroundingObstacles)	delete [] surroundingObstacles;
	if(incomingDirection)	delete [] incomingDirection;
	if(edgeDirections)
	{
		for (subgoalId sg = 0; sg < nSubgoals; sg++)
		{
			if(edgeDirections[sg])
				delete [] edgeDirections[sg];
		}
	}
	#endif
}

void SubgoalGraph::SetDirections()
{
	deltaMapLoc[0] = -width;	// North
	deltaMapLoc[2] = 1;			// East
	deltaMapLoc[4] = width;		// South
	deltaMapLoc[6] = -1;		// West
	deltaMapLoc[1] = deltaMapLoc[0] + deltaMapLoc[2];	// North-East
	deltaMapLoc[3] = deltaMapLoc[2] + deltaMapLoc[4];	// South-East
	deltaMapLoc[5] = deltaMapLoc[4] + deltaMapLoc[6];	// South-West
	deltaMapLoc[7] = deltaMapLoc[6] + deltaMapLoc[0];	// North-West

	// Create the extra copies
	for (direction d = 0; d < 8; d++)
	{
		deltaMapLoc[d + 8] = deltaMapLoc[d];
		deltaMapLoc[d + 16] = deltaMapLoc[d];
	}
}
void SubgoalGraph::IdentifySubgoals()
{
	unsigned int reducedMapSize = (mapSize + 7)/8;
	subgoal = new char[reducedMapSize];
	for (unsigned int i = 0; i < reducedMapSize; i++)
		subgoal[i] = 0;

	cellInfo = new subgoalId[mapSize];
	std::vector<xyLoc> locationVector;
	nSubgoals = 0;

	for (mapLoc l = 0; l < mapSize; l++)
	{
		if(IsTraversable(l))	// If the cell is traversable
		{
			for (direction d = 1; d <= 7; d+=2)	// Check its corners
			{
				// If there is an obstacle in the diagonal direction but no obstacles in the associated cardinal directions
				if(!IsTraversable(l + deltaMapLoc[d]) && IsTraversable(l + deltaMapLoc[d-1]) && IsTraversable(l + deltaMapLoc[d+1]))
				{
					// We have found a subgoal
					SetSubgoal(l);
					cellInfo[l] = nSubgoals;	// This will be the id of the subgoal
					nSubgoals++;
					locationVector.push_back(ToXYLoc(l));
					break;	// Don't look at any more corners
				}
			}
		}
	}

	nGlobalSubgoals = nSubgoals;	// initially every subgoal is global
	location = new xyLoc[nSubgoals+2];
	generated = new uint16_t[nSubgoals+2];	// +2 for possible start and goal states
	gCost = new cost[nSubgoals+2];
	distanceToGoal = new cost[nSubgoals+2];
	parent = new subgoalId[nSubgoals+2];
	sgLevel = new level[nSubgoals+2];
	
	
	#ifdef SUCCESSOR_PRUNING
		surroundingObstacles = new neighborhoodInfo[nSubgoals+2];
		incomingDirection = new edgeDirection[nSubgoals+2];
	#endif

	for (unsigned int i = 0; i < nSubgoals; i++)
	{
		sgLevel[i] = 1;
	}
	sgLevel[nSubgoals] = 0;	// Start and goal are at the lowest level
	sgLevel[nSubgoals+1] = 0;
	graphLevel = 1;

	open = new char[(nSubgoals + 9)/8];

	for (unsigned int i = 0; i < nSubgoals; i++)
	{
		location[i] = locationVector[i];
	}
#ifdef SG_STATISTICS
	//std::cout<<"Initial subgoal graph has "<<nSubgoals<<" subgoals."<<std::endl;
	ReportIntStatistic("Number of subgoals", nSubgoals);
#endif
}
void SubgoalGraph::ComputeClearances()
{
	direction d = 0;	// North clearances
	for (int x = 0; x < (int)width-2; x++)
	{
		int clearance = 0;
		for (int y = 0; y < (int)height-2; y++)
		{
			mapLoc loc = ToMapLoc(xyLoc(x,y));
			if (IsSubgoal(loc) || !IsTraversable(loc))
			{
				clearance = 0;
			}
			else
			{
				clearance ++;
				SetClearance(loc, d, clearance);
			}
		}
	}

	d = 4;	// South clearances
	for (int x = 0; x < (int)width-2; x++)
	{
		int clearance = 0;
		for (int y = (int)height-3; y >= 0; y--)
		{
			mapLoc loc = ToMapLoc(xyLoc(x,y));
			if (IsSubgoal(loc) || !IsTraversable(loc))
			{
				clearance = 0;
			}
			else
			{
				clearance ++;
				SetClearance(loc, d, clearance);
			}
		}
	}

	d = 6;	// West clearances
	for (int y = 0; y < (int)height-2; y++)
	{
		int clearance = 0;
		for (int x = 0; x < (int)width-2; x++)
		{
			mapLoc loc = ToMapLoc(xyLoc(x,y));
			if (IsSubgoal(loc) || !IsTraversable(loc))
			{
				clearance = 0;
			}
			else
			{
				clearance ++;
				SetClearance(loc, d, clearance);
			}
		}
	}

	d = 2;	// East clearances
	for (int y = 0; y < (int)height-2; y++)
	{
		int clearance = 0;
		for (int x = (int)width-3; x >= 0; x--)
		{
			mapLoc loc = ToMapLoc(xyLoc(x,y));
			if (IsSubgoal(loc) || !IsTraversable(loc))
			{
				clearance = 0;
			}
			else
			{
				clearance ++;
				SetClearance(loc, d, clearance);
			}
		}
	}
}
void SubgoalGraph::LinkSubgoals()
{
	std::vector<subgoalId> directHReachableNeigbors;
//	nNeighbors = new uint16_t[nSubgoals+2];	/// AAAAAAAAAAAA
//	neighbors = new subgoalId*[nSubgoals+2];
	int nEdges = 0;
	for (subgoalId sg = 0; sg < nSubgoals; sg++)
	{
		GetDirectHReachableSubgoals(location[sg], directHReachableNeigbors);
		edgeVector.push_back(directHReachableNeigbors);
		neighborhoodVector.push_back(directHReachableNeigbors);
		nEdges += directHReachableNeigbors.size();
	}
#ifdef SG_STATISTICS
	//std::cout<<"Initial subgoal graph has "<<nEdges<<" edges."<<std::endl;
	ReportIntStatistic("Number of edges (initial)", nEdges);
#endif
}
bool SubgoalGraph::PruneSubgoals()
{
#ifdef SG_STATISTICS
	Timer t;
	t.StartTimer();
#endif
	int nPruned = 0;
	int nUnpruned = 0;

	std::vector<std::vector<subgoalId> > edgeVectorCopy(nSubgoals);
	std::vector<std::vector<subgoalId> > neighborhoodVectorCopy(nSubgoals);
	//std::vector<std::vector<subgoalId> > edgeVectorCopy(edgeVector);
	//std::vector<std::vector<subgoalId> > neighborhoodVectorCopy(neighborhoodVector);

	for (subgoalId sg = 0; sg < nSubgoals; sg++)
	{
		if (!IsPruned(sg))
		{
			sgLevel[sg]++;

			#ifdef SG_ASSERTIONS
				assert(edgeVector[sg].size() == neighborhoodVector[sg].size());
			#endif
			edgeVectorCopy[sg] = edgeVector[sg];
			neighborhoodVectorCopy[sg] = neighborhoodVector[sg];
		}
	}
	graphLevel++;
	nLocalSubgoals = 0;
	
	for (subgoalId sg = 0; sg < nSubgoals; sg++)
	{
		bool necessary = IsPruned(sg);	// This is just a short-cut to not do any of the following stuff if the subgoal is already pruned

		std::vector<subgoalId> neighbors = neighborhoodVector[sg];
		for (unsigned int i = 0; i+1 < neighborhoodVector[sg].size() && !necessary; i++)	// Try to find a pair of subgoals that needs this one
			for (unsigned int j = i+1; j < neighborhoodVector[sg].size() && !necessary; j++)
			{
				necessary = IsNecessaryToConnect(sg, neighborhoodVector[sg].at(i), neighborhoodVector[sg].at(j));
			}

		if (!necessary)	// If there is no reason not to prune this subgoal, prune it
		{
			PruneSubgoal(sg);
			nPruned++;
			nLocalSubgoals++;
		}
		if (!IsPruned(sg))
			nUnpruned++;
	}
	
	for (subgoalId sg = 0; sg < nSubgoals; sg++)
	{
		if (!IsPruned(sg))
			neighborhoodVector[sg] = edgeVector[sg];

	}
	
#ifdef SG_STATISTICS
	t.EndTimer();
	#ifdef SG_VERBOSE
		std::cout<<"Graph pruned in "<<t.GetElapsedTime()*1000<<"ms"<<std::endl;
	#endif
#endif

	// If no subgoals are pruned, decrement their levels and the graph level
	if (nPruned == 0)
	{
		for (subgoalId sg = 0; sg < nSubgoals; sg++)
		{
			if (!IsPruned(sg))
				sgLevel[sg]--;
		}
		graphLevel--;
#ifdef SG_STATISTICS
	#ifdef SG_VERBOSE
		std::cout<<"No more subgoals to prune!"<<std::endl;
	#endif
#endif
		return false;
	}

	// If all subgoals are pruned, then reset their edges
	if (nUnpruned == 0)
	{
		graphLevel--;	// First decrement the graph level, so all the subgoals that are pruned this iteration are now considered not-pruned

		for (subgoalId sg = 0; sg < nSubgoals; sg++)
		{
			if (!IsPruned(sg))
			{
				edgeVector[sg] = edgeVectorCopy[sg];
				neighborhoodVector[sg] = neighborhoodVectorCopy[sg];
			}
		}
#ifdef SG_STATISTICS
	#ifdef SG_VERBOSE
		std::cout<<"All subgoals pruned, going back a level!"<<std::endl;
	#endif
#endif
		return false;
	}


#ifdef DISCARD_FIRST_LEVEL_LOCAL_EDGES
	if (graphLevel == 2){
		for (subgoalId sg = 0; sg < nSubgoals; sg++){
			if (sgLevel[sg] == 1){
				neighborhoodVector[sg] = edgeVector[sg];
			}
		}
	}
#endif

	return true;
}
void SubgoalGraph::FinalizeGraph()
{
	// Initialize the hasExtraEdge array
	unsigned int reducedNSubgoals = (nSubgoals + 9)/8;
	hasExtraEdge = new char[reducedNSubgoals];
	for (unsigned int i = 0; i < reducedNSubgoals; i++)
		hasExtraEdge[i] = 0;

	// Rearrange the subgoals so that global subgoals come before local subgoals
	subgoalId front = 0;
	subgoalId back = nSubgoals-1;

	std::vector<subgoalId> subgoalMap;
	subgoalMap.resize(nSubgoals);

	for (subgoalId sg = 0; sg < nSubgoals; sg++)
		subgoalMap[sg] = sg;

	while(true)
	{
		while (!IsPruned(front) && front < back)
			front++;

		while (IsPruned(back) && back > front)
			back--;

		if (front < back)
		{
			subgoalMap[front] = back;
			subgoalMap[back] = front;

			//SetUnpruned(front);
			//SetPruned(back);
			level tempLevel = sgLevel[front];
			sgLevel[front] = sgLevel[back];
			sgLevel[back] = tempLevel;
			
			std::vector<subgoalId> tempVector; 
			
			tempVector = edgeVector[front];
			edgeVector[front] = edgeVector[back];
			edgeVector[back] = tempVector;
			
			tempVector = neighborhoodVector[front];
			neighborhoodVector[front] = neighborhoodVector[back];
			neighborhoodVector[back] = tempVector;

			xyLoc tempXY = location[front];
			location[front] = location[back];
			location[back] = tempXY;

			cellInfo[ToMapLoc(location[front])] = front;
			cellInfo[ToMapLoc(location[back])] = back;
		}
		else{
			break;
		}
	}


	// Process all the edges that lead to a global subgoal
	neighbors = new subgoalId*[nSubgoals+2];
	nNeighbors = new uint16_t[nSubgoals+2];
	nLocalNeighbors = new uint16_t[nSubgoals+2];

	// neighbors[i] = edgeVectorSize of forward edges + 1 spot for extra edge + (neighborhoodVectorSize - edgeVectorSize edges between same level subgoals, level < graphLevel)
	for (subgoalId sg1 = 0; sg1 < nSubgoals; sg1++)
	{
		nNeighbors[sg1] = edgeVector[sg1].size();
		nLocalNeighbors[sg1] = neighborhoodVector[sg1].size() - edgeVector[sg1].size();
		neighbors[sg1] = new subgoalId[nNeighbors[sg1]+1+nLocalNeighbors[sg1]];

		int front = 0;	// Fill the forward edges starting from the front of the array
		int back = neighborhoodVector[sg1].size();	// Fill the same-level edges starting from the back of the array
		
		for (unsigned int i = 0; i < neighborhoodVector[sg1].size(); i++)	// Fill the array, except for the last spot
		{
			subgoalId sg2 = subgoalMap[neighborhoodVector[sg1].at(i)];

			if (sgLevel[sg1] == graphLevel || sgLevel[sg1] < sgLevel[sg2])	// forward edge
			{
				neighbors[sg1][front] = sg2;
				front++;
			}
			else if (sgLevel[sg1] == sgLevel[sg2])	// same-level edge
			{
				neighbors[sg1][back] = sg2;
				back--;
			}
			else	// something is wrong
			{
				std::cout<<"A descending edge is found"<<std::endl;
			}
		}
		if(back != front)
			std::cout<<"nNeighbors"<<edgeVector[sg1].size()<<"\tfront"<<front<<"\tback:"<<back<<std::endl;

		#ifdef SG_ASSERTIONS
			assert(back == front);	// They should both be pointing to the extra edge slot
		#endif
	}

	for (subgoalId sg = 0; sg < nSubgoals; sg++)
	{
		edgeVector[sg].clear();
		neighborhoodVector[sg].clear();
	}
	edgeVector.clear();
	neighborhoodVector.clear();

	for (subgoalId sg = 0; sg < nSubgoals+2; sg ++)
	{
		RemoveExtraEdgeFlag(sg);
	}

#ifdef SUCCESSOR_PRUNING
	AddEdgeDirections();
	ComputeSurroundingObstacles();
#endif

	nNeighbors[nSubgoals] = 0;
	nNeighbors[nSubgoals + 1] = 0;
	nLocalNeighbors[nSubgoals] = 0;
	nLocalNeighbors[nSubgoals + 1] = 0;
#ifdef DISPLAY_EXTRA_EDGES
	location[nSubgoals] = INVALID_XYLOC;
	location[nSubgoals+1] = INVALID_XYLOC;
#endif

	buckets.resize(graphLevel+1);	// From level 0 (for goal) to level graphLevel
	bucketsDisplay.resize(graphLevel+1);

#ifdef SG_STATISTICS
	// Set the search statistics variables
	directHReachableTime = 0.0;
	initializeTime = 0.0;
	searchTime = 0.0;
	finalizeTime = 0.0; ;

	nTotalExpansions = 0;
	nTotalTouched = 0;
	nTotalDirectHReachable = 0;
	nTotalDirectHReachableCalls = 0;
	nTotalExtraEdges = 0;
	nInstances = 0;
#endif

	finalized = true;

#ifdef SG_VERBOSE
	std::cout<<"Finalized!"<<std::endl;
#endif
}
void SubgoalGraph::MemoryAnalysis()
{
	int bitsPerMapCell = 1 + 1 + 8*sizeof(subgoalId);
	// traversable, subgoal, cellInfo
	
	int bitsPerSubgoalStored =
			8*sizeof(xyLoc) + 		// location
			8*sizeof(uint16_t) + 	// nNeighbors
			8*sizeof(uint16_t) +	// nLocalNeighbors
#ifdef SUCCESSOR_PRUNING
			8*sizeof(neighborhoodInfo) +	// surroundingObstacles
#endif
			8*sizeof(level);		// sgLevel

	int bitsPerSubgoalOnline =
			8*sizeof(subgoalId*) +	// neighbors[sg]
			8*sizeof(subgoalId) +	// extraEdge
			1 +						// hasExtraEdge
			8*sizeof(cost) +		// gCost
			8*sizeof(subgoalId) + 	// parent
			8*sizeof(uint16_t) + 	// generated
			1 + 					// open
#ifdef SUCCESSOR_PRUNING
			8*sizeof(edgeDirection) +	// incomingDirection
			8*sizeof(edgeDirection*) +	// edgeDirections
#endif
			8*sizeof(cost);	// distance to goal
	
	int bitsPerSubgoal = bitsPerSubgoalStored + bitsPerSubgoalOnline;
#ifdef SUCCESSOR_PRUNING
	int bitsPerEdge = 8*sizeof(subgoalId) + 8*sizeof(edgeDirection);
#else
	int bitsPerEdge = 8*sizeof(subgoalId);
#endif

	int nEdges = 0;
	for (subgoalId sg = 0; sg < nSubgoals; sg++)
		nEdges += (nNeighbors[sg] + nLocalNeighbors[sg]);
	
	double estimatedMapMemory = (bitsPerMapCell * mapSize)/(8*1024);	//kb
	double estimatedSubgoalMemory = (bitsPerSubgoal*(nSubgoals+2))/(8*1024);	//kb
	double estimatedEdgeMemory = (bitsPerEdge*nEdges)/(8*1024);	//kb
	double estimatedMemory = (bitsPerMapCell * mapSize + bitsPerSubgoal*(nSubgoals+2) + bitsPerEdge*nEdges)/(8*1024);	//kb
	double estimatedStoredMemory = (bitsPerMapCell * mapSize + bitsPerSubgoalStored*(nSubgoals+2) + bitsPerEdge*nEdges)/(8*1024);	//kb
	
#ifdef SG_STATISTICS	
	ReportDoubleStatistic("Estimated memory usage by map (MB)", estimatedMapMemory/(double)1024);
	ReportDoubleStatistic("Estimated memory usage by subgoals (MB)", estimatedSubgoalMemory/(double)1024);
	ReportDoubleStatistic("Estimated memory usage by edges (MB)", estimatedEdgeMemory/(double)1024);
	ReportDoubleStatistic("Estimated memory usage by all (MB)", estimatedMemory/(double)1024);
	ReportDoubleStatistic("Estimated storage (MB)", estimatedStoredMemory/(double)1024);
#endif
}

void SubgoalGraph::SaveGraph(const char *filename)
{
	std::ofstream out(filename, std::ios::out | std::ios::binary);
	
	// Write parameters
	out.write((char*)&graphLevel,sizeof(level));
	out.write((char*)&height,sizeof(unsigned int));
	out.write((char*)&width,sizeof(unsigned int));
	out.write((char*)&mapSize,sizeof(unsigned int));
	out.write((char*)&nSubgoals,sizeof(unsigned int));
	out.write((char*)&nGlobalSubgoals,sizeof(unsigned int));
	out.write((char*)&nLocalSubgoals,sizeof(unsigned int));
	out.write((char*)&deltaMapLoc[0],sizeof(int)*24);

	unsigned int reducedMapSize = (mapSize + 7)/8;
	unsigned int reducedNSubgoals = (nSubgoals + 9)/8;
	
	out.write((char*)cellInfo, sizeof(subgoalId)*(mapSize));
	out.write(traversable, reducedMapSize);
	out.write(subgoal, reducedMapSize);

	out.write((char*)sgLevel, sizeof(level)*(nSubgoals+2));
	out.write((char*)location, sizeof(xyLoc)*(nSubgoals+2));
	out.write((char*)nNeighbors, sizeof(uint16_t)*(nSubgoals+2));
	out.write((char*)nLocalNeighbors, sizeof(uint16_t)*(nSubgoals+2));
	
	for (subgoalId sg = 0; sg < nSubgoals; sg++)
	{
		assert(!this->HasExtraEdge(sg));
		out.write((char*)neighbors[sg], sizeof(subgoalId)*(nNeighbors[sg]+1+nLocalNeighbors[sg]));
	}
	
#ifdef SG_STATISTICS			
	std::cout<<"Graph written to file.."<<std::endl;
#endif
	out.close();
}
void SubgoalGraph::LoadGraph(const char *filename)
{
	std::ifstream in(filename, std::ios::in | std::ios::binary);
	
	// Read parameters
	in.read((char*)&graphLevel,sizeof(level));
	in.read((char*)&height,sizeof(unsigned int));
	in.read((char*)&width,sizeof(unsigned int));
	in.read((char*)&mapSize,sizeof(unsigned int));
	in.read((char*)&nSubgoals,sizeof(unsigned int));
	in.read((char*)&nGlobalSubgoals,sizeof(unsigned int));
	in.read((char*)&nLocalSubgoals,sizeof(unsigned int));
	in.read((char*)&deltaMapLoc[0],sizeof(int)*24);

	// Allocate space
	unsigned int reducedMapSize = (mapSize + 7)/8;
	unsigned int reducedNSubgoals = (nSubgoals + 9)/8;
	
	cellInfo = new subgoalId[mapSize];
	traversable = new char[reducedMapSize];
	subgoal = new char[reducedMapSize];
	open = new char[reducedNSubgoals];
	hasExtraEdge = new char[reducedNSubgoals];
	for (unsigned int i = 0; i < reducedNSubgoals; i++)
	{
		open[i] = 0;
		hasExtraEdge[i] = 0;
	}
	sgLevel = new level[nSubgoals+2];
	location = new xyLoc[nSubgoals+2];
	nNeighbors = new uint16_t[nSubgoals+2];
	nLocalNeighbors = new uint16_t[nSubgoals+2];
	generated = new uint16_t[nSubgoals+2];
	gCost = new cost[nSubgoals+2];
	distanceToGoal = new cost[nSubgoals+2];
	parent = new subgoalId[nSubgoals+2];

	in.read((char*)cellInfo, sizeof(subgoalId)*(mapSize));
	in.read(traversable, reducedMapSize);
	in.read(subgoal, reducedMapSize);
	in.read((char*)sgLevel, sizeof(level)*(nSubgoals+2));
	in.read((char*)location, sizeof(xyLoc)*(nSubgoals+2));
	in.read((char*)nNeighbors, sizeof(uint16_t)*(nSubgoals+2));
	in.read((char*)nLocalNeighbors, sizeof(uint16_t)*(nSubgoals+2));

	neighbors = new subgoalId*[nSubgoals+2];
	// Read the edges
	for (subgoalId sg = 0; sg < nSubgoals; sg++)
	{
		neighbors[sg] = new subgoalId[nNeighbors[sg]+1+nLocalNeighbors[sg]];	// + 1 for the extra edge during search
		in.read((char*)neighbors[sg], sizeof(subgoalId)*(nNeighbors[sg]+1+nLocalNeighbors[sg]));
	}
	
	finalized = true;
	search = 0;
	for (subgoalId sg = 0; sg < nSubgoals+2; sg++)
		generated[sg] = 0;

	buckets.resize(graphLevel+1);
		
#ifdef SG_STATISTICS			
	std::cout<<"Graph read from file.."<<std::endl;
	std::cout<<"Graph level: "<<graphLevel<<std::endl;
#endif
	initializeTime = 0.0;
	directHReachableTime = 0.0;
	searchTime = 0.0;
	finalizeTime = 0.0;

	in.close();
}

void SubgoalGraph::GetMoveDirectionDetails(xyLoc & from, xyLoc & to, direction & c, direction & d, int & nTotal, int & nTotalDiag)
{
	int dx = (from.x>to.x)?(from.x-to.x):(to.x-from.x);
	int dy = (from.y>to.y)?(from.y-to.y):(to.y-from.y);

	nTotal = (dx>dy)?(dx):(dy);	// total number of diagonal and cardinal moves that must be made to reach the target
	nTotalDiag = (dx<dy)?(dx):(dy);	// total number of diagonal moves that must be made to reach the target

	c = (dx>dy)?((from.x<to.x)?2:6):((from.y<to.y)?4:0);	// only make this kind of cardinal moves
	d = (from.x<to.x)?((from.y<to.y)?3:1):((from.y<to.y)?5:7);	// only make this kind of diagonal moves
}
bool SubgoalGraph::IsHReachable(xyLoc & from, xyLoc & to, std::vector<mapLoc> & path, bool append)
{
	int nTotalCard, nTotalDiag, nTotal;
	direction c,d;
	GetMoveDirectionDetails(from,to,c,d,nTotal,nTotalDiag);
	nTotalCard = nTotal-nTotalDiag;

	int *diagCount = new int[nTotal+1];
	diagCount[0] = 0;
	for(int i = 1; i <= nTotal; i++)
		diagCount[i] = -1;

	mapLoc loc = ToMapLoc(from);
	int i = 0;
	while (i < nTotal)
	{
		//if we can move cardinally, do it
		if (i - diagCount[i] < nTotalCard && diagCount[i+1] < diagCount[i] && IsTraversable(loc + deltaMapLoc[c]))	//0 <= x+cardX && x+cardX < width && 0 <= y+cardY && y+cardY < height &&
		{
			diagCount[i+1] = diagCount[i];
			i++;
			loc += deltaMapLoc[c];
		}

		//else, if we can move diagonally, do it
		else if (diagCount[i] < nTotalDiag && diagCount[i+1] <= diagCount[i] && IsTraversable(loc + deltaMapLoc[d]) && IsTraversable(loc + deltaMapLoc[d-1]) && IsTraversable(loc + deltaMapLoc[d+1]))
		{
			diagCount[i+1] = diagCount[i]+1;
			i++;
			loc += deltaMapLoc[d];
		}

		//else, backtrack
		else
		{
			if (i == 0)	// cannot backtrack
			{
				delete [] diagCount;
				return false;
			}

			i--;

			if (diagCount[i] == diagCount[i+1])
				loc -= deltaMapLoc[c];
			else
				loc -= deltaMapLoc[d];
		}
	}

	loc = ToMapLoc(from);
	if (!append)
	{
		path.clear();
		path.push_back(loc);
	}
	for (int i = 1; i <= nTotal; i++)
	{
		if (diagCount[i] > diagCount[i-1])	// Made a diagonal move
			loc += deltaMapLoc[d];
		else
			loc += deltaMapLoc[c];

		path.push_back(loc);
	}

	delete [] diagCount;
	return true;
}
bool SubgoalGraph::IsQuickHReachable(xyLoc & from, xyLoc & to, std::vector<mapLoc> & path, bool append)
{
	int nCard, nDiag, nTotal;
	direction c,d;
	GetMoveDirectionDetails(from,to,c,d,nTotal,nDiag);
	nCard = nTotal-nDiag;

	mapLoc loc = ToMapLoc(from);

	if (!append)
	{
		path.clear();
		path.push_back(loc);
	}
	while (nCard)
	{
		loc += deltaMapLoc[c];
		if (!IsTraversable(loc))
			return false;

		path.push_back(loc);
		nCard--;
	}

	while (nDiag)
	{
		loc += deltaMapLoc[d];
		if (!IsTraversable(loc) || !IsTraversable(loc + deltaMapLoc[d+3]) || !IsTraversable(loc + deltaMapLoc[d+5]))
			return false;

		path.push_back(loc);
		nDiag--;
	}

	return true;
}
bool SubgoalGraph::IsLookaheadHReachable(xyLoc & from, xyLoc & to, std::vector<mapLoc> & path, bool append)
{
	int nCard, nDiag, nTotal;
	direction c,d;
	GetMoveDirectionDetails(from,to,c,d,nTotal,nDiag);
	nCard = nTotal-nDiag;

	/*
	 		6 	3
		7	5	2
		C	4	1

		Current is the current position of the agent, assume it is trying to move north/nort-east.
		It can either move to 4 or 5.
		It can observe any of the numbered cells to determine its movement (observing 1,2, and 3 is the lookahead)

		Note that the loop constraint forces us to have at least one diagonal and at least one cardinal move remaining
		Therefore, if the given start and goal states are in the map limits, we don't need to check if 1,2,4,5, or 7 is in the map bounds
	 */

	int offset1 = deltaMapLoc[c] + deltaMapLoc[c];
	int offset2 = deltaMapLoc[c] + deltaMapLoc[d];
	//int offset3 = deltaMapLoc[d] + deltaMapLoc[d];
	int offset4 = deltaMapLoc[c];
	int offset5 = deltaMapLoc[d];
	//int offset6 = offset3 - deltaMapLoc[c];
	int offset7 = offset5 - deltaMapLoc[c];

	mapLoc loc = ToMapLoc(from);


	if (!append)
	{
		path.clear();
		path.push_back(loc);
	}

	while (nDiag && nCard)
	{
		if (!IsTraversable(loc + offset4))	// If 4 is an obstacle, no movement possible
			return false;

		else if (!IsTraversable(loc + offset5) || !IsTraversable(loc + offset7))	// If 5 or 7 is blocked, we can not move diagonally
		{																								// so, we move cardinally
			loc += offset4; nCard--;	path.push_back(loc);
		}

		else if (!IsTraversable(loc + offset1))	// If 1 is an obstacle and we move cardinally, then we can't move on the next turn
		{															// so, we move diagonally
			loc += offset5; nDiag--;	path.push_back(loc);
		}

		else if (!IsTraversable(loc + offset2))	// If 2 is an obstacle and we move diagonally, then we can't move on the next turn
		{															// so, we move cardinally
			loc += offset4; nCard--;	path.push_back(loc);
		}

		else	// Whether 6 or 3 is an obstacle does not force us to move a certain way, so we make a heuristic decision between cardinal and diagonal
		{
			if (nCard > nDiag)	//If we have more cardinal moves remaining, prefer cardinal move
			{
				loc += offset4; nCard--;	path.push_back(loc);
			}
			else
			{
				loc += offset5; nDiag--;	path.push_back(loc);
			}
		}
	}

	// We have only cardinal or only diagonal moves (or no moves) remaining, proceed quickly without lookahead
	while(nDiag)
	{
		if(!IsTraversable(loc + offset4) || !IsTraversable(loc + offset5) || !IsTraversable(loc + offset7))
			return false;

		loc += offset5; nDiag--;	path.push_back(loc);
	}

	while(nCard)
	{
		if(!IsTraversable(loc + offset4))
			return false;

		loc += offset4; nCard--;	path.push_back(loc);
	}

	return true;
}

void SubgoalGraph::GetDirectHReachableSubgoals(xyLoc & from, std::vector<subgoalId> & subgoals)
{
	mapLoc origin = ToMapLoc(from);
	subgoals.clear();
	int clearance[9];

	// Compute cardinal clearances
	for (direction c = 0; c <= 6; c+=2)
	{
		clearance[c] = 0;

		mapLoc loc = origin + deltaMapLoc[c];
		while (IsTraversable(loc) && !IsSubgoal(loc))
		{
			loc += deltaMapLoc[c];
			clearance[c]++;
		}

		if (IsSubgoal(loc))
			subgoals.push_back(ToSubgoalId(loc));
	}

	clearance[8] = clearance[0];	// Report North twice, to avoid using the modulus operation

	// Compute diagonal clearances
	for (direction d = 1; d <= 7; d+=2)
	{
		clearance[d] = 0;

		mapLoc loc = origin + deltaMapLoc[d];
		while ( IsTraversable(loc) && !IsSubgoal(loc) &&
				IsTraversable(loc + deltaMapLoc[d+3]) && 	// loc is already updated with the diagonal move, so the associated
				IsTraversable(loc + deltaMapLoc[d+5]))		// cardinal directions are +3, +5 instead of +1 +7 (-7 == -1 % 8)
		{
			loc += deltaMapLoc[d];
			clearance[d]++;
		}

		if (IsSubgoal(loc) && IsTraversable(loc + deltaMapLoc[d+3]) && IsTraversable(loc + deltaMapLoc[d+5]))
			subgoals.push_back(ToSubgoalId(loc));
	}

	/* Now, explore the 8 areas that the cardinal and diagonal separators create.
	 * Each area is at most as large as a parallelogram whose dimensions are the
	 * clearances of its associated cardinal and diagonal directions.
	 */
	for (direction d = 1; d <= 7; d+=2)
	{
		for (direction c = d-1; c <= d+1; c+=2)
		{
#ifdef USE_CLEARANCES
			//int maxExt = GetClearance(from,c);
			int maxExt = clearance[c]+1;
			mapLoc loc = origin;
#else
			int maxExt = clearance[c];
#endif

			for (int i = 1; i <= clearance[d]; i++)	// Go along the diagonal, from each cell on the diagonal ..
			{
#ifdef USE_CLEARANCES
				loc +=deltaMapLoc[d];
				int cardClearance = GetTotalClearance(loc,c);
				//int cardClearance = GetClearance(loc,c);
				if (cardClearance < maxExt)
				{
					//if (cardClearance == CLEARANCE_LIMIT)
					//{
					//	cardClearance = GetTotalClearance(loc,c);
					//}
					
					mapLoc loc2 = loc + deltaMapLoc[c]*cardClearance;
					if (IsSubgoal(loc2))
						subgoals.push_back(ToSubgoalId(loc2));
				}

				maxExt = (maxExt < cardClearance)?(maxExt):(cardClearance);
#else
				mapLoc loc = origin + deltaMapLoc[d]*i + deltaMapLoc[c];	// .. start sweeping along the cardinal direction
				int ext = 0;	// we may not have to explore the whole area

				while (ext < maxExt && IsTraversable(loc) && !IsSubgoal(loc))	// may be sped up with precomputed cardinal clearances
				{
					loc += deltaMapLoc[c];
					ext++;
				}

				if (ext < maxExt && IsSubgoal(loc))
					subgoals.push_back(ToSubgoalId(loc));

				if (ext < maxExt)
					maxExt = ext;
#endif
			}
		}
	}
}

cost SubgoalGraph::CostOtherPath(subgoalId & sg, subgoalId & sg1, subgoalId & sg2, cost limit)
{
	// If sg2 is already pruned, make sure it is reachable by adding the relevant global-to-local edges
	if(IsPruned(sg2))
	{
		for (unsigned int i = 0; i < edgeVector[sg2].size(); i++)
		{
			subgoalId tempSg = edgeVector[sg2].at(i);
			edgeVector[tempSg].push_back(sg2);
		}
	}

	// Temporarily remove all the outgoing edges from sg, essentially removing it from the search// Temporarily remove all the outgoing edges from sg, essentially removing it from the search
	std::vector<subgoalId> edges = edgeVector[sg];
	edgeVector[sg].clear();

	/* Do the search and get the cost. Since we are only checking if we can do better
	 * than costThrough, add a search limit to stop the search if we exceed costThrough
	 */
	cost alternateCost = SubgoalAStarSearchSimple(sg1,sg2, limit);
	//cost alternateCost = SubgoalAStarSearch(sg1,sg2);

	// Add back the outgoing edges from sg
	edgeVector[sg] = edges;

	// If sg2 is already pruned, make sure to remove the global-to-local edges we added earlier
	if(IsPruned(sg2))
	{
		for (unsigned int i = 0; i < edgeVector[sg2].size(); i++)
		{
			edgeVector[edgeVector[sg2].at(i)].pop_back();
		}
	}
	return alternateCost;
}
bool SubgoalGraph::IsNecessaryToConnect(subgoalId sg, subgoalId sg1, subgoalId sg2)
{
	/* We want to make sure that if sg is pruned, then sg1 and sg2 will still be somehow connected optimally.
	 */

	/* Method 1: If h(sg1,sg2) == h(sg1,sg) + h(sg,sg2), then that means
	 * sg1 and sg2 are h-reachable, therefore sg is not necessary to connect
	 */

	cost costThrough = HCost(sg1, sg) + HCost(sg, sg2);

#ifdef EXTRA_EDGES
	if (costThrough == HCost(sg1, sg2))
		return false;

	if (IsHReachable(location[sg1], location[sg2]))
		return false;

	/* Method 5 (last resort): Remove sg from the graph, do an A* search from sg1 to sg2.
	 * If the path found is larger then h(sg1,sg) + h(sg,sg2), then sg is necessary to connect
	 */
#endif
	if (CostOtherPath(sg, sg1,sg2, costThrough+1) <= costThrough)
		return false;

	return true;
}
void SubgoalGraph::PruneSubgoal(subgoalId sg)
{
	nGlobalSubgoals--;
	SetPruned(sg);
	std::vector<subgoalId> neighbors = neighborhoodVector[sg];

	// Since this is now a local subgoal, remove all incoming edges from the edgeVectors
	for (unsigned int i = 0; i< neighborhoodVector[sg].size(); i++)
	{
		RemoveEdge(neighborhoodVector[sg].at(i), sg);	// Note that they will only be removed from the edgeVectors
	}

	// Add the necessary connections between surrounding subgoals
	for (unsigned int i = 0; i+1 < neighborhoodVector[sg].size(); i++)
		for (unsigned int j = i+1; j < neighborhoodVector[sg].size(); j++)
		{
			subgoalId sg1 = neighborhoodVector[sg].at(i);
			subgoalId sg2 = neighborhoodVector[sg].at(j);

			//if (!IsPruned(sg1) || !IsPruned(sg2))
			{
				cost hCost = HCost(sg1,sg2);
				// Only connect sg1-sg2 if sg1 and sg2 are h-reachable and sg is on an h-reachable path
				bool connect = (hCost == HCost(sg,sg1) + HCost(sg,sg2));
/*
				// Do not connect if there is another connecting subgoal
				for (unsigned int i = 0; connect && i < neighborhoodVector[sg1].size(); i++)
				{
					subgoalId sg3 = neighborhoodVector[sg1].at(i);
					if (!IsPruned(sg3) && sg3 != sg)
						for (unsigned int j = 0; connect && j < neighborhoodVector[sg2].size(); j++)
						{
							if(sg3 == neighborhoodVector[sg2].at(j))
							{
								if(hCost == HCost(sg1, sg3) + HCost(sg3,sg2))
								{
									connect = false;
								}
								break;
							}
						}
				}
*/
				if (connect && (CostOtherPath(sg, sg1, sg2, hCost+1) != hCost))
				{
#ifndef EXTRA_EDGES
					std::cout<<"Extra edge added! WTF?"<<std::endl;
#endif

					AddEdge(sg1,sg2);
					AddEdge(sg2,sg1);
				}
			}
		}

	// Finally, remove all the local-to-local edges from the edgeVector	(TODO: does not seem necessary?)
	for (unsigned int i = 0; i< edgeVector[sg].size(); i++)
	{
		if(IsPruned(edgeVector[sg].at(i)))
		{
			std::cout<<"a"<<std::flush;
			edgeVector[sg].at(i) = edgeVector[sg].back();
			edgeVector[sg].pop_back();
			i--;
		}
	}
}
void SubgoalGraph::AddEdge(subgoalId sg1, subgoalId sg2)
{
	for (unsigned int i = 0; i < neighborhoodVector[sg1].size(); i++)
	{
		if (neighborhoodVector[sg1].at(i) == sg2)
			return;
	}

	neighborhoodVector[sg1].push_back(sg2);
	if (!IsPruned(sg2))
	{
		edgeVector[sg1].push_back(sg2);
	}
}
void SubgoalGraph::RemoveEdge(subgoalId sg1, subgoalId sg2)
{
	for (unsigned int i = 0; i < edgeVector[sg1].size(); i++)
	{
		if (edgeVector[sg1].at(i) == sg2)
		{
			edgeVector[sg1].at(i) = edgeVector[sg1].back();
			edgeVector[sg1].pop_back();
			return;
		}
	}
}

void SubgoalGraph::AddToOpen(subgoalId sg, cost fVal)
{
#ifdef REPLACE_POPPED
	if(canReplaceTop)	// If the top element of the heap can be replaced,
	{
		theHeap[0] = (heapElement(sg,fVal));	// .. replace it
		PercolateDown(0);		// and percolate down
		canReplaceTop = false;	// the top element is no longer replaceable
	}
	else
#endif
	{	// add element as usual
		theHeap.push_back(heapElement(sg,fVal));
		PercolateUp(theHeap.size()-1);
	}
}
void SubgoalGraph::PopMin()
{
#ifdef REPLACE_POPPED
	canReplaceTop = true;	// Don't pop it immediately, just mark it as replaceable
#else
	theHeap[0] = theHeap.back();
	theHeap.pop_back();
	PercolateDown(0);
#endif
}
#ifdef REPLACE_POPPED
void SubgoalGraph::PopReplacableTop()
{
	if (canReplaceTop)	// If the top of the heap is still not replaced
	{
		// Replace the top element with the last element and percolate down, to have the minimum f-value at the top
		theHeap[0] = theHeap.back();
		theHeap.pop_back();
		PercolateDown(0);
		canReplaceTop = false;
	}
}
#endif
heapElement SubgoalGraph::GetMin()
{
	return theHeap.front();
}

void SubgoalGraph::ResetSearch()
{
	// Set last search and generated values to 0, so that when the search is incremented, all the states will be not-generated
	search = 0;
	for (subgoalId sg = 0; sg < nSubgoals+2; sg++)
		generated[sg] = 0;
}
void SubgoalGraph::PercolateUp(int index)
{
	heapElement elem = theHeap[index];
	int parent;
	parent = (index-1) >> 1;

	while(index > 0 && theHeap[parent].fVal > elem.fVal)
	{
		theHeap[index] = theHeap[parent];
		index = parent;
		parent = (index-1) >> 1;
	}

	theHeap[index] = elem;
}
void SubgoalGraph::PercolateDown(int index)
{
	heapElement elem = theHeap[index];
	int maxSize = theHeap.size();

	while(true)
	{
		int child1 = (index << 1) + 1;
		if (child1 >= maxSize)
			break;

		int child2 = child1+1;

		// If the first child has smaller key
		if (child2 == maxSize || theHeap[child1].fVal <= theHeap[child2].fVal)
		{
			if (theHeap[child1].fVal < elem.fVal)
			{
				theHeap[index] = theHeap[child1];
				index = child1;
			}
			else
				break;
		}

		else if (theHeap[child2].fVal < elem.fVal)
		{
			theHeap[index] = theHeap[child2];
			index = child2;
		}
		else
			break;
	}

	theHeap[index] = elem;
}

void SubgoalGraph::ToXYLocPath(std::vector<mapLoc> & mapLocPath, std::vector<xyLoc> & xyLocPath)
{
	xyLocPath.clear();
	for (unsigned int i = 0; i < mapLocPath.size(); i++)
	{
		xyLocPath.push_back(ToXYLoc(mapLocPath[i]));
	}
}
cost SubgoalGraph::SubgoalAStarSearchSimple(subgoalId & start, subgoalId & goal, cost searchLimit, std::vector<subgoalId> & abstractPath)
{
	// If the search counter reached its maximum value, reset it and the generated flags of all the states
	if (search >= MAX_SEARCH)
		ResetSearch();

	search++;
	theHeap.clear();
	theStack.clear();

#ifdef REPLACE_POPPED
	canReplaceTop = false;
#endif

	// Initialize the start state
	generated[start] = search;
	gCost[start] = 0;
	parent[start] = start;
	SetOpen(start);
	AddToOpen(start, HCost(start,goal));
	
	// Initialize the goal state (so that we can compare its g-value for termination)
	generated[goal] = search;
	gCost[goal] = searchLimit;	// If this point is reached, end the search early (assume no path found).
								// Useful for pruning when checking if there is a path of equal length with one subgoal removed
								// Set to infinity if no search limit is defined
								// Search limit means that no state whose f-value is equal to or greater than the search limit is going to be expanded
								// When calling this function, the searchLimit is the actual cost to look for + 1
	SetOpen(goal);
	// DO NOT ADD GOAL TO THE OPEN LIST YET

	subgoalId* successors;
	unsigned int nSuccessors;
	subgoalId sg;

#ifdef USE_STACK
	//heapElement* bestElement = &theHeap[0];
	cost currFCost;

	//while(bestElement && bestElement->fVal < gCost[goal])
	while((!theStack.empty() && gCost[goal] > theStack[0].fVal) || (!theHeap.empty() && gCost[goal] > theHeap[0].fVal))	// 0-1 Expansions per loop
#else
	while(!theHeap.empty() && gCost[goal] > theHeap[0].fVal)	// 0-1 Expansions per loop
#endif
	{
		// Select the element to expand
		#ifdef USE_STACK
			if (!theStack.empty())	// If the stack has elements, expand the top one
			{
				sg = theStack.back().sg;
				currFCost = theStack.back().fVal;
				theStack.pop_back();
			}
			else	// Expand from the heap
			{
				sg = theHeap[0].sg;
				currFCost = theHeap[0].fVal;
				PopMin();
			}
		#else	// We are only using the heap
			sg = theHeap[0].sg;
			PopMin();
		#endif

		if(IsOpen(sg))	// If it has been closed already, don't re-expand it
		{
			// Expand the state

			SetClosed(sg);
			
			// Get the successors
			if (finalized)	// If the graph is finalized, use 'neighbors' to get the successors
			{
				successors = neighbors[sg];
				nSuccessors = nNeighbors[sg];
			}
			else	// Else, use 'edgeVector' to get the successors
			{
				successors = &edgeVector[sg].front();
				nSuccessors = edgeVector[sg].size();
			}

			// Go over all the successors
			for (unsigned int i = 0; i < nSuccessors; i++)
			{
				subgoalId succ = successors[i];
					
				{
					// Generate the successor, if it has not been generated for this search
					if(generated[succ] != search)
					{
						generated[succ] = search;
						gCost[succ] = gCost[sg] + HCost(sg, succ);
						parent[succ] = sg;
						SetOpen(succ);

						// Insert it into the open list
						#ifdef USE_STACK
							cost newFCost = gCost[succ] + HCost(succ,goal);
							if (newFCost == currFCost)
								theStack.push_back(heapElement(succ, newFCost));
							else
								AddToOpen(succ, newFCost);
						#else
							AddToOpen(succ, gCost[succ] + HCost(succ,goal));
						#endif
					}

					// If it was already generated and is not closed
					else if (IsOpen(succ))
					{
						cost newGCost = gCost[sg] + HCost(sg,succ);
						if (newGCost < gCost[succ])
						{
							gCost[succ] = newGCost;
							parent[succ] = sg;

							// Insert it into the open list (may add code to delete the old version with the higher f-value from the open list)
							#ifdef USE_STACK
								cost newFCost = gCost[succ] + HCost(succ,goal);
								if (newFCost == currFCost)
									theStack.push_back(heapElement(succ, newFCost));
								else
									AddToOpen(succ, newFCost);
							#else
								AddToOpen(succ, gCost[succ] + HCost(succ,goal));
							#endif
						}
					}
				}
			}
		}
		#ifdef REPLACE_POPPED
			PopReplacableTop();
		#endif
	}

	if (gCost[goal] != searchLimit)	// If the g-value of the goal has decreased from its initial value of 'searchLimit' (which means a path is found)
	{
		// Follow the parent pointers to extract the path
		abstractPath.clear();
		subgoalId cur = goal;
		while (cur != start)
		{
			abstractPath.push_back(cur);
			cur = parent[cur];
		}

		abstractPath.push_back(cur);
		std::reverse(abstractPath.begin(), abstractPath.end());
		return gCost[goal];
	}
	else
	{
		return INFINITE_COST;
	}
}
cost SubgoalGraph::SubgoalAStarSearch(subgoalId & start, subgoalId & goal, std::vector<subgoalId> & abstractPath)
{
	//static int searchCount = 0;
	//searchCount++;
	//std::cout<<searchCount<<std::endl;

	// If the search counter reached its maximum value, reset it and the generated flags of all the states
	if (search >= MAX_SEARCH)
		ResetSearch();

	search++;
	theHeap.clear();
	theStack.clear();

#ifdef REPLACE_POPPED
	canReplaceTop = false;
#endif

	// Initialize the start state
	generated[start] = search;
	gCost[start] = 0;
	parent[start] = start;
	SetOpen(start);
	AddToOpen(start, HCost(start,goal));

#ifdef SUCCESSOR_PRUNING
	neighborhoodInfo surroundingStart = surroundingObstacles[start];
	surroundingObstacles[start] = 0;	// Set the surrounding obstacles of start to 0, so that nothing is pruned for the first expansion
	incomingDirection[start] = 0;
#endif

#ifdef EARLY_TERMINATE
	cost bestCost = INFINITE_COST;
	subgoalId bestSubgoal = nSubgoals+2;	// such a subgoal does not exist
#else
	// Initialize the goal state (so that we can compare its g-value for termination)
	generated[goal] = search;
	gCost[goal] = INFINITE_COST;

	SetOpen(goal);
	// DO NOT ADD GOAL TO THE OPEN LIST YET
#endif

	#ifdef SG_STATISTICS
		int nHeapExpansions = 0;
		#ifdef USE_STACK
			int nStackExpansions = 0;
		#endif
		int nTouched = 0;
	#endif
	subgoalId sg;

#ifdef EARLY_TERMINATE
#ifdef USE_STACK
	cost currFCost;
	while((!theStack.empty() && bestCost > theStack[0].fVal) || (!theHeap.empty() && bestCost > theHeap[0].fVal))	// 0-1 Expansions per loop
#else
	while(!theHeap.empty() && bestCost > theHeap[0].fVal)	// 0-1 Expansions per loop
#endif

#else

#ifdef USE_STACK
	cost currFCost;
	while((!theStack.empty() && gCost[goal] > theStack[0].fVal) || (!theHeap.empty() && gCost[goal] > theHeap[0].fVal))	// 0-1 Expansions per loop
#else
	while(!theHeap.empty() && gCost[goal] > theHeap[0].fVal)	// 0-1 Expansions per loop
#endif
#endif
	{
		// Select the element to expand
		#ifdef USE_STACK
			if (!theStack.empty())	// If the stack has elements, expand the top one
			{
				sg = theStack.back().sg;
				currFCost = theStack.back().fVal;
				theStack.pop_back();
				#ifdef SG_STATISTICS
					nStackExpansions++;
				#endif
			}
			else	// Expand from the heap
			{
				sg = theHeap[0].sg;
				currFCost = theHeap[0].fVal;
				PopMin();
				#ifdef SG_STATISTICS
					nHeapExpansions++;
				#endif
			}
		#else	// We are only using the heap
			sg = theHeap[0].sg;
			PopMin();
			#ifdef SG_STATISTICS
				nHeapExpansions++;
			#endif
		#endif

		if(IsOpen(sg))	// If it has been closed already, don't re-expand it
		{
			// Expand the state

			SetClosed(sg);

			#ifdef SUCCESSOR_PRUNING
				//uint32_t pruningMask = ((surroundingObstacles[sg] << 4) | incomingDirection[sg]);
				uint32_t pruningMask = surroundingObstacles[sg];
				pruningMask = (pruningMask << 4) | incomingDirection[sg];
				pruningMask = pruningMask << 4;
			#endif

			// Go over all the successors
			for (unsigned int i = 0; i < nNeighbors[sg]; i++)
			{

				subgoalId succ = neighbors[sg][i];
			#ifdef SUCCESSOR_PRUNING
				if (baseSixteenDirections[pruningMask | (uint32_t)edgeDirections[sg][i]])
			#endif
				{
					#ifdef SG_STATISTICS
						nTouched++;
					#endif
					// Generate the successor, if it has not been generated for this search
					if(generated[succ] != search)
					{
						generated[succ] = search;
						gCost[succ] = gCost[sg] + HCost(sg, succ);
						cost newFCost = gCost[succ] + HCost(succ,goal);
						parent[succ] = sg;

						SetOpen(succ);

					#ifdef EARLY_TERMINATE
						if(HasExtraEdge(succ)){
							if (gCost[succ] + distanceToGoal[succ] < bestCost){
								bestCost = gCost[succ] + distanceToGoal[succ];
								bestSubgoal = succ;
							}
						}

						if (newFCost < bestCost)
					#endif
						{
							#ifdef SUCCESSOR_PRUNING
								incomingDirection[succ] = edgeDirections[sg][i];
							#endif
							// Insert it into the open list
							#ifdef USE_STACK
								if (newFCost == currFCost)
									theStack.push_back(heapElement(succ, newFCost));
								else
									AddToOpen(succ, newFCost);
							#else
								AddToOpen(succ, newFCost));
							#endif
						}
					}

					// If it was already generated and is not closed
					else if (IsOpen(succ))
					{
						cost newGCost = gCost[sg] + HCost(sg,succ);
						if (newGCost < gCost[succ])
						{
							gCost[succ] = newGCost;
							cost newFCost = gCost[succ] + HCost(succ,goal);
							parent[succ] = sg;

						#ifdef EARLY_TERMINATE
							if(HasExtraEdge(succ)){
								if (gCost[succ] + distanceToGoal[succ] < bestCost){
									bestCost = gCost[succ] + distanceToGoal[succ];
									bestSubgoal = succ;
								}
							}

							if (newFCost < bestCost)
						#endif
							{
								#ifdef SUCCESSOR_PRUNING
									incomingDirection[succ] = edgeDirections[sg][i];
								#endif

								// Insert it into the open list (may add code to delete the old version with the higher f-value from the open list)
								#ifdef USE_STACK
									if (newFCost == currFCost)
										theStack.push_back(heapElement(succ, newFCost));
									else
										AddToOpen(succ, newFCost);
								#else
									AddToOpen(succ, newFCost);
								#endif
							}
						}
					}
				}
			}
		}
		#ifdef REPLACE_POPPED
			PopReplacableTop();
		#endif
	}


#ifdef SUCCESSOR_PRUNING
	surroundingObstacles[start] = surroundingStart;	// restore the start's surrounding obstacles
#endif

	#ifdef SG_STATISTICS
	#ifdef SG_STATISTICS_PER_SEARCH
		if (finalized)
		{
			//std::cout<<"Heap expansions: "<<nHeapExpansions<<std::endl;
			#ifdef USE_STACK
			//	std::cout<<"Stack expansions: "<<nStackExpansions<<std::endl;
			#endif
			//std::cout<<"Open list size: "<<theHeap.size() + theStack.size()<<std::endl;
			std::cout<<"Number of expansions: "<<nHeapExpansions + nStackExpansions<<std::endl;
		}
	#endif
		nTotalExpansions += nHeapExpansions + nStackExpansions;
		nTotalTouched += nTouched;
		//std::cout<<"Expanded: "<<nHeapExpansions + nStackExpansions<<"\tTouched: "<<nTouched<<std::endl;
	#endif


#ifdef EARLY_TERMINATE
	if (bestCost != INFINITE_COST)
#else
	if (gCost[goal] != INFINITE_COST)	// If the g-value of the goal has decreased from its initial value of 'searchLimit' (which means a path is found)
#endif
	{
#ifdef EARLY_TERMINATE
		while (bestSubgoal != goal){
			subgoalId next = neighbors[bestSubgoal][nNeighbors[bestSubgoal]-1];
			parent[next] = bestSubgoal;
			bestSubgoal = next;
		}
#endif

		// Follow the parent pointers to extract the path
		abstractPath.clear();
		subgoalId cur = goal;
		while (cur != start)
		{
			abstractPath.push_back(cur);
			cur = parent[cur];
		}

		abstractPath.push_back(cur);
		std::reverse(abstractPath.begin(), abstractPath.end());
#ifdef EARLY_TERMINATE
		return bestCost;
#else
		return gCost[goal];
#endif
	}
	else
	{
		//std::cout<<"MAL"<<std::endl;
		return INFINITE_COST;
	}
}
void SubgoalGraph::ConnectStartAndGoalToGraph(
		xyLoc & start, xyLoc & goal, subgoalId & sgStart, subgoalId & sgGoal,
		std::vector<subgoalId> & startDirectHReachableSubgoals, std::vector<subgoalId> & goalDirectHReachableSubgoals)
{
	mapLoc mapStart = ToMapLoc(start);
	mapLoc mapGoal = ToMapLoc(goal);

	#ifdef SG_STATISTICS
		Timer t;
		t.StartTimer();
	#endif

	if (IsSubgoal(mapStart))	// If the start location is already a subgoal, use it
	{
		sgStart = cellInfo[mapStart];
		#ifdef DISPLAY_EXTRA_EDGES
			location[nSubgoals] = INVALID_XYLOC;
		#endif
	}

	else	// Create a new subgoal for it, with id nSubgoals (at the end of all the actual subgoals)
	{
		sgStart = nSubgoals;
		location[sgStart] = start;
		GetDirectHReachableSubgoals(start, startDirectHReachableSubgoals);
		neighbors[sgStart] = &startDirectHReachableSubgoals.front();
		nNeighbors[sgStart] = startDirectHReachableSubgoals.size();

		#ifdef SUCCESSOR_PRUNING
			if (edgeDirections[sgStart] != NULL)
			{
				delete edgeDirections[sgStart];
				edgeDirections[sgStart] = new edgeDirection[nNeighbors[sgStart]+1];
			}

			for (int i = 0; i < nNeighbors[sgStart]; i++){
				edgeDirections[sgStart][i] = GetDirection(sgStart, neighbors[sgStart][i]);
			}

			edgeDirections[sgStart][nNeighbors[sgStart]] = 0;
		#endif

		#ifdef SG_STATISTICS
			nTotalDirectHReachableCalls++;
			nTotalDirectHReachable += nNeighbors[sgStart];
		#endif
	}

	if (IsSubgoal(mapGoal))	// If the goal location is already a subgoal, use it
	{
		sgGoal = cellInfo[mapGoal];
		#ifdef DISPLAY_EXTRA_EDGES
			location[nSubgoals+1] = INVALID_XYLOC;
		#endif
	}

	else	// Create a new subgoal for it, with id nSubgoals + 1 (at the end of all the actual subgoals and start)
	{
		sgGoal = nSubgoals+1;
		location[sgGoal] = goal;
		GetDirectHReachableSubgoals(goal, goalDirectHReachableSubgoals);
		neighbors[sgGoal] = &goalDirectHReachableSubgoals.front();
		nNeighbors[sgGoal] = goalDirectHReachableSubgoals.size();

		#ifdef SUCCESSOR_PRUNING
			if (edgeDirections[sgGoal] != NULL)
			{
				delete edgeDirections[sgGoal];
				edgeDirections[sgGoal] = new edgeDirection[nNeighbors[sgGoal]+1];
			}

			for (int i = 0; i < nNeighbors[sgGoal]; i++){
				edgeDirections[sgGoal][i] = GetDirection(sgGoal, neighbors[sgGoal][i]);
			}

			edgeDirections[sgGoal][nNeighbors[sgGoal]] = 0;
		#endif

		#ifdef SG_STATISTICS
			nTotalDirectHReachableCalls++;
			nTotalDirectHReachable += nNeighbors[sgGoal];
		#endif
	}

	#ifdef SG_STATISTICS
		//double elapsed = t.EndTimer();
		//directHReachableTime += elapsed;
		//initializeTime -= elapsed;
	#endif

	// As a hack for the next step, increase its number of neighbors by 1 (the extra neighbor will never be checked)
	// We do this because the next step avoids checking the extra neighbor by assuming the number of neigbors is -1
	nNeighbors[sgGoal]++;
	SetExtraEdgeFlag(sgGoal);
	if (sgLevel[sgGoal] != 0)
		neighbors[sgGoal][nNeighbors[sgGoal]-1] = sgGoal;
	distanceToGoal[sgGoal] = 0;

	#ifdef PRUNE_WHILE_CONNECTING
		neighborhoodInfo surroundingGoal = surroundingObstacles[sgGoal];
		surroundingObstacles[sgGoal] = 0;
		incomingDirection[sgGoal] = 0;
	#endif
	// Add new edges to the graph, back from the goal, so that the search is optimal
	buckets[sgLevel[sgGoal]].push_back(sgGoal);
	/* Bucket initially contains only the goal
	 * In the first iteration, only edges to higher level subgoals are considered
	 * When a higher level successor of a subgoal in a bucket is found, it is also added to the bucket, if it is not already included
	 * If it is already in the bucket, its g-value and extra edge is updated if the new g-value is lower
	 * A subgoal that is put in a bucket is not removed until the end of the search (that's how we keep track of subgoals with extra edges)
	 *
	 * In the second iteration, we iterate over all the subgoals already placed in the buckets in the first iteration
	 * This time, we expand its local successors
	 * We add these new local-successors to the bucket, but we do not expand them again
	 */

	//std::cout<<"Start level: "<<sgLevel[sgStart]<<std::endl;
	//std::cout<<"Goal level: "<<sgLevel[sgGoal]<<std::endl;
	// First iteration
	for (level l = sgLevel[sgGoal]; l < graphLevel; l++)	// Exclude the graphLevel subgoals, they are only for keeping track of the extra edges, so we can delete them later
	{
		for (unsigned int i = 0; i < buckets[l].size(); i++)
		{
			subgoalId sg1 = buckets[l][i];

			#ifdef PRUNE_WHILE_CONNECTING
				uint32_t pruningMask = surroundingObstacles[sg1];
				pruningMask = (pruningMask << 4) | incomingDirection[sg1];
				pruningMask = pruningMask << 4;
			#endif

			for (unsigned int j = 0; j < nNeighbors[sg1]-1; j++)	// -1 because we do not want to check the extra edge
			{														// Note that we are always iterating the neighbors of a subgoal with an extra edge (including the goal, with the hack above)
				subgoalId sg2 = neighbors[sg1][j];

				#ifdef SG_ASSERTIONS
					assert(sgLevel[sg1] < sgLevel[sg2]);
				#endif

			#ifdef PRUNE_WHILE_CONNECTING
				if (baseSixteenDirections[pruningMask | (uint32_t)edgeDirections[sg1][j]])
			#endif
				{
					if(!HasExtraEdge(sg2))	// Has no other extra edge, just make sg2->sg1 the extra edge
					{
						neighbors[sg2][nNeighbors[sg2]] = sg1;
						#ifdef SUCCESSOR_PRUNING
							edgeDirections[sg2][nNeighbors[sg2]] = (ReverseDirection(edgeDirections[sg1][j]));
						#endif

						#ifdef PRUNE_WHILE_CONNECTING
							incomingDirection[sg2] = edgeDirections[sg1][j];
						#endif

						nNeighbors[sg2]++;
						SetExtraEdgeFlag(sg2);
						distanceToGoal[sg2] = HCost(sg1, sg2) + distanceToGoal[sg1];	// Store the cost of the best known path to the goal
						buckets[sgLevel[sg2]].push_back(sg2);	// Add it to the relevant bucket
					}

					else	// Already has an extra edge, make sg2->sg the extra edge only if it yields a shorter path
					{
						cost newCost = HCost(sg1, sg2) + distanceToGoal[sg1];
						if (newCost < distanceToGoal[sg2])
						{
							neighbors[sg2][nNeighbors[sg2]-1] = sg1;
							#ifdef SUCCESSOR_PRUNING
								edgeDirections[sg2][nNeighbors[sg2]-1] = (ReverseDirection(edgeDirections[sg1][j]));
							#endif

							#ifdef PRUNE_WHILE_CONNECTING
								incomingDirection[sg2] = edgeDirections[sg1][j];
							#endif

							distanceToGoal[sg2] = newCost;
						}
					}
				}
			}
		}
	}

	// Second iteration
	for (level l = sgLevel[sgGoal]<sgLevel[sgStart]?sgLevel[sgStart]:sgLevel[sgGoal]; l < graphLevel; l++)	// We skip the buckets with level < start's level,
	{																										// since lower level local edges are irrelevant
		int maxSize = buckets[l].size();
		for (int i = 0; i < maxSize; i++)
		{
			subgoalId sg1 = buckets[l][i];

			#ifdef PRUNE_WHILE_CONNECTING
				uint32_t pruningMask = surroundingObstacles[sg1];
				pruningMask = (pruningMask << 4) | incomingDirection[sg1];
				pruningMask = pruningMask << 4;
			#endif

			for (unsigned int j = nNeighbors[sg1]; j < nNeighbors[sg1] + nLocalNeighbors[sg1]; j++)	// Iterate over its local edges
			{
				subgoalId sg2 = neighbors[sg1][j];
			#ifdef PRUNE_WHILE_CONNECTING
				if (baseSixteenDirections[pruningMask | (uint32_t)edgeDirections[sg1][j]])
			#endif
				{
					if(!HasExtraEdge(sg2))	// Has no other extra edge, just make sg2->sg1 the extra edge
					{
						neighbors[sg2][nNeighbors[sg2]] = sg1;
						#ifdef SUCCESSOR_PRUNING
							edgeDirections[sg2][nNeighbors[sg2]] = (ReverseDirection(edgeDirections[sg1][j]));
						#endif

						#ifdef PRUNE_WHILE_CONNECTING
							incomingDirection[sg2] = edgeDirections[sg1][j];
						#endif

						nNeighbors[sg2]++;
						SetExtraEdgeFlag(sg2);
						distanceToGoal[sg2] = HCost(sg1, sg2) + distanceToGoal[sg1];	// Store the cost of the best known path to the goal
						buckets[sgLevel[sg2]].push_back(sg2);	// Add it to the relevant bucket
					}

					else	// Already has an extra edge, make sg2->sg the extra edge only if it yields a shorter path
					{
						cost newCost = HCost(sg1, sg2) + distanceToGoal[sg1];
						if (newCost < distanceToGoal[sg2])
						{
							neighbors[sg2][nNeighbors[sg2]-1] = sg1;
							#ifdef SUCCESSOR_PRUNING
								edgeDirections[sg2][nNeighbors[sg2]-1] = (ReverseDirection(edgeDirections[sg1][j]));
							#endif

							#ifdef PRUNE_WHILE_CONNECTING
								incomingDirection[sg2] = edgeDirections[sg1][j];
							#endif

							distanceToGoal[sg2] = newCost;
						}
					}
				}
			}
		}
	}

	#ifdef PRUNE_WHILE_CONNECTING
		surroundingObstacles[sgGoal] = surroundingGoal;
	#endif

	#ifdef SG_STATISTICS
		int nExtraEdges = startDirectHReachableSubgoals.size();
		for (level l = 0; l <= graphLevel; l++)
			nExtraEdges += buckets[l].size();
		nTotalExtraEdges += nExtraEdges;
	#endif
}

cost SubgoalGraph::FindPath(xyLoc & start, xyLoc & goal, std::vector<xyLoc> & thePath)
{
#ifdef SG_STATISTICS
	Timer t;
	t.StartTimer();
#endif
	
	static std::vector<subgoalId> abstractPath;
	static std::vector<subgoalId> localPath;
	static std::vector<mapLoc> mapLocPath;

	// INITIALIZE

	// First, check if there is a direct path between start and goal
	if (IsQuickHReachable(start, goal, mapLocPath))
	{
		ToXYLocPath(mapLocPath, thePath);
		#ifdef SG_STATISTICS
			initializeTime += t.EndTimer();	
			//std::cout<<"Quick path found with cost: "<<HCost(start,goal)/(double)CARD_COST<<std::endl;
		#endif
		return HCost(start,goal);
	}
	else
		mapLocPath.clear();

	// If not, find a high level path between them
	subgoalId sgStart, sgGoal;
	static std::vector<subgoalId> startDirectHReachableSubgoals;
	static std::vector<subgoalId> goalDirectHReachableSubgoals;
	cost abstractCost;
	
	// Add the relevant edges to the graph
	ConnectStartAndGoalToGraph(start, goal, sgStart, sgGoal, startDirectHReachableSubgoals, goalDirectHReachableSubgoals);

#ifdef SG_STATISTICS
	initializeTime += t.EndTimer();
	t.StartTimer();
#endif

	// SEARCH

	// Do an A* search over the modified subgoal graph
	abstractCost = SubgoalAStarSearch(sgStart, sgGoal, abstractPath);

#ifdef DISCARD_FIRST_LEVEL_LOCAL_EDGES
	// Check if there is a shorter path through two local subgoals (which might be missed on pruned graphs)
	cost localCost = TryLocalPair(sgStart, sgGoal, startDirectHReachableSubgoals, goalDirectHReachableSubgoals, localPath);
	if (localCost < abstractCost)
	{
		#ifdef SG_STATISTICS
			std::cout<<"Shorter local path found: "<<localCost/(double)CARD_COST<<" < "<<abstractCost/(double)CARD_COST<<std::endl;
		#endif
		abstractCost = localCost;
		abstractPath = localPath;
	}

	if(abstractCost == INFINITE_COST)	//Still haven't found a path
	{
		// Try to find any local-local connection
		CheckAllLocalPairs(sgStart, sgGoal, startDirectHReachableSubgoals, goalDirectHReachableSubgoals, abstractPath);
	}
#endif

#ifdef SG_STATISTICS
	searchTime += t.EndTimer();
	t.StartTimer();
#endif
	
	// FINALIZE
#ifdef DISPLAY_EXTRA_EDGES
	bucketsDisplay = buckets;
#endif
	// Restore the original graph

	for (level l = 0; l <= graphLevel; l++)
	{
		while(!buckets[l].empty())
		{
			subgoalId sg = buckets[l].back();
			buckets[l].pop_back();
			RemoveExtraEdgeFlag(sg);
			nNeighbors[sg]--;
		}
	}

#ifdef SG_ASSERTIONS
	for (subgoalId sg = 0; sg <= nSubgoals+1; sg++)
		assert(!HasExtraEdge(sg));
#endif

	startDirectHReachableSubgoals.clear();
	goalDirectHReachableSubgoals.clear();
	
	// Find the low level path (path of xyLoc's) from the high level path (path of subgoalId's)
	if(abstractCost != INFINITE_COST)
	{
		xyLoc from, to;

		if (abstractPath.size() > 0)
			to = location[abstractPath[0]];

		mapLocPath.clear();
		mapLocPath.push_back(ToMapLoc(to));

		for (unsigned int i = 1; i < abstractPath.size(); i++)
		{
			from = to;
			to = location[abstractPath[i]];
			if(!IsHReachable(from, to, mapLocPath, true))	// Append
			{
				#ifdef SG_STATISTICS
					std::cout<<"Hmmm! ";
					std::cout<<"From: ("<<from.x<<" "<<from.y<<") ";
					std::cout<<"to: ("<<to.x<<" "<<to.y<<") "<<std::endl;
				#endif
			}
		}
		ToXYLocPath(mapLocPath, thePath);
	}

#ifdef SG_STATISTICS
	finalizeTime += t.EndTimer();
	#ifdef SG_STATISTICS_PER_SEARCH
		std::cout<<"Found a path of length "<<abstractCost/(double)CARD_COST<<" in "<<(initializeTime + searchTime + finalizeTime)*1000<<"ms."<<std::endl;
		std::cout<<"Time spent connecting start and goal to the graph: "<<directHReachableTime*1000<<"ms."<<std::endl;
		std::cout<<"Time spent adding edges: "<<initializeTime*1000<<"ms."<<std::endl;
		std::cout<<"Time spent finding abstract path: "<<searchTime*1000<<"ms."<<std::endl;
		std::cout<<"Time spent finalizing: "<<finalizeTime*1000<<"ms."<<std::endl;
		directHReachableTime = 0;	initializeTime = 0;		searchTime = 0;		finalizeTime = 0;	// reset for the next search. comment out for cumulative statistics
	#endif
#endif
	
	return abstractCost;
}
cost SubgoalGraph::GetPath(xyLoc & start, xyLoc & goal, std::vector<xyLoc> & thePath)
{
#ifdef SG_STATISTICS
	nInstances++;
#endif
	return FindPath(start, goal, thePath);
}


#ifdef DISCARD_FIRST_LEVEL_LOCAL_EDGES
cost SubgoalGraph::TryLocalPair(subgoalId & sgStart, subgoalId & sgGoal,
		std::vector<subgoalId> & startSafeHReachableSubgoals, std::vector<subgoalId> & goalSafeHReachableSubgoals, std::vector<subgoalId> & localPath)
{
	localPath.clear();

	if ((sgStart == nSubgoals || IsPruned(sgStart)) && (sgGoal == nSubgoals + 1 || IsPruned(sgGoal)))
	{
		subgoalId startLocal;
		cost startCost;

		if (sgStart != nSubgoals)	// then start itself is the local subgoal we were looking for
		{
			startLocal = sgStart;
			startCost = 0;
		}
		else	// then start is a newly added subgoal, look through its local neighbors
		{
			startLocal = nSubgoals + 2; // uninitialized
			startCost = INFINITE_COST;

			for (unsigned int i = 0; i < startSafeHReachableSubgoals.size(); i++)
			{
				if (IsPruned(startSafeHReachableSubgoals[i]))
				{
					cost costThrough = HCost(sgStart, startSafeHReachableSubgoals[i]) + HCost(startSafeHReachableSubgoals[i], sgGoal);
					if (costThrough < startCost)
					{
						startLocal = startSafeHReachableSubgoals[i];
						startCost = costThrough;
					}
				}
			}
		}
		if (startLocal == nSubgoals+2)
			return INFINITE_COST;

		subgoalId goalLocal;
		cost goalCost;

		if (sgGoal != nSubgoals + 1)	// then start itself is the local subgoal we were looking for
		{
			goalLocal = sgGoal;
			goalCost = 0;
		}
		else	// then start is a newly added subgoal, look through its local neighbors
		{
			goalLocal = nSubgoals + 2; // uninitialized
			goalCost = INFINITE_COST;

			for (unsigned int i = 0; i < goalSafeHReachableSubgoals.size(); i++)
			{
				if (IsPruned(goalSafeHReachableSubgoals[i]))
				{
					cost costThrough = HCost(sgStart, goalSafeHReachableSubgoals[i]) + HCost(goalSafeHReachableSubgoals[i], sgGoal);
					if (costThrough < goalCost)
					{
						goalLocal = goalSafeHReachableSubgoals[i];
						goalCost = costThrough;
					}
				}
			}
		}

		if (goalLocal == nSubgoals+2)
			return INFINITE_COST;

		if (startLocal == goalLocal)	// then the same path will also be found by the actual subgoal graph search
			return INFINITE_COST;

		if (IsHReachable(location[startLocal], location[goalLocal]))
		//if (IsLookaheadHReachable(location[startLocal], location[goalLocal]))
		{
			cost localCost = HCost(startLocal, goalLocal);
			if (startLocal != sgStart)
			{
				localPath.push_back(sgStart);
				localCost += HCost(sgStart, startLocal);
			}

			localPath.push_back(startLocal);
			localPath.push_back(goalLocal);

			if (goalLocal != sgGoal)
			{
				localPath.push_back(sgGoal);
				localCost += HCost(goalLocal, sgGoal);
			}

			return localCost;
		}
	}

	return INFINITE_COST;
}
cost SubgoalGraph::CheckAllLocalPairs(subgoalId & sgStart, subgoalId & sgGoal,
		std::vector<subgoalId> & startSafeHReachableSubgoals, std::vector<subgoalId> & goalSafeHReachableSubgoals, std::vector<subgoalId> & localPath)
{
	// check for any connecting pair (don't try to find the best one)
	//static std::vector<subgoalId> locals;
	//locals.clear();
	//localPath.clear();
	//cost minCost = INFINITE_COST;

	for (int i = 0; i < startSafeHReachableSubgoals.size(); i++)
		if (IsPruned(startSafeHReachableSubgoals[i]))
		{
			for (int j = 0; j < goalSafeHReachableSubgoals.size(); j++)
			{
				if (IsHReachable(location[goalSafeHReachableSubgoals[j]],location[startSafeHReachableSubgoals[i]]))
				{
					localPath.clear();
					localPath.push_back(sgStart);
					localPath.push_back(startSafeHReachableSubgoals[i]);
					localPath.push_back(goalSafeHReachableSubgoals[j]);
					localPath.push_back(sgGoal);

					return 	HCost(sgStart, startSafeHReachableSubgoals[i]) +
							HCost(startSafeHReachableSubgoals[i], goalSafeHReachableSubgoals[j]) +
							HCost(goalSafeHReachableSubgoals[j], sgGoal);
				}
			}
		}

	return INFINITE_COST;
}
#endif

#ifdef SG_RUNNING_IN_HOG
void SubgoalGraph::OpenGLDraw(const MapEnvironment *env)
{
	double spectrum[graphLevel+1];
	double rspectrum[graphLevel+1];

	//level minLevel = 1;
	level minLevel = graphLevel;


	if (graphLevel == 1)
	{
		spectrum[1] = 1;
		rspectrum[1] = 1;
	}
	else
	{
		for (unsigned int l = 1; l <= graphLevel; l++)
		{
			spectrum[l] = ((l-1)*1.0)/(graphLevel-1);
			rspectrum[graphLevel-l+1] = spectrum[l];
		}
	}

	spectrum[graphLevel] = 1;
	rspectrum[graphLevel] = 0;

	for (subgoalId sg1 = 0; sg1 < nSubgoals; sg1++)
	{
		if (sgLevel[sg1] >= minLevel)
		{
		// Blue for highest level, red for lowest level, spectrum in between
		env->SetColor(rspectrum[sgLevel[sg1]],0,spectrum[sgLevel[sg1]]);
		env->OpenGLDraw(location[sg1]);

			// Forward edges
			for (unsigned int i = 0; i < nNeighbors[sg1]; i++)
			{
				subgoalId sg2 = neighbors[sg1][i];
				env->SetColor(rspectrum[sgLevel[sg1]],0,spectrum[sgLevel[sg1]]);
				env->GLDrawColoredLine(location[sg1], location[sg2]);
			}

			// Local edges
			for (int i = nNeighbors[sg1]+1; i < nNeighbors[sg1] + nLocalNeighbors[sg1] + 1; i++)
			{
				subgoalId sg2 = neighbors[sg1][i];
				env->SetColor(rspectrum[sgLevel[sg1]],0,spectrum[sgLevel[sg1]]);
				env->GLDrawColoredLine(location[sg1], location[sg2]);
			}
		}
	}
#ifdef DISPLAY_EXTRA_EDGES
	if (!(location[nSubgoals] == INVALID_XYLOC))
	{
		env->SetColor(1,1,0);
		env->OpenGLPriorityDraw(location[nSubgoals], 5000);
		// Print the edges of the start
		for (int i = 0; i < nNeighbors[nSubgoals]; i++)
			env->GLDrawColoredLine(location[nSubgoals], location[neighbors[nSubgoals][i]]);
	}

	if (!(location[nSubgoals+1] == INVALID_XYLOC))
	{
		env->SetColor(1,0,1);
		env->OpenGLPriorityDraw(location[nSubgoals+1], 5000);
		// Print the edges of the goal
		for (int i = 0; i < nNeighbors[nSubgoals+1]; i++)
			env->GLDrawColoredLine(location[nSubgoals+1], location[neighbors[nSubgoals+1][i]]);
	}

	env->SetColor(0,1,0);
	// Print all the extraEdges
	for (level l = 1; l <= graphLevel; l++)
	{
		for (int i = 0; i < bucketsDisplay[l].size(); i++)
		{
			subgoalId sg1 = bucketsDisplay[l][i];
			subgoalId sg2 = neighbors[sg1][nNeighbors[sg1]-1];
			env->GLDrawColoredLine(location[sg1], location[sg2]);
		}
	}
#endif

	for (unsigned int i = 0; i < defaultXYPath.size(); i++)
	{
		//std::cout<<i<<std::endl;
		env->SetColor(1,1,0);
		env->OpenGLDraw(defaultXYPath[i]);
		//env->GLDrawColoredLine(location[defaultAbstractPath[i]], location[defaultAbstractPath[i+1]]);
	}
/*/
	for (mapLoc loc = 0; loc < mapSize; loc++)
	{
		if (IsSubgoal(loc))
		{
			env->SetColor(0,0,1);
			env->OpenGLDraw(ToXYLoc(loc));
		}
	}
//*/
}
#endif
void SubgoalGraph::PrintNLevelGraphStatistics()
{
	std::vector<int> subgoals(graphLevel+1);
	std::vector<int> incomingEdges(graphLevel+1);
	std::vector<int> outgoingEdges(graphLevel+1);
	int nEdges = 0;
	int nAscendingEdges = 0;
	int nGlobalEdges = 0;
	int nLocalEdges = 0;

	std::vector<std::vector<int> > edges;


	for (unsigned int l = 0; l <= graphLevel; l++)
	{
		std::vector<int> levelEdges(graphLevel+1);
		edges.push_back(levelEdges);
	}


	for (subgoalId sg1 = 0; sg1 < nSubgoals; sg1++)
	{
		level sg1level = sgLevel[sg1];
		subgoals[sg1level]++;

		std::vector<subgoalId> neighbors = neighborhoodVector[sg1];
		for (unsigned int j = 0; j < neighbors.size(); j++)
		{
			subgoalId sg2 = neighbors[j];
			level sg2level = sgLevel[sg2];
			//cost hCost = HCost(sg1,sg2);
			edges[sg1level].at(sg2level)++;
			outgoingEdges[sg1level]++;
			incomingEdges[sg2level]++;
			nEdges++;
			if (sg1level == graphLevel && sg2level == graphLevel)
				nGlobalEdges++;
			else if (sg1level < sg2level)
				nAscendingEdges++;
			else if (sg1level == sg2level)
				nLocalEdges++;
			else
				std::cout<<"DESCENDING EDGE?"<<std::endl;
		}
	}

	nGlobalEdges = nGlobalEdges/2;	// We want the number of directed edges
	nLocalEdges = nLocalEdges/2;


	ReportIntStatistic("Graph level", graphLevel);
	ReportIntStatistic("Number of global subgoals", subgoals[graphLevel]);
	ReportIntStatistic("Number of global edges (undirected)", nGlobalEdges);
	ReportIntStatistic("Number of local edges (undirected)", nLocalEdges);
	ReportIntStatistic("Number of ascending edges (directed)", nAscendingEdges);
	ReportIntStatistic("Number of edges (total)", nGlobalEdges + nLocalEdges + nAscendingEdges);
	ReportIntStatistic("Number of edges (directed)", nGlobalEdges*2 + nLocalEdges*2 + nAscendingEdges);

	ReportDoubleStatistic("Average branching factor of the global graph", ((double)nGlobalEdges*2)/subgoals[graphLevel]);
	ReportDoubleStatistic("Average branching factor of the whole graph", ((double)(nGlobalEdges+nLocalEdges+nAscendingEdges)*2)/nSubgoals);
	//ReportDoubleStatistic("Average branching factor")

#ifdef SG_VERBOSE
	std::cout<<"--------------------"<<std::endl;
	// Header
	std::cout<<"\t\t\t";
	for (unsigned int i = 1; i <= graphLevel; i++)
	{
		std::cout<<i<<"\t";
	}
	std::cout<<"\ttotal"<<std::endl;

	// Matrix
	for (unsigned int i = 1; i <= graphLevel; i++)
	{
		std::cout<<"level "<<i<<" ("<<subgoals[i]<<")\t\t";
		for (unsigned int j = 1; j <= graphLevel; j++)
		{
			std::cout<<edges[i].at(j)<<"\t";
		}
		std::cout<<"\t"<<outgoingEdges[i]<<std::endl;
	}

	// Totals
	std::cout<<"total ("<<nSubgoals<<")\t\t";
	for (unsigned int i = 1; i <= graphLevel; i++)
	{
		std::cout<<incomingEdges[i]<<"\t";
	}
	std::cout<<"\t"<<nEdges<<std::endl;
	std::cout<<"--------------------"<<std::endl;
#endif
	return;
}

void SubgoalGraph::PrintDimacs(const char *filename)
{
	int nEdges = 0;
	for (subgoalId sg = 0; sg < nSubgoals; sg++)
	{
		nEdges += nNeighbors[sg];
	}

	std::ofstream dimacs;
	dimacs.open(filename);

	dimacs<<"p sp "<<nSubgoals<<" "<<nEdges<<std::endl;

	for (subgoalId from = 0; from < nSubgoals; from++)
	{
		for (int i = 0; i < nNeighbors[from]; i++)
		{
			subgoalId to = neighbors[from][i];
			dimacs<<"a "<<from + 1<<" "<<to + 1<<" "<<HCost(from,to)<<std::endl;
		}
	}

	dimacs.close();
}

#ifdef SUCCESSOR_PRUNING
#define N 0
#define NE 1
#define E 2
#define SE 3
#define S 4
#define SW 5
#define W 6
#define NW 7
edgeDirection SubgoalGraph::GetDirection(xyLoc from, xyLoc to){

	unsigned int dx = (from.x>to.x)?(from.x-to.x):(to.x-from.x);
	unsigned int dy = (from.y>to.y)?(from.y-to.y):(to.y-from.y);
	
	//std::cout<<"dx:"<<dx<<std::endl;
	//std::cout<<"dy:"<<dy<<std::endl;
	
	// There should be a diagonal move iff both x and y positions are different
	bool diag = (dx != 0) && (dy != 0);

	// There should be a cardinal move iff the absolute difference of the x and y locations are not the same
	bool card = dx != dy;
	
	// Figure out in which cardinal direction to move (not important if card = false)
	edgeDirection c = (dx>dy)?((from.x<to.x)?E:W):((from.y<to.y)?S:N);

	// Figure out in which diagonal direction to move (not important if diag = false)
	edgeDirection d = (from.x<to.x)?((from.y<to.y)?SE:NE):((from.y<to.y)?SW:NW);

	// If we can't move diagonally, then the direction should be c
	if (!diag)
		return c;
		
	// If we can't move cardinally, then the direction should be d
	if (!card)
		return d;
			
	// Else, the direction must be a combination of the two

	// If c = N and d = NW, then, direction of movement is N-NW (aka 15)
	if (c == N && d == NW)
		return 15;
	
	// If the cardinal movement comes first in the clockwise direction, add 8 to c, otherwise, add 8 to d (c, d are smaller than 8)
	return c<d?c|8:d|8;
}

void SubgoalGraph::CreateExpansionPruningMap(){
	std::vector<bool> baseEightDirections;
	
	// Create the base
	for (unsigned int obst = 0; obst < 256; obst++){	// 4 bits for obstacles on the corners (NE, SE, SW, NW)

		bool obstNE = (obst & (1<<3));
		bool obstSE = (obst & (1<<2));
		bool obstSW = (obst & (1<<1));
		bool obstNW = (obst & (1));
		
		for (unsigned int parent = 0; parent < 8; parent++){	// 3 bits for the direction from the parent to the current state (000 = N, 001 = NE, ..., 111 = NW)
			for (unsigned int succ = 0; succ < 8; succ++){	// 3 bits for the direction from the current state to the successor (000 = N, 001 = NE, ..., 111 = NW)
			
				// Initially, assume the path from parent to successor through this state cannot be optimal
				bool allow = false;
				
				// If the incoming direction from the parent and the outgoing direction to the successor differ by at most 45 degrees, this can be an optimal path
				unsigned int diff = (parent + 8 - succ) % 8;
				if (diff == 7 || diff == 0 || diff == 1)
					allow = true;
				
				// Difference by 90 degrees is also possible, if there is an obstacle on an adjacent cell that is diagonally away from this one
				if (obstNE && parent ==  W && succ == N) allow = true;
				if (obstNE && parent ==  S && succ == E) allow = true;
				if (obstSE && parent ==  W && succ == S) allow = true;
				if (obstSE && parent ==  N && succ == E) allow = true;
				if (obstSW && parent ==  E && succ == S) allow = true;
				if (obstSW && parent ==  N && succ == W) allow = true;
				if (obstNW && parent ==  E && succ == N) allow = true;
				if (obstNW && parent ==  S && succ == W) allow = true;


				/*
				 * Difference by 90 degrees on diagonal moves is not allowed
				 * For instance, going NE (coming from SW) and then going SE means that there are no obstacles to the SE, S, and SW,
				 * in which case, there is a shorter path going through these cells and not the cell in question
				 *
				 * Difference by 135 degrees is not allowed.
				 * For instance, going N (coming from S) and then going SE means that there are no obstacles to the S and SE,
				 * in which case, there is a shorter path going through these cells and not the cell in question
				 *
				 * Difference by 180 degrees (largest possible) means the agent is going back the way it came from
				 */

				// Disallow if parent or successor direction has an obstacle
				if (obstNE && (parent == SW || succ == NE)) allow = false;
				if (obstSE && (parent == NW || succ == SE)) allow = false;
				if (obstSW && (parent == NE || succ == SW)) allow = false;
				if (obstNW && (parent == SE || succ == NW)) allow = false;
				
				baseEightDirections.push_back(allow);
				//std::cout<<baseEightDirections.back()<<"\t";
			}
			//std::cout<<std::endl;
		}
		//std::cout<<std::endl;
	}
		
	// Fill the matrix
	for (unsigned int obst = 0; obst < 256; obst++){
		for (unsigned int parent = 0; parent < 16; parent++){
			for (unsigned int succ = 0; succ < 16; succ++){
			
				// Get the one or two associated octile directions for the parent
				std::vector<unsigned int> parentDirs;
				if (parent < 8){	// If it is a single direction
					parentDirs.push_back(parent);
				}
				else{	// For instance direction 9 is N-NE, then the two associated directions are N and NE
					parentDirs.push_back(parent - 8);
					parentDirs.push_back((parent - 8 + 1) % 8);
				}
				
				// Get the one or two associated octile directions for the successor
				std::vector<unsigned int> succDirs;
				if (succ < 8){
					succDirs.push_back(succ);
				}
				else{
					succDirs.push_back(succ - 8);
					succDirs.push_back((succ - 8 + 1) % 8);
				}
				
				bool allow = false;
				
				for (unsigned int i = 0; i < parentDirs.size(); i++){
					for (unsigned int j = 0; j < succDirs.size(); j++){
						unsigned int ind = (obst << 6) | (parentDirs[i] << 3) | succDirs[j];
						//std::cout<<ind<<std::endl;
						if (baseEightDirections[ind]){
							allow = true;
						}
					}
				}
				
				if (obst == 0)	// Special case: if all corners are empty, then it can't be a subgoal. This is used to define the neighborhood of the start state, since it does not have a parent
					allow = true;
				
				baseSixteenDirections.push_back(allow);
				//std::cout<<baseSixteenDirections.back()<<"\t";
			}
			//std::cout<<std::endl;
		}
		//std::cout<<std::endl;
	}	
}

neighborhoodInfo SubgoalGraph::GetNeighborhood(xyLoc loc){
	mapLoc mLoc = ToMapLoc(loc);
	neighborhoodInfo neighborhood = 0;
	// 4 bits in the neighborhood: NE, SE, SW, NW

	/*
	// If a cardinal direction is blocked, block two associated diagonal directions (doesn't seem to have any effect)
	if (!IsTraversable(mLoc + deltaMapLoc[W]))
		neighborhood |= 3;	// NW + SW
	if (!IsTraversable(mLoc + deltaMapLoc[S]))
		neighborhood |= 6;
	if (!IsTraversable(mLoc + deltaMapLoc[N]))
		neighborhood |= 9;
	if (!IsTraversable(mLoc + deltaMapLoc[E]))
		neighborhood |= 12;
	//*/

	// If a diagonal direction is blocked, block it in the neighborhood
	if (!IsTraversable(mLoc + deltaMapLoc[NW]))
		neighborhood |= 1;
	if (!IsTraversable(mLoc + deltaMapLoc[SW]))
		neighborhood |= 2;
	if (!IsTraversable(mLoc + deltaMapLoc[SE]))
		neighborhood |= 4;
	if (!IsTraversable(mLoc + deltaMapLoc[NE]))
		neighborhood |= 8;

	return neighborhood;
}
void SubgoalGraph::AddEdgeDirections(){
	edgeDirections = new edgeDirection*[nSubgoals+2];
	for(subgoalId sg = 0; sg < nSubgoals; sg++){
		edgeDirections[sg] = new edgeDirection[nNeighbors[sg] + nLocalNeighbors[sg] + 1];
		for (int i = 0; i < nNeighbors[sg] + nLocalNeighbors[sg] + 1; i++){
			if (i != nNeighbors[sg]){	// The extra edge slot
				//AddEdgeDirection(sg, i);
				edgeDirections[sg][i] = GetDirection(sg, neighbors[sg][i]);
			}
		}
	}
	edgeDirections[nSubgoals] = new edgeDirection[1];
	edgeDirections[nSubgoals + 1] = new edgeDirection[1];
}
void SubgoalGraph::ComputeSurroundingObstacles(){
	for (subgoalId sg = 0; sg < nSubgoals; sg++){
		surroundingObstacles[sg] = (GetNeighborhood(location[sg]));
		//std::cout<<(uint32_t)surroundingObstacles[sg]<<std::endl;
	}
}
#endif
