#include "scenario_manager.h"
#include "flexible_astar.h"

#include <cstdlib>
#include <string>

static uint32_t head_offset = 0;
static uint32_t tail_offset = 0;
static const int MAXTRIES=10000000;

warthog::scenario_manager::scenario_manager() : version_(1)
{
}

warthog::scenario_manager::~scenario_manager()
{
	for(unsigned int i=0; i < experiments_.size(); i++)
	{
		delete experiments_[i];
	}
	experiments_.clear();
}

void 	
warthog::scenario_manager::generate_experiments(
		warthog::gridmap* map, int num) 
{
	assert(map); // need a test here; throw exception if absMap is null
	
	head_offset = tail_offset = 0;
	int tries=0;
	int generated=0;
	while(generated < num)
	{	
		if(tries >= MAXTRIES)
		{
			std::cerr << "err; scenario_manager::generate_experiments"
				<< " tried "<<tries << " times but could not generate "
				"a valid experiment. giving up.\n";
			exit(1);
		}
		
		experiment* exp = generate_single_experiment(map); 
		if(exp != NULL) 
		{
			this->add_experiment(exp);
			generated++;
			if((generated % 10) == 0)
			{
				head_offset += 10;
				tail_offset += 20;
				std::cerr << "\rgenerated: "<< generated << "/" << num;
				std::cerr << std::flush;
			}
		}
		tries++;
		if((tries % 5) == 0)
		{
			head_offset += 5;
			tail_offset += 10;
		}
	}
	std::cerr << " experiments." << std::endl;
	sort();
}

warthog::experiment* 
warthog::scenario_manager::generate_single_experiment(warthog::gridmap* map)
{
	warthog::gridmap_expansion_policy expander(map);
	warthog::octile_heuristic heuristic(map->width(), map->height());
	warthog::flexible_astar<
		warthog::octile_heuristic,
	   	warthog::gridmap_expansion_policy> astar(&heuristic, &expander);

	warthog::experiment* newexp;
	uint32_t raw_mapsize = map->header_height() * map->header_width();

	uint32_t window_size = raw_mapsize * 0.01;
	if(head_offset + window_size >= raw_mapsize)
		head_offset = 0;
	if(tail_offset + window_size >= raw_mapsize)
		tail_offset = 0;

	int id1_ = ((rand() % window_size) + head_offset) % raw_mapsize;
	int id2_ = (raw_mapsize - ((rand() % window_size) + tail_offset)) % raw_mapsize;
	int id1 = map->to_padded_id(id1_);
	int id2 = map->to_padded_id(id2_);
	double dist = astar.get_length(map->to_padded_id(id1), map->to_padded_id(id2));
	double inf = warthog::INF / (double) warthog::ONE;

	if(dist == inf)
	{
	//	std::cout << " no path;" <<std::endl;
		return 0;
	}
	//std::cout << " found path;" << std::endl;
		
	uint32_t x1, x2, y1, y2;
	map->to_unpadded_xy(id1,x1,y1);
	map->to_unpadded_xy(id2, x2, y2);
	newexp = new experiment(x1, y1, x2, y2, 
			map->header_width(), map->header_height(), 
			dist, std::string(map->filename()));
	return newexp;
}

void 
warthog::scenario_manager::load_scenario(const char* filelocation)
{
	std::ifstream infile;
	infile.open(filelocation,std::ios::in);

	if(!infile.good())
	{
		std::cerr << "err; scenario_manager::load_scenario "
		<< "Invalid scenario file: "<<filelocation << std::endl;
		infile.close();
		exit(1);
	}

	sfile_ = filelocation;


	// Check if a version number is given
	float version=0;
	std::string first;
	infile >> first;
	if(first != "version")
	{
		version = 0.0;
		infile.seekg(0,std::ios::beg);
	}

	infile >> version;
	if(version == 1.0 || version == 0)
	{
		load_v1_scenario(infile);
	}
	else
	{
		std::cerr << "err; scenario_manager::load_scenario "
			<< " scenario has invalid version number. \n";
		infile.close();
		exit(1);
	}

	infile.close();
}

// V1.0 is the version officially supported by HOG
void 
warthog::scenario_manager::load_v1_scenario(std::ifstream& infile)
{
	int sizeX = 0, sizeY = 0; 
	int bucket;
	std::string map;  
	int xs, ys, xg, yg;
	std::string dist;

	while(infile>>bucket>>map>>sizeX>>sizeY>>xs>>ys>>xg>>yg>>dist)
	{
		double dbl_dist = strtod(dist.c_str(),0);
		experiments_.push_back(
				new experiment(xs,ys,xg,yg,sizeX,sizeY,dbl_dist,map));

		int precision = 0;
		if(dist.find(".") != std::string::npos)
		{
			precision = dist.size() - (dist.find(".")+1);
		}
		experiments_.back()->set_precision(precision);
	}
}

void 
warthog::scenario_manager::write_scenario(std::ostream& scenariofile)
{

	std::cerr << "dumping scenario file..\n";
	if(experiments_.size() == 0) // nothing to write
		return;

	//std::ofstream scenariofile;
	scenariofile.precision(16);
	//scenariofile.open(filelocation, std::ios::out);
	scenariofile << "version " << version_<<std::endl;

	for(unsigned int i=0; i<experiments_.size(); i++)
	{	
		experiment*	cur = experiments_.at(i);
		cur->print(scenariofile);
		scenariofile << std::endl;
	}
	//scenariofile.close();		
}

void 
warthog::scenario_manager::sort()
{
	for(unsigned int i=0; i < experiments_.size(); i++)
	{
		for(unsigned int j = i; j < experiments_.size(); j++)
		{
			if(experiments_.at(j)->distance() <
				   	experiments_.at(i)->distance())
			{
				experiment* tmp = experiments_.at(i);
				experiments_.at(i) = experiments_.at(j);
				experiments_.at(j) = tmp;
			}
		}
	}
}
