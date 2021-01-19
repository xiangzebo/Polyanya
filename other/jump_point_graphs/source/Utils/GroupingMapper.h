/*
 * GroupingMapper.h
 *
 *  Created on: Nov 15, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_UTILS_GROUPINGMAPPER_H_
#define APPS_SUBGOALGRAPH_UTILS_GROUPINGMAPPER_H_
#include <vector>

#include "../Graphs/GraphDefinitions.h"

typedef nodeId groupId;
const nodeId kNonGroup = std::numeric_limits<groupId>::max();

class GroupingMapper {
 public:
  GroupingMapper() {}

  nodeId GetDomainSize() const {return map_.size();}
  groupId GetNumGroups() const {return representative_.size();}
  groupId GetGroup(nodeId n) const {return map_[n];}

  nodeId GetRepresentative(nodeId n) const {
    return representative_[n];
  }
  void SetRepresentative(nodeId n) {
    representative_[GetGroup(n)] = n;
  }

  void SetDomain(nodeId num_nodes) {
    map_.resize(num_nodes, kNonGroup);
  }
  void AddGroup(std::vector<nodeId> & nodes) {
    if (nodes.empty())
      return;
    int group_no = representative_.size();
    for (auto n : nodes) {
      assert(map_[n] == kNonGroup);
      map_[n] = group_no;
    }
    representative_.push_back(nodes[0]);
    groups_.push_back(nodes);
  }

  const std::vector<nodeId>* GetGroupNodes(groupId n) const {
    return &groups_[n];
  }
  const std::vector<nodeId>* GetGroupSiblingNodes(nodeId n) const {
    return &groups_[GetGroup(n)];
  }

 private:
  std::vector<groupId> map_;
  std::vector<nodeId> representative_;

  std::vector<std::vector<nodeId> > groups_;
};



#endif /* APPS_SUBGOALGRAPH_UTILS_GROUPINGMAPPER_H_ */
