/*
 *  HPAUtil.h
 *  hog
 *
 *  Created by dharabor on 11/11/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef HPAUTIL_H
#define HPAUTIL_H

#include <map>
#include <stdexcept>

class node;
class path;
class AbstractCluster;

namespace HPAUtil
{
	const int MAX_SINGLE_TRANSITION_ENTRANCE_SIZE = 6;

	typedef std::map<int, path*> pathTable;
	typedef std::map<int, node*> nodeTable;
	typedef std::map<int, AbstractCluster*> clusterTable;

	double h(node* from, node* to) throw(std::invalid_argument);
}

#endif
