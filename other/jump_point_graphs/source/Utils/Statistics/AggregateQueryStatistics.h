#ifndef SUBGOAL_GRAPH_STATISTICS_H
#define SUBGOAL_GRAPH_STATISTICS_H

#include "QueryStatistic.h"
#include "DefaultParams.h"
#include "StatisticsFile.h"

#include <string>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <vector>
#include <limits>
#include <fstream>

// General:
// - If a filename is specified, opens two files:
//   - filename.stat : Preprocessing, agggregate search, etc, statistics.
//   - filename.stat.ins : Instance-based statistics.
// - If filename = "" (or if echo = true), uses cout for rather than .stat
// - Provides functions to add int/double statistics or remarks to .stat file.

// Search:
// - A SearchStatistic can be posted to the Statistic class after each query.
// - Can average search statistics and print them to .stat
// - Can dump search statistics to .stat.ins
// - Search times are reported as seconds (by using CPUTimer).


class AggregateQueryStatistics {
 public:
  AggregateQueryStatistics(std::string instance_file) {
    OpenInstanceFile(instance_file);
  }
  ~AggregateQueryStatistics() {
    CloseInstanceFile();
  }

  void OpenInstanceFile(std::string instance_file) {
    if (instance_file != "") {
      instance_.open(instance_file.c_str());
      instance_ << std::fixed;
    }
  }
  void CloseInstanceFile() {
    if (instance_.is_open())
      instance_.close();
  }

  // Updates the statistics with data from a single search.
  void AddQueryData(QueryStatistic s) {
    results_.push_back(s);
    if (instance_.is_open())
      instance_<<s;
  }

  void Reset() {
    results_.clear();
  }

  // TODO: Make total_init, total_search etc private;
  // have a function for calculating totals;
  // two separate functions, one for printing, one for average statistic
  QueryStatistic GetAggregateStatistics () {
    QueryStatistic aggr;

    double total_init = 0;
    double total_search = 0;
    double total_finalize = 0;
    double total_dist = 0;
    double total_subopt = 0;

    long total_generated = 0;
    long total_expanded = 0;
    long total_stalled = 0;
    long total_relaxed_arcs = 0;
    long total_pruned_arcs = 0;
    long total_pecolations = 0;
    long total_update_key = 0;

    long total_backward_expanded = 0;
    long total_backward_stalled = 0;

    for (unsigned int i = 0; i < results_.size(); i++) {
      total_init += results_[i].connect_time;
      total_search += results_[i].search_time;
      total_finalize += results_[i].refine_time;
      total_dist += results_[i].dist;
      total_subopt += results_[i].suboptimality;

      total_generated += results_[i].num_generated;
      total_stalled += results_[i].num_stalled;
      total_expanded += results_[i].num_expanded;
      total_relaxed_arcs += results_[i].num_relaxed_arcs;
      total_pruned_arcs += results_[i].num_pruned_arcs;
      total_pecolations += results_[i].num_percolations;
      total_update_key += results_[i].num_update_key;

      total_backward_expanded += results_[i].num_backward_expanded;
      total_backward_expanded += results_[i].num_backward_stalled;
    }

    int n = results_.size();
    aggr.connect_time = total_init / (double) n;
    aggr.search_time = total_search / (double) n;
    aggr.refine_time = total_finalize / (double) n;
    aggr.dist = total_dist / (double) n;
    aggr.suboptimality = total_subopt / (double) n;
    aggr.num_generated = total_generated / (double) n;
    aggr.num_stalled = total_stalled / (double) n;
    aggr.num_expanded = total_expanded / (double) n;
    aggr.num_relaxed_arcs = total_relaxed_arcs / (double) n;
    aggr.num_pruned_arcs = total_pruned_arcs / (double) n;
    aggr.num_percolations = total_pecolations / (double) n;
    aggr.num_update_key = total_update_key / (double) n;

    aggr.num_backward_expanded = total_backward_expanded / (double) n;
    aggr.num_backward_stalled = total_backward_stalled / (double) n;

    return aggr;
  }

