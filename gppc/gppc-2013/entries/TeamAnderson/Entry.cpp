/**
 * Copyright (c) 2012, Ken Anderson
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **/

#include <deque>
#include <vector>
#include <algorithm>
#include <assert.h>
#include "Entry.h"
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <stdlib.h>

//////////////// Optional Compiler Defines /////////////////

// Pass in these optional definitions to the compiler, if wanted.
// DEBUG                      -- prints debug data and outputs relevant info to a file.
// TEST_FOR_PATH_CONNECTIVITY -- checks paths for connectivity.  Leaving this option out may improve speed a miniscule amount, but is not safe.
// DISABLE_DIAGONALS          -- Disable diagonal moves.  Useful for debugging.

//////////////// Defines  /////////////////

#ifdef DEBUG
#define PRINT(format,...) printf(format,__VA_ARGS__)
#else
#define PRINT(format,...) ;
#endif

#define DEBUG_MAP_FILE "debug_map.txt"
#define DEBUG_PATH_FILE "debug_map_path.txt"

//////////////// Constants /////////////////

#define NO_OP      0x00
#define	UP         0x01
#define DOWN       0x02
#define LEFT       0x03
#define RIGHT      0x04
#define UP_LEFT    0x05
#define UP_RIGHT   0x06
#define DOWN_LEFT  0x07
#define DOWN_RIGHT 0x08

const float SQRT2 = sqrt(2.0);

#define RAND_SEED 2
#define NUM_TREES 6

//////////////// Structures /////////////////

typedef char Operation;

typedef struct { 
	bool wall;              // Stores the location of the walls
	bool visited;          // Stores whether we visited this node while making the search tree
	float costF;           // Accurate costs for initial calculation
} PreTreeNode; //__attribute__ ((packed));

typedef struct { 
	unsigned char costI;   // Compacted costs for storage.
	Operation opToParent;  // Store only the operator because it takes less space.
} TreeNode; //__attribute__ ((packed));

//////////////// Variables /////////////////

int width, height;

// Temporary structure used to create the cached search tree.
PreTreeNode *preTree = NULL;

// Structure Saved to disk
TreeNode    *tree;

// Structure Loaded from disk
TreeNode    *trees[NUM_TREES];

//////////////// Definitions /////////////////

inline int GetIndex(xyLoc s)
{
	return s.y*width+s.x;
}

template <typename T>
inline void free_memory(std::vector<T>& to_clear)
{
    std::vector<T> v;
    v.swap(to_clear);
}

xyLoc GetNeighbour( xyLoc loc, Operation op)
{
	switch(op)
	{
		case UP:
			loc.y++;
			break;
		case DOWN:
			loc.y--;
			break;
		case LEFT:
			loc.x--;
			break;
		case RIGHT:
			loc.x++;
			break;
		case UP_LEFT:
			loc.x--;
			loc.y++;
			break;
		case UP_RIGHT:
			loc.x++;
			loc.y++;
			break;
		case DOWN_LEFT:
			loc.x--;
			loc.y--;
			break;
		case DOWN_RIGHT:
			loc.x++;
			loc.y--;
			break;
	}
	return loc;
}

Operation reverse(Operation op)
{
	switch(op)
	{
		case UP:
			op = DOWN;
			break;
		case DOWN:
			op = UP;
			break;
		case LEFT:
			op = RIGHT;
			break;
		case RIGHT:
			op = LEFT;
			break;
		case UP_LEFT:
			op = DOWN_RIGHT;
			break;
		case UP_RIGHT:
			op = DOWN_LEFT;
			break;
		case DOWN_LEFT:
			op = UP_RIGHT;
			break;
		case DOWN_RIGHT:
			op = UP_LEFT;
			break;
	}
	return op;
}

float GetCost( Operation op)
{
	switch(op)
	{
		case UP:
		case DOWN:
		case LEFT:
		case RIGHT:
			return 1.0;
		default:
			return SQRT2;
	}
}

// Debug check
void verify()
{
/*	for( int y=0; y<height; y++ ) {
		for( int x=0; x<width; x++ ) {
			xyLoc loc;
			loc.x = x;
			loc.y = y;
			int index = GetIndex(loc);
			tree_entry &entry = tree[index];
			if(entry.visited==true && 
				( entry.parent.x<0 || entry.parent.y<0 || entry.parent.x>=width || entry.parent.y >= height ) 
			)
			{
				PRINT("error");
			}
		}
	}
*/
}

