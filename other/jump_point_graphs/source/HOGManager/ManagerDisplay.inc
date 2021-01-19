#include <algorithm>
#include "Common.h"


template<class SGM>
void Manager<SGM>::SetupVisuals() {
  AddRReachableAreaDisplayType(kDontDisplayRReachableArea,
                               "Not showing the R-Reachable area exploration.");

  r_reachable_area_display_type_ = kDontDisplayRReachableArea;

  AddPathDisplayType(kDontDisplayPath, "Not showing the path.");
  AddPathDisplayType(kDisplayPath, "Showing the path.");
  AddPathDisplayType(kDisplayAllPaths, "Showing all shortest paths.");
  AddPathDisplayType(kDisplayAllPathsWithNodes,
                     "Showing all shortest paths and nodes.");

  // TODO:
  //AddPathDisplayType(kDisplayPathWithNodes, "Showing the path with nodes.");

//  AddPathDisplayType(kDisplayBothPaths, "Showing both paths.");
//  AddPathDisplayType(kDisplayPath, "Showing a shortest path.");
//  AddPathDisplayType(kDisplaySGPath, "Showing the subgoal graph path.");
//  AddPathDisplayType(kDisplayAllPaths, "Showing all shortest paths.");
  path_display_type_ = kDontDisplayPath;

  AddSubgoalGraphDisplayType(kDontDisplayGraph, "Not showing a graph.");
//  AddSubgoalGraphDisplayType(kDisplayGraphNodes,
//                             "Showing the lattice graph nodes.");
  AddSubgoalGraphDisplayType(kDisplaySGNodes,
                             "Showing nodes.");
  AddSubgoalGraphDisplayType(kDisplaySGNodesEdges,
                             "Showing nodes and edges.");
  AddSubgoalGraphDisplayType(kDisplaySGEdges,
                               "Showing edges.");

  sg_display_type_ = kDontDisplayGraph;

  AddSearchTreeDisplayType(kDontDisplaySearchTree,
                           "Not showing a search tree.");
  AddSearchTreeDisplayType(kDisplayForwardSearchTree,
                           "Showing the search tree in the forward direction.");
  AddSearchTreeDisplayType(
      kDisplayBackwardSearchTree,
      "Showing the search tree in the backward direction.");
  AddSearchTreeDisplayType(kDisplayBidirectionalSearchTree,
                           "Showing search tree in both directions.");
  st_display_type_ = kDontDisplaySearchTree;
  method_id_ = 0;
  draw_search_tree_nodes_ = false;

  SetupVisualsDomainSpecific();
}

template<class SGM>
void Manager<SGM>::VisualizePath(const MapEnvironment *env) {
  if (xy_start.x == 10000 || xy_goal.y == 10000
      || path_display_type_ == kDontDisplayPath)
    return;

  env->SetColor(0, 0, 0);

/*
  env->SetColor(1, 0, 0);
  glLineWidth(3);
//*/

  if (path_display_type_ == kDisplayAllPaths || path_display_type_ == kDisplayAllPathsWithNodes) {
    for (unsigned int i = 0; i < sp_arc_from_.size(); i++)
        g_graph_->DrawEdge(env, sp_arc_from_[i], sp_arc_to_[i]);

#ifdef USE_LATTICE
    if (path_display_type_ == kDisplayAllPathsWithNodes)
      for (unsigned int i = 0; i < sp_arc_from_.size(); i++)
        if (sp_arc_from_[i] != g_graph_->ToNodeId(xyt_start))
          g_graph_->DrawArrowNode(env, sp_arc_from_[i]);
#endif
  }
  else
    DrawPath(env, g_graph_, grid_paths_[method_id_]);

  glLineWidth(1);

  return;
}

template<class SGM>
void Manager<SGM>::VisualizeRReachableArea(const MapEnvironment *env) {
    return;
}

