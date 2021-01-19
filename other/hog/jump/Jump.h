#ifndef JUMP_H
#define JUMP_H

// Jump.h
//
// This file contains the namespace for common definitions
// required by the various classes that use Jump Points.
//
// @author: dharabor
// @created: 17/03/2011
//

#include <string>
#include <cassert>

class node;
namespace Jump
{
	typedef enum 
	{
		NONE = 0,
		N = 1, 
		S = 2, 
		E = 4, 
		W = 8, 
		NE = 16, 
		NW = 32, 
		SE = 64, 
		SW = 128
	} Direction;

	Jump::Direction computeDirection(node* cur, node* succ);
	Jump::Direction computeDirection(int, int, int, int);
	std::string toString(Jump::Direction d);

	// check if asserts are enabled or not
	bool hog_asserts_enabled();
}

#endif

