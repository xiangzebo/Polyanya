/*
 * ManagerLattice.h
 *
 *  Created on: Nov 8, 2018
 *      Author: idm-lab
 */
#include <bitset>
#include <iostream>


template<>
std::string Manager<LatticeSG>::GetName() {
  return "Lattice";
}

template<>
void Manager<LatticeSG>::InitializeMethods() {
  LatticeHeuristicType euc = kLatticeEuclideanHeuristic;
  LatticeHeuristicType h2d = kLattice2DProjectionHeuristic;

  ////////////
  // LatticeParam
  std::vector<LatticeParam> lp;
//  lp.push_back(MakeLatticeParamGrid2D());
//  lp.push_back(MakeLatticeParamCircular());
//  lp.push_back(MakeLatticeParamInput3D("unicycle_noturninplace.mprim", "unicycle"));
  lp.push_back(MakeLatticeParamInput3D("unicycle_noturninplace4.mprim", "unicycle4"));
//  lp.push_back(MakeLatticeParamInput3D("unicycle_noturninplace-mod.mprim", "unicycle-mod"));
//  lp.push_back(MakeLatticeParamInput3D("mprim_unic_sideback.mprim", "unicycle-sideback"));
//  lp.push_back(MakeLatticeParamInput3D("mprim_unic_sideback-mod.mprim", "unicycle-sideback-mod"));
//  lp.push_back(MakeLatticeParamInput3D("mprim_unic_sidebackdiag.mprim", "unicylce-sidebackdiag"));
//  lp.push_back(MakeLatticeParamInput3D("prova3.mprim", "prova3"));
//  lp.push_back(MakeLatticeParamInput3D("prova3-mod.mprim", "prova3-mod"));
//  lp.push_back(MakeLatticeParamInput3D("pr2_all_2.5cm_20turncost.mprim", "pr2"));
//  lp.push_back(MakeLatticeParamInput4D("urban.mprim", "urban"));
  ////////////



  // G
  for (auto l : lp) {
    c.push_back(kCHP_G_A);  s.push_back(SGParam(l, euc));
  //  c.push_back(kCHP_G_A);  s.push_back(SGParam(l, h2d));
  }
  ////////////
  // wA*
  // Bounds
  std::vector<double> subopt;
//  for (double w = 1.25; w < 2.01; w += 0.25)
//  for (double w = 3.25; w < 5.01; w += 0.25)
//    subopt.push_back(w);
//  subopt.push_back(10000);
//  subopt.push_back(2);

  // Params
  for (auto l : lp) {
    for (auto w : subopt) {
      CHParam p = kCHP_G_A;
      p.w = w;
      c.push_back(p);  s.push_back(SGParam(l, euc));
      c.push_back(p); s.push_back(SGParam(l, h2d));
    }
  }
  ////////////

  ////////////
  // Reachability type
  std::vector<RParam> rt;
  rt.push_back(RParam(kBoundedDistanceReachability, 0, kRConnectAggressive));
//  rt.push_back(RParam(kBoundedDistanceReachability, 0, kRConnectConservative));
//  rt.push_back(RParam(kBoundedDistanceReachability, 0, kRConnectStall));
//  rt.push_back(RParam(kFreespaceReachability, 0, kRConnectConservative));
  rt.push_back(RParam(kFreespaceReachability, 0, kRConnectAggressive));
//  rt.push_back(RParam(kFreespaceReachability, 0, kRConnectSucc));
  rt.push_back(kCanonicalFreespaceReachability);
//  rt.push_back(kSafeFreespaceReachability);
  ////////////

  ////////////
  // Reachability bound
  std::vector<Distance> bounds;
  // Reachability relations
  Distance bound = kLatticeParamDefaultBound;
//  for (Distance bound = 25; bound < 25.01; bound += 25)
//  for (Distance bound = 75; bound < 125.01; bound += 25)
  {
//    bounds.push_back(bound);
  }

  bounds.push_back(50);
//  bounds.push_back(75);
//  bounds.push_back(100);
//  bounds.push_back(150);
//  bounds.push_back(200);
//  bounds.push_back(300);

  ////////////

  ////////////
  // Construction type
  std::vector<SGConstructionParam> sgp;
  int st = kDefaultSubgoalSelectionStrategy;
  int no = kDefaultNodeOrdering;
  int ps = kDefaultPrioritizeExploringFromSubgoals;

  /// kGrowRSPC
//  for (st = 0; st != kSubgoalSelectionStrategyNoType; st++)
//    for (ps = 0; ps <= 1; ps++) // Process subgoals first
//      for (no = 0; no != kNodeOrderingNoType; no++)
  {
    /*
    sgp.push_back(
        MakeSGGrowConstructionParam((SubgoalSelectionStrategy) st,
                                    (NodeOrdering) no, (bool) ps));
    //*/
  }

  /// kPruneRSPC
  no = kDefaultNodeOrdering;
//  for (no = 0; no != kNodeOrderingNoType; no++)
  {
//    sgp.push_back(MakeSGPruneConstructionParam((NodeOrdering) no));
  }

  /// kConnectedRSPC
  no = kDefaultNodeOrdering;
  double perc_extra = 0;
//  for (no = 0; no != kNodeOrderingNoType; no++)
//  for (perc_extra = 0; perc_extra < 1.01; perc_extra += 0.25)
  {
      sgp.push_back(MakeSGConnectedConstructionParam((NodeOrdering)no, perc_extra));
  }


  /// kRandomRSPC
  std::vector<double> perc;
//    perc.push_back(0);
//    perc.push_back(5);
//  perc.push_back(1);
//  perc.push_back(2);
//  for (int i = 1; i <= 40; i+=59)
//    perc.push_back(i);
  for (auto x : perc) {
    sgp.push_back(MakeSGRandomConstructionParam(x));
  }

  // Spanner on default subgoal graph
  for (auto w : subopt) {
    SGConstructionParam p;
    p.spanner_suboptimality = w;
//    sgp.push_back(p);
  }
  ////////////

  ////////////
  // SUB
  // Create all combinations of lattice, reachability relation, and subgoal
  // construction params to generate various SUBs.
  for (auto l : lp) {
    for (auto b : bounds) {
      for (auto sg : sgp) {
        for (auto r : rt) {
          RParam rp = r;
          rp.bound = b;
          c.push_back(kCHP_SUB_A);
          s.push_back(SGLatticeParam(l, rp, sg));
        }
      }
    }
  }

/*
  c.push_back(kCHP_SUB_A);  s.push_back(SGParam());
  c.push_back(kCHP_CH_AS);  s.push_back(SGParam());
  c.push_back(kCHP_CHR_AS);  s.push_back(SGParam());
  c.push_back(kCHP_SUBCH_AS);  s.push_back(SGParam());
  c.push_back(kCHP_SUBCHR_AS);  s.push_back(SGParam());

//  c.push_back(kCHP_CH_AS);  s.push_back(SGParam());

//  c.push_back(kCHP_SUBCH_AS);  s.push_back(FR50);
//  c.push_back(kCHP_SUBCH_AS);  s.push_back(SFR50);

//  c.push_back(kCHP_CHR_AS);  s.push_back(FR50);
//  c.push_back(kCHP_CHR_AS);  s.push_back(SFR50);

//  c.push_back(kCHP_SUBCHR_AS);  s.push_back(FR50);
//  c.push_back(kCHP_SUBCHR_AS);  s.push_back(SFR50);

//*/
}


