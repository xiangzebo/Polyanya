template <class SGM>
void SGCHQuery<SGM>::ReportPreprocessStatistics(StatisticsFile* st) {
  if (p_.use_subgoal_graph) {
    num_nodes_ = sm_->GetNumSubgoals();
  } else {
    num_nodes_ = g_->GetNumValidNodes();
  }
  st->ReportInt("Number of nodes", num_nodes_);

  long bytes_per_arc = 0;
  if (ch_reachable_arc_graph_ != NULL) {
    num_arcs_ = ch_reachable_arc_graph_->GetNumArcs();
    bytes_per_arc = sgm_->GetBytesPerEdge();
  }
  else if (sg_ != NULL) {
    num_arcs_ = sg_->GetNumArcs();
    bytes_per_arc = sgm_->GetBytesPerEdge();
  }
  else if (ch_shortcut_arc_graph_ != NULL) {
    num_arcs_ = ch_shortcut_arc_graph_->GetNumArcs();
    bytes_per_arc = 12;
  }
  else if (ch_shortcut2_arc_graph_ != NULL) {
    num_arcs_ = ch_shortcut2_arc_graph_->GetNumArcs();
    bytes_per_arc = 16;
  }
  else {
    num_arcs_ = g_->GetNumArcs();
    bytes_per_arc = 8;
  }
  st->ReportInt("Number of arcs", num_arcs_);
  st->AddRemark("");

  preprocess_time_ = sgm_->GetPreprocessingTime() + contract_time_;
  st->ReportDouble("Preprocessing time (s)", preprocess_time_);
  sgm_->ReportPreprocessStatistics(st);
  st->ReportDouble("Contraction time (s)", contract_time_);
  st->AddRemark("");

  graph_memory_ = (num_arcs_ * bytes_per_arc) / (1024.0 * 1024.0);

  bool r_refine = false;
  bool r_connect = false;
  if (p_.use_subgoal_graph) {
    r_refine = true;
    r_connect = true;
  }
  if (p_.r_refine_when_possible) {
    r_refine = true;
  }

  r_memory_ = sgm_->GetRMemoryMB(r_connect, r_refine);

//  double mem_level = (num_nodes_ * 4);
  memory_ = r_memory_ + graph_memory_;
  st->ReportDouble("Memory (MB)", memory_);

  sgm_->ReportMemoryStatistics(st, r_connect, r_refine);
  st->ReportDouble("Arc memory (MB)", graph_memory_);
  st->AddRemark("");
}

