/*
 * GraphQueryAStar.h
 *
 *  Created on: Oct 18, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_METHODMANAGERS_GRAPHQUERYASTAR_H_
#define APPS_SUBGOALGRAPH_METHODMANAGERS_GRAPHQUERYASTAR_H_

#include "../SearchMethods/AStar.h"
#include "../Utils/InstanceManager.h"
#include "../Utils/Statistics.h"
#include "CPUTimer.h"

template <class S, class G, class H>
class GraphQueryAStar {
 public:
  GraphQueryAStar(G* g, H* h) :
    g_(g), h_(h), astar_(g,h) {
  }

  void Preprocess() {}
  void ReportPreprocessStatistics(Statistics* s) {}
  void ReportAdditionalSearchStatistics(Statistics* s) {}

  QueryStatistic Query(S start, S goal, std::vector<S> & state_path) {
    QueryStatistic s;
    std::vector<nodeId> node_path;

    s.dist = astar_.FindPath(g_->ToNodeId(start), g_->ToNodeId(goal),
                             node_path);
    astar_.AddStatistics(s);

    // Convert to state path.
    state_path.clear();
    for (unsigned int i = 0; i < node_path.size(); i++)
      state_path.push_back(g_->ToState(node_path[i]));
    return s;
  }
  QueryStatistic Query(ProblemInstance<S> ins) {
    std::vector<S> dummy_path;
    QueryStatistic s = Query(ins.start, ins.goal, dummy_path);
    return s;
  }

  AStar<G,H>* GetSearchMethod() {
    return &astar_;
  }

 private:
  G* g_;
  H* h_;
  AStar<G,H> astar_;
};



#endif /* APPS_SUBGOALGRAPH_METHODMANAGERS_GRAPHQUERYASTAR_H_ */
