/*	
 *	EmptyClusterAbstaction.h
 *
 *  Decomposes a map into non-regular clusters which are obstacle free. 
 *  This decomposition allows each cluster to be traversed optimally 
 *  by only expanding nodes along the perimeter of the cluster area 
 *  (and never any inside).
 *
 *  See [Harabor & Botea 2010]
 *
 * 	@author: dharabor
 * 	@created: 10/02/2010
 */

#ifndef EMPTYCLUSTERABSTRACTION_H
#define EMPTYCLUSTERABSTRACTION_H

#include "HPAUtil.h"
#include "GenericClusterAbstraction.h"
#include "EmptyCluster.h"

class IClusterFactory;
class INodeFactory;
class IEdgeFactory;
class Map;
class node;
class MacroEdge;
class MacroNode;

class EmptyClusterAbstraction : public GenericClusterAbstraction
{
	#ifdef UNITTEST
		friend class EmptyClusterTest;
	#endif

	public:
		EmptyClusterAbstraction(Map* m, IClusterFactory* cf, INodeFactory* nf, 
				IEdgeFactory* ef, bool allowDiagonals=true, 
				bool perimeterReduction=true, bool bfReduction=false)
			throw(std::invalid_argument);
		virtual ~EmptyClusterAbstraction();

		virtual void buildClusters();

		virtual	EmptyCluster* clusterIterNext(cluster_iterator&) const;
		virtual EmptyCluster* getCluster(int cid);
		
		bool usingPerimeterRedction() { return perimeterReduction; }


		//virtual double h(node* from, node* to);
		int getNumMacro();
		double getAverageClusterSize();
		double getAverageNodesPruned();
		int getNumAbsEdges();

	private:
		void connectSG(MacroNode* absNode);
		void cardinalConnectSG(MacroNode* absNode);
		void connectSGToNeighbour(MacroNode* absNode, MacroNode* absNeighbour);

		MacroEdge* sgEdge;

		bool perimeterReduction;
		bool bfReduction;
};

#endif

