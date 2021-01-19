#pragma once
#include <vector>
#include "Coordinate.h"
//#define max(a,b)            a > b ? a : b
//#define min(a,b)            a < b ? a : b


struct Node
{
	float score,total;
	Coordinate pos;
	Node*parent;

	char dir;
	Node()
	{

	}
	static void diagAndStraightCost(const Coordinate &start, const Coordinate& end, std::pair<int, int> &p)
	{
		int absX = abs(start.x - end.x);
		int absY = abs(start.y - end.y);
		p.first += std::min(absX, absY);
		p.second += std::max(absX, absY) - std::min(absX, absY);
	}
	static float calcDistCost(std::pair<int, int> &p)
	{
		return p.first*1.414213562373095f + p.second;
	}
	static float estimateDistance (const Coordinate &start, const Coordinate& end)
	{
		int absX =abs(start.x-end.x);
		int absY =abs(start.y-end.y);

		int diagDist = std::min(absX,absY);
		int straightDist = std::max(absX,absY)-diagDist;
		return diagDist*1.414213562373095f+straightDist;
		//return sqrt((float)(pos.x-end.x)*(pos.x-end.x)+(pos.y-end.y)*(pos.y-end.y));//max (abs (start.x - end.x), abs (start.y - end.y));
	}
	void reset(const Coordinate& _pos, Node*_parent,const int eX,const int eY,const char _dir)
	{
		pos =_pos;
		parent = _parent;
		dir = _dir;
		if (parent)
			score=estimateDistance(pos,parent->pos)+parent->score;
		else
			score=0;

		total=score + estimateDistance(pos,Coordinate(eX,eY));//sqrt((float)(pos.x-eX)*(pos.x-eX)+(pos.y-eY)*(pos.y-eY));
	}
};

class CompareNode
{
public:
	bool operator() (const Node* t1, const Node* t2) const// Returns true if t1 is earlier than t2
    {
		if (t1->total<=t2->total)//t1->cost+
			return false;
		return true;
    }
};
#define NODE_CONTAINER_BUCKET_SIZE 4096/sizeof(Node)//fit buckets within a 4kb page
struct NodeContainer
{
	NodeContainer()
	{
		numDistributedNodes=bucketId=nodeId=0;
		//for (int i =0;i<3;i++)
			bucket.push_back(new Node[NODE_CONTAINER_BUCKET_SIZE]());
	}
	~NodeContainer()
	{
		for (unsigned int i=0;i<bucket.size();i++)
			delete [] bucket[i];
	}
	Node* getNewNode(const Coordinate & _pos,int _endX,int _endY,char dir,Node* _parent)
	{
		if (bucketId>(int)bucket.size()-1)
			bucket.push_back(new Node[NODE_CONTAINER_BUCKET_SIZE]());
		Node* ret =  &bucket[bucketId][nodeId];
		ret->reset(_pos,_parent,_endX,_endY,dir);
		numDistributedNodes++;
		nodeId++;
		if (nodeId==NODE_CONTAINER_BUCKET_SIZE)
		{
			nodeId=0;
			bucketId++;
		}
		return ret;
	}
	void reset()
	{
		numDistributedNodes=bucketId=nodeId=0;
		//printf("BucketSize:%d\n",bucket.size());
	}
	void deleteLastNode()
	{
		numDistributedNodes--;
		nodeId--;
		if (nodeId<0)
		{
			nodeId=NODE_CONTAINER_BUCKET_SIZE-1;
			bucketId--;
		}
	}
	int numDistributedNodes,bucketId,nodeId;
	std::vector<Node*> bucket;
};