template<>
void Manager<Grid2DSG>::VisualizeRReachableArea(const MapEnvironment *env) {
  if (xy_start.x == 10000
      || r_reachable_area_display_type_ == kDontDisplayRReachableArea)
    return;

  auto r = methods_[method_id_]->GetSubgoalGraphManager()
      ->GetReachabilityRelationIfExists();
  if (r != NULL)
    r->Visualize(env, r_reachable_area_display_type_);
}

template<>
void Manager<LatticeSG>::VisualizeRReachableArea(const MapEnvironment *env) {
  if (xy_start.x == 10000
      || r_reachable_area_display_type_ == kDontDisplayRReachableArea)
    return;

  if (r_reachable_area_display_type_ == kDisplayPrimitives)
    methods_[method_id_]->GetSubgoalGraphManager()->GetOriginalGraph()
        ->VisualizeExecutablePrimitives(env, xyt_start);
  else if (r_reachable_area_display_type_ == kDisplaySGForwardEdges) {
    auto sm =
        methods_[method_id_]->GetSubgoalIdMapper();
    auto sg = methods_[method_id_]->GetSubgoalGraph();
    if (sm != NULL && sg != NULL) {
      nodeId n = g_graph_->ToNodeId(GetStart());
      subgoalId sn = sm->ToSubgoalId(n);
      if (sn != kNonSubgoal)
        for (auto a : sg->GetSuccessors(sn))
          g_graph_->DrawEdge(env, n, sm->ToNodeId(a.target));
    }
  }
  else if (r_reachable_area_display_type_ == kDisplaySGBackwardEdges) {
    auto sm =
        methods_[method_id_]->GetSubgoalIdMapper();
    auto sg = methods_[method_id_]->GetSubgoalGraph();
    if (sm != NULL && sg != NULL) {
      nodeId n = g_graph_->ToNodeId(GetStart());
      subgoalId sn = sm->ToSubgoalId(n);
      if (sn != kNonSubgoal)
        for (nodeId s = 0; s < sm->GetNumSubgoals(); s++)
          for (auto a : sg->GetSuccessors(s))
            if (a.target == sn)
              g_graph_->DrawEdge(env, sm->ToNodeId(s), n);
    }
  }
  else {
    auto r = methods_[method_id_]->GetSubgoalGraphManager()
        ->GetReachabilityRelationIfExists();
    if (r != NULL)
      r->Visualize(env, r_reachable_area_display_type_);
  }
}

template<class SGM>
template<class GraphToDisplay, class IdMapper, class Visualizer>
void Manager<SGM>::VisualizeGraph(const MapEnvironment *env, GraphToDisplay* g,
                             IdMapper* m, Visualizer* v, bool display_nodes,
                             bool display_edges) {
  env->SetColor(0, 0, 1);

  typedef typename GraphToDisplay::SuccessorType SuccessorType;
  std::vector<SuccessorType> neighbors;

  for (nodeId n = 0; n < g->GetMaxNodeId(); n++) {
    nodeId s = m->ToNodeId(n);
    if (display_nodes && v->IsValidNode(s))
      v->DrawNode(env, s);
    if (display_edges) {
      g->GetSuccessors(n, neighbors);
      for (auto neighbor : neighbors) {
        nodeId t = m->ToNodeId(neighbor.target);
        //env->SetColor(rand(), rand(), rand());
        v->DrawEdge(env, s, t);
      }
    }
  }
}

template<class SGM>
void Manager<SGM>::VisualizeSubgoalGraph(const MapEnvironment *env) {
  if (sg_display_type_ == kDontDisplayGraph)
    return;

  bool show_nodes = sg_display_type_ == kDisplaySGNodes
      || sg_display_type_ == kDisplaySGNodesEdges;
  bool show_edges = sg_display_type_ == kDisplaySGEdges
      || sg_display_type_ == kDisplaySGNodesEdges;

  methods_[method_id_]->VisualizeGraph(env, show_nodes, show_edges,
                                       display_levels_[method_id_]);
}

