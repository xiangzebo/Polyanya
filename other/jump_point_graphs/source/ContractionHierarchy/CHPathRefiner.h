/*
 * SGPathRefiner.h
 *
 *  Created on: Nov 5, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_CONTRACTIONHIERARCHY_CHPATHREFINER_H_
#define APPS_SUBGOALGRAPH_CONTRACTIONHIERARCHY_CHPATHREFINER_H_
#include <iostream>
#include <vector>
#include <functional>

// TODO: Might not work for directed graphs! Need the apex information!

#include "../Graphs/GraphDefinitions.h"
#include "../Utils/CPUTimer.h"
template <class FG, class BG = FG>
class CHPathRefiner {
public:
  CHPathRefiner(FG* fg, BG* bg, LevelManager* l)
    : fg_(fg), bg_(bg), l_(l) {}

  // Returns the number of shortcuts that are R-reachable.
  template <class R, class SM>
  int MarkReachableArcsAsOriginalArcs(R* r, SM* sm) {
    std::function<nodeId(SM*, nodeId)> ToNodeId = [](SM* sm, nodeId n) {
      if(sm == NULL)
        return n;
      else
        return sm->ToNodeId(n);
    };

    int num_reachable_shortcuts = 0;
    for (nodeId n = 0; n < fg_->GetNumAllNodes(); n++) {
      for (auto &a :fg_->GetSuccessors(n)) {
        if (a.mid != kNonNode
            && r->IsReachable(ToNodeId(sm,n), ToNodeId(sm,a.target), a.weight)) {
          a.mid = kNonNode;
          num_reachable_shortcuts++;
        }
      }
    }

    if (fg_ != bg_) {
      for (nodeId n = 0; n < bg_->GetNumAllNodes(); n++) {
        for (auto &a:bg_->GetSuccessors(n)) {
          if (a.mid != kNonNode
              && r->IsReachable(ToNodeId(sm, a.target), ToNodeId(sm, n), a.weight)) {
            a.mid = kNonNode;
            num_reachable_shortcuts++;
          }
        }
      }
    }
    return num_reachable_shortcuts;
  }

  // Uses the reachability relation to refine a path on the subgoal graph to a
  // path on the original graph. Returns the time.
  double RefinePath(std::vector<nodeId> & up_down_path,
                    std::vector<nodeId> & path) {
    path.clear();
    if (up_down_path.empty())
      return 0;

    CPUTimer t;
    t.StartTimer();
    nodeId start = up_down_path[0];

    while (up_down_path.size() != 1) {
      nodeId last = up_down_path[up_down_path.size()-1];
      nodeId second_last = up_down_path[up_down_path.size()-2];
      nodeId mid = GetMid(second_last, last);
      if (mid == kNonNode) {
        up_down_path.pop_back();
        path.push_back(last);
      }
      else {
        up_down_path.back() = mid;
        up_down_path.push_back(last);
      }
    }

    path.push_back(start);
    std::reverse(path.begin(), path.end());
    return t.EndTimer();
  }

private:
  nodeId GetMid(nodeId x, nodeId y) {
    if (l_->IsStrictlyLowerLevel(x,y)) {
      for (auto h:fg_->GetSuccessors(x))
        if (h.target == y)
          return h.mid;
    }
    else {
      for (auto h:bg_->GetSuccessors(y))
        if (h.target == x)
          return h.mid;
    }
    std::cout<<"Mid not found for pair "<<x<<" and "<<y<<" !!!"<<std::endl;
    return kNonNode;
  }
  FG* fg_;
  BG* bg_;
  LevelManager* l_;
};

// The same method as above, but also allows for specialized refinement method.
// Very crude initial implementation:
// - For shortcut edges, if mid == -1, that means it is a low-level edge.
// - For this implementation, we use mid == -2 to denote that the edge is not
//   a low-level edge, but needs to be refined with the specialized method
//   instead of the recursive unpacking of CH.
// AR: Arc refiner.

// TODO: Be careful in which direction the arcs are refined!
// Right now, we are assuming that R is symmetric.
template <class AR, class FG, class BG = FG>
class CH_R_PathRefiner {
public:
  CH_R_PathRefiner(AR* ar, FG* fg, BG* bg, LevelManager* l)
    : ar_(ar), fg_(fg), bg_(bg), l_(l) {
  }

  int MarkReachableArcs() {
    int num_reachable_shortcuts = 0;
    std::vector<nodeId> dummy_path;
    for (nodeId n = 0; n < fg_->GetNumAllNodes(); n++) {
      for (auto &a :fg_->GetSuccessors(n)) {
        if (a.mid != kNonNode && ar_->IsReachable(n, a.target, a.weight)) {
          a.mid = std::numeric_limits<nodeId>::max()-1;
          num_reachable_shortcuts++;
        }
      }
    }

    if (fg_ != bg_) {
      for (nodeId n = 0; n < bg_->GetNumAllNodes(); n++) {
        for (auto &a:bg_->GetSuccessors(n)) {
          if (a.mid != kNonNode && ar_->IsReachable(a.target, n, a.weight)) {
            a.mid = std::numeric_limits<nodeId>::max()-1;
            num_reachable_shortcuts++;
          }
        }
      }
    }
    return num_reachable_shortcuts;
  }

  double RefinePath(std::vector<nodeId> & up_down_path,
                    std::vector<nodeId> & path) {
    path.clear();
    if (up_down_path.empty())
      return 0;

    CPUTimer t;
    t.StartTimer();
    nodeId start = up_down_path[0];

    while (up_down_path.size() != 1) {
      nodeId last = up_down_path[up_down_path.size()-1];
      nodeId second_last = up_down_path[up_down_path.size()-2];
      nodeId mid = GetMid(second_last, last);
      if (mid == kNonNode) {  // Low-level arc
        up_down_path.pop_back();
        path.push_back(last);
      }
      //*
      else if (mid == std::numeric_limits<nodeId>::max()-1) { // Arc satisfies R.
        up_down_path.pop_back();
        // FindReachablePath function's append mode finds a path
        // n_0...n_k but only appends the part n_1...n_k.
        // However, for this implementation, we need to append the part
        // n_0..n_{k-1}
        // So we first add n_0 = last, then append, and finally pop_back n_k.
        path.push_back(last);
        ar_->RRefine(last, second_last, path, true);
        path.pop_back();
      }
      //*/
      else {  // CH refinement required
        up_down_path.back() = mid;
        up_down_path.push_back(last);
      }
    }

    path.push_back(start);
    std::reverse(path.begin(), path.end());
    return t.EndTimer();
  }

 private:
  nodeId GetMid(nodeId x, nodeId y) {
    if (l_->IsStrictlyLowerLevel(x,y)) {
      for (auto h:fg_->GetSuccessors(x))
        if (h.target == y)
          return h.mid;
    }
    else {
      for (auto h:bg_->GetSuccessors(y))
        if (h.target == x)
          return h.mid;
    }
    std::cout<<"Mid not found for pair "<<x<<" and "<<y<<" !!!"<<std::endl;
    return kNonNode;
  }

  AR* ar_;
  FG* fg_;
  BG* bg_;
  LevelManager* l_;
};

#endif /* APPS_SUBGOALGRAPH_CONTRACTIONHIERARCHY_CHPATHREFINER_H_ */
