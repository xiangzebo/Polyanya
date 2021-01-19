#include "blockmap.h"
#include "cuckoo_table.h"
#include "cpool.h"
#include "flexible_astar.h"
#include "gridmap.h"
#include "gridmap_expansion_policy.h"
#include "hash_table.h"
#include "jps_expansion_policy.h"
#include "pqueue.h"
#include "octile_heuristic.h"
#include "search_node.h"
#include "scenario_manager.h"

#include "getopt.h"

#include <iomanip>
#include <sstream>
#include <tr1/unordered_map>
#include <memory>

void blockmap_access_test();
void gridmap_access_test();
void pqueue_insert_test();
void cuckoo_table_test();
void unordered_map_test();
void hash_table_test();
void gridmap_expansion_policy_test();
void flexible_astar_test();
void test_alloc();
void online_jps_test();

int main(int argc, char** argv)
{
	//flexible_astar_test();
	online_jps_test();
}

void test_alloc()
{

	warthog::mem::cpool pool(sizeof(warthog::search_node));
	char** nodes = new char*[10000];

	for(int i=0; i < 5000; i++)
	{
		for(int j = 0; j < 10000; j++)
		{
			nodes[j] = pool.allocate();
		}
		for(int j = 0; j < 10000; j++)
		{
			pool.deallocate(nodes[j]);
		}
	}
//	std::cerr << "cpool::mem: "<<pool.mem()<<std::endl;
//	pool.print(std::cerr);
//	std::cerr << std::endl;
	delete [] nodes;

//	for(int i=0; i < 5000; i++)
//	{
//		char** c = new char*[10];
//		for(int i=0; i < 10; i++)
//		{
//			c[i] = new char[sizeof(warthog::search_node)*10000];
//		}
//		for(int i=0; i < 10; i++)
//		{
//			delete [] c[i];
//		}
//		delete [] c;
//	}

//	for(int i=0; i < 5000; i++)
//	{
//		warthog::search_node** c = new warthog::search_node*[10000];
//		for(int i=0; i < 10000; i++)
//		{
//			c[i] = new warthog::search_node(i);	
//		}
//		for(int i=0; i < 10000; i++)
//		{
//			delete c[i];
//		}
//		delete [] c;
//	}
}

void online_jps_test()
{
	bool check_opt = false;
	//bool check_opt = true;
	warthog::scenario_manager scenmgr;
	scenmgr.load_scenario("orz700d.map.scen");
	warthog::gridmap map(scenmgr.get_experiment(0)->map().c_str());

//	warthog::gridmap map("CSC2F.map", true);
//	map.printdb(std::cerr);
//	map.print(std::cerr);
//	exit(1);

	warthog::jps_expansion_policy expander(&map);
	warthog::octile_heuristic heuristic(map.width(), map.height());

	warthog::flexible_astar<
		warthog::octile_heuristic,
	   	warthog::jps_expansion_policy> astar(&heuristic, &expander);
	//astar.set_verbose(true);

	for(unsigned int i=0; i < scenmgr.num_experiments(); i++)
	{
		warthog::experiment* exp = scenmgr.get_experiment(i);

		int startid = exp->starty() * exp->mapwidth() + exp->startx();
		int goalid = exp->goaly() * exp->mapwidth() + exp->goalx();
		double len = astar.get_length(startid, goalid);
		if(len == warthog::INF)
		{
			len = 0;
		}

		if(!check_opt)
			continue;

		std::cerr << "exp "<<i<<" ";
		exp->print(std::cerr);
		std::cerr << " (ne=" << astar.get_nodes_expanded() << 
			" ng=" << astar.get_nodes_generated() << 
			" nt=" << astar.get_nodes_touched() <<
			" st=" << astar.get_search_time() <<")";
		std::cerr << std::endl;

		std::stringstream stroptlen;
		stroptlen << std::fixed << std::setprecision(exp->precision());
		stroptlen << exp->distance();

		std::stringstream strpathlen;
		strpathlen << std::fixed << std::setprecision(exp->precision());
		strpathlen << len;

		if(stroptlen.str().compare(strpathlen.str()))
		{
			std::cerr << std::setprecision(6);
			std::cerr << "optimality check failed!" << std::endl;
			std::cerr << std::endl;
			std::cerr << "optimal path length: "<<stroptlen.str()
				<<" computed length: ";
			std::cerr << strpathlen.str()<<std::endl;
			std::cerr << "precision: " << exp->precision()<<std::endl;
			exit(1);
		}
	}
	std::cerr << "done. total memory: "<< astar.mem() + scenmgr.mem() << "\n";

}

