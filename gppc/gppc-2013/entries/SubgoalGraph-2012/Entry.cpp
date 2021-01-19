#include "Entry.h"

const char *GetName()
{
	return "SubgoalGraph-2012";
}

struct entryData {
	bool useSubgoals;
	SubgoalGraph* subgoalGraph;
	QuickBucket* quickBucket;
};

entryData entry;

void PreprocessMap(std::vector<bool> &bits, int width, int height, const char *filename)
{
	// Create and save the subgoal graph
	entry.subgoalGraph = new SubgoalGraph(bits,width,height,filename,MEMORY_LIMIT);
	//entry.subgoalGraph = new SubgoalGraph(bits,width,height,filename,50000000);
	entry.useSubgoals = true;
#ifndef NO_SAVE_LOAD
	delete entry.subgoalGraph;
	entry.subgoalGraph = NULL;
#endif
}

void *PrepareForSearch(std::vector<bool> &bits, int w, int h, const char *filename)
{
#ifndef NO_SAVE_LOAD
	// Read the subgoal graph
	entry.subgoalGraph = new SubgoalGraph(filename);
#endif
	
	// Check if we will be using the subgoal graph (we might not if the graph does not if the memory limit)
	entry.useSubgoals = entry.subgoalGraph->UsingSubgoals();
	
	// If we are not using subgoals, use the bucket implementation
	if (!entry.useSubgoals)
	{
		entry.quickBucket = new QuickBucket();
		entry.quickBucket->LoadMap(bits, w, h);
	}
	return (void *)&entry;
}

bool GetPath(void *data, xyLoc s, xyLoc g, std::vector<xyLoc> &path)
{
	cost c;
	if (entry.useSubgoals)
		c = entry.subgoalGraph->GetPath(s,g,path);
	else
		c = entry.quickBucket->GetPath(s,g,path);
	return true;
}
void CleanUp()
{
	if (entry.useSubgoals)
	{
		entry.subgoalGraph->SearchStatistics();
		entry.subgoalGraph->GetHeapCapacity();
//		delete entry.subgoalGraph;
	}
	else
	{
		entry.quickBucket->GetStatistics();
//		delete entry.quickBucket;
//		delete entry.subgoalGraph;
	}
	
}
