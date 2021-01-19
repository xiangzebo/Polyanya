#ifndef CLUSTERNODE_H
#define CLUSTERNODE_H

// ClusterNode.h
//
// An extension the basic node class, ClusterNode objects
// can be associated with Cluster objects.
//
// @author: dharabor
// @created: 11/ 2008

#include "graph.h"
#include <ostream>

class Cluster;
class ClusterNode : public node
{
	public:
		ClusterNode(const char* name, int clusterId);
		ClusterNode(const char* name);
		ClusterNode(const ClusterNode* n);
		~ClusterNode(); 
		virtual graph_object* clone() const { return new ClusterNode(this); }
		virtual void reset();
		int getParentClusterId() { return parentClusterId; }
		void setParentClusterId(int c) { parentClusterId = c; }

		void print(std::ostream& out);

	private:
		void init();
		int parentClusterId;
};

#endif