void WriteTree( FILE *fp, const std::vector<xyLoc>* path = NULL )
{
	// Writes a Human-readible file for debugging
	for( int y=0; y<height; y++ ) {
		for( int x=0; x<width; x++ ) {
			char c = '@';
			xyLoc loc;
			loc.x = x;
			loc.y = y;
			int index = GetIndex(loc);

			if( tree[index].opToParent != NO_OP )
			{
				xyLoc parent = GetNeighbour(loc,tree[index].opToParent);
				if( parent.x==loc.x && (parent.y==loc.y+1 || parent.y==loc.y-1) )
				{
					c = '|';
				}
				else if( (parent.x==loc.x+1 || parent.x==loc.x-1) && parent.y==loc.y )
				{
					c = '-';
				}
				else if( (parent.x==loc.x+1 && parent.y==loc.y+1) || (parent.x==loc.x-1 && parent.y==loc.y-1) )
				{
					c = '\\';
				}
				else if( (parent.x==loc.x+1 && parent.y==loc.y-1) || (parent.x==loc.x-1 && parent.y==loc.y+1) )
				{
					c = '/';
				}
				else
				{
					c = '.';
				}
			}
			// Check if in solution (inefficient, but this is just for debugging)
			if( path )
			{
				for( unsigned int i=0; i<path->size(); i++)
				{
					if( (*path)[i].x==loc.x && (*path)[i].y==loc.y )
					{
						c = '*';
					}
				}
			}

			fprintf(fp, "%c", c);
		}
		fprintf(fp, "\n");
	}
}


// generates 8-connected neighbors
// a diagonal move must have both cardinal neighbors free to be legal
void GetSuccessors(xyLoc s, std::vector<Operation> &ops)
{
	ops.resize(0);
	int opFlags = 0;
	xyLoc next = s;

	next.x++;
	if (next.x < width && preTree[GetIndex(next)].wall)
	{
		ops.push_back(RIGHT);
		opFlags |= 0x01<<RIGHT;
	}

	next = s;
	next.x--;
	if (next.x >= 0 && preTree[GetIndex(next)].wall)
	{
		ops.push_back(LEFT);
		opFlags |= 0x01<<LEFT;
	}

	next = s;
	next.y--;
	if (next.y >= 0 && preTree[GetIndex(next)].wall)
	{
		ops.push_back(DOWN);
		opFlags |= 0x01<<DOWN;
	}

	next = s;
	next.y++;
	if (next.y < height && preTree[GetIndex(next)].wall)
	{
		ops.push_back(UP);
		opFlags |= 0x01<<UP;
	}
#ifndef DISABLE_DIAGONALS
	next = s;
	next.y++;
	next.x++;
	if((opFlags&(0x01<<UP)) && (opFlags&(0x01<<RIGHT)) && preTree[GetIndex(next)].wall)
	{
		ops.push_back(UP_RIGHT);
	}
	
	next = s;
	next.y++;
	next.x--;
	if((opFlags&(0x01<<UP)) && (opFlags&(0x01<<LEFT)) && preTree[GetIndex(next)].wall)
	{
		ops.push_back(UP_LEFT);
	}
	
	next = s;
	next.y--;
	next.x++;
	if((opFlags&(0x01<<DOWN)) && (opFlags&(0x01<<RIGHT)) && preTree[GetIndex(next)].wall)
	{
		ops.push_back(DOWN_RIGHT);
	}
	
	next = s;
	next.y--;
	next.x--;
	if((opFlags&(0x01<<DOWN)) && (opFlags&(0x01<<LEFT)) && preTree[GetIndex(next)].wall )
	{
			ops.push_back(DOWN_LEFT);
	}
#endif
}

// Basicaly does a Djikstra's search to create a search tree
// anchored at the given location.
void CreateTree(xyLoc loc)
{
	PRINT("CreateTree(%i %i)\n",loc.x,loc.y);
	std::vector<Operation> ops;
	std::deque<xyLoc> q;

	int index = GetIndex(loc);
	q.push_back(loc);
	preTree[index].visited = true;
	tree[index].opToParent = NO_OP;
	preTree[index].costF = 0;

	while( q.size() > 0 )
	{
		loc = q.front();
		q.pop_front();
		index = GetIndex(loc);

		GetSuccessors(loc, ops);
		for( unsigned int i=0; i<ops.size(); i++ )
		{
			Operation op = ops[i];
			xyLoc successor = GetNeighbour(loc,op);
			float edge_cost = GetCost(op);
			float succ_cost = preTree[index].costF + edge_cost;
			int index2 = GetIndex(successor);

			if( succ_cost < preTree[index2].costF )
			{
				tree[index2].opToParent = reverse(op);
				preTree[index2].costF = succ_cost;
				if( preTree[index2].visited == false )
				{
					preTree[index2].visited = true;
					q.push_back(successor);
				}
			}
		}
	}
}

// Find the index of an unvisited node, if one exists
// Return index if node is found, -1 otherwise
int FindUnvisitedNodeIndex()
{
	int size = width*height;
	int index;

	// Random search
	for( int i=0; i<10000; i++)
	{
		index = rand() % size;
		if( preTree[index].wall==true && preTree[index].visited==false )
			return index;
	}

	// If cannot find after many random iterations, search linearly through all nodes
	for( int i=0; i<width*height; i++)
	{
		index = 0;
		if( preTree[index].wall==true && preTree[index].visited==false )
		{
			return index;
		}
	}

	// None left!
	return -1;
}

/////////////// Public functions //////////////////

const char *GetName()
{
	return "MultiTreeCache";
}

