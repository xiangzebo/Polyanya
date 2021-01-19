#include "../Utils/Experiments/RExploreExperiment.h"

template<class SGM>
Manager<SGM>::Manager() {
  is_preprocess_complete_ = false;

  // Tests
  SubgoalGraphTester();
  FG_Val_PPQ_Higher_G_Tester();

  g_ins_man_ = NULL;

  // FIXME: Read from lattice when it is initialized.
  num_angles_ = 1;

  dij_ = NULL;

  // Set initial query.
  xy_start = xyLoc(10000, 10000);
  xy_goal = xyLoc(10000, 10000);
  t_start = 0;
  t_goal = 0;
  UpdateXYThetaPoses();

  explore_backward_ = false;
  SetupVisuals();

  method_name_length_ = 0;


#ifdef INCLUDE_GPPC_CH
  gppc_ch_ = NULL;
  gppc_ch_exp_ = NULL;
#endif
}

template<class SGM>
Manager<SGM>::~Manager() {
  if (g_ins_man_)
    delete g_ins_man_;
  for (auto e: experiments_)
    delete e;
  for (auto q: methods_)
    delete q;

#ifdef INCLUDE_GPPC_CH
  if (gppc_ch_)
    delete gppc_ch_;
  if (gppc_ch_exp_)
    delete gppc_ch_exp_;
#endif
}

template<class SGM>
void Manager<SGM>::Initialize(std::string mapname) {
  dp_.SetMapPath(mapname);

  c.clear();
  s.clear();
  override_method_names_.clear();
  InitializeMethods();
  if (kManagerForceDefaultAlgNames)
    override_method_names_.clear();

  // Construct the query methods and experiments based on ch and sg params.
  assert(c.size() == s.size());
  for (unsigned int i = 0; i < c.size(); i++) {
    methods_.push_back(new QueryMethod(dp_, c[i], s[i]));
    method_names_.push_back(methods_[i]->GetName());
    if (i < override_method_names_.size() && override_method_names_[i] != "")
      method_names_.back() = override_method_names_[i];
    method_name_length_ = max(method_name_length_, method_names_[i].length());
    display_levels_.push_back(kBaseLevel);
  }

  g_graph_ = methods_[0]->GetOriginalGraph();
  dij_ = new Dijkstra<Graph>(g_graph_);

  g_ins_man_ = new InstanceGenerator(g_graph_);

  InitializeNumAngles();

  for (auto q : methods_)
    experiments_.push_back(
        new SGExperiment<QueryMethod, InstanceGenerator>(
            q, g_ins_man_, q->GetStatisticsFilename(),
            q->GetInstanceStatisticsFilename()));
  grid_paths_.resize(experiments_.size());

  assert(!methods_.empty());


#ifdef INCLUDE_GPPC_CH
    gppc_ch_ = new ch_gppc::CHQueryGPPC(dp_.GetMapFilename());
    gppc_ch_exp_ = new GppcCHExperiment(gppc_ch_, g_ins_man_, "", "");
#endif

}

template<class SGM>
void Manager<SGM>::Preprocess() {
  for (auto e:experiments_)
    if (e->GetQueryMethod()->GetParam()->clear)
      e->GetQueryMethod()->ClearFiles();

  for (auto e:experiments_) {
    std::cout<<"Preprocessing "<<e->GetQueryMethod()->GetName()<<std::endl;
    e->Preprocess();
    e->ReportPreprocessingStatistics();
    std::cout<<std::endl;
  }

#ifdef INCLUDE_GPPC_CH
    gppc_ch_exp_->Preprocess();
#endif

  is_preprocess_complete_ = true;
  Experiment();

#ifdef INCLUDE_GPPC_CH
  std::cout << "\nContraction Hierarchy (GPPC):\n";
  gppc_ch_exp_->RunInstances();
  //gppc_ch_exp_->ReportQueryStatistics();
  PrintSearchStatistic(
      "CH-GPPC",
      gppc_ch_exp_->GetStatistics()->GetAggregateStatistics());
  std::cout << "CH (GPPC) speed-up: "
      << experiments_[0]->GetStatistics()->GetAverageTime()
          / gppc_ch_exp_->GetStatistics()->GetAverageTime() << std::endl;
#endif

}

