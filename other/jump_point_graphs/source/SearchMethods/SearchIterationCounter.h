/*
 * SearchIterationCounter.h
 *
 *  Created on: Oct 28, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_UTILS_SEARCHITERATIONCOUNTER_H_
#define APPS_SUBGOALGRAPH_UTILS_SEARCHITERATIONCOUNTER_H_
#include <vector>

#include "../Graphs/GraphDefinitions.h"

class SearchIterationCounter {
 public:
  SearchIterationCounter(int max_node_id) {
    search_ = 0;
    generated_.resize(max_node_id, 0);
  }

  void Reset() {
    if (search_ == std::numeric_limits<short>::max()) {
      search_ = 0;
      for (unsigned int i = 0; i < generated_.size(); i++)
        generated_[i] = 0;
    }
    search_++;
  }

  bool IsGenerated(nodeId n) {
    return generated_[n] == search_;
  }

  void GenerateNode(nodeId n) {
    generated_[n] = search_;
  }

 private:
  short search_;
  std::vector<short> generated_;
};


#endif /* APPS_SUBGOALGRAPH_UTILS_SEARCHITERATIONCOUNTER_H_ */
