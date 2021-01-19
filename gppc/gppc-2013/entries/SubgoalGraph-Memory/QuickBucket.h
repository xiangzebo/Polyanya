#ifndef QUICK_BUCKET_H
#define QUICK_BUCKET_H

#include <deque>
#include <queue>
#include <vector>
//#include <ext/hash_map>
#include "Timer.h"
#include <string>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <stdlib.h>
#include <limits>
#include <algorithm>
#include "SubgoalDefinitions.h"

//#define BUCKET_STATISTICS
#define BUCKET_MEMORY_OPTIMIZATION
#define DEFAULT_BUCKET_SIZE 64

typedef uint32_t cost;
#define INFINITE_COST std::numeric_limits<cost>::max()
/*
#define CARD_COST 70
#define DIAG_COST 99
#define DIAG_DIFF 29
#define A_COST 41
#define B_COST 58
*/
#define CARD_COST 2378
#define DIAG_COST 3363
#define DIAG_DIFF 985
#define A_COST 1393
#define B_COST 1970

// 0: N, 1: NE, 2:E,..

static int X[16] = { 0, 1, 1, 1, 0,-1,-1,-1, 0, 1, 1, 1, 0,-1,-1,-1};
static int Y[16] = {-1,-1, 0, 1, 1, 1, 0,-1,-1,-1, 0, 1, 1, 1, 0,-1};
static int A[6] = {0, 1, 0, 1, 2, 2};
static int B[6] = {0, 0, 1, 1, 1, 2};

struct cdCost {	// cost in terms of cardinal and diagonal moves
public:
	cdCost() {}
	cdCost(int16_t _c, int16_t _d) :c(_c), d(_d) {}
	cdCost operator+(const cdCost& rhs){return cdCost(c+rhs.c, d+rhs.d);}
	cdCost operator-(const cdCost& rhs){return cdCost(c-rhs.c, d-rhs.d);}
	
	int16_t c;
	int16_t d;
};

struct abCost {	// cost in terms of a and b where a = 2c-d and b = 2d-2c
public:
	abCost() {}
	abCost(int16_t _a, int16_t _b) :a(_a), b(_b) {}
	abCost(cdCost cost) {a = cost.c + cost.d; b = (cost.c >> 1) + cost.d;}
	abCost operator+(const abCost& rhs){return abCost(a+rhs.a, b+rhs.b);}
	abCost operator-(const abCost& rhs){return abCost(a-rhs.a, b-rhs.b);}
	
	cost GetCost() const {
		return a*A_COST + b*B_COST;
	}
	int16_t a;
	int16_t b;
};

struct abCostComp
{
    bool operator()(abCost const& c1, abCost const& c2) const {
        return c1.GetCost() > c2.GetCost();
    }
};

struct mapCell {
public:
	mapCell() {}
	
	bool open;
	uint16_t generated;
	unsigned char parent;
	unsigned char neighbors;
};

struct bucketElement {
public:
	bucketElement() {}
	bucketElement(xyLoc _loc, int16_t _parent) :loc(_loc), parent(_parent) {}
	
	xyLoc loc;
	unsigned char parent;
};

struct bucket {
public:
	bucket() {}

	bool open;
	std::vector<bucketElement> contents;
};

class QuickBucket
{
public:
	QuickBucket();
	~QuickBucket()
		{for (int x = 0; x < width; x++) delete [] theMap[x]; delete [] theMap;}
	
	void LoadMap(std::vector<bool> &bits, int _width, int _height);
	void GenerateCases();
	
	cost HCost(xyLoc l1, xyLoc l2)
	{	int dx = (l1.x>l2.x)?(l1.x-l2.x):(l2.x-l1.x);
		int dy = (l1.y>l2.y)?(l1.y-l2.y):(l2.y-l1.y);
		return (dx>dy)?(dx*CARD_COST + dy*DIAG_DIFF):(dy*CARD_COST + dx*DIAG_DIFF);
	}

	cdCost HCostCD(xyLoc l1, xyLoc l2)
	{	int dx = (l1.x>l2.x)?(l1.x-l2.x):(l2.x-l1.x);
		int dy = (l1.y>l2.y)?(l1.y-l2.y):(l2.y-l1.y);
		return cdCost(dx>dy?dx-dy:dy-dx, dx>dy?dy:dx);
	}
		
	cdCost DeltaCostCD(xyLoc & l1, xyLoc & l2, xyLoc & g)
		{return HCostCD(l1,l2) + HCostCD(l2, g) - HCostCD(l1,g);}
	
	abCost DeltaCostAB(xyLoc & l1, xyLoc & l2, xyLoc & g)
		{return abCost(HCostCD(l1,l2) + HCostCD(l2, g) - HCostCD(l1,g));}
			
	int GetCase(xyLoc & l1, xyLoc & l2, xyLoc & g);
	
	// Functions that are used during search
	int GetBucketId(abCost cost){return (cost.a<<2) + (cost.b&3);} //4a + b%4
	inline void GenerateBucket(int bucketId);

	void AddToOpen(abCost cost)	{theHeap.push_back(cost); push_heap (theHeap.begin(),theHeap.end(), abCostComp());}
	void PopMin(){pop_heap (theHeap.begin(),theHeap.end(), abCostComp()); theHeap.pop_back();}

	int GetBaseCase(xyLoc & l, xyLoc & g);	// Generates a hash that identifies the position of l relative to g
	cost GetPath(xyLoc & s, xyLoc & g, std::vector<xyLoc> & path);
		
	void GetStatistics();
private:
	int height;
	int width;
	uint16_t search;
	
	int cases[512];
	int parentMask[9];
	
	mapCell** theMap;
	std::vector<bucket> buckets;
	int bucketsInUse;
	std::vector<abCost> theHeap;
	
	int maxOpenSize, elementExpansions, alreadyExpanded, bucketExpansions;
};

#endif
