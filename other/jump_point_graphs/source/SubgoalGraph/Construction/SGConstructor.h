#ifndef SG_CONSTRUCTOR
#define SG_CONSTRUCTOR

#include "GraphDefinitions.h"
#include "SubgoalGenerator.h"
#include "CPUTimer.h"

#include <algorithm>
#include "SGArcConstructor.h"

// TODO: Change template arguments to provide a subgoal identifier?
// Easy hack: just identify subgoals some other way, and then call the
// constructor for edges

// TODO: Construct with all 3 components of a subgoal graph + the original graph

// For now, only useful for visualization: We create the constructor and
// can select which source cell to generate subgoals from.

template<class SM, class SGM, class G, class E>
class SGConstructor {
 public:
  SGConstructor(SM* sm, SGM* sg, G* graph, E* explorer)
      : sm_(sm),
        sg_(sg),
        g_(graph),
        explorer_(explorer),
        subgoal_generator_(sm, graph, explorer) {
    total_construction_time_in_seconds_ = 0;
    identify_subgoal_time_in_seconds_ = 0;
    identify_edge_time_in_seconds_ = 0;
  }

  ~SGConstructor() {}

  double IdentifySubgoals() {
    return subgoal_generator_.IdentifySubgoals();
  }
  double IdentifyEdges() {
    SGArcConstructor arc_constructor;
    return arc_constructor.ConstructWeightedArcGraph_DirectRReachable(sm_, sg_,
                                                                      explorer_);
  }
  void ConstructSubgoalGraph() {
    std::cout << "Constructing Subgoal Graph..." << std::endl;

    identify_subgoal_time_in_seconds_ = IdentifySubgoals();
    identify_edge_time_in_seconds_ = IdentifyEdges();
    total_construction_time_in_seconds_ = identify_subgoal_time_in_seconds_
        + identify_edge_time_in_seconds_;

    std::cout << sm_->GetNumSubgoals() << " subgoals identified in "
        << identify_subgoal_time_in_seconds_ << " seconds." << std::endl;
    std::cout << sg_->GetNumArcs() << " directed edges identified in "
        << identify_edge_time_in_seconds_ << " seconds." << std::endl;
    std::cout << "Subgoal Graph constructed in "
        << total_construction_time_in_seconds_ << " seconds." << std::endl;
  }

  int GenerateSubgoalsForSourceNode(nodeId n, bool forward = true) {
    return subgoal_generator_.GenerateSubgoalsForSourceNode(n, forward);
  }
  double GetConstructionTimeInSeconds() const {
    return total_construction_time_in_seconds_;
  }
  double GetIdentifySubgoalTimeInSeconds() const {
    return identify_subgoal_time_in_seconds_;
  }
  double GetIdentifyEdgeTimeInSeconds() const {
    return identify_edge_time_in_seconds_;
  }

 private:
  SM* sm_;
  SGM* sg_;
  G* g_;
  E* explorer_;
  SubgoalGenerator<SM, G, E> subgoal_generator_;

  double total_construction_time_in_seconds_, identify_subgoal_time_in_seconds_,
      identify_edge_time_in_seconds_;

};

#endif
