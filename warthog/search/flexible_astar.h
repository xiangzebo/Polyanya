#ifndef FLEXIBLE_ASTAR_H
#define FLEXIBLE_ASTAR_H

// flexible_astar.h
//
// A* implementation that allows arbitrary combinations of 
// (weighted) heuristic functions and node expansion policies.
// This implementation uses a binary heap for the open_ list
// and a bit array for the closed_ list.
//
// TODO: is it better to store a separate closed list and ungenerate nodes
// or use more memory and not ungenerate until the end of search??
// 32bytes vs... whatever unordered_map overhead is a two integer key/value pair
// 
// @author: dharabor
// @created: 21/08/2012
//

#include "cpool.h"
#include "pqueue.h"
#include "problem_instance.h"
#include "search_node.h"
#include "timer.h"

#include <iostream>
#include <memory>
#include <stack>

namespace warthog
{

// H is a heuristic function
// E is an expansion policy
template <class H, class E>
class flexible_astar 
{
	public:
		flexible_astar(H* heuristic, E* expander)
			: heuristic_(heuristic), expander_(expander)
		{
			open_ = new warthog::pqueue(1024, true);
			verbose_ = false;
            hscale_ = 1.0;
		}

		~flexible_astar()
		{
			cleanup();
			delete open_;
		}

		inline std::stack<uint32_t>
		get_path(uint32_t startid, uint32_t goalid)
		{
			std::stack<uint32_t> path;
			warthog::search_node* goal = search(startid, goalid);
			if(goal)
			{
				// follow backpointers to extract the path
				assert(goal->get_id() == goalid);
				for(warthog::search_node* cur = goal;
						cur != 0;
					    cur = cur->get_parent())
				{
					path.push(cur->get_id());
				}
				assert(path.top() == startid);
			}
			cleanup();
			return path;
		}

		double
		get_length(uint32_t startid, uint32_t goalid)
		{
			warthog::search_node* goal = search(startid, goalid);
			warthog::cost_t len = warthog::INF;
			if(goal)
			{
				assert(goal->get_id() == goalid);
				len = goal->get_g();
			}

#ifndef NDEBUG

			if(verbose_)
			{
				std::stack<warthog::search_node*> path;
				warthog::search_node* current = goal;
				while(current != 0)	
				{
					path.push(current);
					current = current->get_parent();
				}

				while(!path.empty())
				{
					warthog::search_node* n = path.top();
					uint32_t x, y;
					y = (n->get_id() / expander_->mapwidth());
					x = n->get_id() % expander_->mapwidth();
					std::cerr << "final path: ("<<x<<", "<<y<<")...";
					n->print(std::cerr);
					std::cerr << std::endl;
					path.pop();
				}
			}
#endif
			cleanup();
			return len / (double)warthog::ONE;
		}

		inline size_t
		mem()
		{
			size_t bytes = 
				// memory for the priority quete
				open_->mem() + 
				// gridmap size and other stuff needed to expand nodes
				expander_->mem() +
				// misc
				sizeof(*this);
			return bytes;
		}

		inline uint32_t 
		get_nodes_expanded() { return nodes_expanded_; }

		inline uint32_t
		get_nodes_generated() { return nodes_generated_; }

		inline uint32_t
		get_nodes_touched() { return nodes_touched_; }

		inline double
		get_search_time() { return search_time_; }


		inline bool
		get_verbose() { return verbose_; }

		inline void
		set_verbose(bool verbose) { verbose_ = verbose; } 

        inline double
        get_hscale() { return hscale_; } 

        inline void
        set_hscale(double hscale) { hscale_ = hscale; } 



	private:
		H* heuristic_;
		E* expander_;
		warthog::pqueue* open_;
		bool verbose_;
		static uint32_t searchid_;
		uint32_t nodes_expanded_;
		uint32_t nodes_generated_;
		uint32_t nodes_touched_;
		double search_time_;
        double hscale_; // heuristic scaling factor

		// no copy
		flexible_astar(const flexible_astar& other) { } 
		flexible_astar& 
		operator=(const flexible_astar& other) { return *this; }