template<>
void Manager<LatticeSG>::InitializeNumAngles() {
  num_angles_ = g_graph_->GetLattice()->GetNumAngles();
}

template<>
void Manager<LatticeSG>::ExploreRReachableAreaSpecific() {

  auto sgm = methods_[method_id_]->GetSubgoalGraphManager();
  ReachabilityRelation* r = sgm ->GetReachabilityRelationIfExists();
  if (r == NULL)
    return;

  std::cout<<"Expanded: "<<r->GetExpansionOrder()->size()<<std::endl;

  return;

  nodeId n = g_graph_->ToNodeId(GetStart());

  // Check if the query node is reachable in the forward (backward) direction
  // from all of its backward (forward) reachable nodes.
  std::vector<nodeId> expanded = *r->GetExpansionOrder();
  int explored = 0;
  int expansions = 0;
  for (auto u: expanded) {
    if (!explore_backward_)
      r->SetExploreBackward();
    else
      r->SetExploreForward();

    r->RConnect(u);
    bool reachable = false;
    for (auto v: *r->GetExpansionOrder()) {
      if (v == n) {
        reachable = true;
      }
      expansions++;
    }
    if (!reachable) {
      std::cout << "BUG: Can connect " << n << " and " << u
           << "in only one direction!" << std::endl;
    }
    explored++;
  }
  std::cout << "Performed " << explored << " explorations and " << expansions
       << " expansions to identify a counter example!" << std::endl;

  // Redo the exploration for correct visualization.
  if (explore_backward_)
    r->SetExploreBackward();
  else
    r->SetExploreForward();
  r->RConnect(n);

/*
  LatticeFreespaceExplorer* explorer = l_sg_->GetReachabilityRelation();
  explorer->SetExploreForward();
  std::cout << "Exploring the h-reachable area around " << xyt_start << std::endl;
  explorer->ExploreReachableAreaFringe(
      l_sg_->GetOriginalGraph()->ToNodeId(xyt_start));
  int num_new_subgoals = l_sg_->GetConstructor()
      ->GenerateSubgoalsForSourceNode(
      l_sg_->GetOriginalGraph()->ToNodeId(xyt_start));

  std::cout << explorer->GetExpansionOrder()->size() << " nodes explored; "
       << std::endl;
  std::cout << explorer->GetFringeNodes()->size() << " fringe nodes identified."
       << std::endl;
  std::cout << explorer->GetExistingSubgoals()->size()
       << " existing direct-h-reachable subgoals found." << std::endl;
  std::cout << num_new_subgoals << " new subgoals identified." << std::endl;
*/

}

