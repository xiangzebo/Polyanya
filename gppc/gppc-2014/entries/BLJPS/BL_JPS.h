#pragma once
#include <string.h>
#include "Node.h"
#include "PathFindingAlgorithm.h"
#include "binaryHeap.h"
using namespace std;
//#define DIAG_UNBLOCKED

#ifdef DIAG_UNBLOCKED
#define BL_JPS_OFFSET 1
#define BLJPS_ALG_NAME "BLJPS_UNBLOCKED"
#else
#define BL_JPS_OFFSET 1
#define BLJPS_ALG_NAME "BLJPS_BLOCKED"
#endif
//Directions
// N, NE, E, SE, S, SW, W, NW , StartPosition
#define NO_DIRECTION 8
class BL_JPS: public  PathFindingAlgorithm
{
	private:
		//Special container classes that help with general allocation of memory and search
		NodeContainer nodesC;
		BinaryHeap openListBh;

		//Table of flags indicating is a map location has been searched (Closed List)
		char *testedGrid;

		//Boundary lookup tables for the x and y axis
		vector<vector<short> > xBoundaryPoints,yBoundaryPoints;
		//vector<vector<short>> xBoundaryPointsBackup,yBoundaryPointsBackup;

		//Map data
		char * gridData;
		int gridWidth, gridHeight;

		//Goal node position and index
		int eX,eY,endIndex;


		bool inBounds(const int index)
		{
			return index<gridWidth*gridHeight&&index>=0;
		}
		int gridIndex(const Coordinate &c)
		{
			if (c.x<0 || c.x>=gridWidth || c.y<0 || c.y>=gridHeight)
				return -1;
			return (c.y*gridWidth)+c.x;
		}

		Coordinate nextCoordinate(const Coordinate& c,const int dir)
		{
			static char dirMov[]={0,-1,1,-1,1,0,1,1,0,1,-1,1,-1,0,-1,-1,0,0};
			return Coordinate(c.x+dirMov[dir*2],c.y+dirMov[dir*2+1]);
		}

		int dirIsDiagonal (const int dir)
		{
			return (dir % 2) != 0;
		}
		inline int implies (const int a,const int b)
		{
			return a ? b : 1;
		}
		inline unsigned char addDirectionToSet (const unsigned char dirs,const int dir)
		{
			return dirs | 1 << dir;
		}
		unsigned char forcedNeighbours (const Coordinate &coord, const int dir)
		{
			if (dir > 7)
				return 0;

			unsigned char dirs = 0;
		#define ENTERABLE(n) isPassable ( nextCoordinate (coord, (dir + (n)) % 8))
			if (dirIsDiagonal (dir)) {
				if (!implies (ENTERABLE (6), ENTERABLE (5)))
					dirs = addDirectionToSet (dirs, (dir + 6) % 8);
				if (!implies (ENTERABLE (2), ENTERABLE (3)))
					dirs = addDirectionToSet (dirs, (dir + 2) % 8);

			}
			else {
#ifdef DIAG_UNBLOCKED
				if (!implies(ENTERABLE(2), ENTERABLE(3)))
				{
					dirs = addDirectionToSet(dirs, (dir + 2) % 8);
					dirs = addDirectionToSet(dirs, (dir + 1) % 8);
				}
				if (!implies(ENTERABLE(6), ENTERABLE(5)))
				{
					dirs = addDirectionToSet(dirs, (dir + 6) % 8);
					dirs = addDirectionToSet(dirs, (dir + 7) % 8);
				}
#else
				if (!implies(ENTERABLE(7), ENTERABLE(6)))
					dirs = addDirectionToSet(dirs, (dir + 7) % 8);
				if (!implies(ENTERABLE(1), ENTERABLE(2)))
					dirs = addDirectionToSet(dirs, (dir + 1) % 8);
#endif
			}
			#undef ENTERABLE

			return dirs;
		}

	public:

