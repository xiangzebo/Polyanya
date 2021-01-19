#ifndef EMPTYCLUSTER_H
#define EMPTYCLUSTER_H

// EmptyCluster.h
//
// An older implementation of Empty Rectangular Rooms
// this class does not fit into the newer EmptyCluster
// hierarchy.
//
// @author: dharabor
// @created: 6/1/2011

#include "AbstractCluster.h"

class EmptyClusterAbstraction;
class MacroNode;
class EmptyCluster : public AbstractCluster
{
	public:
		EmptyCluster(const int x, const int y,
			   EmptyClusterAbstraction* map, bool pr=false, bool bfr=false);
		virtual ~EmptyCluster();

		virtual void buildCluster();
		virtual void buildEntrances();
		virtual void connectParent(node*) 
			throw(std::invalid_argument);

		inline void setPerimeterReduction(bool pr) 
		{ this->perimeterReduction = pr; }

		inline void setBFReduction(bool bfr) 
		{ this->bfReduction = bfr; }

		MacroNode* nextNodeInColumn(int x, int y, bool topToBottom);
		MacroNode* nextNodeInRow(int x, int y, bool leftToRight);

		edge* findSecondaryEdge(unsigned int fromId, unsigned int toId);
		inline unsigned int getNumSecondaryEdges() { return secondaryEdges.size(); }
		int macro; // macro edge refcount
		
		// cluster dimensions
		inline void setVOrigin(int starty_) { starty = starty_; }
		inline void setHOrigin(int startx_) { startx = startx_; }
		inline int getVOrigin() { return starty; }
		inline int getHOrigin() { return startx; }
		inline int getWidth() { return width; }
		inline void setWidth(int _width) { width = _width; }
		inline int getHeight() { return height; }
		inline void setHeight(int _height) { height = _height; }

		virtual void openGLDraw();

	private:
		// support methods for ::buildCluster 
		bool canExtendClearanceSquare();
		bool canExtendHorizontally();
		bool canExtendVertically();

		// support methods for ::buildEntrances
		void frameCluster();
		void addInterEdges();
		void addMacroEdges();
		void addCardinalMacroEdges();
		void addDiagonalMacroEdges();
		void addDiagonalFanMacroEdges();

		// support methods for both
		bool isIncidentWithInterEdge(node* n_);
		void addSingleMacroEdge(node* from, node* to, double weight, 
				graph* absg, bool secondaryEdge = false);

		int width, height;
		int startx, starty;
		bool perimeterReduction;
		bool bfReduction;
		std::vector<edge*> secondaryEdges; 
		
};

#endif