template <class SGM>
void SGCHQuery<SGM>::ReportAdditionalQueryStatistics(StatisticsFile* st) {

  // Connection statistics
  AggregateConnectionStatistics s;
  if (forward_sg_query_ != NULL)
    s = forward_sg_query_->GetAggregateConnectionStatistics();
  if (bidirectional_ch_query_ != NULL)
    s = bidirectional_ch_query_->GetAggregateConnectionStatistics();
  if (bidirectional_rch_query_ != NULL)
    s = bidirectional_rch_query_->GetAggregateConnectionStatistics();
  if (bidirectional_ch2_query_ != NULL)
    s = bidirectional_ch2_query_->GetAggregateConnectionStatistics();
  s.ReportStatistics(st);
  st->AddRemark("");

  int num_core_nodes = 0;
  int num_core_arcs = 0;
  int num_ascending_arcs = 0;
  int num_same_level_local_arcs = 0;

  auto OnNode = [&](nodeId n) {
    if (!level_.IsInitialized() || level_.IsHighestLevel(n))
      num_core_nodes ++;
  };
  auto OnArc = [&](nodeId from , nodeId to) {
    if (!level_.IsInitialized())
      num_core_arcs++;
    else {
      if (level_.IsStrictlyLowerLevel(from, to))
        num_ascending_arcs++;
      else if (level_.IsHighestLevel(from) && level_.IsHighestLevel(to))
        num_core_arcs++;
      else if (level_.IsStrictlyLowerLevel(to, from))
        assert(false && "Hierarchy has descending arc!");
      else {
        assert(level_.SameLevel(from, to) && !level_.IsHighestLevel(from));
        num_same_level_local_arcs++;
      }
    }
  };

  auto OnShortcutArc = [&](nodeId n, ShortcutArcHead a, bool backward) {
    OnArc(n, a.target);
  };
  auto OnShortcut2Arc = [&](nodeId n, Shortcut2ArcHead a, bool backward) {
    OnArc(n, a.target);
  };
  auto OnReachableArc = [&](nodeId n, WeightedArcHead a, bool backward) {
    OnArc(n, a.target);
  };

  if (ch_shortcut_arc_graph_ != NULL) {
    ch_shortcut_arc_graph_->IterateOverArcsAndDo(OnNode, OnShortcutArc);
  }
  else if (ch_shortcut2_arc_graph_ != NULL) {
    ch_shortcut2_arc_graph_->IterateOverArcsAndDo(OnNode, OnShortcut2Arc);
  }
  else if (ch_reachable_arc_graph_ != NULL) {
    ch_reachable_arc_graph_->IterateOverArcsAndDo(OnNode, OnReachableArc);
  }
  else if (sg_ != NULL) {
    for (nodeId n = 0; n < sg_->GetNumAllNodes(); n++) {
      std::vector<WeightedArcHead> neighbors;
      sg_->GetSuccessors(n, neighbors);
      int num_reachable_neighbors = neighbors.size();
      if (!level_.IsInitialized() || level_.IsHighestLevel(n)) {
        num_core_nodes++;
        num_core_arcs += neighbors.size();
      }
    }
  }

  st->ReportInt("Max level", GetMaxLevel());
  st->ReportInt("Number of core nodes", num_core_nodes);
  st->ReportInt("Number of core arcs", num_core_arcs);
  st->ReportInt("Number of ascending arcs", num_ascending_arcs);
  st->ReportInt("Number of same level local arcs", num_same_level_local_arcs);
  st->AddRemark("");

  // Contraction statistics
  st->ReportInt("Number of original graph nodes",
                ch_construction_stat_.num_initial_nodes);
  st->ReportInt("Number of original graph arcs",
                ch_construction_stat_.num_initial_arcs);

  st->ReportInt("Number of contracted nodes", ch_construction_stat_.num_contracted);
  st->ReportInt("Number of original arcs removed", ch_construction_stat_.num_original_arcs_removed);
  st->ReportInt("Number of shortcuts added", ch_construction_stat_.num_shortcuts_added);
  st->ReportInt("Number of shortcuts removed", ch_construction_stat_.num_shortcuts_removed);
  st->ReportInt("Number of original arcs in hierarchy", ch_construction_stat_.num_original_arcs_in_hierarchy);
  st->ReportInt("Number of shortcuts in hierarchy", ch_construction_stat_.num_shortcuts_in_hierarchy);
  st->ReportInt("Number of shortcuts marked for R-refine", num_reachable_shortcuts_);
  st->AddRemark("");

  st->ReportInt("Importance estimated", ch_construction_stat_.num_estimate_importance_calls);
  st->ReportInt("Witness searches for estimating importance",
                ch_construction_stat_.num_witness_searches_importance);
  st->ReportInt("Witness searches for determining shortcuts",
                ch_construction_stat_.num_witness_searches_shortcuts);
  st->ReportDouble(
      "Average number of expansions for estimate importance witness searches",
      ch_construction_stat_.stat_importance.num_expanded
          / (double) ch_construction_stat_.num_witness_searches_importance);
  st->ReportDouble(
      "Average number of expansions for contraction witness searches",
      ch_construction_stat_.stat_shortcuts.num_expanded
          / (double) ch_construction_stat_.num_witness_searches_shortcuts);
  st->AddRemark("");
}

#ifndef NO_HOG

template<class SGM>
int SGCHQuery<SGM>::DetermineLevelForVisualization(nodeId n) {
  if (!level_.IsInitialized())
    return 0;
  return level_.GetLevel(n) - (p_.use_subgoal_graph ? 0 : 1);
}

template <class SGM>
recColor SGCHQuery<SGM>::DetermineLevelColor(nodeId n, recColor c0, recColor cm) {
  if (!level_.IsInitialized())
     return cm;
  recColor c;
  c.r = c0.r + (level_.GetRatioToMaxLevel(n))*(cm.r - c0.r);
  c.g = c0.g + (level_.GetRatioToMaxLevel(n))*(cm.g - c0.g);
  c.b = c0.b + (level_.GetRatioToMaxLevel(n))*(cm.b - c0.b);
  return c;
}