		bool isCoordinateBlocked(const Coordinate &c)
		{
			return isPassable(c);
		}
		void flushReProcess()
		{
			for (int y =0;y<gridHeight;y++)
			{
				if (xBoundaryPoints[y].size()==0)
				{
					bool currentPassable=false;
					xBoundaryPoints[y].clear();
					for (int x =0;x<gridWidth;x++)
					{
						if (isRawPassable(Coordinate(x,y))!= currentPassable)
						{
							xBoundaryPoints[y].push_back(x);
							currentPassable=!currentPassable;
						}
					}
					//if (currentPassable)
						xBoundaryPoints[y].push_back(gridWidth);
				}
			}

			for (int x =0;x<gridWidth;x++)
			{
				if (yBoundaryPoints[x].size()==0)
				{
					bool currentPassable=false;
					yBoundaryPoints[x].clear();

					for (int y =0;y<gridHeight ;y++)
					{
						if (isRawPassable(Coordinate(x,y))!= currentPassable)
						{
							yBoundaryPoints[x].push_back(y);
							currentPassable=!currentPassable;
						}
					}
					//if (currentPassable)
						yBoundaryPoints[x].push_back(gridHeight);
				}
			}

		}
		void reProcessGrid(int lx,int rx,int ty,int by)
		{
			for (int y =ty;y<by;y++)
				xBoundaryPoints[y].clear();

			for (int x =lx;x<rx;x++)
				yBoundaryPoints[x].clear();
		}
		void backupPreProcess()
		{
			//xBoundaryPointsBackup=xBoundaryPoints;
			//yBoundaryPointsBackup=yBoundaryPoints;
		}
		void useBackupData()
		{
			//xBoundaryPoints=xBoundaryPointsBackup;
			//yBoundaryPoints=yBoundaryPointsBackup;
		}
		void dumpPreprocessedDataToFile(const char * fileName)
		{
			FILE * fp = fopen(fileName,"wb");
			if (fp==0)
			{
                printf("Unable to open preprocessing file %s\n",fileName);
                return;
			}
			for (int y =0;y<gridHeight;y++)
			{
				short numBoundaries = xBoundaryPoints[y].size()-1;
				fwrite(&numBoundaries,2,1,fp);
				for (int i =0;i<xBoundaryPoints[y].size()-1;i++)
					fwrite(&xBoundaryPoints[y][i],2,1,fp);
			}
			for (int x =0;x<gridWidth;x++)
			{
				short numBoundaries = yBoundaryPoints[x].size()-1;
				fwrite(&numBoundaries,2,1,fp);
				for (int i =0;i<yBoundaryPoints[x].size()-1;i++)
					fwrite(&yBoundaryPoints[x][i],2,1,fp);
			}
			fclose(fp);
		}
		void readPreprocessedDataToFile(const char * fileName)
		{
			FILE * fp = fopen(fileName,"rb");
			if (fp==0)
			{
                printf("Unable to open preprocessing file %s\n",fileName);
                return;
			}
			for (int y =0;y<gridHeight;y++)
			{
				xBoundaryPoints.push_back(vector<short>());
				short numBoundaries = 0;
				fread(&numBoundaries,2,1,fp);
				for (int i =0;i<numBoundaries;i++)
				{
					short tempVal = 0;
					fread(&tempVal,2,1,fp);
					xBoundaryPoints[y].push_back(tempVal);
				}
				xBoundaryPoints[y].push_back(gridWidth);
			}
			for (int x =0;x<gridWidth;x++)
			{
				yBoundaryPoints.push_back(vector<short>());
				short numBoundaries = 0;
				fread(&numBoundaries,2,1,fp);
				for (int i =0;i<numBoundaries;i++)
				{
					short tempVal = 0;
					fread(&tempVal,2,1,fp);
					yBoundaryPoints[x].push_back(tempVal);
				}
				yBoundaryPoints[x].push_back(gridHeight);
			}
			fclose(fp);
		}
		void preProcessGrid()
		{
			preprocessedData=true;
			for (int y =0;y<gridHeight;y++)
			{
				bool currentPassable=false;
				xBoundaryPoints.push_back(vector<short>());
				for (int x =0;x<gridWidth;x++)
				{
					if (isRawPassable(Coordinate(x,y))!= currentPassable)
					{
						xBoundaryPoints[y].push_back(x);
						currentPassable=!currentPassable;
					}
				}
				//if (currentPassable)
					xBoundaryPoints[y].push_back(gridWidth);
			}

			for (int x =0;x<gridWidth;x++)
			{
				bool currentPassable=false;
				yBoundaryPoints.push_back(vector<short>());
				for (int y =0;y<gridHeight ;y++)
				{
					if (isRawPassable(Coordinate(x,y))!= currentPassable)
					{
						yBoundaryPoints[x].push_back(y);
						currentPassable=!currentPassable;
					}
				}
				//if (currentPassable)
					yBoundaryPoints[x].push_back(gridHeight);
			}

		}


