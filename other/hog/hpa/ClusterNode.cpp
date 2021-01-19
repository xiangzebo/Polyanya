#include "ClusterNode.h"
#include "constants.h"

ClusterNode::ClusterNode(const ClusterNode* n) : node(n)
{
	this->parentClusterId = n->parentClusterId;
}

ClusterNode::ClusterNode(const char* name, int clusterId) : node(name)
{
	this->parentClusterId = clusterId; 
	init();
}

ClusterNode::ClusterNode(const char* name) : node(name) 
{ 
	parentClusterId = -1;
	init();
}

ClusterNode::~ClusterNode()
{
}

void 
ClusterNode::init()
{
	this->setLabelL(kParent, -1);
}

void 
ClusterNode::print(std::ostream& out)
{
	out << "node @ ("<<getLabelL(kFirstData)<<",";
	out << getLabelL(kFirstData+1)<<") cluster: ";
	out << parentClusterId << std::endl;
}

void 
ClusterNode::reset()
{
	this->markEdge(0);
	this->backpointer = 0;
}
