#include "ScenarioManager.h"
#include "aStar3.h"
#include "mapAbstraction.h"
#include "FlexibleAStar.h"
#include "OctileExpansionPolicy.h"
#include "TileExpansionPolicy.h"
#include "ManhattanHeuristic.h"

#include <cstdlib>
#include <string>

static int head_offset = 0;
static int tail_offset = 0;
AbstractScenarioManager::~AbstractScenarioManager()
{
	for(unsigned int i=0; i < experiments.size(); i++)
	{
		delete experiments[i];
	}
	experiments.clear();
}

void 
AbstractScenarioManager::writeScenarioFile(const char* filelocation)
{
	if(experiments.size() == 0) // nothing to write
		return;

	std::ofstream scenariofile;
	scenariofile.precision(16);
	scenariofile.open(filelocation, std::ios::out);
	scenariofile << "version " << version<<std::endl;

	for(unsigned int i=0; i<experiments.size(); i++)
	{	
		Experiment*	cur = experiments.at(i);
		cur->print(scenariofile);
		scenariofile << std::endl;
	}
	
	scenariofile.close();		
}

void 
AbstractScenarioManager::sortExperiments()
{
	for(int i=0; i < experiments.size(); i++)
	{
		for(int j = i; j < experiments.size(); j++)
		{
			if(experiments.at(j)->getDistance() < experiments.at(i)->getDistance())
			{
				Experiment* tmp = experiments.at(i);
				experiments.at(i) = experiments.at(j);
				experiments.at(j) = tmp;
			}
		}
	}
}

ScenarioManager::ScenarioManager()
{
	version = 1;
}

ScenarioManager::~ScenarioManager()
{
}

void 	
ScenarioManager::generateExperiments(mapAbstraction* absMap, int numexperiments) 
	throw(TooManyTriesException)
{
	assert(absMap != 0); // need a test here; throw exception if absMap is null
	
	head_offset = tail_offset = 0;
	int tries=0;
	int generated=0;
	int fails=0;
	int num_nodes = absMap->getAbstractGraph(0)->getNumNodes();
	while(generated < numexperiments)
	{	
		if(fails > MAX_CONSECUTIVE_FAILS)
		{
			throw TooManyTriesException(generated, numexperiments);
		}
		
		Experiment* exp = generateSingleExperiment(absMap); 
		if(exp != NULL) 
		{
			this->addExperiment(exp);
			generated++;
			if((generated % 10) == 0)
			{
				head_offset += num_nodes*0.005;
				tail_offset += num_nodes*0.025;
				std::cout << "\rgenerated: "<< generated << "/" << numexperiments;
				std::cout << std::flush;
			}
			fails=0;
		}
		else
		{
			fails++;
		}
		tries++;
		if((tries % 5) == 0)
		{
			head_offset += num_nodes*0.005;
			tail_offset += num_nodes*0.01;
		}
	}
	std::cout << " experiments." << std::endl;
	sortExperiments();
}

Experiment* 
ScenarioManager::generateSingleExperiment(mapAbstraction* absMap)
{
	graph *g = absMap->getAbstractGraph(0);
	const char* _map = absMap->getMap()->getMapName();

	node *r1, *r2;
	Experiment* newexp;

	int range = g->getNumNodes()*0.005;
	r1 = r2 = 0;
	path *p=0;

	if(head_offset + range >= g->getNumNodes())
		head_offset = (rand()%range);
	if(tail_offset + range >= g->getNumNodes())
		tail_offset = (rand()%range);

	int id1 = (rand() % range) + head_offset;
	//int id2 = g->getNumNodes() - ((rand() % range) + tail_offset);
	int id2 = ((rand() % range) + tail_offset);
	//std::cout << "id1: "<<id1 << " id2: "<< id2;

	r1 = g->getNode(id1);
	r2 = g->getNode(id2);

	FlexibleAStar* searchalg;
	if(absMap->getAllowDiagonals())
	{
		//std::cout << "allowing diagonals\n";
		searchalg = new FlexibleAStar(
				new OctileExpansionPolicy(),
				new ManhattanHeuristic());
	}
	else
	{
		searchalg = new FlexibleAStar(
				new TileExpansionPolicy(),
				new ManhattanHeuristic());
	}

	//aStarOld searchalg;
	p = searchalg->getPath(absMap, r1, r2);

	if(!p)
	{
	//	std::cout << " no path;" <<std::endl;
		return NULL;
	}
	//std::cout << " found path;" << std::endl;
		
	double dist = r2->getLabelF(kTemporaryLabel); // fValue
	int x1, x2, y1, y2;
	int mapwidth = absMap->getMap()->getMapWidth();
	int mapheight = absMap->getMap()->getMapHeight();
	
	x1 = r1->getLabelL(kFirstData); y1 = r1->getLabelL(kFirstData+1);
	x2 = r2->getLabelL(kFirstData); y2 = r2->getLabelL(kFirstData+1);
	newexp = new Experiment(x1, y1, x2, y2, mapwidth, mapheight, 0, dist, _map);
	
	delete p;
	delete searchalg;
	return newexp;
}

void 
ScenarioManager::loadScenarioFile(const char* filelocation)
	throw(std::invalid_argument)
{
	std::ifstream infile;
	infile.open(filelocation,std::ios::in);

	if(!infile.good())
	{
		std::stringstream ss;
		ss << "Invalid scenario file: "<<filelocation;
		throw std::invalid_argument(ss.str());
	}


	// Check if a version number is given
	float version=0;
	string first;
	infile >> first;
	if(first != "version")
	{
		version = 0.0;
		infile.seekg(0,std::ios::beg);
	}

	infile >> version;
	if(version == 1.0 || version == 0)
	{
		loadV1ScenarioFile(infile);
	}

	else if(version == 3)
	{
		loadV3ScenarioFile(infile);
	}
	else
	{
		std::cout << "Scenario file contains invalid version number. "
			"Exiting\n";
		infile.close();
		exit(-1);
	}

	infile.close();
}

// V1.0 is the version officially supported by HOG
void 
ScenarioManager::loadV1ScenarioFile(std::ifstream& infile)
{
	int sizeX = 0, sizeY = 0; 
	int bucket;
	string map;  
	int xs, ys, xg, yg;
	string dist;

	while(infile>>bucket>>map>>sizeX>>sizeY>>xs>>ys>>xg>>yg>>dist)
	{
		double dbl_dist = strtod(dist.c_str(),0);
		experiments.push_back(
				new Experiment(xs,ys,xg,yg,sizeX,sizeY,bucket,dbl_dist,map));

		int precision = 0;
		if(dist.find(".") != string::npos)
		{
			precision = dist.size() - (dist.find(".")+1);
		}
		experiments.back()->setPrecision(precision);
	}
}

void 
ScenarioManager::loadV3ScenarioFile(std::ifstream& infile)
{
	int xs, ys, xg, yg;
	string dist;
	string mapfile;
	while(infile>>mapfile>>xs>>ys>>xg>>yg>>dist)
	{
		double dbl_dist = strtod(dist.c_str(),0);
		experiments.push_back(
			new Experiment(xs, ys, xg, yg, 1, 1, 0, dbl_dist, mapfile));

		int precision = 0;
		if(dist.find(".") != string::npos)
		{
			precision = dist.size() - (dist.find(".")+1);
		}
		experiments.back()->setPrecision(precision);
	}
}

