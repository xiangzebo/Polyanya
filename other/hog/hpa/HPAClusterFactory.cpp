#include "HPAClusterFactory.h"
#include "ClusterAStar.h"
#include "HPAClusterAbstraction.h"
#include "HPACluster.h"
#include <stdexcept>

HPAClusterFactory::HPAClusterFactory() 
{
}

AbstractCluster* 
HPAClusterFactory::createCluster(int xpos, int ypos, 
		GenericClusterAbstraction* map_)
{
	HPAClusterAbstraction *map = dynamic_cast<HPAClusterAbstraction*>(map_);
	if(map == 0)
		throw std::invalid_argument("HPAClusterFactory: new cluster "
				"requires a map abstraction of type HPAClusterAbstraction");

	ClusterAStar* castar = new ClusterAStar();
	return createCluster(xpos, ypos, 10, 10, castar, map);
}

HPACluster* 
HPAClusterFactory::createCluster(int xpos, int ypos, 
		int width, int height, AbstractClusterAStar* castar,
		HPAClusterAbstraction* map)
{
	if(castar == 0)
		throw std::invalid_argument("HPAClusterFactory: new cluster "
				"requires a non-null AbstractClusterAStar object");
	if(map == 0)
		throw std::invalid_argument("HPAClusterFactory: new cluster "
				"requires a non-null HPAClusterAbstraction object");


	return new HPACluster(xpos, ypos, width, height, castar, map);
}

