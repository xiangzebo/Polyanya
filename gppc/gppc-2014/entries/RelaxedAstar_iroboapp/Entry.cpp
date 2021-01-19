#include <deque>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <stdio.h>
#include<iostream>
#include <limits>
#include <limits.h>
#include "Entry.h"

using namespace std;

//std::vector<bool> map;
//std::vector<int> visited;
//std::vector<xyLoc> succ;
int width, height;
//vector <float> g_score_vec;
float** g_score;
uint mapSize;
float value = 0;
float const infinity = std::numeric_limits< float >::infinity();
 const float MOVE_COST = 1.0; //!< horizental or vertical move
 const float DIAGONAL_MOVE_COST = 1.4142; //!< Diagonal move
 const float INFINIT_COST = INT_MAX; //!< cost of non connected nodes

inline int GetIndex(xyLoc s);
inline int GetXcoordinate (int cellIndex);
inline int GetYcoordinate (int cellIndex);
inline bool _compareFCost(cells const &c1, cells const &c2);
inline std::vector <xyLoc> findFreeNeighborCell (bool map[], xyLoc location);
inline bool areTwoCellsDiagonal(int i1, int j1, int i2, int j2);
inline float  getMoveCost(int i1, int j1, int i2, int j2);
float calculateHCost(xyLoc cellID, xyLoc goal);
std::vector<xyLoc> constructPath(bool map[], xyLoc s, xyLoc g);

const char *GetName()
{
	return "Relaxed A*";
}

void PreprocessMap(std::vector<bool> &bits, int width, int height, const char *filename)
{

	//printf("Not writing to file '%s'\n", filename);
	
}

void *PrepareForSearch(std::vector<bool> &bits, int w, int h, const char *filename)
{
	//printf("Not reading from file '%s'\n", filename);

	width = w;
	height = h;
	mapSize = bits.size();
	// static array for the map
	bool* mapArray = new bool [mapSize]; 

	for (uint i=0; i<mapSize; i++){
		mapArray[i]=bits[i];
	}

	g_score = (float**) malloc(sizeof( float*)*mapSize);
	for(int i=0; i<mapSize; i++)
		g_score[i] = (float*) malloc(sizeof(float)*2);

	return mapArray;
}

bool GetPath(void *data, xyLoc s, xyLoc g, std::vector<xyLoc> &path)
{
	value = value+1;
	bool* map = static_cast<bool*>(data);
	cells CP;
	std::vector<cells> OPL;
	float tBreak = 1+1/mapSize;  // coefficient for breaking ties

	//calculate g_score and f_score of the start position
	int startCell = GetIndex(s);
	int goalCell = GetIndex(g);
	g_score[startCell][0]=0;
	g_score[startCell][1]=value;
	int currentCell;
	xyLoc currentCoordinate;
	//add the start cell to the open list
	CP.currentCell=startCell;
	CP.fCost=g_score[startCell][0]+tBreak*calculateHCost(s,g);

	OPL.push_back(CP);
	currentCell=startCell;

	//while the open list is not empty continuie the search or g_score(goalCell) is equal to infinity
	while (!OPL.empty()&& g_score[goalCell][1]!=value) 
	{	  
	    //choose the cell that has the lowest cost fCost in the open list
	    vector<cells>::iterator minCell = min_element(OPL.begin(), OPL.end(), &_compareFCost);
	    cells COfCells=*minCell;
	    currentCell=COfCells.currentCell;
	    //remove the currentCell from the openList
     	    vector<cells>::iterator it1=OPL.erase(minCell);
 
	    //search the neighbors of the current Cell
	    //vector <int> neighborCells;
	    std::vector <xyLoc> neighborCells; 
	    currentCoordinate.x = GetXcoordinate(currentCell);
	    currentCoordinate.y = GetYcoordinate(currentCell);
	    neighborCells=findFreeNeighborCell(map, currentCoordinate);
	   //for each neighbor v of current cell
	    for(uint i=0; i<neighborCells.size(); i++) 
	    {
		xyLoc neighborCoordinate = neighborCells[i];
		int neighborIndex = GetIndex(neighborCoordinate);
	
		  // if the g_score[neighborIndex][1]!=value: unvisited cell
		  if(g_score[neighborIndex][1]!=value)
		  {   
			g_score[neighborIndex][1]=value;
			float moveCost = getMoveCost(currentCoordinate.y ,currentCoordinate.x, neighborCoordinate.y, neighborCoordinate.x);  
			g_score[neighborIndex][0]=g_score[currentCell][0]+ moveCost ;
		   	//add Neighbor Cell To Open List 
			cells CP;
			CP.currentCell=neighborIndex; //insert the neighbor cell
			//calculate fcost
			CP.fCost=g_score[neighborIndex][0]+tBreak*calculateHCost(neighborCoordinate,g);     
			OPL.push_back(CP);   
			}//end if
		    }//end for
		}//end while

	if(g_score[goalCell][1]==value)  // if g_score[goalCell][1]==value : construct path 
		path=constructPath(map,  s, g);
	else
		std::cout <<std::endl<<" Failure to find a path !" << std::endl;
	return true;
}

