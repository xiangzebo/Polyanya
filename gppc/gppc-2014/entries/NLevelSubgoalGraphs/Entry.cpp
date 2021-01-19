#include "Entry.h"
#include <string>

const char *GetName()
{
	return "NLevelSubgoalGraph";
}


void PreprocessMap(std::vector<bool> &bits, int width, int height, const char *filename)
{
	// Create and save the subgoal graph
	SubgoalGraph* subgoalGraph;
	subgoalGraph = new SubgoalGraph(bits,width,height);
	subgoalGraph->SaveGraph(filename);
	delete subgoalGraph;
	subgoalGraph = NULL;
}

void *PrepareForSearch(std::vector<bool> &bits, int w, int h, const char *filename)
{
	// Read the subgoal graph
	SubgoalGraph* subgoalGraph;
	subgoalGraph = new SubgoalGraph(filename);	
	return (void *)subgoalGraph;
}

bool GetPath(void *data, xyLoc s, xyLoc g, std::vector<xyLoc> &path)
{
	SubgoalGraph* subgoalGraph = (SubgoalGraph *) data;
	subgoalGraph->GetPath(s,g,path);
	return true;
}

