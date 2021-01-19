/*
 * DummyPathReiner.h
 *
 *  Created on: Apr 11, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_SUBGOALGRAPH_SEARCH_DUMMYPATHREFINER_H_
#define APPS_SUBGOALGRAPH_SUBGOALGRAPH_SEARCH_DUMMYPATHREFINER_H_

#include <vector>

#include "../../Graphs/GraphDefinitions.h"


class DummyPathRefiner {
 public:
  DummyPathRefiner() {}
  double RefinePath(std::vector<nodeId> & unrefined_path,
                  std::vector<nodeId> & refined_path) {
    refined_path = unrefined_path;
    return 0;
  }
};


#endif /* APPS_SUBGOALGRAPH_SUBGOALGRAPH_SEARCH_DUMMYPATHREFINER_H_ */
