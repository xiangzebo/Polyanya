/*
 * RedundantArcCheck.h
 *
 *  Created on: Feb 13, 2019
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_UTILS_REDUNDANTARCCHECKER_H_
#define APPS_SUBGOALGRAPH_UTILS_REDUNDANTARCCHECKER_H_
#include "../Graphs/ArcListQueryGraph.h"
#include "../Graphs/Grid2D/Grid2DGraph.h"
#include "../SearchMethods/BidirectionalDijkstra.h"
#include "../SubgoalGraph/SubgoalIdMapper.h"
#include "GroupingMapper.h"

// FIXME: Not used at the moment...

class RedundantArcChecker {
 public:
  RedundantArcChecker(int num_nodes, std::vector<WeightedArc> & arcs,
                      GroupingMapper* gm) {
    g_ = NULL;

  }


  ~RedundantArcChecker() {}

  template<class A>
  bool IsRedundant(A & a) {
    return IsRedundantUsingGroup(a);
  }


 private:
  Grid2DGraph* g_;
  ArcListQueryGraph<WeightedArcHead>* fg_, bg_;
  BidirectionalDijkstra<ArcListQueryGraph<WeightedArcHead>,
      ArcListQueryGraph<WeightedArcHead>>* bidij_;
  GroupingMapper* gm_;
  SubgoalIdMapper* sm_;

  template<class A>
  bool IsRedundantUsingGroup(A & a) {
    bidij_->ResetSearchInfo();
    bidij_->GetForward()->Reset();
    bidij_->GetBackward()->Reset();
    auto s_nodes = gm_->GetGroupNodes(a.source);
    auto g_nodes = gm_->GetGroupNodes(a.target);
    for (auto n : *s_nodes) {
      bidij_->GetForward()->AddStart(n);
    }
    for (auto n : *g_nodes) {
      bidij_->GetBackward()->AddStart(n);
    }
    while (!bidij_->CanStop() && bidij_->GetSumOfRadii() <= a.weight + 0.001) {
      bidij_->ExpandNext();
    }
    return bidij_->GetBestDistance() + kEpsDistance < a.weight;
  }

  template<class A>
  bool IsRedundantUsingIdMapper(A & a) {
    bidij_->ResetSearchInfo();
    bidij_->GetForward()->Reset();
    bidij_->GetBackward()->Reset();
    auto s_nodes = gm_->GetGroupNodes(a.source);
    auto g_nodes = gm_->GetGroupNodes(a.target);
    for (auto n : *s_nodes) {
      bidij_->GetForward()->AddStart(n);
    }
    for (auto n : *g_nodes) {
      bidij_->GetBackward()->AddStart(n);
    }
    while (!bidij_->CanStop() && bidij_->GetSumOfRadii() <= a.weight + 0.001) {
      bidij_->ExpandNext();
    }
    return bidij_->GetBestDistance() + kEpsDistance < a.weight;
  }

};



#endif /* APPS_SUBGOALGRAPH_UTILS_REDUNDANTARCCHECKER_H_ */
