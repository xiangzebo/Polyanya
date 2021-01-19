#ifndef REVERSECLUSTERFILTER_H
#define REVERSECLUSTERFILTER_H

// ReverseClusterFilter.h
//
// A reverse filter for clusters: 
// This class filters all nodes that DO NOT BELONG to one of the target clusters.
//
// NB: This filter is specific to ClusterNodes. When given a different type of node
// the filtering rules do not apply and ::filter always returns true. 
//
// @author: dharabor
// @created: 03/05/2011
//

#include "NodeFilter.h"

class ReverseClusterFilter : public NodeFilter
{
	public:
		ReverseClusterFilter();
		virtual ~ReverseClusterFilter();

		virtual bool filter(node* n);
		void addTargetCluster(int clusterId);
		void removeTargetCluster(int clusterId);

	private:
		std::vector<int> targetClusters;
};

#endif

