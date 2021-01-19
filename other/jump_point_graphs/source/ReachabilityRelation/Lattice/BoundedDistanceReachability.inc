/*
 * BoundedDistanceReachability.inc
 *
 *  Created on: Mar 11, 2018
 *      Author: idm-lab
 */

/*
 * HReachability.inc
 *
 *  Created on: Mar 10, 2018
 *      Author: idm-lab
 */

template<class G, class S, class H>
void BoundedDistanceReachability<G, S, H>::RConnect(nodeId start, bool can_identify_superset) {
  auto IsReachable =
      [&](nodeId source, nodeId curr, nodeId succ, Distance g_val) -> bool {
    return g_val <= g_limit_ + kEpsDistance;
  };

  if (!can_identify_superset || rconn_type_ == kRConnectConservative)
    ReachabilityRelationExploreCommon<G, S>::ExploreDijkstraConservative(
        start, IsReachable);
  else if (rconn_type_ == kRConnectAggressive)
    ReachabilityRelationExploreCommon<G, S>::ExploreDijkstraAggressive(
      start, IsReachable);
  else
    ReachabilityRelationExploreCommon<G, S>::ExploreDijkstraStall(
      start, IsReachable);


}
template<class G, class S, class H>
void BoundedDistanceReachability<G, S, H>::RRefine(
    nodeId from, nodeId to, std::vector<nodeId> & path, bool append) {
  // FIXME?: Implement A* from scratch using ReachabilityRelationExploreCommon
  // data structures?
  if (!append) {
    astar_.FindPath(from, to, path);
  }
  else {
    std::vector<nodeId> new_path;
    if (astar_.FindPath(from, to, new_path)) {
      path.insert(path.end(), new_path.begin(), new_path.end());
    }
  }
}

template<class G, class S, class H>
int BoundedDistanceReachability<G, S, H>::FindLastReachableIndex(std::vector<nodeId> & path,
                                                   int start, int goal) {
  auto IsReachable =
      [&](nodeId source, nodeId curr, nodeId succ, Distance g_val) -> bool {
    return g_val <= g_limit_ + kEpsDistance;
  };
  return ReachabilityRelationExploreCommon<G, S>::FindLastReachableIndexUsingDistance(
      path, start, goal, IsReachable);
}

template<class G, class S, class H>
bool BoundedDistanceReachability<G, S, H>::ExploreReachableAreaFringeDijkstra(
    nodeId start, bool terminate_if_fringe, Distance exploration_bound) {
  auto IsReachable =
      [&](nodeId source, nodeId curr, nodeId succ, Distance g_val) -> bool {
    return g_val <= g_limit_ + kEpsDistance;
  };
  return ReachabilityRelationExploreCommon<G, S>::ExploreReachableAreaFringeDijkstra(
      start, terminate_if_fringe, exploration_bound, IsReachable);
}