		const Coordinate indexToCoordinate(const int index)
		{
			return Coordinate(index%gridWidth,index/gridWidth );
		}
		bool isRawPassable(const Coordinate &c)
		{
			return isPassable(c);
		}

		bool isPassable(const Coordinate &c)
		{
			int index = gridIndex(c);
			if (index == -1)
				return false;
			return  !(gridData[index/8]&(1<<(index%8)));
		}
		int getGridWidth()
		{
			return gridWidth;
		}
		int getGridHeight()
		{
			return gridHeight;
		}
		BL_JPS(char * grid, int width, int height) : PathFindingAlgorithm(BLJPS_ALG_NAME, AT_BL_JPS)
		{
			gridData=grid;
			gridWidth=width;
			gridHeight=height;
			testedGrid=new char[gridWidth*gridHeight/8+1];
		}
		~BL_JPS()
		{
			delete [] testedGrid;
		}
		unsigned char naturalNeighbours (const int dir)
		{
			if (dir == NO_DIRECTION)
				return 255;

			unsigned char dirs = 0;
			dirs = addDirectionToSet (dirs, dir);
			if (dirIsDiagonal (dir)) {
				dirs = addDirectionToSet (dirs, (dir + 1) % 8);
				dirs = addDirectionToSet (dirs, (dir + 7) % 8);
			}
			return dirs;
		}


		void setChecked(const int index)
		{
			testedGrid[index/8]= testedGrid[index/8]|(1<<(index%8));
		}
		bool isChecked(const int index)
		{
			return (testedGrid[index/8]&(1<<(index%8)))>0;
		}




		void findSolution(int sX,int sY,int _eX,int _eY, vector<Coordinate> & sol)
		{
			eX = _eX;
			eY = _eY;

			sol.clear();

			endIndex=gridIndex(Coordinate(eX,eY));
            bool sB=isPassable(Coordinate(sX,sY));
            bool dB=isPassable(Coordinate(eX,eY));
			if (!(sX>=0 && sX<gridWidth &&
				sY>=0 && sY<gridHeight &&
				eX>=0 && eX<gridWidth &&
				eY>=0 && eY<gridHeight &&
				isPassable(Coordinate(sX,sY)) && isPassable(Coordinate(eX,eY))
				))
			{
				return;
			}
			if (sX==eX && sY==eY)
				return ;



			//Ready the openlist, closed list and node container for search
			openListBh.clear();
			memset(testedGrid, 0, gridWidth*gridHeight / 8 + 1);
			nodesC.reset();

			//Insert the start nodes into the openList
			Node* startNode = nodesC.getNewNode(Coordinate(sX, sY), eX, eY, 8, 0);
			openListBh.Insert(startNode);
			setChecked(gridIndex(startNode->pos));

			//Keep iterating over openlist until a solution is found or list is empty
			while (openListBh.Count())
			{
				Node* currentNode=openListBh.PopMax();
				unsigned char dirs = forcedNeighbours (currentNode->pos, currentNode->dir)  | naturalNeighbours (currentNode->dir);

				for (int dir = 0; dir < 8; dir ++)
				{
					if ((1<<dir)&dirs)
					{
						int index =jumpNew(currentNode->pos,dir);

						if (inBounds(index))
						{
							Coordinate CoordinateNewC= indexToCoordinate(index);
							if (index==endIndex)
							{
								Coordinate end(eX,eY);
								sol.push_back(end);
								Node*solutionNode=currentNode;
								while (solutionNode)
								{
									sol.push_back(solutionNode->pos);
									solutionNode= solutionNode->parent;
								}
								return;
							}

							if (!isChecked(index) )
							{
								openListBh.Insert(nodesC.getNewNode(CoordinateNewC,eX,eY,dir,currentNode));
								setChecked(index);
							}
							else
							{
								Node * t =nodesC.getNewNode(CoordinateNewC,eX,eY,dir,currentNode);
								openListBh.InsertSmaller(t);
							}
						}
					}
				}
			}
		}


		short binarySearch(const vector<short> & v,short val)
		{
			short l = 0, r = v.size()-1;
			short index = r / 2;
			while (1)
			{
				if (v[index] <= val && v[index + 1] > val)
					return  index;
				if (v[index] > val)
					r = index - 1;
				else
					l = index + 1;

				index = l + (r - l) / 2;

			}
			return -1;
		}

