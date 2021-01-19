/*
 * Lattice2DHeuristic.h
 *
 *  Created on: Mar 25, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_GRAPHS_LATTICE_LATTICE2DHEURISTIC_H_
#define APPS_SUBGOALGRAPH_GRAPHS_LATTICE_LATTICE2DHEURISTIC_H_
#include "../../SearchMethods/Dijkstra.h"
#include "../Grid2D/Grid2DGraph.h"
#include "LatticeGraph.h"


class Lattice2DHeuristic {
 public:
  Lattice2DHeuristic(LatticeGraph* lg)
      : lg_(lg), g_(new GPPCGrid(lg->GetLattice()->GetGrid())), gg_(g_), dij_(&gg_) {
    goal = kNonNode;
    is_consistent_ = false;
//    MakeConsistent();
  }
  Distance GetHeuristicDistance(nodeId from, nodeId to) {
    // If to is different from the last time this is called, start a new search
    // backwards from to's cell.
    if (to != goal) {
      goal = to;
      dij_.Reset();
      dij_.AddStart(DownProject(to));
    }

    // Make sure that we know the correct g-value from to's cell to from's cell.
    nodeId f = DownProject(from);
    if (!dij_.IsTentativeDistanceCertain(f))
      dij_.FindGoal(f);

//    std::cout<<dij_.GetTentativeDistance(f)<<" "<<std::flush;
    return dij_.GetTentativeDistance(f);
  }

  bool IsConsistent() {return is_consistent_;}

 private:
  nodeId DownProject(nodeId lattice_node_id) {
    xyThetaPos p = lg_->ToState(lattice_node_id);
    return gg_.ToNodeId(xyLoc(p.x, p.y));
  }

  /*
  void MakeConsistent() {
    auto CalculateMaxDiagCost = [&](MotionPrimitive & p) -> Distance {
      xyThetaPos pos = p.GetResultingPos(
          xyThetaPos(0,0,p.GetDiscreteStartAngle()));
      int x = abs(pos.x);
      int y = abs(pos.y);
      if (x > y)
        std::swap(x,y);
      int d = x;
      int c = y-x;

      return (p.GetCost() - c)/d;
    };

    Distance max_diag_cost = 2;
    for (int a = 0; a < lg_->GetLattice()->GetNumAngles(); a++)
      for (auto p : *(lg_->GetLattice()->GetAllForwardPrimitives(a)))
        max_diag_cost = std::min((double)max_diag_cost, (double)CalculateMaxDiagCost(p));

    g_->SetCosts(1, max_diag_cost);
    std::cout << "Lattice 2D heuristic sets diag costs to " << max_diag_cost
              << " in order to make the heuristic consistent." << std::endl;
    is_consistent_ = true;
  }
  */

  LatticeGraph* lg_;
  GPPCGrid* g_;
  Grid2DGraph gg_;
  Dijkstra<Grid2DGraph> dij_;
  nodeId goal;

  bool is_consistent_;
};


#endif /* APPS_SUBGOALGRAPH_GRAPHS_LATTICE_LATTICE2DHEURISTIC_H_ */