void flexible_astar_test()
{
	bool check_opt = false;
	//bool check_opt = true;
	warthog::scenario_manager scenmgr;
	//scenmgr.load_scenario("CSC2F.map.scen");
	scenmgr.load_scenario("orz700d.map.scen");

	warthog::gridmap map(scenmgr.get_experiment(0)->map().c_str());
	//map.print(std::cerr << "\n");
	warthog::gridmap_expansion_policy expander(&map);
	warthog::octile_heuristic heuristic(map.width(), map.height());

	warthog::flexible_astar<
		warthog::octile_heuristic,
	   	warthog::gridmap_expansion_policy> astar(&heuristic, &expander);
	//astar.set_verbose(true);

	for(unsigned int i=0; i < scenmgr.num_experiments(); i++)
	{
		warthog::experiment* exp = scenmgr.get_experiment(i);

		int startid = exp->starty() * exp->mapwidth() + exp->startx();
		int goalid = exp->goaly() * exp->mapwidth() + exp->goalx();
		double len = astar.get_length(startid, goalid);
		if(len == warthog::INF)
		{
			len = 0;
		}

		if(!check_opt)
			continue;

		std::cerr << "exp "<<i<<" ";
		exp->print(std::cerr);
		std::cerr << " (ne=" << astar.get_nodes_expanded() << 
			" ng=" << astar.get_nodes_generated() << 
			" nt=" << astar.get_nodes_touched() <<
			" st=" << astar.get_search_time() <<")";
		std::cerr << std::endl;

		std::stringstream stroptlen;
		stroptlen << std::fixed << std::setprecision(exp->precision());
		stroptlen << exp->distance();

		std::stringstream strpathlen;
		strpathlen << std::fixed << std::setprecision(exp->precision());
		strpathlen << len;

		if(stroptlen.str().compare(strpathlen.str()))
		{
			std::cerr << std::setprecision(6);
			std::cerr << "optimality check failed!" << std::endl;
			std::cerr << std::endl;
			std::cerr << "optimal path length: "<<stroptlen.str()
				<<" computed length: ";
			std::cerr << strpathlen.str()<<std::endl;
			std::cerr << "precision: " << exp->precision()<<std::endl;
			exit(1);
		}
	}
	std::cerr << "done. total memory: "<< astar.mem() + scenmgr.mem() << "\n";

}

void gridmap_expansion_policy_test()
{
	warthog::gridmap map("CSC2F.map");
	warthog::gridmap_expansion_policy policy(&map);
	unsigned int nodeid[2] = {89, 0};

	for(int i=0; i < 2; i++)
	{
		std::cout << "nid: "<<nodeid[i]<<std::endl;
		unsigned int mapwidth = map.width();
		policy.expand(policy.generate(nodeid[i]), 0);
		warthog::search_node* n = 0;
		warthog::cost_t cost_to_n = warthog::INF;
		for(policy.first(n, cost_to_n);
				n != 0;
				policy.next(n, cost_to_n))
		{
			uint32_t nid = n->get_id();
			unsigned int x = UINT_MAX;
			unsigned int y = UINT_MAX;
			x = nid % mapwidth;
			y = nid / mapwidth;
			std::cout << "neighbour: " << nid << " (" << x << ", "<<y
				<< ") cost: " << cost_to_n << std::endl;
		}
	}
}

