/*
 * GraphQueryBidirectionalDijkstra.h
 *
 *  Created on: Oct 18, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_METHODMANAGERS_GRAPHBIDIRECTIONALDIJKSTRA_H_
#define APPS_SUBGOALGRAPH_METHODMANAGERS_GRAPHBIDIRECTIONALDIJKSTRA_H_

#include "../Utils/InstanceManager.h"
#include "../Utils/Statistics.h"
#include "BidirectionalDijkstra.h"
#include "AStar.h"
#include "CPUTimer.h"

template <class S, class FG, class BG = FG>
class GraphQueryBidirectionalDijkstra {
 public:
  GraphQueryBidirectionalDijkstra(FG* fg, BG* bg)
      : fg_(fg),
        bidirectional_dijkstra_(fg, bg) {
  }

  void Preprocess() {}
  void ReportPreprocessStatistics(Statistics* s) {}
  void ReportAdditionalSearchStatistics(Statistics* s) {}

  QueryStatistic Query(S start, S goal, std::vector<S> & state_path) {
    QueryStatistic s;
    std::vector<nodeId> node_path;

    s.dist = bidirectional_dijkstra_.FindPath(fg_->ToNodeId(start),
                                              fg_->ToNodeId(goal), node_path);
    bidirectional_dijkstra_.AddStatistics(s);

    // Convert to state path.
    state_path.clear();
    for (unsigned int i = 0; i < node_path.size(); i++)
      state_path.push_back(fg_->ToState(node_path[i]));
    return s;
  }
  QueryStatistic Query(ProblemInstance<S> ins) {
    std::vector<S> dummy_path;
    return Query(ins.start, ins.goal, dummy_path);
  }

 private:
  FG* fg_;
  BidirectionalDijkstra<FG,BG> bidirectional_dijkstra_;
};



#endif /* APPS_SUBGOALGRAPH_METHODMANAGERS_GRAPHBIDIRECTIONALDIJKSTRA_H_ */
