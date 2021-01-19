#ifndef JUMPINFO_H
#define JUMPINFO_H

// JumpInfo.h
//
// A structure for describing jump point search operations.
// A single jumping operation may involve several steps/jumps or just one.
//
// Each step is described in terms of three variables:
// 	* a jump node 
// 	* a jump cost (from the immediate predecessor to the current jump node)
// 	* a direction (of travel, taken from the immediate predecessor)
//
// @author: dharabor
// @created: 26/02/2012
//

#include "graph.h"
#include "Jump.h"

#include <ostream>
#include <vector>

class JumpInfo
{
	public:
		JumpInfo();
		virtual ~JumpInfo();
		JumpInfo(JumpInfo& other);
		void print(std::ostream& out);

		inline void clear()
		{
			this->jumpnodes.clear();
			this->jumpdirs.clear();
			this->jumpcosts.clear();
		}

		inline double totalCost()
		{
			double retVal = 0;
			for(unsigned int i=0; i < jumpcosts.size(); i++)
			{
				retVal += jumpcosts.at(i);
			}
			return retVal;
		}

		inline void addJump(node* n, Jump::Direction lastdir, double cost)
		{
			if(!n || lastdir == Jump::NONE)
				return;

			jumpnodes.push_back(n);
			jumpdirs.push_back(lastdir);
			jumpcosts.push_back(cost);
		}

		inline node* getNode(unsigned int index) 
		{ 
			if(index < jumpnodes.size()) 
				return jumpnodes.at(index);
			return 0;
		}

		inline Jump::Direction getDirection(unsigned int index)
		{
			if(index < jumpdirs.size()) 
				return jumpdirs.at(index);
			return Jump::NONE;
		}

		inline double getCost(unsigned int index)
		{
			if(index < jumpcosts.size()) 
				return jumpcosts.at(index);
			return 0;
		}

		inline void setCost(unsigned int index, double cost)
		{
			if(index < jumpcosts.size()) 
				jumpcosts[index] = cost;
		}

		inline unsigned int nodecount() 
		{ 
			return jumpnodes.size(); 
		}

		inline unsigned int edgecount()
		{
			return nodecount() - 1;
		}

	private:
		std::vector<node*> jumpnodes;
		std::vector<double> jumpcosts; 
		std::vector<Jump::Direction> jumpdirs; 
};

#endif

