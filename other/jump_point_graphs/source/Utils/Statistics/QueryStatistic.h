/*
 * SearchStatistic.h
 *
 *  Created on: Mar 25, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_UTILS_QUERYSTATISTIC_H_
#define APPS_SUBGOALGRAPH_UTILS_QUERYSTATISTIC_H_
#include <limits>
#include <ostream>

// Search statistic maintenance:
// - Each search algorithm has a function 'AddStatistics' that takes as input a
//   SearchStatistic to which they 'add' their maintained statistics.
// - Maintained statistics are reset when 'Reset' or 'FindPath' is called.
// - In case of a hierarchy of search algorithms (for instance, CH search uses
//   bidirectional dijkstra, which in turn uses two dijkstra's), we can
//   recursively call AddStatistics functions.
// - To make this recursive call report correct statistics, we use the
//   following strategy:
//   - A search algorithm only times the search in its FindPath function.
//   - They keep track of any other statistic normally.
// - Example: BidirectionalDijkstra does not call the FindPath functions of its
//   two Dijkstra searches. Instead, it calls their 'ExpandNext' functions.
//   - Reporting number of expansions: Just call both Dijkstra's AddStatistics.
//     Dijkstra keeps track of expanded nodes regardless of whether FindPath is
//     called.
//   - Report time: Keep track of it in FindPath and add it to the sum. Since
//     Dijkstra's FindPath are not called during search, Dijkstra does not
//     keep track of the time, and the calls to Dijktra's AddStatistics
//     will not increase search time.

struct QueryStatistic {
  QueryStatistic() {
    dist = std::numeric_limits<double>::max();
    valid = false;
    shortest = false;
    suboptimality = 1.0;

    // Connect statistics
    connect_time = 0;

    // Search statistics
    search_time = 0;
    num_expanded = 0;
    num_stalled = 0;
    num_generated = 0;
    num_pruned_arcs = 0;

    num_relaxed_arcs = 0;
    num_percolations = 0;
    num_update_key = 0;

    // Refine statistics
    refine_time = 0;

    // backward
    num_backward_expanded = 0;
    num_backward_stalled = 0;
    num_backward_generated = 0;
    num_backward_relaxed_arcs = 0;
    num_backward_pruned_arcs = 0;
  }

  double GetTotalTime() const {
    return connect_time + search_time + refine_time;
  }

  // Filled by query algorithms.
  double connect_time;
  double search_time;
  double refine_time;
  double dist;

  // Filled by SGExperiment.
  bool valid;
  bool shortest;
  double suboptimality;

  // Filled by search algorithms.
  long num_expanded;
  long num_stalled;
  long num_generated;
  long num_relaxed_arcs;
  long num_pruned_arcs;

  // Filled by heap.
  long num_percolations;
  long num_update_key;

  // Backward
  long num_backward_expanded;
  long num_backward_stalled;
  long num_backward_generated;
  long num_backward_relaxed_arcs;
  long num_backward_pruned_arcs;
};


static QueryStatistic operator+(const QueryStatistic &s1, const QueryStatistic &s2) {
  QueryStatistic s;
  s.connect_time = s1.connect_time + s2.connect_time;
  s.search_time = s1.search_time + s2.search_time;
  s.refine_time = s1.refine_time + s2.refine_time;
  s.dist = s1.dist + s2.dist;
  s.valid = s1.valid && s2.valid;
  s.shortest = s1.shortest && s2.shortest;
//  s.suboptimality = s1.suboptimality + s2.suboptimality;
  s.num_expanded = s1.num_expanded + s2.num_expanded;
  s.num_stalled = s1.num_stalled + s2.num_stalled;
  s.num_generated = s1.num_generated + s2.num_generated;
  s.num_relaxed_arcs = s1.num_relaxed_arcs + s2.num_relaxed_arcs;
  s.num_pruned_arcs = s1.num_pruned_arcs + s2.num_pruned_arcs;
  s.num_percolations = s1.num_percolations + s2.num_percolations;
  s.num_update_key = s1.num_update_key + s2.num_update_key;

  return s;
}

static std::ostream& operator <<(std::ostream & out, const QueryStatistic & s)
{
  out << s.connect_time << "\t";
  out << s.search_time << "\t";
  out << s.refine_time << "\t";
  out << s.GetTotalTime() << "\t";

  out << s.dist << "\t";

  out << s.valid << "\t";
  out << s.shortest << "\t";
  out << s.suboptimality << "\t";

  out << s.num_expanded << "\t";
  out << s.num_stalled << "\t";
  out << s.num_generated << "\t";
  out << s.num_relaxed_arcs << "\t";
  out << s.num_pruned_arcs << "\t";

  out << s.num_percolations << "\t";
  out << s.num_update_key << "\n";
  return out;
}




#endif /* APPS_SUBGOALGRAPH_UTILS_QUERYSTATISTIC_H_ */
