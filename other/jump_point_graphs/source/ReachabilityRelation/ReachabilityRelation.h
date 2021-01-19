/*
 * RReachability.h
 *
 *  Created on: Mar 4, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_REACHABILITYRELATION_REACHABILITYRELATION_H_
#define APPS_SUBGOALGRAPH_REACHABILITYRELATION_REACHABILITYRELATION_H_

// TODO: map environment only if hog
#include "../Utils/PreallocatedPriorityQueue.h"
#include "../Utils/Statistics/ExplorationStatistics.h"
#include "AreaExplorationData.h"

#ifndef NO_HOG
#include "../../../environments/Map2DEnvironment.h"
enum HRAreaDisplayType {
  kDontDisplayRReachableArea,
  kRReachableAreaDisplayNode,
  kRReachableAreaDisplayParent,
  kRReachableAreaDisplayNodeAndParent,
  kRReachableAreaDisplayAllPaths,
  kDisplayPrimitives,
  kDisplaySGForwardEdges,
  kDisplaySGBackwardEdges,
  kDisplayClearanceLines
};
#endif

// Basic requirements from a reachability relation.
class ReachabilityRelation {
 public:
  ReachabilityRelation() {
    exploring_backward_ = false;
  }
  virtual ~ReachabilityRelation() {}

  // R-Connect
  void SetExploreForward() {
    exploring_backward_ = false;
  }
  void SetExploreBackward() {
    exploring_backward_ = true;
  }
  virtual void RConnect(nodeId start, bool can_identify_superset = false) = 0;

  const std::vector<nodeId>* GetExistingSubgoals() const {
    return &existing_subgoals_;
  }
  const std::vector<double>* GetExistingSubgoalDistances() const {
    return &existing_subgoal_distances_;
  }
  const std::vector<nodeId>* GetExpansionOrder() const {
    return &expansion_order_;
  }

  // R-refine
  // Assumes that the points are RReachable.
  virtual void RRefine(nodeId start, nodeId goal, std::vector<nodeId> & path,
                         bool append = false) = 0;

  // TODO: Rename to IsRReachable?
  // Checks for RReachability
  virtual bool IsReachable(nodeId start, nodeId goal,
                           Distance d = kMaxDistance) = 0;

  // RConnect
  virtual bool CanDetectReachabilityOfQueryPoints() = 0;
  // Returns true iff R-reachable. If R-reachable, updates d and path.
  virtual bool GetQueryPathIfReachable(nodeId start, nodeId goal, Distance & d,
                                  std::vector<nodeId> & path) {
    assert(false && "GetPathIfReachable called for R that does not implement it.");
    return false;
  }

  // Edge construction
  virtual bool CanConstructSGEdgesExactly() const {return true;}
  virtual bool CanConstructSCSGEdgesExactly() const {return false;}

  // Statistics
  virtual int GetDirectReachableAreaSize(nodeId source) {
    assert(
        false
            && "GetReachableAreaSize called for R that does not implement it.");
    return 0;
  }
  virtual RConnectStatistic GetRConnectStatistic() {
    assert(
        false
            && "AddRConnectStatistic called for R that does not implement it.");
    return RConnectStatistic();
  }

#ifndef NO_HOG
  virtual void Visualize(const MapEnvironment *env, int display_type =
                             kRReachableAreaDisplayParent) {}
#endif

  virtual void Reset() = 0;

 protected:
  // Common parameters.
  bool exploring_backward_;
  std::vector<nodeId> existing_subgoals_;
  std::vector<double> existing_subgoal_distances_;
  std::vector<nodeId> expansion_order_;
};


#endif /* APPS_SUBGOALGRAPH_REACHABILITYRELATION_REACHABILITYRELATION_H_ */