template<class SGM>
template<class IM>
void SGCHQuery<SGM>::DrawLevelNode(const MapEnvironment* env, IM* im,
                                   nodeId n, double priority) {
#ifdef LEVEL_VIZUALIZATION
  g_->DrawLevelNode(env, im->ToNodeId(n), DetermineLevelForVisualization(n), priority);
#else
  g_->DrawNode(env, im->ToNodeId(n), priority);
#endif
}

template<class SGM>
template<class IM>
void SGCHQuery<SGM>::DrawLevelEdge(const MapEnvironment* env, IM* im, nodeId n1,
                                   nodeId n2) {

#ifdef LEVEL_VIZUALIZATION
  g_->DrawLevelEdge(env, im->ToNodeId(n1), DetermineLevelForVisualization(n1), im->ToNodeId(n2),
      DetermineLevelForVisualization(n2));
#else
  g_->DrawEdge(env, im->ToNodeId(n1), im->ToNodeId(n2));
#endif
}


template <class SGM>
void SGCHQuery<SGM>::DrawSearchTreeEdge(const MapEnvironment* env, nodeId s, nodeId t,
                                  bool forward, bool draw_search_tree_nodes,
                                  recColor c0, recColor cm) {
  if (s == kNonNode || t == kNonNode)
    return;

  // If we are using subgoal graph, the nodeIds are subgoalIds, so we need
  // to convert them to nodeIds for display.
  // If it is a query node, then it wouldn't have a subgoal id, so we use
  // the last_start_ or last_goal_ to determine the subgoal id.
  // All arcs are from node to parent, therefore, if the search is forward,
  // s can only be the last goal and the can only be the last start (similar
  // for backward search with s and t swapped).


  recColor low (1, 1, 0);
  recColor high (1, 0, 0);

#ifdef LEVEL_VIZUALIZATION
//  env->SetColor(DetermineLevelColor(s, c0, cm));
  env->SetColor(DetermineLevelColor(t, low, high));
#else
  env->SetColor(c0);
#endif

  int s_l = 0;
  int t_l = 0;

  if (p_.use_subgoal_graph) {
    if (s < sm_->GetNumSubgoals()) {
      s_l = DetermineLevelForVisualization(s);
      s = sm_->ToNodeId(s);
    }
    else {
      if (forward)
        s = last_goal_;
      else
        s = last_start_;
    }
    if (t < sm_->GetNumSubgoals()) {
      t_l = DetermineLevelForVisualization(t);
      t = sm_->ToNodeId(t);
    }
    else {
      if (forward)
        t = last_start_;
      else
        t = last_goal_;
    }
  }
  else {
    s_l = DetermineLevelForVisualization(s);
    t_l = DetermineLevelForVisualization(t);
  }

  IdentityMapper im;
  if (draw_search_tree_nodes)
    DrawLevelNode(env, &im, s, -500);

  // TODO: Search direction
//  DrawLevelEdge(env, &im, t, s);

/*
#ifdef LEVEL_VIZUALIZATION
  g_->DrawLevelEdge(env, s, s_l, t, t_l);
#else
  g_->DrawEdge(env, s, t);
#endif
*/
#ifdef LEVEL_VIZUALIZATION
  g_->DrawLevelEdge(env, t, t_l, s, s_l);
#else
  if (forward)
    g_->DrawEdge(env, t, s);
  else
    g_->DrawEdge(env, s, t);
#endif
}

template<class SGM>
void SGCHQuery<SGM>::DrawSearchTree(const MapEnvironment* env,
                                    std::vector<Arc> & exp,
                                    std::vector<Arc> & stall,
                                    std::vector<Arc> & gen, bool forward,
                                    bool draw_nodes) {

  recColor yellow (1, 1, 0);
  recColor red (1, 0, 0);

  recColor dark_yellow (0.5, 0.5, 0);
  recColor dark_red (0.5, 0, 0);

  recColor black(0, 0 , 0);
  recColor gray(0.5, 0.5, 0.5);

  recColor blue(0, 0, 1);

  recColor orange(255/255.0, 174/255.0, 0/255.0);
  //recColor turquise(0/255.0, 234/255.0, 255/255.0);
  recColor turquise(0, 0.5, 1);

  for (auto a : exp)
    //DrawSearchTreeEdge(env, a.source, a.target, forward, yellow, red);
    DrawSearchTreeEdge(env, a.source, a.target, forward, draw_nodes, gray, gray);

  for (auto a : stall)
    //DrawSearchTreeEdge(env, a.source, a.target, forward, black, black);
    DrawSearchTreeEdge(env, a.source, a.target, forward, draw_nodes, red, red);


  recColor deneme(51/255.0,204/255.0,51/255.0);
  for (auto a : gen)
    //DrawSearchTreeEdge(env, a.source, a.target, forward, dark_yellow, dark_red);
    DrawSearchTreeEdge(env, a.source, a.target, forward, draw_nodes, deneme, deneme);
}

