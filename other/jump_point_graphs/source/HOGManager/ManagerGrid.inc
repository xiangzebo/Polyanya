/*
 * ManagerGrid.inc
 *
 *  Created on: Nov 8, 2018
 *      Author: idm-lab
 */


template<>
std::string Manager<Grid2DSG>::GetName() {
  return "Grid2D";
}


template<>
void Manager<Grid2DSG>::InitializeMethods() {
  auto AddParam = [&](CHParam chp, SGParam sgp, std::string override = "") {
    c.push_back(chp);
    s.push_back(sgp);
    override_method_names_.push_back(override);
  };

  bool dc = true;  // use double clearance.

  SGGrid2DParam sub = SGGrid2DParam(kGrid2DSGCanonical, dc);
  SGGrid2DParam jpag = SGGrid2DParam(kGrid2DJP, dc);
  SGGrid2DParam jpa = jpag;
  jpa.use_jp_search = false;
  SGGrid2DParam jp = jpa;
  jp.use_avoidance_table = false;


  SGGrid2DParam subf = SGGrid2DParam(kGrid2DSGFreespace, dc);
  /*
  SGGrid2DParam csg = SGGrid2DParam(kGrid2DCSG, dc);
  SGGrid2DParam jpdg = SGGrid2DParam(kGrid2DJPDiagonalCorners, dc);
  SGGrid2DParam jpdgm = SGGrid2DParam(kGrid2DJPDiagonalCornersMerged, dc);
  SGGrid2DParam jpdgm_r = jpdgm;
  SGGrid2DParam jp_r = jp;
  jpdgm_r.avoid_redundant_edges_during_contraction = false;
  jp_r.avoid_redundant_edges_during_contraction = false;
  */


  SGGrid2DParam none = SGGrid2DParam(kGrid2DSGFreespace, false);

  // Dijkstra
  //AddParam(MakeGParam(0, 0));

  // Bidirectional dijkstra
  //AddParam(MakeBidijGParam());

  // A*
  AddParam(MakeGParam(0), none, "A*");

  // wA*
  //AddParam(MakeWAStarGParam(2));

  // Always true (just for easy reading for the rest of the function).
  int p_unpack = true;
  bool sg = true;
  bool r_refine = true;
  bool astar = true;
  bool stall = true;


  // SG variants
  //AddParam(MakeGParam(sg, false), sub, "SG-D");
  //AddParam(MakeGParam(sg, false), jp, "JP-D");

  AddParam(MakeGParam(sg), sub, "SG");
  //AddParam(MakeGParam(sg), csg, "CSG");
  //AddParam(MakeGParam(sg), jpdg, "JP-Dg");
  //AddParam(MakeGParam(sg), jpdgm, "JP-DgM");
  AddParam(MakeGParam(sg), jp, "JP");
  //AddParam(MakeGParam(sg), jpa, "JP-A");
  //AddParam(MakeGParam(sg), jpag, "JP-AG");

  // CH variants
  AddParam(MakeCHParam(!sg, !r_refine, !p_unpack, !astar, stall), none, "CH-GPPC");
  //AddParam(MakeCHParam(!sg, !r_refine, p_unpack, astar, stall), none, "CH");
  //AddParam(MakeCHParam(!sg, r_refine, p_unpack), kGrid2DSGCanonical);

  // SG-CH variants
  //*
  //for (int stall_ = 0; stall_ <=1; stall_++)
  {
    //auto sgch_d = MakeCHParam(sg, !r_refine, !p_unpack, !astar, !stall);
    //AddParam(sgch_d, sub, "CH-SG-D");
    //AddParam(sgch_d, jp, "CHm-JP-D");

    auto sgch = MakeCHParam(sg, !r_refine, p_unpack, astar, stall);

    //AddParam(sgch, sub, "CH-SG");
    //AddParam(sgch, jp, "CH-JP");
    //AddParam(sgch, jpa, "CH-JP-A");
    //AddParam(sgch, jpag, "CH-JP-AG");

    //AddParam(sgch, jpdgm_r, "CH-JP-DgM");
    //AddParam(sgch, jpdgm, "CHm-JP-DgM");
    //AddParam(sgch, jp_r, "CH-JP");

    //AddParam(sgch, csg, "CH-CSG");
    //AddParam(sgch, jpdg, "CH-JP-Dg");
  }
  //*/
  // CHSL variants
  //AddParam(MakeCHSLParam(p_unpack), none);

  //*

  // RCH variants
  //AddParam(MakeRCHParam(!sg), sub);
  //AddParam(MakeRCHParam(!sg), subf);
  //AddParam(MakeRCHParam(sg), sub);
  //AddParam(MakeRCHParam(sg), subf);

  // N-SUB variants
  //AddParam(MakeNSUBParam(), sub);
  //AddParam(MakeNSUBParam(), subf);
  //*/


}


template<>
void Manager<Grid2DSG>::InitializeNumAngles() {
  num_angles_ = 8;
}

template<>
void Manager<Grid2DSG>::SinglePointQueryGraphInfo() {
  g_graph_->GetGrid()->TestDirections(GetStart());
  methods_[method_id_]->GetSubgoalGraphManager()->PrintNodeInfo(GetStart(),
                                                                t_start);
}

template<>
void Manager<Grid2DSG>::ExploreRReachableArea() {
  if (xy_start.x == 10000)
    return;

  auto sgm = methods_[method_id_]->GetSubgoalGraphManager();
  ReachabilityRelation* r = sgm ->GetReachabilityRelationIfExists();

  if (r == NULL)
    return;

  if (!explore_backward_) {
  std::cout<<"RConnect for "<<sgm->GetRName()<<std::endl;
  r->SetExploreForward();
  }
  else {
    std::cout<<"Reverse RConnect for "<<sgm->GetRName()<<std::endl;
    r->SetExploreBackward();
  }

  nodeId n;
  /*
  if (sgm->GetOriginalGraph()->IsDirectionExtended())
    n = sgm->GetOriginalGraph()->GetGrid()->ToLinearizedXYDCoordinate(
        GetStart(), t_start);
  else
  */
    n = g_graph_->ToNodeId(GetStart());

  CPUTimer t;
  t.StartTimer();
  r->RConnect(n, true);

  std::cout<<"Time (mu.s): "<<t.EndTimer()*10000000<<std::endl;
  std::cout<<"Subgoals: "<<r->GetExistingSubgoals()->size()<<std::endl;

  return;
}


template<>
void Manager<Grid2DSG>::DrawStartNode(const MapEnvironment *env) {
  g_graph_->DrawNode(env, g_graph_->ToNodeId(xy_start));
  //g_graph_->DrawNode(env, xy_start, (Direction2D) t_start);
}
template<>
void Manager<Grid2DSG>::DrawGoalNode(const MapEnvironment *env) {
  g_graph_->DrawNode(env, g_graph_->ToNodeId(xy_goal));
}


template<>
void Manager<Grid2DSG>::SetupVisualsDomainSpecific() {
  AddRReachableAreaDisplayType(
      kDisplayClearanceLines,
      "Showing clearances.");
}


