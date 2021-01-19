/*
 *  IClusterAStarFactory.h
 *  hog
 *
 *  Created by dharabor on 14/11/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef ICLUSTERASTARFACTORY_H
#define ICLUSTERASTARFACTORY_H

class AbstractClusterAStar;
class IClusterAStarFactory
{
	public:
		virtual ~IClusterAStarFactory() {}
		virtual AbstractClusterAStar* newClusterAStar() = 0;
};

#endif

