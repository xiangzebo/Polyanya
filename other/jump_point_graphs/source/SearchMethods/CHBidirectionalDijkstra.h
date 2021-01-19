/*
 * CHBidirectionalDijkstra.h
 *
 *  Created on: Oct 31, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_SEARCHMETHODS_CHBIDIRECTIONALDIJKSTRA_H_
#define APPS_SUBGOALGRAPH_SEARCHMETHODS_CHBIDIRECTIONALDIJKSTRA_H_

#include <vector>

#include "../ContractionHierarchy/CHPathRefiner.h"
#include "../Graphs/GraphDefinitions.h"
#include "../Utils/CPUTimer.h"
#include "BidirectionalDijkstra.h"

// TODO: Seems to be unused at the moment.
// Can be useful: Move Stall related functions here?
// Maybe worth considering if we do a search method overhaul.

// Extends the bidirectional Dijkstra for CH queries.
// Uses a regular bidirectional Dijkstra for search, but also includes a
// refinement step where the shortcut arcs are converted into regular arcs.
// Assumes that arcs of G have the fields 'target', 'weight' and 'mid'.

template <class FG, class BG = FG>
class CHBidirectionalDijkstra {
public:
  CHBidirectionalDijkstra(FG* fg, BG* bg, bool stall_on_demand)
      : fg_(fg),
        bg_(bg),
        bidij_(fg,bg, true, stall_on_demand),  // true: Use CH search
        path_refiner_(fg,bg) {
    search_time_ = 0;
    refine_time_ = 0;
  }
  ~CHBidirectionalDijkstra() {}

  Distance FindPath(nodeId start, nodeId goal, std::vector<nodeId> & path) {
    search_time_ = 0;
    refine_time_ = 0;

    // Find the up-down-path.
    CPUTimer t;
    t.StartTimer();
    std::vector<nodeId> up_down_path;

    Distance d = bidij_.FindPath(start, goal, up_down_path);

    // Refine it into a path on the original graph.
    t.StartTimer();
    path_refiner_.RefinePath(up_down_path, path);
    refine_time_ = t.EndTimer();

    return d;
  }

  void AddStatistics(QueryStatistic & s) const {
    bidij_.AddStatistics(s);
    s.search_time += search_time_;
    s.refine_time += refine_time_;
  }
private:
  FG* fg_;
  BG* bg_;
  BidirectionalDijkstra<FG,BG> bidij_;
  CHPathRefiner<FG,BG> path_refiner_;
  double search_time_;
  double refine_time_;
};



#endif /* APPS_SUBGOALGRAPH_SEARCHMETHODS_CHBIDIRECTIONALDIJKSTRA_H_ */