		warthog::search_node*
		search(uint32_t startid, uint32_t goalid)
		{
			nodes_expanded_ = nodes_generated_ = nodes_touched_ = 0;
			search_time_ = 0;

			warthog::timer mytimer;
			mytimer.start();

			#ifndef NDEBUG
			if(verbose_)
			{
				std::cerr << "search: startid="<<startid<<" goalid=" <<goalid
					<< std::endl;
			}
			#endif

			warthog::problem_instance instance;
			instance.set_goal(goalid);
			instance.set_start(startid);
			instance.set_searchid(searchid_++);

			warthog::search_node* goal = 0;
			warthog::search_node* start = expander_->generate(startid);
			start->reset(instance.get_searchid());
			start->set_g(0);
			start->set_f(heuristic_->h(startid, goalid) * hscale_);
			open_->push(start);

			while(open_->size())
			{
				nodes_touched_++;
				if(open_->peek()->get_id() == goalid)
				{
					#ifndef NDEBUG
					if(verbose_)
					{
						uint32_t x, y;
						warthog::search_node* current = open_->peek();
						y = (current->get_id() / expander_->mapwidth());
						x = current->get_id() % expander_->mapwidth();
						std::cerr << "goal found ("<<x<<", "<<y<<")...";
						current->print(std::cerr);
						std::cerr << std::endl;
					}
					#endif
					goal = open_->peek();
					break;
				}
				nodes_expanded_++;

				warthog::search_node* current = open_->pop();
				#ifndef NDEBUG
				if(verbose_)
				{
					uint32_t x, y;
					y = (current->get_id() / expander_->mapwidth());
					x = current->get_id() % expander_->mapwidth();
					std::cerr << "expanding ("<<x<<", "<<y<<")...";
					current->print(std::cerr);
					std::cerr << std::endl;
				}
				#endif
				current->set_expanded(true); // NB: set this before calling expander_ 
				assert(current->get_expanded());
				expander_->expand(current, &instance);

				warthog::search_node* n = 0;
				warthog::cost_t cost_to_n = warthog::INF;
				for(expander_->first(n, cost_to_n); 
						n != 0;
					   	expander_->next(n, cost_to_n))
				{
					nodes_touched_++;
					if(n->get_expanded())
					{
						// skip neighbours already expanded
						continue;
					}

					if(open_->contains(n))
					{
						// update a node from the fringe
						warthog::cost_t gval = current->get_g() + cost_to_n;
						if(gval < n->get_g())
						{
							n->relax(gval, current);
							open_->decrease_key(n);
							#ifndef NDEBUG
							if(verbose_)
							{
								uint32_t x, y;
								y = (n->get_id() / expander_->mapwidth());
								x = n->get_id() % expander_->mapwidth();
								std::cerr << "  updating ("<<x<<", "<<y<<")...";
								n->print(std::cerr);
								std::cerr << std::endl;
							}
							#endif
						}
						else
						{
							#ifndef NDEBUG
							if(verbose_)
							{
								uint32_t x, y;
								y = (n->get_id() / expander_->mapwidth());
								x = n->get_id() % expander_->mapwidth();
								std::cerr << "  updating ("<<x<<", "<<y<<")...";
								n->print(std::cerr);
								std::cerr << std::endl;
							}
							#endif
						}
					}
					else
					{
						// add a new node to the fringe
						warthog::cost_t gval = current->get_g() + cost_to_n;
						n->set_g(gval);
						n->set_f(gval + heuristic_->h(n->get_id(), goalid) * hscale_);
					   	n->set_parent(current);
						open_->push(n);
						#ifndef NDEBUG
						if(verbose_)
						{
							uint32_t x, y;
							y = (n->get_id() / expander_->mapwidth());
							x = n->get_id() % expander_->mapwidth();
							std::cerr << "  generating ("<<x<<", "<<y<<")...";
							n->print(std::cerr);
							std::cerr << std::endl;
						}
						#endif
						nodes_generated_++;
					}
				}
				#ifndef NDEBUG
				if(verbose_)
				{
					uint32_t x, y;
					y = (current->get_id() / expander_->mapwidth());
					x = current->get_id() % expander_->mapwidth();
					std::cerr <<"closing ("<<x<<", "<<y<<")...";
					current->print(std::cerr);
					std::cerr << std::endl;
			}
			#endif
			}

			mytimer.stop();
			search_time_ = mytimer.elapsed_time_micro();
			return goal;
		}

		void
		cleanup()
		{
			open_->clear();
			expander_->clear();
		}


};

template <class H, class E>
uint32_t warthog::flexible_astar<H, E>::searchid_ = 0;

}

#endif

