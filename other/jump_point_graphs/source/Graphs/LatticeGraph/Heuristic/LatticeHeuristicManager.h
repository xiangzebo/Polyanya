/*
 * LatticeHeuristicManager.h
 *
 *  Created on: Mar 28, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_GRAPHS_LATTICE_LATTICEHEURISTICMANAGER_H_
#define APPS_SUBGOALGRAPH_GRAPHS_LATTICE_LATTICEHEURISTICMANAGER_H_

#include <stddef.h>

#include "../../../Parameters/SGLatticeParam.h"
#include "../../CommonHeuristics.h"
#include "../../GraphDefinitions.h"
#include "../LatticeGraph.h"
#include "Lattice2DHeuristic.h"


class LatticeHeuristicManager {
 public:
  LatticeHeuristicManager(LatticeGraph* g, LatticeHeuristicType ht =
                              kLattice2DProjectionHeuristic) {
    euc_ = NULL;
    h2d_ = NULL;
    ht_ = ht;

    if (ht_ == kLatticeEuclideanHeuristic)
      euc_ = new EuclideanDistanceHeuristic<LatticeGraph>(g);
    else if (ht_ == kLattice2DProjectionHeuristic)
      h2d_ = new Lattice2DHeuristic(g);
  }
  ~LatticeHeuristicManager() {
   if (h2d_)
     delete h2d_;
   if (euc_)
     delete euc_;
  }

  Distance GetHeuristicDistance(nodeId from, nodeId to) {
    if (ht_ == kLatticeEuclideanHeuristic)
      return euc_->GetHeuristicDistance(from, to);
    else if (ht_ == kLattice2DProjectionHeuristic)
      return h2d_->GetHeuristicDistance(from, to);
    return 0;
  }

  bool IsConsistent() {
    if (ht_ == kLattice2DProjectionHeuristic)
      return h2d_->IsConsistent();
    return true;
  }

 private:
  EuclideanDistanceHeuristic<LatticeGraph>* euc_;
  Lattice2DHeuristic* h2d_;
  LatticeHeuristicType ht_;
};


#endif /* APPS_SUBGOALGRAPH_GRAPHS_LATTICE_LATTICEHEURISTICMANAGER_H_ */
