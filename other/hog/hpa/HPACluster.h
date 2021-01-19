/*
 *  HPACluster.h
 *  hog
 *
 *  Created by dharabor on 11/11/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef HPACLUSTER_H
#define HPACLUSTER_H

#include "AbstractCluster.h"

#include <map>
#include <iostream>
#include <stdexcept>

class node;
class searchAlgorithm;
class AbstractClusterAStar;
class HPAClusterAbstraction;

class HPACluster : public AbstractCluster
{
	#ifdef UNITTEST
		friend class HPAClusterAbstractionTest; 
		friend class HPAClusterTest; 
	#endif
	
	public:
		HPACluster( const int x, const int y, const int _width, 
					const int _height, AbstractClusterAStar* alg,
					HPAClusterAbstraction* map) 
			throw(std::invalid_argument);

		virtual ~HPACluster();

		virtual void buildCluster();
		virtual void buildEntrances();
		virtual void connectParent(node*) 
			throw(std::invalid_argument);
		
		inline	void setSearchAlgorithm(AbstractClusterAStar* _alg) 
		{ alg = _alg; }
		inline AbstractClusterAStar* getSearchAlgorithm() 
		{ return alg; }

		inline int getWidth() { return width; }
		inline void setWidth(int _width) { width = _width; }
		inline int getHeight() { return height; }
		inline void setHeight(int _height) { height = _height; }

		inline void setVOrigin(int starty_) { starty = starty_; }
		inline void setHOrigin(int startx_) { startx = startx_; }
		inline int getVOrigin() { return starty; }
		inline int getHOrigin() { return startx; }

		virtual void openGLDraw();
		virtual void print(std::ostream& out);

	protected:
		virtual void addNode(node* mynode) throw(std::invalid_argument);

	
	private:
		void init(const int _x, const int _y, const int _width, 
				const int _height, AbstractClusterAStar* _alg) 
			throw(std::invalid_argument);

		void insertNodeIntoAbstractGraph(node* n);
		void buildHorizontalEntrances();
		void buildVerticalEntrances();
		void buildDiagonalEntrances();
		void processHorizontalEntrance(int x, int y, int length);
		void processVerticalEntrance(int x, int y, int length);
		int findVerticalEntranceLength(int x, int y);
		int findHorizontalEntranceLength(int x, int y);

		int width, height;
		int startx, starty;

		AbstractClusterAStar* alg;
		bool allowDiagonals;
		
};

#endif
