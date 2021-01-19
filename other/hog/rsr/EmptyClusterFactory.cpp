#include "EmptyClusterFactory.h"
#include "EmptyCluster.h"
#include "EmptyClusterAbstraction.h"

EmptyClusterFactory::EmptyClusterFactory()
{
}

EmptyClusterFactory::~EmptyClusterFactory()
{
}

AbstractCluster* 
EmptyClusterFactory::createCluster(int x, int y, 
	GenericClusterAbstraction* map_)
{
	EmptyClusterAbstraction *map = dynamic_cast<EmptyClusterAbstraction*>(map_);
	if(map == 0)
		throw std::invalid_argument("EmptyClusterFactory: new cluster "
				"requires a map abstraction of type EmptyClusterAbstraction");

	return createCluster(x, y, map, false, false);
}

EmptyCluster*
EmptyClusterFactory::createCluster(int x, int y,
	EmptyClusterAbstraction* map, bool pr, bool bfr)
{
	return new EmptyCluster(x, y, map, pr, bfr);
}
