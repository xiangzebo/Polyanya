#include "ReverseClusterFilter.h"

#include "ClusterNode.h"

ReverseClusterFilter::ReverseClusterFilter() : NodeFilter()
{
}

ReverseClusterFilter::~ReverseClusterFilter()
{
	targetClusters.clear();
}


// returns false if _n belongs to one of the target clusters
// and true otherwise
bool 
ReverseClusterFilter::filter(node* _n)
{
	ClusterNode* n = dynamic_cast<ClusterNode*>(_n);
	if(!n)
		return true;

	int currentId = n->getParentClusterId();

	if(currentId == -1)
		return true;

	for(unsigned int i=0; i < targetClusters.size(); i++)
	{
		if(currentId == targetClusters.at(i))
			return false;
	}

	return true;
}

void 
ReverseClusterFilter::addTargetCluster(int _clusterId)
{
	bool alreadyAdded = false;
	for(unsigned int i=0; i < targetClusters.size(); i++)
	{
		if(targetClusters.at(i) == _clusterId)
		{
			alreadyAdded = true;
			break;
		}
	}

	if(!alreadyAdded)
		targetClusters.push_back(_clusterId);
}

void 
ReverseClusterFilter::removeTargetCluster(int _clusterId)
{
	for(unsigned int i=0; i < targetClusters.size(); i++)
	{
		int currentId = targetClusters.at(i);
		if(currentId == _clusterId)
		{
			targetClusters.erase(targetClusters.begin() + i);
			break;
		}
	}
}