int GetIndex(xyLoc s)
{
	return s.y*width+s.x;
}

int GetXcoordinate (int cellIndex)
{
	return cellIndex%width;
}
int GetYcoordinate (int cellIndex)
{
	return cellIndex/width;
}

bool _compareFCost(cells const &c1, cells const &c2) 
{ 
  return c1.fCost < c2.fCost; 
}

/*******************************************************************************
 * Function Name: findFreeNeighborCell
 * Inputs: the row and columun of the current Cell
 * Output: a vector of free neighbor cells of the current cell
 * Description:it is used to find the free neighbors Cells of a the current Cell in the grid
 * Check Status: Checked by Anis, Imen and Sahar
*********************************************************************************/
std::vector <xyLoc> findFreeNeighborCell (bool map[], xyLoc location){
 
  int rowID=location.y;
  int colID=location.x;
  xyLoc neighbor;
  int neighborIndex;
  xyLoc neighbor1;
  int neighbor1Index;
  xyLoc neighbor2;
  int neighbor2Index;
  std::vector <xyLoc> freeNeighborCells;
	for (int i=-1;i<=1;i++){
		for (int j=-1; j<=1;j++){
			//check whether the index is valid
			if ((rowID+i>=0)&&(rowID+i<height)&&(colID+j>=0)&&(colID+j<width)){
				neighbor.x = colID+j; 
				neighbor.y = rowID+i;
				neighborIndex = GetIndex (neighbor);
				//check if that neighbor is free cell
				if(map[neighborIndex] && (!(i==0 && j==0))){
					if (areTwoCellsDiagonal(rowID, colID, rowID+i, colID+j)){
						neighbor1.x = colID+j; 
						neighbor1.y = rowID;
						neighbor1Index = GetIndex (neighbor1);
						neighbor2.x = colID; 
						neighbor2.y = rowID+i;
						neighbor2Index = GetIndex (neighbor2);
						if (map[neighbor1Index] && map[neighbor2Index]){
							freeNeighborCells.push_back(neighbor);
						}//end if 4
					}//end if 3
					else{
					freeNeighborCells.push_back(neighbor);
					}//end else
				}//end if 2
			}//end if 1
		} //end for 2
	}//end for 1
return freeNeighborCells;
}

bool areTwoCellsDiagonal(int i1, int j1, int i2, int j2){

	if((j2==j1+1 && i2==i1+1)||(i2==i1-1 && j2==j1+1) ||(i2==i1-1 && j2==j1-1)||(j2==j1-1 && i2==i1+1)){
		return true;
	}
	return false;
}

float  getMoveCost(int i1, int j1, int i2, int j2){
   float moveCost=INFINIT_COST;//start cost with maximum value. Change it to real cost of cells are connected
   //if cell2(i2,j2) exists in the diagonal of cell1(i1,j1)
   if((j2==j1+1 && i2==i1+1)||(i2==i1-1 && j2==j1+1) ||(i2==i1-1 && j2==j1-1)||(j2==j1-1 && i2==i1+1)){
     moveCost = DIAGONAL_MOVE_COST;
   }
    //if cell 2(i2,j2) exists in the horizontal or vertical line with cell1(i1,j1)
   else{
     if ((j2==j1 && i2==i1-1)||(i2==i1 && j2==j1-1)||(i2==i1+1 && j2==j1) ||(i1==i2 && j2==j1+1)){
       moveCost = MOVE_COST;
     }
   }
   return moveCost;
 } 

float calculateHCost(xyLoc cellID, xyLoc goal)
{    
    //manhatten distance for 8 neighbor
    return abs(goal.y-cellID.y)+abs(goal.x-cellID.x);
}

std::vector<xyLoc> constructPath(bool map[], xyLoc s, xyLoc g)
{
  std::vector<xyLoc> bestPath;
  std::vector<xyLoc> path;

  path.insert(path.begin()+bestPath.size(), g);
  int startCell = GetIndex(s);
  xyLoc currentCell = g;

  while(GetIndex(currentCell)!=startCell)
  { 
     std::vector <xyLoc> neighborCells;
     // search the neighbor that has the minimum g_score
     neighborCells=findFreeNeighborCell(map, currentCell);
    
     std::vector <float> gScoresNeighbors;
     for(uint i=0; i<neighborCells.size(); i++)
     {
	int neighborIndex = GetIndex(neighborCells[i]);
	
	if ( g_score[neighborIndex][1] != value )
		g_score[neighborIndex][0]=infinity;

       gScoresNeighbors.push_back(g_score[neighborIndex][0]);
     }
     int posMinGScore=distance(gScoresNeighbors.begin(), min_element(gScoresNeighbors.begin(), gScoresNeighbors.end()));
     currentCell=neighborCells[posMinGScore];

     path.insert(path.begin()+path.size(), currentCell);

  }
  for(uint i=0; i<path.size(); i++){
  	bestPath.insert(bestPath.begin()+bestPath.size(), path[path.size()-(i+1)]);
  }
  return bestPath;
}