		pair<short, short> getEastEndPoshortReOpen(short x, short y)
		{
			if (y<0 || y >= gridHeight)
				return pair<short, short>(gridWidth, gridWidth);

			if (xBoundaryPoints[y][0]>x)
				return pair<short, short>(xBoundaryPoints[y][0], xBoundaryPoints[y][0]);

			short i = binarySearch(xBoundaryPoints[y], x);
			if (i % 2)
				return pair<short, short>(xBoundaryPoints[y][i + 1], xBoundaryPoints[y][i + 1]);
			else
				return pair<short, short>(xBoundaryPoints[y][i + 1] - 1, i + 2 < ((int)xBoundaryPoints[y].size()) ? xBoundaryPoints[y][i + 2] : gridWidth);
		}
		pair<short,short> getWestEndPoshortReOpen(short x,short y)
		{
			if (y<0 || y>=gridHeight)
				return pair<short,short>(-1,-1);

			if (xBoundaryPoints[y][0]>x)
				return pair<short,short> (-1,-1);

			short i = binarySearch(xBoundaryPoints[y], x);
			if (i % 2)
				return pair<short, short>(xBoundaryPoints[y][i] - 1, xBoundaryPoints[y][i] - 1);
			else
				return pair<short, short>(xBoundaryPoints[y][i], i - 1 < 0 ? -1:xBoundaryPoints[y][i - 1]-1);
		}

		pair<short,short> getSouthEndPoshortReOpen(short x,short y)
		{
			if (x<0 || x>=gridWidth)
				return pair<short,short>(gridHeight,gridHeight);

			if (yBoundaryPoints[x][0]>y)
				return pair<short,short> (yBoundaryPoints[x][0],yBoundaryPoints[x][0]);

			short i = binarySearch(yBoundaryPoints[x], y);
			if (i % 2)
				return pair<short, short>(yBoundaryPoints[x][i + 1], yBoundaryPoints[x][i + 1]);
			else
				return pair<short, short>(yBoundaryPoints[x][i + 1] - 1, i + 2<yBoundaryPoints[x].size() ? yBoundaryPoints[x][i + 2] : gridHeight);

		}
		pair<short,short> getNorthEndPointReOpen(short x,short y)
		{
			if (x<0 || x>=gridWidth)
				return pair<short,short>(-1,-1);

			if (yBoundaryPoints[x][0]>y)
				return pair<short,short> (-1,-1);

			short i = binarySearch(yBoundaryPoints[x], y);
			if (i % 2)
				return pair<short, short>(yBoundaryPoints[x][i] - 1, yBoundaryPoints[x][i] - 1);
			else
				return pair<short, short>(yBoundaryPoints[x][i], i - 1 < 0 ? -1 : yBoundaryPoints[x][i - 1] - 1);
		}

