/*
 * SGPathRefiner.h
 *
 *  Created on: Nov 5, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_SUBGOALGRAPH_SGPATHREFINER_H_
#define APPS_SUBGOALGRAPH_SUBGOALGRAPH_SGPATHREFINER_H_
#include <vector>

#include "../SubgoalIdMapper.h"
#include "../Utils/CPUTimer.h"
#include "SubgoalGraphDefinitions.h"

template <class R, class SM = IdentityMapper>
class SGPathRefiner {
public:
  SGPathRefiner(R* r, SM* sm = NULL)
    : r_reachability_(r), sm_(sm) {}

  // Uses the reachability relation to refine a path on the subgoal graph to a
  // path on the original graph. Returns the time.
  double RefinePath(std::vector<subgoalId> & sg_path,
                    std::vector<nodeId> & node_path) {
    CPUTimer t;
    t.StartTimer();

    node_path.clear();
    if (sg_path.empty())
      return 0;

    node_path.push_back(ToNodeId(sg_path[0]));
    for (unsigned int i = 1; i < sg_path.size(); i++) {
      r_reachability_->RRefine(ToNodeId(sg_path[i - 1]), ToNodeId(sg_path[i]),
                               node_path, true);
    }
    r_reachability_->Reset();
    return t.EndTimer();
  }

private:
  R* r_reachability_;
  SM* sm_;
  nodeId ToNodeId(nodeId n) {
    if (sm_ == NULL)
      return n;
    else
      return sm_->ToNodeId(n);
  }
};

#endif /* APPS_SUBGOALGRAPH_SUBGOALGRAPH_SGPATHREFINER_H_ */
