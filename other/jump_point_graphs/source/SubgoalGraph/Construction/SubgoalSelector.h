#ifndef SUBGOAL_SELECTOR_H
#define SUBGOAL_SELECTOR_H

//#define SUBGOAL_SELECTOR_DEBUG

#include <vector>
#include <algorithm>
#include <stdlib.h>

#include "../Parameters/SGConstructionParam.h"
#include "GraphDefinitions.h"
#include "AreaExplorationData.h"

struct SubgoalCandidate {
  SubgoalCandidate()
      : score(0),
        is_fringe_node(false),
        is_candidate(false),
        is_relevant(false),
        should_update_children(false) {
  }

  std::vector<nodeId> parents;
  std::vector<nodeId> children;
  double score;

  bool is_fringe_node;
  bool is_candidate;
  bool is_relevant;
  bool should_update_children;
};

template<class G>
class SubgoalSelector {
 public:
  SubgoalSelector(G* graph, SubgoalSelectionStrategy s = kSecretSauce);
  ~SubgoalSelector() {
  }

  void SetSelectionStrategy(SubgoalSelectionStrategy s) {
    selection_strategy_ = s;
  }
  void SelectSubgoals(const AreaExplorationData* data,
                      const std::vector<nodeId>* fringe_nodes,
                      const std::vector<nodeId>* expansion_order,
                      std::vector<nodeId> & subgoals,
                      bool select_only_one_subgoal = false);

 private:
  G* g_;

  SubgoalSelectionStrategy selection_strategy_;

  int num_nodes_;
  bool explored_backward_;	// TODO: Not yet implemented correctly. Also, it does not seem to be necessary?
  std::vector<SubgoalCandidate> node_data_;
  std::vector<nodeId> expansion_order_;
  std::vector<nodeId> fringe_nodes_;
  std::vector<nodeId> candidate_list_;

  void GetParents(nodeId n, const AreaExplorationData* data,
                  std::vector<nodeId> & parents);
  void GenerateCandidates(const AreaExplorationData* data,
                          const std::vector<nodeId>* fringe_nodes,
                          const std::vector<nodeId>* expansion_order);
  void UpdateAfterPlacingSubgoal(nodeId subgoal);
  nodeId SelectSubgoal(const AreaExplorationData* data);
  void Debug(const AreaExplorationData* data);
};

//typedef SubgoalSelector<XYThetaLatticeGraph> XYThetaLatticeSubgoalSelector;

#include "SubgoalSelector.inc"

#endif