		bool getJumpPointNew(Coordinate s,const char direction, Coordinate & jp)
		{
			//if (!isPassable(s))
			//	return false;
#ifndef DIAG_UNBLOCKED

			s=nextCoordinate(s,direction);
#endif
			if (!isPassable(s))
				return false;
			bool ret = false;
			pair<short, short> up, center, down;
			switch (direction)
			{

				case 0://North
					up = getNorthEndPointReOpen(s.x - 1, s.y);
					center = getNorthEndPointReOpen(s.x, s.y);
					down = getNorthEndPointReOpen(s.x + 1, s.y);

					if (s.x == eX && s.y >= eY && center.first <= eY)
					{
						jp = Coordinate(eX, eY);
						return true;
					}
					if (down.first != -1 && ((down.second>-1 && down.first > center.first && down.second + 2>center.first) || (down.first == down.second && down.first + 2>center.first)))
					{
						jp = Coordinate(s.x, down.second + BL_JPS_OFFSET);
						ret = true;
					}
					if (up.first != -1 && ((up.second>-1 && up.first>center.first&&up.second + 2>center.first) || (up.first == up.second && up.first + 2>center.first)))
					{
						jp = Coordinate(s.x, ret ? max(jp.y, (short)(up.second + BL_JPS_OFFSET)) : up.second + BL_JPS_OFFSET);
						return true;
					}
					return ret;
				case 2://EAST
					up = getEastEndPoshortReOpen(s.x, s.y - 1);
					center = getEastEndPoshortReOpen(s.x, s.y);
					down = getEastEndPoshortReOpen(s.x, s.y + 1);

					if (s.y == eY && s.x <= eX && center.first >= eX)
					{
						jp = Coordinate(eX, eY);
						return true;
					}

					if (down.first != gridWidth && ((down.second<gridWidth&&down.first < center.first && down.second - 2<center.first) || (down.first == down.second && down.first - 2<center.first)))
					{
						jp = Coordinate(down.second - BL_JPS_OFFSET, s.y);
						ret = true;
					}
					if (up.first != gridWidth && ((up.second<gridWidth&&up.first<center.first&&up.second - 2<center.first) || (up.first == up.second && up.first - 2<center.first)))
					{
						jp = Coordinate(ret ? min(jp.x, (short)(up.second - BL_JPS_OFFSET)) : up.second - BL_JPS_OFFSET, s.y);
						return true;
					}
					return ret;
				case 4://SOUTH
					up = getSouthEndPoshortReOpen(s.x - 1, s.y);
					center = getSouthEndPoshortReOpen(s.x, s.y);
					down = getSouthEndPoshortReOpen(s.x + 1, s.y);

					if (s.x == eX && s.y <= eY && center.first >= eY)
					{
						jp = Coordinate(eX, eY);
						return true;
					}
					if (down.first != gridHeight && ((down.second<gridHeight&& down.first < center.first && down.second - 2<center.first) || (down.first == down.second && down.first - 2<center.first)))
					{
						jp = Coordinate(s.x, down.second - BL_JPS_OFFSET);
						ret = true;
					}
					if (up.first != gridHeight && ((up.second<gridHeight&&up.first<center.first&&up.second - 2<center.first) || (up.first == up.second && up.first - 2<center.first)))
					{
						jp = Coordinate(s.x, ret ? min(jp.y, (short)(up.second - BL_JPS_OFFSET)) : up.second - BL_JPS_OFFSET);
						return true;
					}
					return ret;
				case 6://WEST
					up = getWestEndPoshortReOpen(s.x, s.y - 1);
					center = getWestEndPoshortReOpen(s.x, s.y);
					down = getWestEndPoshortReOpen(s.x, s.y + 1);

					if (s.y == eY && s.x >= eX && center.first <= eX)
					{
						jp = Coordinate(eX, eY);
						return true;
					}
					if (down.first != -1 && ((down.second>-1 && down.first > center.first && down.second + 2>center.first) || (down.first == down.second && down.first + 2>center.first)))
					{
						jp = Coordinate(down.second + BL_JPS_OFFSET, s.y);
						ret = true;
					}
					if (up.first != -1 && ((up.second>-1 && up.first>center.first&&up.second + 2>center.first) || (up.first == up.second && up.first + 2>center.first)))
					{
						jp = Coordinate(ret ? max(jp.x, (short)(up.second + BL_JPS_OFFSET)) : up.second + BL_JPS_OFFSET, s.y);
						return true;
					}
					return ret;
			}
			return ret;

		}

		int jumpNew(const Coordinate &c,const char dir)
		{

			#ifdef DIAG_UNBLOCKED
				Coordinate lastC = c;
			#endif
			Coordinate nc = nextCoordinate(c,dir);
			bool isDiag = dirIsDiagonal(dir);
			Coordinate offset(0,0);
			offset = nextCoordinate(offset,dir);

			while (1)
			{
				bool b = true;
#ifdef DIAG_UNBLOCKED
				if (dir & 1)
				{
					Coordinate nextC2 = nextCoordinate(lastC, (dir + 1) % 8);
					Coordinate nextC3 = nextCoordinate(lastC, (dir + 7) % 8);
					int index2 = gridIndex(nextC2);
					int index3 = gridIndex(nextC3);
					b = inBounds(index2) && inBounds(index3) && isPassable(nextC2) && isPassable(nextC3);
				}
#endif
				if (!isPassable(nc) || !b)
					return -1;
				int index  = gridIndex(nc);
				if (forcedNeighbours(nc,dir) ||endIndex==index)
					return index;
				if (isDiag)
				{
					Coordinate newP(-1,-1);
					if (getJumpPointNew(nc,(dir+7)%8,newP) )
						return index;
					if (getJumpPointNew(nc,(dir+1)%8,newP) )
						return index;
				}
				else
				{
					Coordinate newP(-1,-1);
					getJumpPointNew(nc,dir,newP);
					return gridIndex(newP);
				}
				#ifdef DIAG_UNBLOCKED
					lastC = nc;
				#endif
				nc.add(offset);

			}
			return -1;
		}



};