void hash_table_test()
{
	warthog::hash_table mytable;
	for(int i=0; i < 10000000; i++)
	{
		mytable.insert(i);
	}
	for(int i=0; i < 10000000; i++)
	{
		if(!mytable.contains(i))
		{
			std::cerr << "table does not contain "<<i<<std::endl;
			exit(1);
		}
	}
	//mytable.print();
}

void unordered_map_test()
{
	std::tr1::unordered_map<unsigned int, unsigned int> mymap;
	mymap.rehash(1024);
	for(int i=0; i < 10000000; i++)
	{
		mymap[i] = i;
	}

	for(int i=0; i < 10000000; i++)
	{
		mymap.find(i);
	}
}

void cuckoo_table_test()
{
	std::cout << "cuckoo_table_test\n";
	warthog::cuckoo_table table(1024);
	//table.set_verbose(true);
	int errors = 0;
	for(int i=0; i < 10000000; i++)
	{
		table.insert(i);
		if(!table.contains(i))
		{
			errors++;
		}
	}
	table.metrics(std::cout);
	std::cout << "errors: "<<errors<<std::endl;
	std::cout << "/cuckoo_table_test\n";
}

void blockmap_access_test()
{
	std::cout << "blockmap_access_test..."<<std::endl;
	const char* file = "orz700d.map";
	std::cout << "loading "<<file<<std::endl;
	warthog::blockmap mymap(file);

	for(int i=0; i < 1<<28; i++)
	{
		int x = (rand()/RAND_MAX)*mymap.width();
		int y = (rand()/RAND_MAX)*mymap.height();
		for(int nx = x-1; nx < x+2; nx++)
		{
			for(int ny = y-1; ny < y+2; ny++)
			{
				mymap.get_label(nx, ny);
			}
		}
	}
	std::cout << "/blockmap_access_test..."<<std::endl;
}

void pqueue_insert_test()
{
	std::cout << "pqueue_insert_test...\n";
	unsigned int pqueuenodes = 1000000;
	warthog::pqueue mypqueue(pqueuenodes, true);
	warthog::search_node** nodes = new warthog::search_node*[pqueuenodes];
	for(int i=pqueuenodes; i > 0 ; i--)
	{
		nodes[i] = new warthog::search_node(i);
		nodes[i]->set_g(i);
		mypqueue.push(nodes[i]);
	}
	// test duplicate detection
	for(int i=pqueuenodes; i > 0 ; i--)
	{
		mypqueue.push(nodes[i]);
	}
	assert(mypqueue.size() == pqueuenodes);

	// test pop
	for(unsigned int i=0; i < pqueuenodes; i++)
	{
		assert(mypqueue.size() == pqueuenodes-i);
		delete mypqueue.pop();
	}
	delete [] nodes;
	assert(warthog::search_node::get_refcount() == 0);
	std::cout << "/pqueue_insert_test...\n";
}

void gridmap_access_test()
{
	std::cout << "gridmap_access_test..."<<std::endl;
	//const char* file = "orz700d.map";
	const char* file = "CSC2F.map";
	std::cout << "loading map..."<<file<<std::endl;
	warthog::gridmap mymap(file);
	std::cout << "map\n";
	mymap.print(std::cout);
	std::cout << "done."<<std::endl;
	return;

	for(int i=0; i < 1<<28; i++)
	{
		int x = (rand()/RAND_MAX)*mymap.width();
		int y = (rand()/RAND_MAX)*mymap.height();
		for(int nx = x-1; nx < x+2; nx++)
		{
			for(int ny = y-1; ny < y+2; ny++)
			{
				mymap.get_label(nx, ny);
			}
		}
		//std::cout << i << "\r" << std::flush;
	}
	std::cout << "gridmap_access_test..."<<std::endl;
}