template<class SGM>
void Manager<SGM>::Experiment() {

  if (!kExperiment) {
    std::cout<<"Skipping experiments."<<std::endl;
    return;
  }
  // Generate instances.
  int num_instances = kDefaultNumInstances;
//  int num_instances = 1000;

  std::string instance_filename = methods_[0]->GetInstanceFilename();
  if (kInstanceClear) {
    std::string cmd = "rm -f " + instance_filename;
    system(cmd.c_str());
//    instance_filename = "";
  }
  if (instance_filename != "")
    g_ins_man_->GenerateValidInstances(num_instances, instance_filename);
  else
    g_ins_man_->GenerateValidInstances(num_instances);

  // Run experiments.
  for (unsigned int i = 0; i < experiments_.size(); i++) {
    std::cout << "\n" << method_names_[i] << ":\n";
    experiments_[i]->RunInstances();
    experiments_[i]->ReportQueryStatistics();
    std::cout
        << method_names_[i]
        << " speed-up: "
        << experiments_[0]->GetStatistics()->GetAverageTime()
            / experiments_[i]->GetStatistics()->GetAverageTime()
        << std::endl;
  }

  // Print preprocessing summary.
  for (unsigned int i = 0; i < method_names_.size(); i++) {
    printf("%-*s | ", method_name_length_, method_names_[i].c_str());
//    printf("%-20s | ", method_names_[i].c_str());
    methods_[i]->PrintInfo();
    std::cout<< std::endl;
  }

  // Print query summary.
  for (unsigned int i = 0; i < method_names_.size(); i++) {
    PrintSearchStatistic(
        method_names_[i],
        experiments_[i]->GetStatistics()->GetAggregateStatistics());
    printf(
        " SU: %6.2f\n",
        experiments_[0]->GetStatistics()->GetAverageTime()
            / experiments_[i]->GetStatistics()->GetAverageTime());
  }

  // Print connection summary.
  for (unsigned int i = 0; i < method_names_.size(); i++) {
    PrintConnectionStatistic(i);
    std::cout<<std::endl;
  }

//*
#ifdef USE_LATTICE
  // Experiment and print exploration statistics.
  std::vector<nodeId> single_node_instances;
  for (unsigned int i = 0; i < g_ins_man_->GetNumInstances(); i++)
    single_node_instances.push_back(
        g_graph_->ToNodeId(g_ins_man_->GetInstance(i).start));

  for (unsigned int i = 0; i < method_names_.size(); i++) {
    RExploreExperiment<SGM> exp(methods_[i]->GetSubgoalGraphManager());
    exp.Experiment(1000);
//    exp.Experiment(single_node_instances);

    printf("%-*s", method_name_length_, method_names_[i].c_str());
    exp.PrintOneLineSummary();
    std::cout<<std::endl;
  }
#endif
//*/
}

template<class SGM>
void Manager<SGM>::DoublePointQuery() {
  if (xy_start.x == 10000 || xy_goal.x == 10000)
    return;

  std::cout << "Searching from " << GetStart() << " to " << GetGoal() << " ("
       << g_graph_->ToNodeId(GetStart()) << " to "
       << g_graph_->ToNodeId(GetGoal()) << ")" << std::endl;

  statistics_.clear();
  for (unsigned int i = 0; i < methods_.size(); i++)
    statistics_.push_back(
        methods_[i]->Query(GetStart(), GetGoal(), grid_paths_[i]));

  for (unsigned int i = 0; i < method_names_.size(); i++) {
    PrintSearchStatistic(i);
    std::cout << std::endl;
    if (statistics_[i].dist > statistics_[0].dist + 0.01) {
      std::cout <<"SUBOPTIMAL!!!"<<std::endl;
    }
  }

  dij_->ExtractAllEdgesOnShortestPaths(g_graph_->ToNodeId(GetStart()),
                                       g_graph_->ToNodeId(GetGoal()),
                                       sp_arc_from_, sp_arc_to_);

  DoublePointQuerySpecific();
}


// TODO: Make this a vector?
template<class SGM>
void Manager<SGM>::PrintSearchStatistic(std::string name,
                                   QueryStatistic s, std::ostream & out) {
//*
  char buffer [1000];

  sprintf(buffer, "%-*s", method_name_length_, name.c_str());

  // Time
  //double t_conv = 1000;  // Convert from seconds to milliseconds.
  double t_conv = 1000000;  // Convert from seconds to microseconds.
  sprintf(buffer + strlen(buffer),
          " | T: %5.0f + %6.0f + %4.0f",
          s.connect_time * t_conv, s.search_time * t_conv,
          s.refine_time * t_conv);

  // Length
  Distance dist = s.dist < kMaxDistance/1000 ? s.dist : -1;
  //sprintf(buffer + strlen(buffer), " | L: %6.1f (%4.2f)",
  //        dist, s.suboptimality);
  sprintf(buffer + strlen(buffer), " | L: %6.1f", dist);

  // Expansions
  sprintf(buffer + strlen(buffer), " | S/E: %6.0f [%6.0f]",
          (double) (s.num_expanded + s.num_stalled),
          (double) (s.num_backward_expanded + s.num_backward_stalled));

  // Relaxed
  sprintf(buffer + strlen(buffer), " R: %7.0f",
          (double) (s.num_relaxed_arcs));

  // Pruned
  //sprintf(buffer + strlen(buffer), " P: %7.0f",
  //        (double) (s.num_pruned_arcs));

  sprintf(buffer + strlen(buffer), " | ");

  out << buffer;

  return;
}