template <class SGM>
void SGCHQuery<SGM>::VisualizeSearch(const MapEnvironment* env, bool forward,
                               bool backward, bool draw_nodes) {
  if (forward)
    DrawSearchTree(env, f_exp, f_stall, f_gen, true, draw_nodes);
  if (backward)
    DrawSearchTree(env, b_exp, b_stall, b_gen, false, draw_nodes);
}

template <class SGM>
template<class Gr, class IM>
void SGCHQuery<SGM>::VisualizeGraph(const MapEnvironment* env, Gr* dg, IM* m,
                              bool display_nodes, bool display_arcs,
                              int display_level) {

  bool skip_refinable_arcs =
      level_.IsInitialized() && ch_shortcut_arc_graph_ != NULL;
  skip_refinable_arcs = false;

  recColor low (1, 1, 0);
  recColor high (1, 0, 0);

  for (nodeId n = 0; n < dg->GetNumAllNodes(); n++) {
    if (level_.IsInitialized() && level_.GetLevel(n) < display_level)
      continue;

    env->SetColor(DetermineLevelColor(n, low, high));
#ifdef VIS_RANDOM_COLOR
    srand(n);
    env->SetColor(rand()/(double)RAND_MAX, rand()/(double)RAND_MAX, rand()/(double)RAND_MAX);
#endif
    if (display_nodes && g_->IsValidNode(m->ToNodeId(n))) {
      DrawLevelNode(env, m, n);
    }

    if (display_arcs) {
      for (auto a: dg->GetSuccessors(n)) {
        if (skip_refinable_arcs && a.GetMid() >= 0
            && a.GetMid() < dg->GetNumAllNodes()
            && level_.GetLevel(a.GetMid()) >= display_level)
          continue;

        DrawLevelEdge(env, m, n, a.target);
      }
    }
  }
}

// FIXME: Only works for directed
template <class SGM>
void SGCHQuery<SGM>::VisualizeGraph(const MapEnvironment* env, bool display_nodes,
                    bool display_arcs, int display_level) {

  IdentityMapper mapper;
  if (!p_.use_subgoal_graph && p_.contraction_type == kNoContraction) {
    VisualizeGraph(env, g_, &mapper, display_nodes, display_arcs,
                   display_level);
  }
  else if (ch_shortcut_arc_graph_ != NULL) {
    if (p_.use_subgoal_graph)
      VisualizeGraph(env, ch_shortcut_arc_graph_->GetForwardGraph(), sm_,
                     display_nodes, display_arcs, display_level);
    else
      VisualizeGraph(env, ch_shortcut_arc_graph_->GetForwardGraph(), &mapper,
                     display_nodes, display_arcs, display_level);
  }
  else if (ch_shortcut2_arc_graph_ != NULL) {
    if (p_.use_subgoal_graph)
      VisualizeGraph(env, ch_shortcut2_arc_graph_->GetForwardGraph(), sm_,
                     display_nodes, display_arcs, display_level);
    else
      VisualizeGraph(env, ch_shortcut2_arc_graph_->GetForwardGraph(), &mapper,
                     display_nodes, display_arcs, display_level);
  }
  else if (ch_reachable_arc_graph_ != NULL) {
    if (p_.use_subgoal_graph)
      VisualizeGraph(env, ch_reachable_arc_graph_->GetForwardGraph(), sm_,
                     display_nodes, display_arcs, display_level);
    else
      VisualizeGraph(env, ch_reachable_arc_graph_->GetForwardGraph(), &mapper,
                     display_nodes, display_arcs, display_level);
  }
  else if (sg_ != NULL) {
    VisualizeGraph(env, sg_, sm_, display_nodes, display_arcs, display_level);
  }
}
#endif
