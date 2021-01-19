/*
 * SequentialPathRefiner.h
 *
 *  Created on: Apr 11, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_SUBGOALGRAPH_SEARCH_SEQUENTIALPATHREFINER_H_
#define APPS_SUBGOALGRAPH_SUBGOALGRAPH_SEARCH_SEQUENTIALPATHREFINER_H_

#include <vector>

#include "../../Graphs/GraphDefinitions.h"
#include "../../Utils/CPUTimer.h"

template <class R1, class R2>
class SequentialPathRefiner {
 public:
  SequentialPathRefiner(R1* r1, R2* r2)
     : r1_(r1), r2_(r2) {}
  ~SequentialPathRefiner() {}

  double RefinePath(std::vector<nodeId> & unrefined_path,
                  std::vector<nodeId> & refined_path) {
    std::vector<nodeId> intermediate_path;
    intermediate_path.reserve(2000);
    CPUTimer t;
    t.StartTimer();
    r1_->RefinePath(unrefined_path, intermediate_path);
    r2_->RefinePath(intermediate_path, refined_path);
    return t.EndTimer();
  }

 private:
  R1* r1_;
  R2* r2_;
};




#endif /* APPS_SUBGOALGRAPH_SUBGOALGRAPH_SEARCH_SEQUENTIALPATHREFINER_H_ */
