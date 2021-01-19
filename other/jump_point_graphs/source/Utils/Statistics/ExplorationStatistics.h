/*
 * ExplorationStatistics.h
 *
 *  Created on: Jan 1, 2019
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_UTILS_STATISTICS_EXPLORATIONSTATISTICS_H_
#define APPS_SUBGOALGRAPH_UTILS_STATISTICS_EXPLORATIONSTATISTICS_H_
#include "StatisticsFile.h"
#include "StatisticsUtil.h"

struct RConnectStatistic {
  RConnectStatistic() {
    time = 0;
    num_popped = 0;
    num_relaxed = 0;
    num_stall_relaxed = 0;
    num_subgoals = 0;
    num_covered = 0;
    num_stalled = 0;
    radius = 0;
    num_expanded = 0;
  }
  ~RConnectStatistic() {}

  double time;
  long num_popped;
  long num_expanded;
  long num_relaxed;
  long num_stall_relaxed;
  long num_subgoals;
  long num_covered;
  long num_stalled;
  double radius;
};

inline void ReportStatistics(std::vector<RConnectStatistic> & st,
                             StatisticsFile* sf, bool forward) {
  std::string direction = "Forward connect ";
  if (!forward)
    direction = "Backward connect ";

  ReportAll(direction + "time in ms", sf, st,
            [&](RConnectStatistic v) -> double {return v.time;}, 1000);
  ReportAll(direction + "radius", sf, st,
            [&](RConnectStatistic v) -> double {return v.radius;});
  ReportAll(direction + "subgoal", sf, st,
            [&](RConnectStatistic v) -> double {return v.num_subgoals;});
  ReportAll(direction + "popped", sf, st,
            [&](RConnectStatistic v) -> double {return v.num_popped;});
  ReportAll(direction + "expanded", sf, st,
            [&](RConnectStatistic v) -> double {return v.num_expanded;});
  ReportAll(direction + "covered", sf, st,
            [&](RConnectStatistic v) -> double {return v.num_covered;});
  ReportAll(direction + "stalled", sf, st,
            [&](RConnectStatistic v) -> double {return v.num_stalled;});
  ReportAll(direction + "relaxed", sf, st,
            [&](RConnectStatistic v) -> double {return v.num_relaxed;});
  ReportAll(direction + "stall relaxed", sf, st,
            [&](RConnectStatistic v) -> double {return v.num_stall_relaxed;});
}



#endif /* APPS_SUBGOALGRAPH_UTILS_STATISTICS_EXPLORATIONSTATISTICS_H_ */
