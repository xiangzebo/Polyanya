// warthog.cpp
//
// @author: dharabor
// @created: August 2012
//

#include "cfg.h"
#include "flexible_astar.h"
#include "gridmap.h"
#include "gridmap_expansion_policy.h"
#include "jps_expansion_policy.h"
#include "jps2_expansion_policy.h"
#include "jpsplus_expansion_policy.h"
#include "jps2plus_expansion_policy.h"
#include "octile_heuristic.h"
#include "scenario_manager.h"
#include "weighted_gridmap.h"
#include "wgridmap_expansion_policy.h"
#include "zero_heuristic.h"

#include "getopt.h"

#include <iomanip>
#include <sstream>
#include <tr1/unordered_map>
#include <memory>

// check computed solutions are optimal
int checkopt = 0;
// print debugging info during search
int verbose = 0;
// display program help on startup
int print_help = 0;
// treat the map as a weighted-cost grid
int wgm = 0;

void
help()
{
	std::cerr << "valid parameters:\n"
	<< "--alg [astar | jps | jps2 | jps+ | jps2+ | jps | sssp ]\n"
	<< "--scen [scenario filename]\n"
	<< "--gen [map filename]\n"
	<< "--wgm (optional)\n"
	<< "--checkopt (optional)\n"
	<< "--verbose (optional)\n";
}

void
check_optimality(double len, warthog::experiment* exp)
{
	if(!checkopt)
	{
		return;
	}

	uint32_t precision = 1;
	int epsilon = (warthog::ONE / (int)pow(10, precision)) / 2;

	int32_t int_len = len * warthog::ONE;
	int32_t int_opt = exp->distance() * warthog::ONE;

	for(int i = 10; i <= pow(10, precision); i = i*10)
	{
		int last_digit = int_len % i;
		if(last_digit >= (i/2))
		{
			int_len += (i - last_digit);
		}
	}

	int32_t delta = abs(int_len - int_opt);
	if( abs(delta - epsilon) > epsilon)
	{
		std::stringstream strpathlen;
		strpathlen << std::fixed << std::setprecision(exp->precision());
		strpathlen << len*warthog::ONE;

		std::stringstream stroptlen;
		stroptlen << std::fixed << std::setprecision(exp->precision());
		stroptlen << exp->distance() * warthog::ONE;

		std::cerr << std::setprecision(exp->precision());
		std::cerr << "optimality check failed!" << std::endl;
		std::cerr << std::endl;
		std::cerr << "optimal path length: "<<stroptlen.str()
			<<" computed length: ";
		std::cerr << strpathlen.str()<<std::endl;
		std::cerr << "precision: " << precision << " epsilon: "<<epsilon<<std::endl;
		std::cerr<< "delta: "<< delta << std::endl;
		exit(1);
	}
}

void
run_jpsplus(warthog::scenario_manager& scenmgr)
{
    warthog::gridmap map(scenmgr.get_experiment(0)->map().c_str());
	warthog::jpsplus_expansion_policy expander(&map);
	warthog::octile_heuristic heuristic(map.width(), map.height());

	warthog::flexible_astar<
		warthog::octile_heuristic,
	   	warthog::jpsplus_expansion_policy> astar(&heuristic, &expander);
	astar.set_verbose(verbose);

	std::cout << "id\talg\texpd\tgend\ttouched\ttime\tcost\tsfile\n";
	for(unsigned int i=0; i < scenmgr.num_experiments(); i++)
	{
		warthog::experiment* exp = scenmgr.get_experiment(i);

		int startid = exp->starty() * exp->mapwidth() + exp->startx();
		int goalid = exp->goaly() * exp->mapwidth() + exp->goalx();
		double len = astar.get_length(
				map.to_padded_id(startid),
			   	map.to_padded_id(goalid));
		if(len == warthog::INF)
		{
			len = 0;
		}

		std::cout << i<<"\t" << "jps+" << "\t" 
		<< astar.get_nodes_expanded() << "\t" 
		<< astar.get_nodes_generated() << "\t"
		<< astar.get_nodes_touched() << "\t"
		<< astar.get_search_time()  << "\t"
		<< len << "\t" 
		<< scenmgr.last_file_loaded() << std::endl;

		check_optimality(len, exp);
	}
	std::cerr << "done. total memory: "<< astar.mem() + scenmgr.mem() << "\n";
}