void PreprocessMap(std::vector<bool> &bits, int w, int h, const char *filename)
{
	width = w;
	height = h;
	srand (RAND_SEED);
	FILE *fp = fopen(filename, "wb");

	PRINT("PreprocessMap: width=%i height=%i \n", width, height);
#ifdef DEBUG
	FILE *fp_d = fopen(DEBUG_MAP_FILE, "w");
#endif

	// init
	unsigned int size = bits.size();
	tree = new TreeNode[size];
	preTree = new PreTreeNode[size];

	for( unsigned int j=0; j<NUM_TREES; j++)
	{

		for( unsigned int i=0; i<size; i++ )
		{
			preTree[i].wall    = bits[i];
			tree[i].opToParent = NO_OP;
			preTree[i].visited = false;
			preTree[i].costF   = FLT_MAX;
			tree[i].costI      = (unsigned char)255;
		}

		// Create Trees (multiple roots are possible)
		int rootIndex=FindUnvisitedNodeIndex();
		while( rootIndex >= 0)
		{
			xyLoc loc = {rootIndex%width,rootIndex/width};
			CreateTree(loc);
			rootIndex=FindUnvisitedNodeIndex();
		}
			
		// Compact
		for( unsigned int i=0; i<size; i++ )
		{
			// Round up to ensure that only the root node gets a cost of 0.
			const double scaledCostF = std::min( 254.0, ceil(preTree[i].costF/5.0));
			const unsigned int scaledCostI = scaledCostF;
			tree[i].costI = (unsigned char) scaledCostI;
		}

		// Write file
		PRINT("PreprocessMap: Writing to file '%s'\n", filename);
		fwrite(tree, sizeof(tree[0]), size, fp);

#ifdef DEBUG
		verify();
		WriteTree(fp_d);
#endif
	}

#ifdef DEBUG
	fclose(fp_d);
#endif

	// Cleanup
	fclose(fp);
	delete[] preTree;
	delete[] tree;
}

void *PrepareForSearch(std::vector<bool> &bits, int w, int h, const char *filename)
{
	width = w;
	height = h;
	int size = w*h;

	PRINT("PrepareForSearch: Reading from file '%s'\n", filename);
	FILE* fp = fopen(filename, "rb");

	for( int i=0; i<NUM_TREES; i++)
	{
		trees[i] = new TreeNode[size];
		fread(trees[i], sizeof(TreeNode), size, fp);
	}

	fclose(fp);

#ifdef DEBUG
	verify();
#endif

	return NULL;//(void *)13182;
}

void PrintPath(std::vector<xyLoc> &path)
{
	PRINT("Len=%u \n", (unsigned int)path.size());
	for(unsigned int i=0; i<path.size(); i++)
	{
		PRINT("%i,%i \n", path[i].x, path[i].y );
	}
	PRINT("%c\n", ' ');
}

bool GetPath(void *data, xyLoc s, xyLoc g, std::vector<xyLoc> &path)
{
	PRINT("PrepareForSearch: %c\n", ' ');

	//assert((long)data == 13182);
	std::deque<xyLoc> q;
	xyLoc loc, loc2;
	int index, index2;

	loc = s;
	loc2 = g;
	index = GetIndex(loc);
	index2 = GetIndex(loc2);

#ifdef DEBUG
	verify();
#endif

	// Select tree that appears to have the smallest cost
	tree = trees[0];
	int minCost = INT_MAX;
	for (int i=0; i<NUM_TREES; i++)
	{
		int cost = trees[i][index].costI + trees[i][index2].costI;
		
		if(cost < minCost)
		{
			minCost = cost;
			tree = trees[i];
		}
	}

	// Get both halves of path
	//path.clear();
	//q.clear();
	path.push_back(loc);
	q.push_back(loc2);
	while( loc.x != loc2.x || loc.y != loc2.y )
	{
		if( tree[index].costI > tree[index2].costI )
		{
#ifdef TEST_FOR_PATH_CONNECTIVITY
			if( tree[index].opToParent == NO_OP )
			{
				path.clear();
				return true; // Two points not connected
			}
#endif
			loc = GetNeighbour(loc, tree[index].opToParent);
			index = GetIndex(loc);
			path.push_back(loc);
		}
		else
		{
#ifdef TEST_FOR_PATH_CONNECTIVITY
			if( tree[index2].opToParent == NO_OP )
			{
				path.clear();
				return true; // Two points not connected
			}
#endif
			loc2 = GetNeighbour(loc2, tree[index2].opToParent);
			index2 = GetIndex(loc2);
			q.push_back(loc2);
		}
	}

	// Skip the root of the tree (don't want to include it twice)
	path.pop_back();	

	// Reverse order and add to solution
	std::deque<xyLoc>::reverse_iterator it;
	for( it = q.rbegin(); it != q.rend(); ++it )
	{
		path.push_back(*it);
	}
	
#ifdef DEBUG
	PrintPath(path);
	fflush(stdout);
	FILE * fp = fopen(DEBUG_PATH_FILE, "w");
	if(fp)
		WriteTree(fp, &path);
	fclose(fp);
#endif

	return true;
}
