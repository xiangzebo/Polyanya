/*
 * $Id: searchAlgorithm.h,v 1.8 2006/10/18 23:52:25 nathanst Exp $
 *
 *  Hierarchical Open Graph File
 *
 *  Created by Nathan Sturtevant on 9/28/04.
 *  Copyright 2004 Nathan Sturtevant. All rights reserved.
 *
 * This file is part of HOG.
 *
 * HOG is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * HOG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with HOG; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef SEARCHALGORITHM_H
#define SEARCHALGORITHM_H

#include "graph.h"
#include "path.h"
#include "mapAbstraction.h"
#include "unitSimulation.h"
#include "reservationProvider.h"

/**
 * A generic algorithm which can be used for pathfinding.
 */

class searchAlgorithm {
public:
	searchAlgorithm() { nodesExpanded = nodesTouched = 0; searchTime = 0; verbose = 0;}
	virtual ~searchAlgorithm() {}
	virtual const char *getName() = 0;
	virtual path *getPath(graphAbstraction *aMap, node *from, node *to, reservationProvider *rp = 0) = 0;
	long getNodesExpanded() { return nodesExpanded; }
	long getNodesTouched() { return nodesTouched; }
	long getNodesGenerated() { return nodesGenerated; }
	double getSearchTime() { return searchTime; }
	virtual void logFinalStats(statCollection *);

	//protected:
	long nodesExpanded;
	long nodesTouched;
	long nodesGenerated;
	double searchTime;

	int verbose;
};

extern void doRandomPath(graphAbstraction *aMap, searchAlgorithm *sa, bool repeat = false);

#endif