void
run_jps2plus(warthog::scenario_manager& scenmgr)
{
    warthog::gridmap map(scenmgr.get_experiment(0)->map().c_str());
	warthog::jps2plus_expansion_policy expander(&map);
	warthog::octile_heuristic heuristic(map.width(), map.height());

	warthog::flexible_astar<
		warthog::octile_heuristic,
	   	warthog::jps2plus_expansion_policy> astar(&heuristic, &expander);
	astar.set_verbose(verbose);

	std::cout << "id\talg\texpd\tgend\ttouched\ttime\tcost\tsfile\n";
	for(unsigned int i=0; i < scenmgr.num_experiments(); i++)
	{
		warthog::experiment* exp = scenmgr.get_experiment(i);

		int startid = exp->starty() * exp->mapwidth() + exp->startx();
		int goalid = exp->goaly() * exp->mapwidth() + exp->goalx();
		double len = astar.get_length(
				map.to_padded_id(startid),
			   	map.to_padded_id(goalid));
		if(len == warthog::INF)
		{
			len = 0;
		}

		std::cout << i<<"\t" << "jps2+" << "\t" 
		<< astar.get_nodes_expanded() << "\t" 
		<< astar.get_nodes_generated() << "\t"
		<< astar.get_nodes_touched() << "\t"
		<< astar.get_search_time()  << "\t"
		<< len << "\t" 
		<< scenmgr.last_file_loaded() << std::endl;

		check_optimality(len, exp);
	}
	std::cerr << "done. total memory: "<< astar.mem() + scenmgr.mem() << "\n";
}

void
run_jps2(warthog::scenario_manager& scenmgr)
{
    warthog::gridmap map(scenmgr.get_experiment(0)->map().c_str());
	warthog::jps2_expansion_policy expander(&map);
	warthog::octile_heuristic heuristic(map.width(), map.height());

	warthog::flexible_astar<
		warthog::octile_heuristic,
	   	warthog::jps2_expansion_policy> astar(&heuristic, &expander);
	astar.set_verbose(verbose);

	std::cout << "id\talg\texpd\tgend\ttouched\ttime\tcost\tsfile\n";
	for(unsigned int i=0; i < scenmgr.num_experiments(); i++)
	{
		warthog::experiment* exp = scenmgr.get_experiment(i);

		int startid = exp->starty() * exp->mapwidth() + exp->startx();
		int goalid = exp->goaly() * exp->mapwidth() + exp->goalx();
		double len = astar.get_length(
				map.to_padded_id(startid),
			   	map.to_padded_id(goalid));
		if(len == warthog::INF)
		{
			len = 0;
		}

		std::cout << i<<"\t" << "jps2" << "\t" 
		<< astar.get_nodes_expanded() << "\t" 
		<< astar.get_nodes_generated() << "\t"
		<< astar.get_nodes_touched() << "\t"
		<< astar.get_search_time()  << "\t"
		<< len << "\t" 
		<< scenmgr.last_file_loaded() << std::endl;

		check_optimality(len, exp);
	}
	std::cerr << "done. total memory: "<< astar.mem() + scenmgr.mem() << "\n";
}

