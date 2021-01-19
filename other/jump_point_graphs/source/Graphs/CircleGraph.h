/*
 * CircleGraph.h
 *
 *  Created on: Nov 18, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_CIRCLEGRAPH_H_
#define APPS_SUBGOALGRAPH_CIRCLEGRAPH_H_
#include "../../environments/Map2DEnvironment.h"
#include "ContractionHierarchy/CHConstructor.h"
#include "Graphs/DynamicGraph.h"
#include "Parameters/CHParam.h"
#include "Utils/GraphUtils.h"
#include "Utils/LevelManager.h"

// TODO:

class CircleGraph {
 public:
  CircleGraph(int num_nodes = 20, ContractionType c = kNoContraction,
              Distance dist_limit = 4) {
    num_nodes_ = num_nodes;
    c_ = c;
    dist_limit_ = dist_limit;

    std::vector<WeightedArc> arcs;

    for (nodeId n = 0; n < num_nodes; n++) {
      nodeId m = (n+1) % num_nodes;
      arcs.push_back(WeightedArc(n, m, 1));
      arcs.push_back(WeightedArc(m, n, 1));
    }

    if (c_ == kNoContraction) {
      std::vector<ShortcutArc> c_arcs;
      for (auto a: arcs)
        c_arcs.push_back(ShortcutArc(a.source, a.target, a.weight, -1));

      ch_.CreateGraph(num_nodes, c_arcs);
      l_.GenerateInitialLevels(num_nodes_);

      return;
    }

    CHParam p;
    p.contraction_type = c_;
    CHConstructor ch_constructor(p, num_nodes, arcs, true);

    if (c_ == kRegularContraction)
      ch_constructor.ConstructCH();
    if (c_ == kRContraction || c_ == kHeavyRContraction)
      ch_constructor.ConstructCH(this); // Provides GetHeuristicDistance function.

    ch_.CreateGraph(num_nodes, *ch_constructor.GetCore()->GetForwardSearchArcs());
    l_ = *ch_constructor.GetCore()->GetLevelManager();
  }

  Distance GetHeuristicDistance(nodeId n, nodeId m) {
    int d = abs(n-m);
    d = min(d, num_nodes_ - d);
    return fmin((double)d, dist_limit_);
  }

  bool IsReachable(nodeId n, nodeId m, Distance d = kMaxDistance) {
    return fabs(d - GetHeuristicDistance(n,m)) <= kEpsDistance;
  }

  void Draw(const MapEnvironment *env) {
    // Calculate node positions.
    // Indexed by nodeId and level.
    std::vector<std::vector<float> > x, y;
    int w = env->GetMap()->GetMapWidth();
    int h = env->GetMap()->GetMapHeight();
    float c_x = w/2.0;
    float c_y = h/2.0;
    float r = min(c_x, c_y)-2;  // Radius
    for (nodeId n = 0; n < num_nodes_; n++) {
      x.push_back(std::vector<float> (l_.GetMaxLevel()-kBaseLevel+1));
      y.push_back(std::vector<float> (l_.GetMaxLevel()-kBaseLevel+1));

      float a = (n*2.0*3.141592)/(float)num_nodes_;  // Angle in radians.
      for (level l = 0; l <= l_.GetMaxLevel()-kBaseLevel; l++) {
        float l_r = (1-l/10.0); // Distance multiplier from center.
        x[n][l] = (c_x + cos(a)*r*l_r);
        y[n][l] = (c_y + sin(a)*r*l_r);
      }
    }

    env->SetColor(0,0,0);
    for (nodeId n = 0; n < num_nodes_; n++) {
      level l = l_.GetLevel(n)-kBaseLevel;

      //env->SetColor(0.9,(5-l)*0.2,0);
      //env->SetColor(l*0.2,0,0);
      env->SetColor(0.5+l*0.1,0.5-l*0.1,0.5-l*0.1);
      int priority = 1000;
      env->OpenGLPriorityDraw(x[n][l], y[n][l], priority);

      for (auto a: ch_.GetSuccessors(n)) {
        env->SetColor(0,0,0);
        level l2 = l_.GetLevel(a.target)-kBaseLevel;
        env->GLDrawColoredLine(x[n][l], y[n][l], x[a.target][l2], y[a.target][l2],100);
      }
    }

    for (level l = 0; l <= l_.GetMaxLevel()-kBaseLevel; l++) {
      for (nodeId n = 0; n < num_nodes_; n++) {

        env->SetColor(0.9,0.9,0);
        //env->SetColor(0.9,0.7,0);
        //env->SetColor(l*0.2,(5-l)*0.2,0);
        //env->SetColor(0.9,(5-l)*0.2,0);

        glDisable(GL_LIGHTING);

        nodeId m = (n+1)%num_nodes_;
        env->GLDrawColoredLine(x[n][l], y[n][l], x[m][l], y[m][l]);
      }
    }
  }

 private:
  int num_nodes_;
  Distance dist_limit_;
  DynamicGraph<WeightedArcHead> g_;
  DynamicGraph<ShortcutArcHead> ch_;
  LevelManager l_;
  ContractionType c_;
};


#endif /* APPS_SUBGOALGRAPH_CIRCLEGRAPH_H_ */
