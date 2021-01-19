#pragma once
//#define DIAG_UNBLOCKED
#include "Node.h"
#include "Entry.h"
using namespace std;
#include <vector>
#include <string>
enum AlgorithmType
{
	AT_ASTAR = 1,
	AT_JPS = 2,
	AT_BL_JPS = 4,
	AT_JPS_PLUS = 8,
	AT_BL_JPS_PLUS = 16,
	AT_BL_JPS_EXP = 32,
	AT_BL_JPS_EXP2=64,
	AT_BL_JPS_EXP3 = 128,
	AT_BL_JPS_EXP4 = 256,
	AT_JPS_BJ = 512,
	AT_BL_JPS_PLUS_SUBGOAL = 1024,
    AT_BL_JPS_SUBGOAL_EXP = 2048,

	AT_ALL=0xFFFF,
};

class PathFindingAlgorithm
{
	private:
		std::string algorithmName;
		AlgorithmType algorithmType;
	protected:
		bool preprocessedData;
	public:
		PathFindingAlgorithm(std::string _algorithmName,AlgorithmType id)
		{
			algorithmName=_algorithmName;
			algorithmType=id;
			preprocessedData=false;
		}
		virtual ~PathFindingAlgorithm()
		{
		}
		virtual void reProcessGrid(int lx,int rx,int ty,int by)
		{
		}
		virtual void preProcessGrid()
		{
			preprocessedData=true;
		}
		virtual void flushReProcess()
		{
		}
		virtual void dumpPreprocessedDataToFile(const char * fileName)
		{
		}
		virtual void readPreprocessedDataToFile(const char * fileName)
		{
		}
		virtual int returnMemorySize()
		{
			return 0;
		}
		bool hasPreprocessedMap()
		{
			return preprocessedData;
		}
		virtual void backupPreProcess()=0;
		virtual void useBackupData()=0;
		virtual void findSolution(int sX,int sY,int _eX,int _eY, std::vector<Coordinate> & sol)=0;
		virtual bool isCoordinateBlocked(const Coordinate &c)=0;
		const char * getAlgorithmName()
		{
			return algorithmName.c_str();
		}
		AlgorithmType getAgorithmType()
		{
			return algorithmType;
		}
		virtual int getGridWidth()=0;
		virtual int getGridHeight()=0;
};
