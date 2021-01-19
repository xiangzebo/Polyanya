#ifndef GENERICCLUSTERABSTRACTION_H
#define GENERICCLUSTERABSTRACTION_H

// GenericClusterAbstraction.h
//
//  A base class for abstraction methods which make use of clusters.
//  The basic implementation is similar to the one used for HPA*:
//  	1. Divide the map into a series of adjacent clusters.
//  	2. Identify entrances between clusters and in the process
//  		create an abstract graph.
//  	3. Use insertion to connect start and goal nodes to 
//  		the rest of the abstract graph.
//
//	This class allows some flexibility over the creation of clusters, 
//	entrances and the insertion process.
//
//	@author: dharabor
//	@created: 10/12/2010
//

#include "mapAbstraction.h"
#include "HPAUtil.h"

#include <iostream>
#include <stdexcept>

class ClusterNode;
class IClusterFactory;
class INodeFactory;
class IEdgeFactory;
class AbstractCluster;
class Map;
class Heuristic;

typedef HPAUtil::clusterTable::const_iterator cluster_iterator;
class GenericClusterAbstraction : public mapAbstraction
{
	public:
		GenericClusterAbstraction(Map* m, IClusterFactory* cf, 
				INodeFactory* nf, IEdgeFactory* ef, bool allowDiagonals=true)
			throw(std::invalid_argument);
		virtual ~GenericClusterAbstraction();

		virtual void buildClusters() = 0;
		virtual void buildEntrances();
	
		// manhattan or octile if allowDiag
		virtual double h(node *a, node *b); 
		
		// path caching for quick refinement 
		void addPathToCache(edge* e, path* p);
		path* getPathFromCache(edge* e);
		void deletePathFromCache(edge* e);
		int getPathCacheSize() { return pathCache.size(); }

		// drawing and overlay methods 
		virtual void openGLDraw(); 

		void setDrawClusters(bool draw) { drawClusters = draw; }
		bool getDrawClusters() { return drawClusters; }
		
		// needed to implement a concrete mapAbstraction 
		virtual bool pathable(node*, node*) { return false; }
		virtual void verifyHierarchy() {}
		virtual void removeNode(node*) {} 
		virtual void removeEdge(edge*, unsigned int) {}
		virtual void addNode(node*) {}
		virtual void addEdge(edge*, unsigned int) {}
		virtual void repairAbstraction() {}
		virtual mapAbstraction* clone(Map *) { return NULL; }

		// cluster getters and iterator functions 
		cluster_iterator getClusterIter() const { return clusters.begin(); }
		AbstractCluster* clusterIterNext(cluster_iterator&) const;
		AbstractCluster* getCluster(int cid);
		int getNumClusters() { return clusters.size(); } 
	
		IEdgeFactory* getEdgeFactory() { return ef; }
		INodeFactory* getNodeFactory() { return nf; }
		IClusterFactory* getClusterFactory() { return cf; }

		inline bool getVerbose() { return verbose; }
		inline void setVerbose(bool _v) { verbose = _v; }
		virtual void print(std::ostream& out);
		
	protected:
		void addCluster(AbstractCluster* cluster);
		int getNumberOfAbstractionLevels() { return abstractions.size(); }
		void printUniqueIdsOfAllNodesInGraph(graph *g);

		bool drawClusters; 
		bool verbose;
	
	private:
		Heuristic* heuristic;
		IClusterFactory* cf;
		INodeFactory* nf;
		IEdgeFactory* ef;
		HPAUtil::pathTable pathCache;
		HPAUtil::clusterTable clusters;

};

#endif
