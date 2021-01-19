/*
 * SearchUtils.h
 *
 *  Created on: Feb 25, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_UTILS_SEARCHUTILS_H_
#define APPS_SUBGOALGRAPH_UTILS_SEARCHUTILS_H_
#include "../Graphs/GraphDefinitions.h"

inline
void ExtractPath(nodeId start, nodeId goal, std::vector<nodeId> & parent,
                 std::vector<nodeId> & path) {
  path.clear();
  nodeId curr = goal;
  while (curr != start) {
    path.push_back(curr);
    curr = parent[curr];
  }
  path.push_back(curr);
  std::reverse(path.begin(), path.end());
}


#endif /* APPS_SUBGOALGRAPH_UTILS_SEARCHUTILS_H_ */
