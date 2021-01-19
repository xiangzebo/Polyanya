/*
 * ConnectionStatistic.h
 *
 *  Created on: Mar 25, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_UTILS_AGGREGATECONNECTIONSTATISTICS_H_
#define APPS_SUBGOALGRAPH_UTILS_AGGREGATECONNECTIONSTATISTICS_H_
#include "StatisticsFile.h"

struct AggregateConnectionStatistics {
  AggregateConnectionStatistics() {
    num_queries = 0;
    num_reachable_path_found = 0;

    num_forward_connect = 0;
    num_forward_expansions = 0;
    num_forward_subgoals = 0;
    forward_time = 0;

    num_backward_subgoals = 0;
    num_backward_connect = 0;
    num_backward_expansions = 0;
    backward_time = 0;

    num_backward_pruned_subgoals = 0;
  }

  double GetConnectRatio(bool forward, bool backward) {
    if (num_queries == 0 || (!forward && !backward))
      return 0;
    return (forward*num_forward_connect + backward*num_backward_connect)/
        ((double) (num_queries * (forward + backward)));
  }

  double GetAverageExpanded(bool forward, bool backward) {
    if (num_queries == 0 || (!forward && !backward))
      return 0;
    return
        (forward * num_forward_expansions + backward * num_backward_expansions)
        / ((double) (num_forward_connect * forward
            + num_backward_connect * backward));
  }
  double GetAverageSubgoals(bool forward, bool backward) {
    if (num_queries == 0 || (!forward && !backward))
      return 0;
    return
        (forward * num_forward_subgoals + backward * num_backward_subgoals)
        / ((double) (num_forward_connect * forward
            + num_backward_connect * backward));
  }

  double GetAverageTime(bool forward, bool backward) {
    if (num_queries == 0 || (!forward && !backward))
      return 0;
    return
        (forward * forward_time + backward * backward_time)
        / ((double) (num_forward_connect * forward
            + num_backward_connect * backward));
  }

  void ReportStatistics(StatisticsFile* st) {

    st->ReportInt("Number of queries with connect", num_queries);
    st->ReportInt("Number of queries where reachable path is found",
                  num_reachable_path_found);

    st->AddRemark("");
    st->ReportInt("Number of forward connects executed", num_forward_connect);
    double n = num_forward_connect;
    if (num_forward_connect == 0)
      n = 1.0;
    st->ReportDouble("Average number of forward connect expansions",
                     num_forward_expansions/(double)n);
    st->ReportDouble("Average number of forward connected subgoals",
                     num_forward_subgoals/(double)n);
    st->ReportDouble("Average forward connection time (ms)",
                     forward_time*1000.0/(double)n);

    st->AddRemark("");
    st->ReportInt("Number of backward connects executed", num_backward_connect);
    n = num_backward_connect;
    if (num_backward_connect == 0)
      n = 1.0;
    st->ReportDouble("Average number of backward connect expansions",
                     num_backward_expansions/(double)n);
    st->ReportDouble("Average number of backward connected subgoals",
                     num_backward_subgoals/(double)n);
    st->ReportDouble("Average number of backward pruned subgoals",
                     num_backward_pruned_subgoals/(double)n);
    st->ReportDouble("Average backward connection time (ms)",
                     backward_time*1000.0/(double)n);
  }

  int num_queries, num_reachable_path_found, num_forward_connect,
      num_backward_connect, num_forward_expansions, num_backward_expansions,
      num_forward_subgoals, num_backward_subgoals, num_backward_pruned_subgoals;
  double forward_time, backward_time;
};


#endif /* APPS_SUBGOALGRAPH_UTILS_AGGREGATECONNECTIONSTATISTICS_H_ */