template<class SGM>
void Manager<SGM>::VisualizeSearchTree(const MapEnvironment *env) {
  bool forward = st_display_type_ == kDisplayForwardSearchTree
      || st_display_type_ == kDisplayBidirectionalSearchTree;
  bool backward = st_display_type_ == kDisplayBackwardSearchTree
      || st_display_type_ == kDisplayBidirectionalSearchTree;

  methods_[method_id_]->VisualizeSearch(env, forward, backward,
                                                draw_search_tree_nodes_);
}

template<class SGM>
void Manager<SGM>::Visualize(const MapEnvironment *env) {
  VisualizePath(env);
  VisualizeRReachableArea(env);
  VisualizeSubgoalGraph(env);
  VisualizeSearchTree(env);

//  resizeWindow(2000,1000);

  if (xy_start.x == 10000)
    return;

  // Draw the start.
  env->SetColor(0, 0, 1);
  if (r_reachable_area_display_type_ != kDisplayPrimitives)
    DrawStartNode(env);

  if (xy_goal.x == 10000)
    return;

  // Draw the goal.
  env->SetColor(1, 0, 0);
  DrawGoalNode(env);
}


template<class SGM>
void Manager<SGM>::ProcessKeyboardCommand(char key, bool shift, bool ctrl) {
  switch (key) {
    case 'r': {
      auto instance = g_ins_man_->GenerateValidInstance(100);
      // TODO: remove 'r' from sample and implement here.
      break;
    }
    case 'c': {  // Construct Subgoal Graph
      Preprocess();
      break;
    }
    case 'a': {  // Change area display mode.
      static int t = 0;
      if (r_reachable_area_display_types_.empty())
        break;
      if (ctrl)
        t = 0;
      if (shift)
        t = (t + r_reachable_area_display_types_.size() - 1)
            % r_reachable_area_display_types_.size();
      else
        t = (t + 1) % r_reachable_area_display_types_.size();

      r_reachable_area_display_type_ = r_reachable_area_display_types_[t];
      std::cout << r_reachable_area_display_type_messages_[t] << std::endl;
      break;
    }
    case 'g': {  // Change graph display mode.
      static int t = 0;
      if (sg_display_types_.empty())
        break;
      if (ctrl)
        t = 0;
      if (shift)
        t = (t + sg_display_types_.size() - 1) % sg_display_types_.size();
      else
        t = (t + 1) % sg_display_types_.size();

      sg_display_type_ = sg_display_types_[t];
      std::cout << sg_display_type_messages_[t] << std::endl;
      break;
    }
    case 'v': {  // Change path display mode.
      static int t = 0;
      if (path_display_types_.empty())
        break;
      if (ctrl)
        t = 0;
      if (shift)
        t = (t + path_display_types_.size() - 1) % path_display_types_.size();
      else
        t = (t + 1) % path_display_types_.size();

      path_display_type_ = path_display_types_[t];
      std::cout << path_display_type_messages_[t] << std::endl;
      break;
    }
    case 't': {  // Change search tree display mode.
      if (ctrl) {
        draw_search_tree_nodes_ = !draw_search_tree_nodes_;
        break;
      }
      static int t = 0;
      if (st_display_types_.empty())
        break;
      if (ctrl)
        t = 0;
      if (shift)
        t = (t + st_display_types_.size() - 1) % st_display_types_.size();
      else
        t = (t + 1) % st_display_types_.size();

      st_display_type_ = st_display_types_[t];
      std::cout << st_display_type_messages_[t] << std::endl;
      break;
    }

    case 'z': {
      int increment = 1;
      std::string inc_text = "incremented";

      if (shift) {
        increment = num_angles_ - 1;
        inc_text = "decremented";
      }

      SetStartOrientation((t_start + increment) % num_angles_);
      std::cout << "Start point orientation " << inc_text
                << ". New start point: " << xyt_start << std::endl;
      DoublePointQuery();
      SinglePointQuery();
//      ExploreHReachableArea();
      break;
    }

    case 'x': {
      int increment = 1;
      std::string inc_text = "incremented";

      if (shift) {
        increment = num_angles_ - 1;
        inc_text = "decremented";
      }

      SetGoalOrientation((t_goal + increment) % num_angles_);
      std::cout << "Goal point orientation " << inc_text << ". New goal point: "
                << xyt_goal << std::endl;
      DoublePointQuery();
      break;
    }
    case 'l': {
      int curr_level_ = display_levels_[method_id_] - kBaseLevel;
      int max_level =
          methods_[method_id_]->GetMaxLevel() - kBaseLevel + 1;

      int increment = 1;
      if (ctrl)
        increment = 10;

      if (shift) {
        increment = max_level - increment;
      }
      curr_level_ = (curr_level_ + increment) % max_level;
      display_levels_[method_id_] = curr_level_ + kBaseLevel;

      std::cout << "Display level: " << display_levels_[method_id_] << "("
                << method_names_[method_id_] << ")" << std::endl;
      break;
    }
    case 'b': {
      explore_backward_ = !explore_backward_;
      std::cout << "Exploring " << (std::string) (explore_backward_ ?
          "backward" : "forward") << std::endl;
      ExploreRReachableArea();
      break;
    }
    case 'q': {
      if (!shift) {
        std::cout<<"Input x1, y1, x2, y2: "<<std::flush;
        xyLoc l1, l2;
        std::cin >> l1.x >> l1.y >> l2.x >> l2.y;
        DoublePointQuery(l1, l2);
      }
      else {
        std::cout<<"Input two node ids: "<<std::flush;
        nodeId n1, n2;
        std::cin >> n1 >> n2;
        SetStart(g_graph_->ToState(n1));
        SetGoal(g_graph_->ToState(n2));
        DoublePointQuery();
      }
      break;
    }
    case 'i' : {
      nodeId n;

      if (shift) {
        auto sm = methods_[method_id_]->GetSubgoalIdMapper();
        std::cout<<"Input a subgoal id: "<<std::flush;
        std::cin >> n;
        if (sm == NULL) {
          std::cout << "Subgoal graph does not exist!"<<std::endl;
        }
        else if (n >= sm->GetNumSubgoals()) {
          std::cout << "Invalid id! Max subgoal id is: "<<sm->GetNumSubgoals()<<std::endl;
        }
        else {
          SetStart(g_graph_->ToState(sm->ToNodeId(n)));
          SinglePointQuery();
        }
      }
      else {
        std::cout<<"Input a node id: "<<std::flush;
        std::cin >> n;
        if (n >= g_graph_->GetNumAllNodes()) {
          std::cout << "Invalid id! Max id is: " << g_graph_->GetNumAllNodes()
               << std::endl;
        }
        else {
          SetStart(g_graph_->ToState(n));
          SinglePointQuery();
        }
      }
      break;
    }
    default: {
      if ('0' <= key && key <= '9') {
        int new_method_id = (key - '0');
        if (new_method_id == 0)
          new_method_id = 10;
        new_method_id--;

        if (ctrl)
          new_method_id += 10;

        if (shift)
          new_method_id += 20;

        if (new_method_id < method_names_.size()) {
          method_id_ = new_method_id;

          std::cout<<"("<<method_id_+1<<") ";
          if (!statistics_.empty()
              && (path_display_type_ != kDontDisplayPath
                  || st_display_type_ != kDontDisplaySearchTree))
            PrintSearchStatistic(method_id_);
          else if (sg_display_type_ != kDontDisplayGraph) {
            printf("%-20s", method_names_[method_id_].c_str());
            //cout<<method_names_[method_id_];
            //cout<<"\t";
            methods_[method_id_]->PrintInfo();
          }
          std::cout<<std::endl;
        }
      }
    }
      break;
  }
}