  // Prints all the accumulated statistics.
  void Report(StatisticsFile* sf) {
    double total_init = 0;
    double total_search = 0;
    double total_finalize = 0;
    int num_valid = 0;
    int num_suboptimal = 0;
    double total_dist = 0;
    double total_subopt = 0;

    long total_generated = 0;
    long total_expanded = 0;
    long total_stalled = 0;
    long total_relaxed_arcs = 0;
    long total_pruned_arcs = 0;
    long total_pecolations = 0;
    long total_update_key = 0;

    double max_suboptimality = 1;
    double total_shortest_dist = 0;

    for (unsigned int i = 0; i < results_.size(); i++) {
      total_init += results_[i].connect_time;
      total_search += results_[i].search_time;
      total_finalize += results_[i].refine_time;
      total_dist += results_[i].dist;
      total_subopt += results_[i].suboptimality;
      if (results_[i].valid)
        num_valid++;
      if (!results_[i].shortest)
        num_suboptimal++;

      total_generated += results_[i].num_generated;
      total_stalled += results_[i].num_stalled;
      total_expanded += results_[i].num_expanded;
      total_relaxed_arcs += results_[i].num_relaxed_arcs;
      total_pruned_arcs += results_[i].num_pruned_arcs;
      total_pecolations += results_[i].num_percolations;
      total_update_key += results_[i].num_update_key;

      max_suboptimality = std::max(max_suboptimality, results_[i].suboptimality);
      total_shortest_dist += results_[i].dist/results_[i].suboptimality;
    }
    // Convert from seconds to milliseconds.
    total_init = total_init * 1000;
    total_search = total_search * 1000;
    total_finalize = total_finalize * 1000;

    int n = results_.size();
    sf->ReportInt("Number of instances", n);
    sf->ReportInt("Number of solutions", num_valid);
    sf->ReportInt("Number of suboptimal solutions", num_suboptimal);
    sf->ReportDouble("Average solution length", total_dist / (double) n);
    sf->ReportDouble("Average suboptimality", total_subopt / (double) n);
    sf->ReportDouble("Max suboptimality", max_suboptimality);
    sf->ReportDouble("Overall suboptimality", total_dist / total_shortest_dist);
    sf->AddRemark("");

    sf->ReportDouble("Average initialize time (ms)",
                          total_init / (double) n);
    sf->ReportDouble("Average search time (ms)",
                          total_search / (double) n);
    sf->ReportDouble("Average finalize time (ms)",
                          total_finalize / (double) n);
    sf->ReportDouble(
        "Average query time (ms)",
        (total_init + total_search + total_finalize) / (double) n);
    sf->AddRemark("");
    sf->ReportDouble("Average number of stalled or expanded nodes",
                          (total_expanded + total_stalled) / (double) n);

    sf->ReportDouble("Average number of expanded nodes",
                          total_expanded / (double) n);
    sf->ReportDouble("Average number of stalled nodes",
                          total_stalled / (double) n);

    sf->ReportDouble("Average number of generated nodes",
                          total_generated / (double) n);
    sf->ReportDouble("Average number of relaxed arcs",
                          total_relaxed_arcs / (double) n);
    sf->ReportDouble("Average number of pruned arcs",
                          total_pruned_arcs / (double) n);
    sf->ReportDouble("Average number of heap percolations",
                          total_pecolations / (double) n);
    sf->ReportDouble("Average number of insert or update key operations",
                          total_update_key / (double) n);
    sf->AddRemark("");
  }

  double GetAverageTime() {
    double total_time = 0;
    for (unsigned int i = 0; i < results_.size(); i++)
      total_time += results_[i].GetTotalTime();
    return total_time / results_.size();
  }
 private:
  std::vector<QueryStatistic> results_;
  std::ofstream instance_;
};

#endif