template<class SGM>
void Manager<SGM>::PrintConnectionStatistic(std::string name,
                                       AggregateConnectionStatistics s,
                                       std::ostream & out) {
  char buffer [1000];
  sprintf(buffer, "%-*s", method_name_length_, name.c_str());

  for (int f = 1; f >= 0; f -= 1) {
    bool forward = f == 1;
    sprintf(buffer + strlen(buffer), " | %s: %4.3f %5.0f %4.0f %5.0f",
            forward ? "F" : "B", s.GetConnectRatio(forward, !forward),
            s.GetAverageExpanded(forward, !forward),
            s.GetAverageSubgoals(forward, !forward),
            s.GetAverageTime(forward, !forward)*1000000);
  }

  out << buffer;

  return;
}

template<class SGM>
void Manager<SGM>::ExploreRReachableArea() {
  if (xy_start.x == 10000)
    return;

  auto sgm = methods_[method_id_]->GetSubgoalGraphManager();
  ReachabilityRelation* r = sgm ->GetReachabilityRelation();

  if (!explore_backward_) {
  std::cout<<"RConnect for "<<sgm->GetRName()<<std::endl;
  r->SetExploreForward();
  }
  else {
    std::cout<<"Reverse RConnect for "<<sgm->GetRName()<<std::endl;
    r->SetExploreBackward();
  }

  nodeId n = g_graph_->ToNodeId(GetStart());
  CPUTimer t;
  t.StartTimer();
  r->RConnect(n, true);

  std::cout<<"Time (mu.s): "<<t.EndTimer()*10000000<<std::endl;
  std::cout<<"Subgoals: "<<r->GetExistingSubgoals()->size()<<std::endl;

  ExploreRReachableAreaSpecific();

  return;
}

template<class SGM>
void Manager<SGM>::SinglePointQuery(xyLoc loc) {
  SetStartXY(loc);
  SinglePointQuery();
}

template<class SGM>
void Manager<SGM>::SinglePointQuery() {
  nodeId n = g_graph_->ToNodeId(GetStart());

  QueryMethod* method = methods_[method_id_];

  std::cout << "NodeId: " << n << std::endl;
  auto g = method->GetOriginalGraph();
  if (g != NULL) {
    if (g->IsValidNode(n))
      std::cout<<"Valid node.\n";
    else
      std::cout<<"Not a valid node.\n";
    std::cout << "Original graph arcs: "<<std::endl;
    for (auto a: g->GetSuccessors(n)){
      std::cout<<a;
      std::cout<<"\t"<<g_graph_->ToState(a.target)<<std::endl;
    }
    SinglePointQueryGraphInfo();
  }
  std::cout << std::endl;

  subgoalId s = kNonSubgoal;
  SubgoalIdMapper* sm = method->GetSubgoalIdMapper();
  if (sm != NULL)
    s = sm->ToSubgoalId(n);
  std::cout << "SubgoalId: "<< s << std::endl;

  auto sg = method->GetSubgoalGraph();
  if (sg != NULL && s != kNonSubgoal) {
    std::cout << "Subgoal graph arcs: "<<std::endl;
    for (auto a: sg->GetSuccessors(s)){
      std::cout<<a<<std::endl;
//      std::cout<<"\t"<<g_graph_->ToState(a.target)<<std::endl;
    }
  }
  std::cout << std::endl;

  level l = kBaseLevel;
  LevelManager* lm = method->GetLevelManager();
  if (lm->IsInitialized()) {
    if (sm != NULL) {
      if (s != kNonNode)
        l = lm->GetLevel(s);
    }
    else
      l = lm->GetLevel(n);
  }
  std::cout << "Level: " << l << std::endl;
  ExploreRReachableArea();
}

template<class SGM>
void Manager<SGM>::DoublePointQuery(xyLoc l1, xyLoc l2) {
  SetStartXY(l1);
  SetGoalXY(l2);
  DoublePointQuery();
}

template<class SGM>
typename Manager<SGM>::State Manager<SGM>::GetStart() {
  return xyt_start;
}

template<>
typename Manager<Grid2DSG>::State Manager<Grid2DSG>::GetStart() {
  return xy_start;
}

template<class SGM>
typename Manager<SGM>::State Manager<SGM>::GetGoal() {
  return xyt_goal;
}

template<>
typename Manager<Grid2DSG>::State Manager<Grid2DSG>::GetGoal() {
  return xy_goal;
}