template<>
void Manager<LatticeSG>::DoublePointQuerySpecific() {
  //  return;
    {

      auto sgm = methods_[method_id_]->GetSubgoalGraphManager();
  //    auto r = sgm ->GetReachabilityRelation();
      auto h = sgm->GetFreespaceManager();
      if (h != NULL) {
        std::cout << "Heuristic distance: "
             << h->GetHeuristicDistance(GetStart(), GetGoal()) << std::endl;
        std::cout << "Num_pred of goal: " << (int) h->GetNumPred(GetStart(), GetGoal())
             << std::endl;
        std::cout << "Num_succ of start: "
             << (int) h->GetNumSucc(GetStart(), GetGoal()) << std::endl;
        std::cout << "Canonical parent flag: "
             << (int) h->GetCanonicalParentId(GetStart(), GetGoal()) << std::endl;
        std::cout << "Canonical successor flags: "
             << std::bitset<16>(h->GetCanonicalSuccessorFlags(GetStart(), GetGoal()))
             << std::endl;
        std::cout
            << "Reverse cannoical successor flags: "
            << (std::bitset<16> ) (h->GetReverseCanonicalSuccessorFlags(GetStart(),
                                                                   GetGoal()))
            << std::endl;
      }
    }
}


template<>
void Manager<LatticeSG>::DrawStartNode(const MapEnvironment *env) {
  if (num_angles_ == 1)
    g_graph_->DrawNode(env, xyt_start, 2);
  else {
    g_graph_->DrawArrowNode(env, xyt_start, 2);
    //g_graph_->DrawDirectedNode(env, xyt_start, 2);
  }
}
template<>
void Manager<LatticeSG>::DrawGoalNode(const MapEnvironment *env) {
  if (num_angles_ == 1)
    g_graph_->DrawNode(env, xyt_goal, 2);
  else {
    g_graph_->DrawArrowNode(env, xyt_goal, 2);
    //g_graph_->DrawDirectedNode(env, xyt_goal, 2);
  }
}

template<>
void Manager<LatticeSG>::SetupVisualsDomainSpecific() {
  AddRReachableAreaDisplayType(
      kRReachableAreaDisplayNode,
      "Showing all expanded nodes in the R-Reachable area.");
  AddRReachableAreaDisplayType(
      kRReachableAreaDisplayParent,
      "Showing a single parent for all the expanded nodes in the R-Reachable area.");
  AddRReachableAreaDisplayType(
      kRReachableAreaDisplayNodeAndParent,
      "Showing the node and a single parent for all the expanded nodes in the R-Reachable area.");
  AddRReachableAreaDisplayType(
      kRReachableAreaDisplayAllPaths,
      "Showing all parents for all the expanded nodes in the R-Reachable area.");
  AddRReachableAreaDisplayType(kDisplayPrimitives, "Showing the primitives.");
  AddRReachableAreaDisplayType(kDisplaySGForwardEdges,
                               "Showing forward SG edges.");
  AddRReachableAreaDisplayType(kDisplaySGBackwardEdges,
                               "Showing backward SG edges.");
}
