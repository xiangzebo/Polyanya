#include "MacroNode.h"

MacroNode::MacroNode(const char* _name) : ClusterNode(_name)
{
	p = 0;
}

MacroNode::MacroNode(const MacroNode* n) : ClusterNode(n)
{
	p = n->p;
}

MacroNode::~MacroNode()
{

}

void 
MacroNode::reset()
{
	ClusterNode::reset();
	setParent(0);
}

void 
MacroNode::addSecondaryEdge(edge* e)
{
	secondaryEdges.push_back(e); 
}

void 
MacroNode::removeSecondaryEdge(unsigned int edgeNum)
{
	std::vector<edge*>::iterator iter = secondaryEdges.begin();
	while(iter != secondaryEdges.end())
	{
		edge* e = *iter; 
		if(e->getEdgeNum() == (int)edgeNum)
		{
			secondaryEdges.erase(iter);
			break;
		}
		iter++;
	}
}

edge* 
MacroNode::getSecondaryEdge(unsigned int index)
{
	if(0 <= index && index <= secondaryEdges.size())
		return secondaryEdges.at(index);
	return 0;

}