void
run_jps(warthog::scenario_manager& scenmgr)
{
    warthog::gridmap map(scenmgr.get_experiment(0)->map().c_str());
	warthog::jps_expansion_policy expander(&map);
	warthog::octile_heuristic heuristic(map.width(), map.height());

	warthog::flexible_astar<
		warthog::octile_heuristic,
	   	warthog::jps_expansion_policy> astar(&heuristic, &expander);
	astar.set_verbose(verbose);

	std::cout << "id\talg\texpd\tgend\ttouched\ttime\tcost\tsfile\n";
	for(unsigned int i=0; i < scenmgr.num_experiments(); i++)
	{
		warthog::experiment* exp = scenmgr.get_experiment(i);

		int startid = exp->starty() * exp->mapwidth() + exp->startx();
		int goalid = exp->goaly() * exp->mapwidth() + exp->goalx();
		double len = astar.get_length(
				map.to_padded_id(startid),
			   	map.to_padded_id(goalid));
		if(len == warthog::INF)
		{
			len = 0;
		}

		std::cout << i<<"\t" << "jps" << "\t" 
		<< astar.get_nodes_expanded() << "\t" 
		<< astar.get_nodes_generated() << "\t"
		<< astar.get_nodes_touched() << "\t"
		<< astar.get_search_time()  << "\t"
		<< len << "\t" 
		<< scenmgr.last_file_loaded() << std::endl;

		check_optimality(len, exp);
	}
	std::cerr << "done. total memory: "<< astar.mem() + scenmgr.mem() << "\n";
}

void
run_astar(warthog::scenario_manager& scenmgr)
{
    warthog::gridmap map(scenmgr.get_experiment(0)->map().c_str());
	warthog::gridmap_expansion_policy expander(&map);
	warthog::octile_heuristic heuristic(map.width(), map.height());

	warthog::flexible_astar<
		warthog::octile_heuristic,
	   	warthog::gridmap_expansion_policy> astar(&heuristic, &expander);
	astar.set_verbose(verbose);


	std::cout << "id\talg\texpd\tgend\ttouched\ttime\tcost\tsfile\n";
	for(unsigned int i=0; i < scenmgr.num_experiments(); i++)
	{
		warthog::experiment* exp = scenmgr.get_experiment(i);

		int startid = exp->starty() * exp->mapwidth() + exp->startx();
		int goalid = exp->goaly() * exp->mapwidth() + exp->goalx();
		double len = astar.get_length(
				map.to_padded_id(startid), 
				map.to_padded_id(goalid));
		if(len == warthog::INF)
		{
			len = 0;
		}

		std::cout << i<<"\t" << "astar" << "\t" 
		<< astar.get_nodes_expanded() << "\t" 
		<< astar.get_nodes_generated() << "\t"
		<< astar.get_nodes_touched() << "\t"
		<< astar.get_search_time()  << "\t"
		<< len << "\t" 
		<< scenmgr.last_file_loaded() << std::endl;

		check_optimality(len, exp);
	}
	std::cerr << "done. total memory: "<< astar.mem() + scenmgr.mem() << "\n";
}

void
run_wgm_astar(warthog::scenario_manager& scenmgr)
{
    warthog::weighted_gridmap map(scenmgr.get_experiment(0)->map().c_str());
	warthog::wgridmap_expansion_policy expander(&map);
	warthog::octile_heuristic heuristic(map.width(), map.height());

	warthog::flexible_astar<
		warthog::octile_heuristic,
	   	warthog::wgridmap_expansion_policy> astar(&heuristic, &expander);
	astar.set_verbose(verbose);
    // cheapest terrain (movingai benchmarks) has ascii value '.'; we scale
    // all heuristic values accordingly (otherwise the heuristic doesn't 
    // impact f-values much and search starts to behave like dijkstra)
    astar.set_hscale('.');  

	std::cout << "id\talg\texpd\tgend\ttouched\ttime\tcost\tsfile\n";
	for(unsigned int i=0; i < scenmgr.num_experiments(); i++)
	{
		warthog::experiment* exp = scenmgr.get_experiment(i);

		int startid = exp->starty() * exp->mapwidth() + exp->startx();
		int goalid = exp->goaly() * exp->mapwidth() + exp->goalx();
		double len = astar.get_length(
				map.to_padded_id(startid), 
				map.to_padded_id(goalid));
		if(len == warthog::INF)
		{
			len = 0;
		}

		std::cout << i<<"\t" << "astar_wgm" << "\t" 
		<< astar.get_nodes_expanded() << "\t" 
		<< astar.get_nodes_generated() << "\t"
		<< astar.get_nodes_touched() << "\t"
		<< astar.get_search_time()  << "\t"
		<< len << "\t" 
		<< scenmgr.last_file_loaded() << std::endl;

		check_optimality(len, exp);
	}
	std::cerr << "done. total memory: "<< astar.mem() + scenmgr.mem() << "\n";
}

