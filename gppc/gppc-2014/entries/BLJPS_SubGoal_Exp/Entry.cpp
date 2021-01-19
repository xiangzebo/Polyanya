#define DIAG_UNBLOCKED
#include "Entry.h"
#include "BL_JPS_SubGoal_Exp.h"

PathFindingAlgorithm * currentAlgorithm=0;
void GetSuccessors(xyLoc s, std::vector<xyLoc> &neighbors);
int GetIndex(xyLoc s);
void ExtractPath(xyLoc end, std::vector<xyLoc> &finalPath);

PathFindingAlgorithm * getAlgorithmType(AlgorithmType algType,std::vector<bool> &bits, char * mapData, int mapWidth, int mapHeight)
{
	if (algType == AT_BL_JPS_SUBGOAL_EXP)
		return (new BL_JPS_SUBGOAL_EXP(bits, mapWidth, mapHeight));
	return 0;
}

const char *GetName()
{
//    if (currentAlgorithm==0)
    //    return "Unitialised";
	return "BL-JPS";
}

void PreprocessMap(std::vector<bool> &bits, int width, int height, const char *filename)
{
   // char * c = new char[bits.size()/8+1];
   // for (int i =0;i<bits.size();i++)
    //    c[i/8]|= (bits[i]!=1?1:0)<<(i&7);


    currentAlgorithm=getAlgorithmType(AT_BL_JPS_SUBGOAL_EXP,bits,NULL,width,height);
    currentAlgorithm->preProcessGrid();
    currentAlgorithm->dumpPreprocessedDataToFile(filename);
	printf("Not writing to file '%s'\n", filename);
}

void *PrepareForSearch(std::vector<bool> &bits, int w, int h, const char *filename)
{
	printf("Not reading from file '%s'\n", filename);

	//map = bits;
	//width = w;
	//height = h;
    if (currentAlgorithm==0)
	{
	    //char * c = new char[bits.size()/8+1];
       // for (int i =0;i<bits.size();i++)
        //    c[i/8]|= (bits[i]!=1?1:0)<<(i&7);
        currentAlgorithm=getAlgorithmType(AT_BL_JPS_SUBGOAL_EXP,bits,NULL,w,h);
        currentAlgorithm->readPreprocessedDataToFile(filename);
    }
	return (void *)13182;
}

bool GetPath(void *data, xyLoc s, xyLoc g, std::vector<xyLoc> &path)
{

    currentAlgorithm->findSolution((int)g.x,(int)g.y,(int)s.x,(int)s.y,path);
	/*for (int i =0;i<((int)path.size())-1;i++)
	{
            int yDif = path[i+1].y-path[i].y;
            int xDif = path[i+1].x-path[i].x;
            int yStep = yDif==0?0:(yDif<0?-1:1);
            int xStep = xDif==0?0:(xDif<0?-1:1);
            yDif= abs(yDif);
            xDif= abs(xDif);
            int stepRange = max(yDif,xDif);
            for (int j =1;j<stepRange;j++)
                path.insert(path.begin()+(i+j),xyLoc(path[i].x+xStep*j,path[i].y+yStep*j));

            i+=stepRange-1;
	}*/

    return true;
}

