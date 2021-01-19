/*
 * SGGridParameters.h
 *
 *  Created on: Nov 4, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_PARAMETERS_CHPARAM_H_
#define APPS_SUBGOALGRAPH_PARAMETERS_CHPARAM_H_
#include <iostream>
#include <string>
#include <sstream>
#include "DefaultParams.h"

enum ContractionType {
  kNoContraction,
  kRegularContraction,
  kRContraction,
  kHeavyRContraction
};

struct CHParam {
  // Default: Dijkstra on original graph.
  CHParam() {
    use_subgoal_graph = false;
    contraction_type = kNoContraction;
    contract_subgoals_last = false;
    r_refine_when_possible = false;
    unpack_using_pointers_ = false;

    use_stall_on_demand = false;
    use_astar = false;
    use_bidirectional_dijkstra = false;

    // Not varied for experiments
    witness_search_expansion_bound = std::numeric_limits<int>::max();
    online_order_update_type = kCHParamDefaultUpdatePolicy;

    save = kCHParamDefaultSave;
    load = kCHParamDefaultLoad;
    clear = kCHParamDefaultClear;

    report_increment_in_seconds = 10;

    w = 1.0;
  }

  CHParam(bool sg,
              ContractionType ct,
              bool stall = true,
              bool astar = false,
              bool r_refine_when_possible_ = false,
              bool contract_subgoals_last_ = false,
              bool unpack_using_pointers = false) {
    use_subgoal_graph = sg;
    contraction_type = ct;
    use_stall_on_demand = stall;
    use_astar = astar;
    use_bidirectional_dijkstra = false;

    r_refine_when_possible = r_refine_when_possible_;
    contract_subgoals_last = contract_subgoals_last_;

    unpack_using_pointers_ = unpack_using_pointers;

    witness_search_expansion_bound = std::numeric_limits<int>::max();
    online_order_update_type = kCHParamDefaultUpdatePolicy;

    save = kCHParamDefaultSave;
    load = kCHParamDefaultLoad;
    clear = kCHParamDefaultClear;

    report_increment_in_seconds = 10;

    w = 1.0;
  }

  std::string GetGraphName() {
    if (contraction_type == kNoContraction) {
      return use_subgoal_graph ? "SUB": "G";
    }
    else {
      std::string s;
      if (use_subgoal_graph)
        s = "SUB-";
      if (contraction_type == kRegularContraction) {
        s = s + "CH";
        if (contract_subgoals_last)
          s += "-SL";
      }
      if (contraction_type == kRContraction)
        s = s + "RCH";
      if (contraction_type == kHeavyRContraction)
        s = s + "N";
      return s;
    }
  }

  std::string GetMethodName() {
    std::string s = GetGraphName();
    if (r_refine_when_possible) {
      s += "-R";
    }
    return s;
  }

  std::string GetSearchName() {
    std::stringstream s;

    if (use_astar) {
      s << "A";
      if (fabs(w - 1.0) > 0.001)
        s << w;
    }
    else if (use_bidirectional_dijkstra)
      s << "BD";
    else
      s << "D";

    if (use_stall_on_demand)
      s << "S";

    if (contraction_type == kRegularContraction) {
      if (unpack_using_pointers_)
        s << "P";
      else
        s << "M";
    }

    return s.str();
  }

/*
  std::string GetConfigName() {
    std::string s = GetMethodName();
//    if (contraction_type == kRegularContraction && unpack_using_pointers_)
//      s += "2";
    s = s + "-" + GetSearchName();
    return s;
  }
*/
  bool use_subgoal_graph; // As base graph.
  ContractionType contraction_type; // Regular / R / Heavy-R.
  ContractionOrderUpdatePolicy online_order_update_type;
  bool use_stall_on_demand;
  bool use_astar; // Rather than bidirectional dijkstra.

  bool use_bidirectional_dijkstra;  // On the original graph.
  double w;

  bool contract_subgoals_last;
  bool r_refine_when_possible;

  bool unpack_using_pointers_;

  int witness_search_expansion_bound;

  int report_increment_in_seconds;

  bool save, load, clear;
};

inline
CHParam MakeGParam(bool sg, bool astar = true) {
  CHParam p;
  p.use_subgoal_graph = sg;
  p.use_astar = astar;
  return p;
}

inline
CHParam MakeBidijGParam() {
  CHParam p;
  p.use_subgoal_graph = false;
  p.use_astar = false;
  p.use_bidirectional_dijkstra = true;
  return p;
}

inline
CHParam MakeWAStarGParam(double w) {
  CHParam p;
  p.use_subgoal_graph = false;
  p.use_astar = true;
  p.w = w;
  return p;
}