void
run_wgm_sssp(warthog::scenario_manager& scenmgr)
{
    warthog::weighted_gridmap map(scenmgr.get_experiment(0)->map().c_str());
	warthog::wgridmap_expansion_policy expander(&map);
	warthog::zero_heuristic heuristic(map.width(), map.height());

	warthog::flexible_astar<
		warthog::zero_heuristic,
	   	warthog::wgridmap_expansion_policy> astar(&heuristic, &expander);
	astar.set_verbose(verbose);

	std::cout << "id\talg\texpd\tgend\ttouched\ttime\tsfile\n";
	for(unsigned int i=0; i < scenmgr.num_experiments(); i++)
	{
		warthog::experiment* exp = scenmgr.get_experiment(i);

		int startid = exp->starty() * exp->mapwidth() + exp->startx();
		astar.get_length(map.to_padded_id(startid), warthog::INF);

		std::cout << i<<"\t" << "sssp_wgm" << "\t" 
		<< astar.get_nodes_expanded() << "\t" 
		<< astar.get_nodes_generated() << "\t"
		<< astar.get_nodes_touched() << "\t"
		<< astar.get_search_time()  << "\t"
		<< scenmgr.last_file_loaded() << std::endl;
	}
	std::cerr << "done. total memory: "<< astar.mem() + scenmgr.mem() << "\n";
}

int 
main(int argc, char** argv)
{
	// parse arguments
	warthog::util::param valid_args[] = 
	{
		{"scen",  required_argument, 0, 0},
		{"alg",  required_argument, 0, 1},
		{"gen", required_argument, 0, 3},
		{"help", no_argument, &print_help, 1},
		{"checkopt",  no_argument, &checkopt, 1},
		{"verbose",  no_argument, &verbose, 1},
		{"wgm",  no_argument, &wgm, 1}
	};

	warthog::util::cfg cfg;
	cfg.parse_args(argc, argv, valid_args);

    if(print_help)
    {
		help();
        exit(0);
    }

	std::string sfile = cfg.get_param_value("scen");
	std::string alg = cfg.get_param_value("alg");
	std::string gen = cfg.get_param_value("gen");

    // generate scenarios
	if(gen != "")
	{
		warthog::scenario_manager sm;
		warthog::gridmap gm(gen.c_str());
		sm.generate_experiments(&gm, 1000) ;
		sm.write_scenario(std::cout);
        exit(0);
	}

	// run experiments
	if(alg == "" || sfile == "")
	{
        std::cerr << "Err. Must specify a scenario file and search algorithm. Try --help for options.\n";
		exit(0);
	}

	warthog::scenario_manager scenmgr;
	scenmgr.load_scenario(sfile.c_str());
    std::cerr << "wgm: " << (wgm ? "true" : "false") << std::endl;

	if(alg == "jps+")
	{
		run_jpsplus(scenmgr);
	}

	if(alg == "jps2")
	{
		run_jps2(scenmgr);
	}

	if(alg == "jps2+")
	{
		run_jps2plus(scenmgr);
	}

    if(alg == "jps")
    {
        run_jps(scenmgr);
    }

	if(alg == "astar")
	{
        if(wgm) 
        { 
            run_wgm_astar(scenmgr); 
        }
        else 
        { 
            run_astar(scenmgr); 
        }
	}

	if(alg == "sssp")
	{
        if(wgm) 
        { 
            run_wgm_sssp(scenmgr); 
        }
        else 
        { 
            //run_astar(scenmgr); 
        }
	}
}

