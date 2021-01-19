/*
 * CommonHeuristics.h
 *
 *  Created on: Oct 23, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_UTILS_COMMONHEURISTICS_H_
#define APPS_SUBGOALGRAPH_UTILS_COMMONHEURISTICS_H_

#include "Grid2DGraph.h"
#include "Lattice.h"

double OctileDistance(xyLoc l1, xyLoc l2);
double EuclideanDistance(xyLoc l1, xyLoc l2);
double EuclideanDistance(xyThetaPos l1, xyThetaPos l2);

template <class G = Grid2DGraph>
class OctileDistanceHeuristic {
 public:
  OctileDistanceHeuristic(G* g)
     : g_(g) {}
  ~OctileDistanceHeuristic() {}
  Distance GetHeuristicDistance(nodeId n1, nodeId n2) {
    return OctileDistance(g_->ToXYLoc(n1), g_->ToXYLoc(n2));
  }
  bool IsConsistent() {return true;}
 private:
  G* g_;
};

template <class G = Grid2D>
class EuclideanDistanceHeuristic {
 public:
  EuclideanDistanceHeuristic(G* g)
     : g_(g) {}
  ~EuclideanDistanceHeuristic() {}
  Distance GetHeuristicDistance(nodeId n1, nodeId n2) {
    return EuclideanDistance(g_->ToState(n1), g_->ToState(n2));
  }
  bool IsConsistent() {return true;}
 private:
  G* g_;
};


class ZeroHeuristic {
 public:
  ZeroHeuristic() {}
  ~ZeroHeuristic() {}
  Distance GetHeuristicDistance(nodeId n1, nodeId n2) {
    return 0;
  }
  bool IsConsistent() {return true;}
};

class InfiniteHeuristic {
 public:
  InfiniteHeuristic() {}
  ~InfiniteHeuristic() {}
  Distance GetHeuristicDistance(nodeId n1, nodeId n2) {
    return kMaxDistance;
  }
  bool IsConsistent() {return false;}
};

// M: Id mapper.
// H: Heuristic.
template<class M, class H>
class MappedHeuristic {
 public:
  MappedHeuristic(M* sm, H* heuristic)
      : sm_(sm),
        h_(heuristic) {
    assert(h_ != NULL);assert(sm_ != NULL);
  }

  double GetHeuristicDistance(nodeId n1, nodeId n2) {
    return h_->GetHeuristicDistance(sm_->ToNodeId(n1), sm_->ToNodeId(n2));
  }

  bool IsConsistent() {return h_->IsConsistent();}
 private:
  M* sm_;
  H* h_;
};


#endif /* APPS_SUBGOALGRAPH_UTILS_COMMONHEURISTICS_H_ */