// TODO: CH-SL?
inline
CHParam MakeCHParam(bool sg, bool r_refine, bool pointer_unpacking,
                                 bool astar = true, bool stall = true) {
  CHParam p;
  p.contraction_type = kRegularContraction;
  p.use_subgoal_graph = sg;
  p.r_refine_when_possible = r_refine;
  p.unpack_using_pointers_ = pointer_unpacking;
  p.use_astar = astar;
  p.use_stall_on_demand = stall;
  return p;
}

inline
CHParam MakeCHSLParam(bool pointer_unpacking, bool astar = true,
                      bool stall = true) {
  CHParam p;
  p.contraction_type = kRegularContraction;
  p.contract_subgoals_last = true;
  p.unpack_using_pointers_ = pointer_unpacking;
  p.use_astar = astar;
  p.use_stall_on_demand = stall;
  return p;
}


inline
CHParam MakeRCHParam(bool sg, bool astar = true, bool stall = true) {
  CHParam p;
  p.contraction_type = kRContraction;
  p.use_subgoal_graph = sg;
  p.use_astar = astar;
  p.use_stall_on_demand = stall;
  return p;
}

inline
CHParam MakeNSUBParam(bool astar = true, bool stall = true) {
  CHParam p;
  p.use_subgoal_graph = true;
  p.contraction_type = kHeavyRContraction;
  p.use_astar = astar;
  p.use_stall_on_demand = stall;
  return p;
}

// S: Use subgoal graph; St: Stall; R: R-refine when possible
//            Name           S  Contraction          St A* R M
const CHParam kCHP_G_D      (0, kNoContraction,      0, 0);
const CHParam kCHP_SUB_D    (1, kNoContraction,      0, 0);
const CHParam kCHP_G_A      (0, kNoContraction,      0, 1);
const CHParam kCHP_SUB_A    (1, kNoContraction,      0, 1);

// A* + stall
const CHParam kCHP_CH_AS    (0, kRegularContraction, 1, 1);
const CHParam kCHP_CHR_AS   (0, kRegularContraction, 1, 1, 1);
const CHParam kCHP_RCH_AS   (0, kRContraction,       1, 1);
const CHParam kCHP_SUBCH_AS (1, kRegularContraction, 1, 1);
const CHParam kCHP_SUBCHR_AS(1, kRegularContraction, 1, 1, 1);
const CHParam kCHP_SUBRCH_AS(1, kRContraction,       1, 1);
const CHParam kCHP_SUBN_AS  (1, kHeavyRContraction,  1, 1);

const CHParam kCHP_CH_ASM    (0, kRegularContraction, 1, 1, 0, 1);
const CHParam kCHP_CHR_ASM   (0, kRegularContraction, 1, 1, 1, 1);
const CHParam kCHP_SUBCH_ASM (1, kRegularContraction, 1, 1, 0, 1);
const CHParam kCHP_SUBCHR_ASM(1, kRegularContraction, 1, 1, 1, 1);

// A* + NO stall
const CHParam kCHP_CH_A     (0, kRegularContraction, 0, 1);
const CHParam kCHP_CHR_A    (0, kRegularContraction, 0, 1, 1);
const CHParam kCHP_RCH_A    (0, kRContraction,       0, 1);
const CHParam kCHP_SUBCH_A  (1, kRegularContraction, 0, 1);
const CHParam kCHP_SUBCHR_A (1, kRegularContraction, 0, 1, 1);
const CHParam kCHP_SUBRCH_A (1, kRContraction,       0, 1);
const CHParam kCHP_SUBN_A   (1, kHeavyRContraction,  0, 1);

// Dijkstra + stall
const CHParam kCHP_CH_DS    (0, kRegularContraction, 1, 0);
const CHParam kCHP_CHR_DS   (0, kRegularContraction, 1, 0, 1);
const CHParam kCHP_RCH_DS   (0, kRContraction,       1, 0);
const CHParam kCHP_SUBCH_DS (1, kRegularContraction, 1, 0);
const CHParam kCHP_SUBCHR_DS(1, kRegularContraction, 1, 0, 1);
const CHParam kCHP_SUBRCH_DS(1, kRContraction,       1, 0);
const CHParam kCHP_SUBN_DS  (1, kHeavyRContraction,  1, 0);

// Dijkstra + NO stall
const CHParam kCHP_CH_D     (0, kRegularContraction, 0, 0);
const CHParam kCHP_CHR_D    (0, kRegularContraction, 0, 0, 1);
const CHParam kCHP_RCH_D    (0, kRContraction,       0, 0);
const CHParam kCHP_SUBCH_D  (1, kRegularContraction, 0, 0);
const CHParam kCHP_SUBCHR_D (1, kRegularContraction, 0, 0, 1);
const CHParam kCHP_SUBRCH_D (1, kRContraction,       0, 0);
const CHParam kCHP_SUBN_D   (1, kHeavyRContraction,  0, 0);

#endif /* APPS_SUBGOALGRAPH_PARAMETERS_CHPARAM_H_ */
