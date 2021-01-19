/*
 * SGPathRefiner.h
 *
 *  Created on: Nov 5, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_CONTRACTIONHIERARCHY_CH_ARCPATHREFINER_H_
#define APPS_SUBGOALGRAPH_CONTRACTIONHIERARCHY_CH_ARCPATHREFINER_H_
#include <iostream>
#include <vector>
#include <functional>

#include "../Graphs/GraphDefinitions.h"
#include "../Utils/CPUTimer.h"
#include "../Utils/LevelManager.h"

template<class FG, class BG>
void ConvertToUpDownArcPath(std::vector<nodeId> & node_path,
                            std::vector<arcId> & up_arc_path,
                            std::vector<arcId> & down_arc_path,
                            FG* fg,
                            BG* bg,
                            LevelManager* l
                            ) {
  up_arc_path.clear();
  down_arc_path.clear();

  bool going_up = true;
  for (int i = 0; i < node_path.size()-1; i++) {
    nodeId from = node_path[i];
    nodeId to = node_path[i+1];
    arcId arc = kNonArc;
    if (l->GetLevel(from) < l->GetLevel(to)) {
      assert(going_up);
      arc = fg->GetArcId(from, to);
    }
    else {  //if (l->GetLevel(from) > l->GetLevel(to))
      going_up = false;
      arc = bg->GetArcId(to, from);
    }

    if (arc != kNonArc) {
      if (going_up)
        up_arc_path.push_back(arc);
      else
        down_arc_path.push_back(arc);
    }
  }
}


template<class FG, class BG>
int FindApex(std::vector<nodeId> & path, FG* fg, BG* bg, LevelManager* l) {
  for (int i = 0; i < path.size()-1; i++)
    if (l->GetLevel(path[i]) > l->GetLevel(path[i+1]))
      return i;
  return path.size()-1;
}

// TODO: Might not work for directed graphs! Need the apex information!
// TODO: A lot. Just cobbled something together that compiles.
// TODO: Sort out whether the input/output are node/arc paths.

template <class FG, class BG = FG>
class CHArcPathRefiner {
public:
  CHArcPathRefiner(FG* fg, BG* bg, LevelManager* l)
    : fg_(fg), bg_(bg), l_(l) {}

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
        if (a.first != kNonArc
            && r->IsReachable(ToNodeId(sm,n), ToNodeId(sm,a.target), a.weight)) {
          a.first = kNonArc;
          a.second = n;
          num_reachable_shortcuts++;
        }
      }
    }

    if (fg_ != bg_) {
      for (nodeId n = 0; n < bg_->GetNumAllNodes(); n++) {
        for (auto &a:bg_->GetSuccessors(n)) {
          if (a.first != kNonArc
              && r->IsReachable(ToNodeId(sm, a.target), ToNodeId(sm, n), a.weight)) {
            a.first = kNonArc;
            a.second = n;
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

    if (up_down_path.empty())
      return 0;

    std::vector<arcId> up_path;
    std::vector<arcId> down_path;
    ConvertToUpDownArcPath(up_down_path, up_path, down_path, fg_, bg_, l_);

    // Excludes the conversion time.
    CPUTimer t;
    t.StartTimer();

    path.clear();
    path.push_back(up_down_path[0]);
    for (auto a : up_path)
      UnpackForwardArc(a, path);

    for (auto a: down_path)
      UnpackBackwardArc(a, path);

    // If we have a query graph, the endpoints of the up_down_path can be
    // newly inserted subgoals, meaning that they don't have the first/second
    // information encoded. For the forward arc refinement, this is not a problem
    // as its target is added to the path. However, the backward arc refinement
    // adds its 'second' field to the path as its source.
    // This can result in the last node on the path to have an invalid node id.
    // The below hack addresses this.
    path.pop_back();
    path.push_back(up_down_path.back());

    return t.EndTimer();
  }

private:
  // Both functions take as input a pair s, arc where s is the source node
  // and arc is the id of an outgoing arc from the source node.
  // (For UnpackBackwardArc, the source and the arc are in the reverse direction.
  // Both functions recursively unpack the path into a node path s_0 ... s_k
  // and append s_1 ... s_k to 'path'.

  void UnpackForwardArc(arcId arc, std::vector<nodeId> & path) {
    if (fg_->GetArcList()->at(arc).first == kNonArc) {
      path.push_back(fg_->GetArcList()->at(arc).target);
    }
    else {
      UnpackBackwardArc(fg_->GetArcList()->at(arc).first, path);
      UnpackForwardArc(fg_->GetArcList()->at(arc).second, path);
    }
  }
  void UnpackBackwardArc(arcId arc, std::vector<nodeId> & path) {
    if (bg_->GetArcList()->at(arc).first == kNonArc) {
      // For non-shortcut arcs, the second field stores the source.
      path.push_back(bg_->GetArcList()->at(arc).second);
    }
    else {
      UnpackBackwardArc(bg_->GetArcList()->at(arc).second, path);
      UnpackForwardArc(bg_->GetArcList()->at(arc).first, path);
    }
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
class CH_R_ArcPathRefiner {
public:
  CH_R_ArcPathRefiner(AR* ar, FG* fg, BG* bg, LevelManager* l)
    : ar_(ar), fg_(fg), bg_(bg), l_(l) {
  }

  int MarkReachableArcs() {
    int num_reachable_shortcuts = 0;
    for (nodeId n = 0; n < fg_->GetNumAllNodes(); n++) {
      for (auto &a :fg_->GetSuccessors(n)) {
        if (a.first != kNonArc && ar_->IsReachable(n, a.target, a.weight)) {
          a.first = kNonArc-1;
          a.second = n;
          num_reachable_shortcuts++;
        }
      }
    }
    if (fg_ != bg_) {
      for (nodeId n = 0; n < bg_->GetNumAllNodes(); n++) {
        for (auto &a:bg_->GetSuccessors(n)) {
          if (a.first != kNonArc && ar_->IsReachable(a.target, n, a.weight)) {
            a.first = kNonArc-1;
            a.second = n;
            num_reachable_shortcuts++;
          }
        }
      }
    }
    return num_reachable_shortcuts;
  }

  double RefinePath(std::vector<nodeId> & up_down_path,
                    std::vector<nodeId> & path) {

    if (up_down_path.empty())
      return 0;

    std::vector<arcId> up_path;
    std::vector<arcId> down_path;
    ConvertToUpDownArcPath(up_down_path, up_path, down_path, fg_, bg_, l_);

    // Excludes the conversion time.
    CPUTimer t;
    t.StartTimer();

    path.clear();
    path.push_back(up_down_path[0]);
    for (auto a : up_path)
      UnpackForwardArc(a, path);

    for (auto a: down_path)
      UnpackBackwardArc(a, path);

    // If we have a query graph, the endpoints of the up_down_path can be
    // newly inserted subgoals, meaning that they don't have the first/second
    // information encoded. For the forward arc refinement, this is not a problem
    // as its target is added to the path. However, the backward arc refinement
    // adds its 'second' field to the path as its source.
    // This can result in the last node on the path to have an invalid node id.
    // The below hack addresses this.
    path.pop_back();
    path.push_back(up_down_path.back());

    return t.EndTimer();
  }

 private:
  void UnpackForwardArc(arcId arc, std::vector<nodeId> & path) {
    if (fg_->GetArcList()->at(arc).first == kNonArc) {
      path.push_back(fg_->GetArcList()->at(arc).target);
    }
    else if (fg_->GetArcList()->at(arc).first == kNonArc-1) {
      ar_->RRefine(fg_->GetArcList()->at(arc).second,
                   fg_->GetArcList()->at(arc).target, path, true);
    }
    else {
      UnpackBackwardArc(fg_->GetArcList()->at(arc).first, path);
      UnpackForwardArc(fg_->GetArcList()->at(arc).second, path);
    }
  }
  void UnpackBackwardArc(arcId arc, std::vector<nodeId> & path) {
    if (bg_->GetArcList()->at(arc).first == kNonArc) {
      // For non-shortcut arcs, the second field stores the source.
      path.push_back(bg_->GetArcList()->at(arc).second);
    }
    else if (bg_->GetArcList()->at(arc).first == kNonArc-1) {
      ar_->RRefine(bg_->GetArcList()->at(arc).target,
                   bg_->GetArcList()->at(arc).second, path, true);
    }
    else {
      UnpackBackwardArc(bg_->GetArcList()->at(arc).second, path);
      UnpackForwardArc(bg_->GetArcList()->at(arc).first, path);
    }
  }

  AR* ar_;
  FG* fg_;
  BG* bg_;
  LevelManager* l_;
};

#endif /* APPS_SUBGOALGRAPH_CONTRACTIONHIERARCHY_CHPATHREFINER_H_ */
