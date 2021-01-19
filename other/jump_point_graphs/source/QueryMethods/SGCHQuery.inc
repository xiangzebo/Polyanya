/*
 * Grid2DSG.cpp
 *
 *  Created on: Oct 22, 2017
 *      Author: idm-lab
 */

#include "SGCHQuery.h"

#include <iostream>
#include <vector>

#include "../ContractionHierarchy/CHSearchGraph.h"
#include "../ContractionHierarchy/ContractionSimulator.h"
#include "../Graphs/GraphDefinitions.h"
#include "../SearchMethods/BidirectionalDijkstra.h"
#include "../Utils/FileReadWrite.h"
#include "../Utils/GraphUtils.h"

class DummyPathRefiner;

template <class SGM>
SGCHQuery<SGM>::SGCHQuery(DirectoryParam dp, CHParam p, SGParam sg_p)
    : dp_(dp),
      sgm_(new SGM(dp_.GetMapFilename(), sg_p)),
      g_(sgm_->GetOriginalGraph()),
      h_(sgm_->GetHeuristic()) {

  dp_.MakeDirectories(sgm_->GetGName());
  p_ = p;

  // Subgoal graph
  reachability_relation_ = NULL;
  sm_ = NULL;
  sg_ = NULL;

  // Subgoal graph heuristic
  sg_h_ = NULL;

  // Graph types
  ch_shortcut_arc_graph_ = NULL;
  ch_reachable_arc_graph_ = NULL;

  // SG Query formers
  forward_sg_query_ = NULL;
  bidirectional_ch_query_ = NULL;
  bidirectional_rch_query_ = NULL;

  // Search algorithms
  g_dij_ = NULL;
  g_bidij_ = NULL;
  g_astar_ = NULL;
  sg_dij_ = NULL;
  sg_astar_ = NULL;
  reachable_ch_bidij_ = NULL;
  shortcut_ch_bidij_ = NULL;

  ch_bi_astar_reachable_oct_ = NULL;
  ch_bi_astar_reachable_sg_oct_ = NULL;
  ch_bi_astar_shortcut_oct_ = NULL;
  ch_bi_astar_shortcut_sg_oct_ = NULL;

  jp_astar_ = NULL;
  jp_ch_bidij_ = NULL;
  jp_ch_bi_astar_ = NULL;

  // Refinement algorithms
  node_id_sg_path_refiner_ = NULL;
  subgoal_id_sg_path_refiner_ = NULL;
  ch_shortcut_arc_path_refiner_ = NULL;
  subgoal_id_ch_sg_path_refiner_ = NULL;
  ch_shortcut_arc_path_rm_refiner_ = NULL;

  // Shortcut2 versions
  ch_shortcut2_arc_graph_ = NULL;
  bidirectional_ch2_query_ = NULL;
  shortcut2_ch_bidij_ = NULL;
  ch_bi_astar_shortcut2_oct_ = NULL;
  ch_bi_astar_shortcut2_sg_oct_ = NULL;
  ch_bi_astar_shortcut2_jp_oct_ = NULL;
  ch_shortcut2_arc_path_refiner_ = NULL;
  subgoal_id_ch2_sg_path_refiner_ = NULL;
  ch_shortcut2_arc_path_rm_refiner_ = NULL;

  num_nodes_ = 0;
  num_arcs_ = 0;
  num_reachable_shortcuts_ = 0;

  preprocess_time_ = 0;
  contract_time_ = 0;
  memory_ = 0;
  graph_memory_ = 0;
  r_memory_ = 0;

  last_start_ = kNonNode;
  last_goal_ = kNonNode;
}

template <class SGM>
SGCHQuery<SGM>::~SGCHQuery() {
  // Subgoal graph
  if (sgm_)
    delete sgm_;

  // Subgoal graph heuristic
  if (sg_h_)
    delete sg_h_;

  // Graph types
  if (ch_shortcut_arc_graph_)
    delete ch_shortcut_arc_graph_;
  if (ch_reachable_arc_graph_)
    delete ch_reachable_arc_graph_;

  // SG Query formers
  if (forward_sg_query_)
    delete forward_sg_query_;
  if (bidirectional_ch_query_)
    delete bidirectional_ch_query_;
  if (bidirectional_rch_query_)
    delete bidirectional_rch_query_;

  // Search algorithms
  if (g_dij_)
    delete g_dij_;
  if (g_bidij_)
    delete g_bidij_;
  if (g_astar_)
    delete g_astar_;
  if (sg_dij_)
    delete sg_dij_;
  if (sg_astar_)
    delete sg_astar_;

  if (reachable_ch_bidij_)
    delete reachable_ch_bidij_;
  if (shortcut_ch_bidij_)
    delete shortcut_ch_bidij_;

  if (ch_bi_astar_reachable_oct_)
    delete ch_bi_astar_reachable_oct_;
  if (ch_bi_astar_reachable_sg_oct_)
    delete ch_bi_astar_reachable_sg_oct_;
  if (ch_bi_astar_shortcut_oct_)
    delete ch_bi_astar_shortcut_oct_;
  if (ch_bi_astar_shortcut_sg_oct_)
    delete ch_bi_astar_shortcut_sg_oct_;

  // Refinement algorithms
  if (node_id_sg_path_refiner_)
    delete node_id_sg_path_refiner_;
  if (subgoal_id_sg_path_refiner_)
    delete subgoal_id_sg_path_refiner_;
  if (ch_shortcut_arc_path_refiner_)
    delete ch_shortcut_arc_path_refiner_;
  if (subgoal_id_ch_sg_path_refiner_)
    delete subgoal_id_ch_sg_path_refiner_;
  if (ch_shortcut_arc_path_rm_refiner_)
    delete ch_shortcut_arc_path_rm_refiner_;

  // Shortcut2 versions
  if (ch_shortcut2_arc_graph_)
    delete ch_shortcut2_arc_graph_;
  if (bidirectional_ch2_query_)
    delete bidirectional_ch2_query_;
  if (shortcut2_ch_bidij_)
    delete shortcut2_ch_bidij_;
  if (ch_bi_astar_shortcut2_oct_)
    delete ch_bi_astar_shortcut2_oct_;
  if (ch_bi_astar_shortcut2_sg_oct_)
    delete ch_bi_astar_shortcut2_sg_oct_;
  if (ch_bi_astar_shortcut2_jp_oct_)
    delete ch_bi_astar_shortcut2_jp_oct_;
  if (ch_shortcut2_arc_path_refiner_)
    delete ch_shortcut2_arc_path_refiner_;
  if (subgoal_id_ch2_sg_path_refiner_)
    delete subgoal_id_ch2_sg_path_refiner_;
  if (ch_shortcut2_arc_path_rm_refiner_)
    delete ch_shortcut2_arc_path_rm_refiner_;

  // JP versions
  if (jp_astar_)
    delete jp_astar_;
  if (jp_ch_bi_astar_)
    delete jp_ch_bi_astar_;
  if (jp_ch_bidij_)
    delete jp_ch_bidij_;

}

template <class SGM>
QueryStatistic SGCHQuery<SGM>::Query(State start, State goal,
                                std::vector<State> & state_path) {
  QueryStatistic stat;
  nodeId s = g_->ToNodeId(start);
  nodeId g = g_->ToNodeId(goal);
  std::vector<nodeId> p;

  DummyQuerySubgoalGraph* dummy_q = NULL;
  DummyPathRefiner* dummy_r = NULL;

  // Original graph
  if (!p_.use_subgoal_graph && p_.contraction_type == kNoContraction) {
    if (p_.use_astar)
      stat = FindPath(dummy_q, g_astar_, dummy_r, s, g, p);
    else if (p_.use_bidirectional_dijkstra)
      stat = FindPath(dummy_q, g_bidij_, dummy_r, s, g, p);
    else
      stat = FindPath(dummy_q, g_dij_, dummy_r, s, g, p);
  }
  if (!p_.use_subgoal_graph && p_.contraction_type == kRegularContraction) {
    if (!p_.unpack_using_pointers_) {
      if (p_.use_astar) {
        if (!p_.r_refine_when_possible)
          stat = FindPath(dummy_q, ch_bi_astar_shortcut_oct_,
                          ch_shortcut_arc_path_refiner_, s, g, p);
        else
          stat = FindPath(dummy_q, ch_bi_astar_shortcut_oct_,
                          ch_shortcut_arc_path_rm_refiner_, s, g, p);
      }
      else {
        if (!p_.r_refine_when_possible)
          stat = FindPath(dummy_q, shortcut_ch_bidij_,
                          ch_shortcut_arc_path_refiner_, s, g, p);
        else
          stat = FindPath(dummy_q, shortcut_ch_bidij_,
                          ch_shortcut_arc_path_rm_refiner_, s, g, p);
      }
    }
    else {
      if (p_.use_astar) {
        if (!p_.r_refine_when_possible)
          stat = FindPath(dummy_q, ch_bi_astar_shortcut2_oct_,
                          ch_shortcut2_arc_path_refiner_, s, g, p);
        else
          stat = FindPath(dummy_q, ch_bi_astar_shortcut2_oct_,
                          ch_shortcut2_arc_path_rm_refiner_, s, g, p);
      }
      else {
        if (!p_.r_refine_when_possible)
          stat = FindPath(dummy_q, shortcut2_ch_bidij_,
                          ch_shortcut2_arc_path_refiner_, s, g, p);
        else
          stat = FindPath(dummy_q, shortcut2_ch_bidij_,
                          ch_shortcut2_arc_path_rm_refiner_, s, g, p);
      }
    }
  }
  if (!p_.use_subgoal_graph && p_.contraction_type == kRContraction) {
    if (p_.use_astar)
      stat = FindPath(dummy_q, ch_bi_astar_reachable_oct_,
                      node_id_sg_path_refiner_, s, g, p);
    else
      stat = FindPath(dummy_q, reachable_ch_bidij_,
                      node_id_sg_path_refiner_, s, g, p);
  }

  // SG
  if (p_.use_subgoal_graph && p_.contraction_type == kNoContraction) {
    if (sgm_->UseAvoidanceTable()) {
      if (sgm_->UseJPSearch()) {
        stat = FindPathJP(forward_sg_query_, jp_astar_,
                          subgoal_id_sg_path_refiner_, s, g, p);
      }
      else {
        stat = FindPathJP(forward_sg_query_, sg_astar_,
                          subgoal_id_sg_path_refiner_, s, g, p);
      }
    }
    else {
      stat = FindPath(forward_sg_query_, sg_astar_, subgoal_id_sg_path_refiner_,
                      s, g, p);
    }
  }
  if (p_.use_subgoal_graph && p_.contraction_type == kRegularContraction) {
    if (!p_.unpack_using_pointers_) {
      if (sgm_->UseAvoidanceTable()) {
        if (sgm_->UseJPSearch()) {
          stat = FindPathJP(bidirectional_ch_query_, jp_ch_bi_astar_,
                            subgoal_id_ch_sg_path_refiner_, s, g, p);
        }
        else {
          stat = FindPathJP(bidirectional_ch_query_,
                            ch_bi_astar_shortcut_sg_oct_,
                            subgoal_id_ch_sg_path_refiner_, s, g, p);
        }
      }
      else {
        stat = FindPath(bidirectional_ch_query_, ch_bi_astar_shortcut_sg_oct_,
                        subgoal_id_ch_sg_path_refiner_, s, g, p);
      }
    }
    else {
      if (sgm_->UseAvoidanceTable()) {
        if (sgm_->UseJPSearch()) {
          stat = FindPathJP(bidirectional_ch2_query_,
                            ch_bi_astar_shortcut2_jp_oct_,
                            subgoal_id_ch2_sg_path_refiner_, s, g, p);
        }
        else {
          stat = FindPathJP(bidirectional_ch2_query_,
                            ch_bi_astar_shortcut2_sg_oct_,
                            subgoal_id_ch2_sg_path_refiner_, s, g, p);
        }
      }
      else {
        stat = FindPath(bidirectional_ch2_query_, ch_bi_astar_shortcut2_sg_oct_,
                        subgoal_id_ch2_sg_path_refiner_, s, g, p);
      }
    }
  }
  if (p_.use_subgoal_graph && p_.contraction_type == kRContraction) {
    if (p_.use_astar)
      stat = FindPath(bidirectional_rch_query_, ch_bi_astar_reachable_sg_oct_,
                      subgoal_id_sg_path_refiner_, s, g, p);
    else
      stat = FindPath(bidirectional_rch_query_, reachable_ch_bidij_,
                      subgoal_id_sg_path_refiner_, s, g, p);
  }
  if (p_.use_subgoal_graph && p_.contraction_type == kHeavyRContraction) {
    stat = FindPath(bidirectional_rch_query_, ch_bi_astar_reachable_sg_oct_,
                    subgoal_id_sg_path_refiner_, s, g, p);
  }

  state_path.clear();
  for (unsigned int i = 0; i < p.size(); i++)
    state_path.push_back(g_->ToState(p[i]));

  return stat;
}

template <class SGM>
template <class CHGraph>
bool SGCHQuery<SGM>::WriteCH(CHGraph* ch, std::string filename) {
  assert(p_.save);
  FileReadWrite rw;
  bool open = rw.StartWrite(filename);
  if (!open)
    return false;

  rw.Write(contract_time_);
  rw.Write(ch_construction_stat_);

  ch->Write(rw);
  level_.Write(rw);

  rw.CloseWrite();
  return true;
}

template <class SGM>
template <class CHGraph>
bool SGCHQuery<SGM>::ReadCH(CHGraph* & ch, std::string filename) {
  assert(p_.load);
  FileReadWrite rw;
  bool open = rw.StartRead(filename);
  if (!open)
    return false;

  rw.Read(contract_time_);
  rw.Read(ch_construction_stat_);

  if (ch == NULL)
    ch = new CHGraph();
  ch->Read(rw);
  level_.Read(rw);

  rw.CloseRead();
  return true;
}

template <class SGM>
void SGCHQuery<SGM>::Preprocess() {

  std::cout << "Map filename: " << dp_.GetMapFilename() << std::endl;
//  std::cout << "Instance filename: " << GetInstanceFilename() << std::endl;
//  std::cout << "Statistics filename: " << GetStatisticsFilename() << std::endl;
//  std::cout << "SG save filename: " << GetSubSaveFilename() << std::endl;
//  std::cout << "CH save filename: " << GetCHSaveFilename() << std::endl;

  ConstructSGComponents();
  ConstructCH();
  if (p_.use_subgoal_graph) {
    ConstructSubgoalSearch();
  } else {
    ConstructGridSearch();
  }

  // Assert that exactly one of reachable_arc_graph_, shortcut_arc_graph_ or
  // sg_g_ is initialized.
  int g_count = 0;
  if (ch_reachable_arc_graph_ )
    g_count++;
  if (ch_shortcut_arc_graph_)
    g_count++;
  if (ch_shortcut2_arc_graph_)
    g_count++;
  if (sg_)
    g_count++;

  // std::cout<<p_.GetConfigName()<<"\t"<<g_count<<std::endl;
  assert(
      (!p_.use_subgoal_graph && p_.contraction_type == kNoContraction)
          || g_count == 1);
}

template <class SGM>
void SGCHQuery<SGM>::ConstructSGComponents() {
  bool r = p_.r_refine_when_possible
      || p_.contraction_type == kRContraction
      || p_.contraction_type == kHeavyRContraction
      || p_.use_subgoal_graph;
  bool sm = p_.contract_subgoals_last || p_.use_subgoal_graph;
  bool sg = p_.use_subgoal_graph;
  bool sg_h = p_.use_subgoal_graph;

  if (!(r || sm || sg || sg_h))
    return;

  FileReadWrite rw;
  if (r) {
    bool read = false;
    std::string filename = GetRSaveFilename();
    if (p_.load && rw.StartRead(filename)) {
      sgm_->ReadR(rw);
      rw.CloseRead();
      std::cout<<"Loaded R from "<<filename<<std::endl;
      read = true;
    }

    reachability_relation_ = sgm_ -> GetReachabilityRelation();

    if (!read && p_.save && rw.StartWrite(filename)) {
      sgm_->WriteR(rw);
      rw.CloseWrite();
      std::cout<<"Saved R to "<<filename<<std::endl;
    }
  }
  if (sm) {
    bool read = false;
    std::string filename = GetRSPCSaveFilename();
    if (p_.load && rw.StartRead(filename)) {
      sgm_->ReadRSPC(rw);
      rw.CloseRead();
      std::cout<<"Loaded R-SPC from "<<filename<<std::endl;
      read = true;
    }

    sm_ = sgm_->GetSubgoalIdMapper();

    if (!read && p_.save && rw.StartWrite(filename)) {
      sgm_->WriteRSPC(rw);
      rw.CloseWrite();
      std::cout<<"Saved R-SPC to "<<filename<<std::endl;
    }
  }
  if (sg) {
    bool read = false;
    std::string filename = GetSubSaveFilename();
    if (p_.load && rw.StartRead(filename)) {
      sgm_->ReadSub(rw);
      rw.CloseRead();
      std::cout<<"Loaded subgoal graph from "<<filename<<std::endl;
      read = true;
    }

    sg_ = sgm_->GetSubgoalGraph();

    if (!read && p_.save && rw.StartWrite(filename)) {
      sgm_->WriteSub(rw);
      rw.CloseWrite();
      std::cout<<"Saved subgoal graph to "<<filename<<std::endl;
    }
  }
  if (sg_h) {
    sg_h_ = new SGH(sm_, h_);
  }
}

template <class SGM>
void SGCHQuery<SGM>::ConstructCH() {

  if (p_.contraction_type == kNoContraction)
    return;

  // If the hierarchy is saved to a file, read it (along with statistics).

  // Choose a filename that uniquely identifies the structure of the graph.
  // That is, CH and CH-R should have the same name
  // but CH and R-CH or CH-SUB should have different names.
  // Also depends on the map, the reachability relation, and subgoal graph.
  // Current method: mapname-graphname(-subname/refinementname).

  std::string filename = GetCHSaveFilename();
//  std::cout<<p_.GetMethodName() << "\t" << filename<<std::endl;

  // Read the hierarchy from file if one exists.
  if (p_.load) {
    bool successfully_read = false;
    if (p_.contraction_type == kRegularContraction) {
      if (ReadCH(ch_shortcut_arc_graph_, filename))
        successfully_read = true;
    }
    else {
      if (ReadCH(ch_reachable_arc_graph_, filename))
        successfully_read = true;
    }

    if (successfully_read) {
      std::cout << "Loaded hierarchy from " << filename << std::endl;
      if (p_.use_subgoal_graph)
        sg_ = NULL;
      return;
    }
  }

  // Construct CH and note the preprocessing time.
  int num_nodes;
  std::vector<WeightedArc> arcs;
  ExplicitGraphExtractor graph_extractor;

  if (p_.use_subgoal_graph) {
    assert(sg_ != NULL);
    assert(sm_ != NULL);
    graph_extractor.ExtractGraph(sg_, num_nodes, arcs);
    num_nodes = sm_->GetNumSubgoals();
    sg_ = NULL; // TODO: Delete subgoal graph?
  }
  else {
    graph_extractor.ExtractGraph(g_, num_nodes, arcs);
  }

  // Contract graph.
  CPUTimer t;
  t.StartTimer();
  CHConstructor ch_constructor(p_, num_nodes, arcs, sgm_->IsUndirected());
  if (p_.contraction_type == kRegularContraction) {
    if (!p_.use_subgoal_graph && p_.contract_subgoals_last){
      assert(sm_ != NULL);
      ch_constructor.ConstructCHContractSubgoalsLast(sm_);
    }
    else if (p_.use_subgoal_graph
        && sgm_->AvoidRedundantEdgesDuringContraction()) {
#ifdef JP_CH_AVOID_REDUNDANT_BY_GRID_SEARCH
      int num_grid_nodes;
      std::vector<WeightedArc> grid_arcs;
      graph_extractor.ExtractGraph(g_, num_grid_nodes, grid_arcs);

      auto gm = sgm_->GetGroupingMapperBaseGraph();
      ch_constructor.ConstructCHAvoidRedundantEdges(num_grid_nodes, grid_arcs,
                                                    gm, sm_);
      delete gm;
#else
      ch_constructor.ConstructCHAvoidRedundantEdges(num_nodes, arcs,
                                                    sgm_->GetGroupingMapper()
                                                    );
#endif
    }
    else {
      ch_constructor.ConstructCH();
    }
    ch_shortcut_arc_graph_ =
        ch_constructor.CreateSearchGraph<ShortcutArcGraph>();
  }
  else if (p_.contraction_type == kRContraction) {
    if (p_.use_subgoal_graph) {
      MappedR<SM, R> r(sm_, reachability_relation_);
      ch_constructor.ConstructCH(&r);
    }
    else
      ch_constructor.ConstructCH(reachability_relation_);

    ch_reachable_arc_graph_ =
        ch_constructor.CreateSearchGraph<ReachableArcGraph>();
  }
  else if (p_.contraction_type == kHeavyRContraction) {
    if (p_.use_subgoal_graph) {
      MappedR<SM, R> r(sm_, reachability_relation_);
      ch_constructor.ConstructCH(&r);
    }
    else
      ch_constructor.ConstructCH(reachability_relation_);

    ch_reachable_arc_graph_ =
        ch_constructor.CreateSearchGraph<ReachableArcGraph>();
  }
  contract_time_ = t.EndTimer();
  ch_construction_stat_ = ch_constructor.GetStatistics();
  level_ = *ch_constructor.GetCore()->GetLevelManager();

  if (ch_shortcut_arc_graph_ != NULL && p_.save) {
    WriteCH(ch_shortcut_arc_graph_, filename);
    std::cout<<"Saved hierarchy to "<<filename<<std::endl;
  }
  if (ch_reachable_arc_graph_ != NULL && p_.save) {
    WriteCH(ch_reachable_arc_graph_, filename);
    std::cout<<"Saved hierarchy to "<<filename<<std::endl;
  }
}

template <class SGM>
void SGCHQuery<SGM>::ConstructSubgoalSearch() {
  bool bidirectional = true;
  // If no contraction, just create search components and return.
  if (p_.contraction_type == kNoContraction) {
    // Components: forward_sg_query_, sg_astar_, subgoal_id_sg_path_refiner_
    // Connect
    forward_sg_query_ = new ForwardSGQueryFormer(sm_, reachability_relation_,
                                                 !bidirectional,
                                                 sg_);
    // Search
    if (!sgm_->UseJPSearch()) {
      // SG / SG-D
      sg_astar_ = new SGAStar(sg_, sg_h_, p_.w);
      if (!p_.use_astar)
        sg_astar_->UseDijkstra();
    }
    else {
      // JP / JP-D
      jp_astar_ = new JPAStar(sg_, sg_h_, sgm_->GetGroupingMapper(),
                              sgm_->GetForwardAvoidanceTable());
      if (!p_.use_astar)
        jp_astar_->UseDijkstra();
    }
/*
    if (p_.use_astar) {
      if (!sgm_->UseJPSearch())
        sg_astar_ = new SGAStar(sg_, sg_h_, p_.w);
      else {
        jp_astar_ = new JPAStar(sg_, sg_h_, sgm_->GetGroupingMapper(),
                                sgm_->GetForwardAvoidanceTable());
      }
    }

    else
      sg_dij_ = new SGDijkstra(sg_);
*/

    // Refine
    subgoal_id_sg_path_refiner_ = new SubgoalIdSGPathRefiner(
        reachability_relation_, sm_);
    return;
  }

  bool use_ch_search = true;

  // For regular contractions, create a graph with ShortcutArcs and
  // initialize the related search components.
  if (p_.contraction_type == kRegularContraction) {
    if (!p_.unpack_using_pointers_) {
      // Components: bidirectional_ch_query_, shortcut_ch_bidij_,
      //             subgoal_id_ch_sg_path_refiner_
      // Connect
      bidirectional_ch_query_ = new BidirectionalCHQueryFormer(
          sm_, reachability_relation_, bidirectional,
          ch_shortcut_arc_graph_->GetForwardGraph(),
          ch_shortcut_arc_graph_->GetBackwardGraph());

      // Search
      if (!sgm_->UseJPSearch()) {
        // CH-SG-M / CH-SG-DM
        ch_bi_astar_shortcut_sg_oct_ = new CHBiAstarShortcutSGOct(
            ch_shortcut_arc_graph_->GetForwardGraph(),
            ch_shortcut_arc_graph_->GetBackwardGraph(), sg_h_, sg_h_,
            p_.use_stall_on_demand);
        ch_bi_astar_shortcut_sg_oct_->SetAStar(p_.use_astar);
      }
      else {
        // CH-JP-M / CH-JP-DM
        jp_ch_bi_astar_ = new JPCHBidirectional(
            ch_shortcut_arc_graph_->GetForwardGraph(),
            ch_shortcut_arc_graph_->GetBackwardGraph(), sg_h_, sg_h_,
            sgm_->GetGroupingMapper(),
            sgm_->GetForwardAvoidanceTable(),
            sgm_->GetBackwardAvoidanceTable(),
            p_.use_stall_on_demand);
        jp_ch_bi_astar_->SetAStar(p_.use_astar);
      }

      // Refine
      subgoal_id_sg_path_refiner_ = new SubgoalIdSGPathRefiner(
          reachability_relation_, sm_);
      ch_shortcut_arc_path_refiner_ = new CHShortcutArcPathRefiner(
          ch_shortcut_arc_graph_->GetForwardGraph(),
          ch_shortcut_arc_graph_->GetBackwardGraph(), &level_);

      subgoal_id_ch_sg_path_refiner_ = new SubgoalIdCHSGPathRefiner(
          ch_shortcut_arc_path_refiner_, subgoal_id_sg_path_refiner_);

      // The CH part of CH-SUB might have reachable arcs that are being refined
      // the CH way. A quick fix is to simply change their mid points to kNonNode
      // so the CH refiner doesn't refine them and instead passes them along
      // to the SG refiner.
      if (p_.r_refine_when_possible)
        num_reachable_shortcuts_ = ch_shortcut_arc_path_refiner_
            ->MarkReachableArcsAsOriginalArcs(reachability_relation_, sm_);
    }
    else { // Unpacking using pointers
      // Components: bidirectional_ch_query_, shortcut_ch_bidij_,
      //             subgoal_id_ch_sg_path_refiner_
      // Graph
      ConvertCHGraphToShortcut2();
      assert(ch_shortcut_arc_graph_ == NULL);
      assert(ch_shortcut2_arc_graph_ != NULL);

      // Connect
      bidirectional_ch2_query_ = new BidirectionalCH2QueryFormer(
          sm_, reachability_relation_, bidirectional,
          ch_shortcut2_arc_graph_->GetForwardGraph(),
          ch_shortcut2_arc_graph_->GetBackwardGraph());

      if (!sgm_->UseJPSearch()) {
        // CH-SG-M / CH-SG-DM
        ch_bi_astar_shortcut2_sg_oct_ = new CHBiAstarShortcut2SGOct(
            ch_shortcut2_arc_graph_->GetForwardGraph(),
            ch_shortcut2_arc_graph_->GetBackwardGraph(), sg_h_, sg_h_,
            p_.use_stall_on_demand);
        ch_bi_astar_shortcut2_sg_oct_->SetAStar(p_.use_astar);
      }
      else {
        // CH-JP-M / CH-JP-DM
        ch_bi_astar_shortcut2_jp_oct_ = new CHBiAstarShortcut2JPOct(
            ch_shortcut2_arc_graph_->GetForwardGraph(),
            ch_shortcut2_arc_graph_->GetBackwardGraph(), sg_h_, sg_h_,
            sgm_->GetGroupingMapper(),
            sgm_->GetForwardAvoidanceTable(),
            sgm_->GetBackwardAvoidanceTable(),
            p_.use_stall_on_demand);
        ch_bi_astar_shortcut2_jp_oct_->SetAStar(p_.use_astar);
      }

/*
      // Search
      if (p_.use_astar)
        ch_bi_astar_shortcut2_sg_oct_ = new CHBiAstarShortcut2SGOct(
            ch_shortcut2_arc_graph_->GetForwardGraph(),
            ch_shortcut2_arc_graph_->GetBackwardGraph(), sg_h_, sg_h_,
            p_.use_stall_on_demand);
      else
        shortcut2_ch_bidij_ = new Shortcut2CHBidij(
            ch_shortcut2_arc_graph_->GetForwardGraph(),
            ch_shortcut2_arc_graph_->GetBackwardGraph(), use_ch_search,
            p_.use_stall_on_demand);
*/

      // Refine
      subgoal_id_sg_path_refiner_ = new SubgoalIdSGPathRefiner(
          reachability_relation_, sm_);
      ch_shortcut2_arc_path_refiner_ = new CHShortcut2ArcPathRefiner(
          ch_shortcut2_arc_graph_->GetForwardGraph(),
          ch_shortcut2_arc_graph_->GetBackwardGraph(), &level_);

      subgoal_id_ch2_sg_path_refiner_ = new SubgoalIdCH2SGPathRefiner(
          ch_shortcut2_arc_path_refiner_, subgoal_id_sg_path_refiner_);

      // The CH part of CH-SUB might have reachable arcs that are being refined
      // the CH way. A quick fix is to simply change their mid points to kNonNode
      // so the CH refiner doesn't refine them and instead passes them along
      // to the SG refiner.
      if (p_.r_refine_when_possible)
        num_reachable_shortcuts_ = ch_shortcut2_arc_path_refiner_
            ->MarkReachableArcsAsOriginalArcs(reachability_relation_, sm_);
    }
  }

  // For R-contractions, create a graph with reachable arcs and initialize the
  // related search components.
  if (p_.contraction_type == kRContraction) {
    // Components: bidirectional_rch_query_, reachable_ch_bidij_,
    //             subgoal_id_sg_path_refiner_

    // Connect
    bidirectional_rch_query_ = new BidirectionalRCHQueryFormer(
        sm_, reachability_relation_, bidirectional,
        ch_reachable_arc_graph_->GetForwardGraph(),
                ch_reachable_arc_graph_->GetBackwardGraph());

    // Search
    if (p_.use_astar) {
      ch_bi_astar_reachable_sg_oct_ = new CHBiAStarReachableSGOct(
          ch_reachable_arc_graph_->GetForwardGraph(),
          ch_reachable_arc_graph_->GetBackwardGraph(), sg_h_, sg_h_,
          p_.use_stall_on_demand);
      ch_bi_astar_reachable_sg_oct_->SetRCHSearch(&level_);
    }
    else {
      reachable_ch_bidij_ = new ReachableCHBidij(
          ch_reachable_arc_graph_->GetForwardGraph(),
          ch_reachable_arc_graph_->GetBackwardGraph(), use_ch_search,
          p_.use_stall_on_demand);
      reachable_ch_bidij_->SetRCHSearch(&level_);
    }

    // Refine
    subgoal_id_sg_path_refiner_ = new SubgoalIdSGPathRefiner(
        reachability_relation_, sm_);
  }

  if (p_.contraction_type == kHeavyRContraction) {
    // Connect
    bidirectional_rch_query_ = new BidirectionalRCHQueryFormer(
        sm_, reachability_relation_, bidirectional,
        ch_reachable_arc_graph_->GetForwardGraph(),
        ch_reachable_arc_graph_->GetBackwardGraph());

    // Search
    if (p_.use_astar) {
      ch_bi_astar_reachable_sg_oct_ = new CHBiAStarReachableSGOct(
          ch_reachable_arc_graph_->GetForwardGraph(),
          ch_reachable_arc_graph_->GetBackwardGraph(), sg_h_, sg_h_,
          p_.use_stall_on_demand);
      ch_bi_astar_reachable_sg_oct_->SetNLevelSearch(&level_);
    }
    else {
      assert(false || "Bidirectional Dijkstra not implemented for Nlevel!");
    }

    // Refine
    subgoal_id_sg_path_refiner_ = new SubgoalIdSGPathRefiner(
        reachability_relation_, sm_);
  }
}

template <class SGM>
void SGCHQuery<SGM>::ConstructGridSearch() {

  // If no contraction, just create grid AStar and return.
  if (p_.contraction_type == kNoContraction) {
    if (p_.use_astar)
      g_astar_ = new GraphAStar(g_, h_, p_.w);
    else if (p_.use_bidirectional_dijkstra)
      g_bidij_ = new GraphBidij(g_, g_);
    else
      g_dij_ = new GraphDijkstra(g_);
    return;
  }

  // Create the graph and the search components.
  bool use_ch_search = true;


  // CH
  // For regular contractions, create a graph with ShortcutArcs and
  // initialize the related search components.
  if (p_.contraction_type == kRegularContraction) {
    // Components: dummy_q, shortcut_ch_bidij_, ch_shortcut_arc_path_refiner_

    if (!p_.unpack_using_pointers_) {
      // Search
      if (p_.use_astar) {
        ch_bi_astar_shortcut_oct_ = new CHBiAStarShortcutOct(
            ch_shortcut_arc_graph_->GetForwardGraph(),
            ch_shortcut_arc_graph_->GetBackwardGraph(), h_, h_,
            p_.use_stall_on_demand);
      }
      else {
        shortcut_ch_bidij_ = new ShortcutCHBidij(
            ch_shortcut_arc_graph_->GetForwardGraph(),
            ch_shortcut_arc_graph_->GetBackwardGraph(), use_ch_search,
            p_.use_stall_on_demand);
      }

      // Refine
      if (!p_.r_refine_when_possible) {
        ch_shortcut_arc_path_refiner_ = new CHShortcutArcPathRefiner(
            ch_shortcut_arc_graph_->GetForwardGraph(),
            ch_shortcut_arc_graph_->GetBackwardGraph(), &level_);
      }
      else {
        assert(reachability_relation_ != NULL);
        ch_shortcut_arc_path_rm_refiner_ = new CHShortcutArcPathRMRefiner(
            reachability_relation_, ch_shortcut_arc_graph_->GetForwardGraph(),
            ch_shortcut_arc_graph_->GetBackwardGraph(), &level_);
        num_reachable_shortcuts_ = ch_shortcut_arc_path_rm_refiner_
            ->MarkReachableArcs();
      }
    }
    else {
      // Graph
      ConvertCHGraphToShortcut2();
      assert(ch_shortcut_arc_graph_ == NULL);
      assert(ch_shortcut2_arc_graph_ != NULL);

      // Search
      if (p_.use_astar) {
        ch_bi_astar_shortcut2_oct_ = new CHBiAStarShortcut2Oct(
            ch_shortcut2_arc_graph_->GetForwardGraph(),
            ch_shortcut2_arc_graph_->GetBackwardGraph(), h_, h_,
            p_.use_stall_on_demand);
      }
      else {
        shortcut2_ch_bidij_ = new Shortcut2CHBidij(
            ch_shortcut2_arc_graph_->GetForwardGraph(),
            ch_shortcut2_arc_graph_->GetBackwardGraph(), use_ch_search,
            p_.use_stall_on_demand);
      }

      // Refine
      if (!p_.r_refine_when_possible) {
        ch_shortcut2_arc_path_refiner_ = new CHShortcut2ArcPathRefiner(
            ch_shortcut2_arc_graph_->GetForwardGraph(),
            ch_shortcut2_arc_graph_->GetBackwardGraph(), &level_);
      }
      else {
        assert(reachability_relation_ != NULL);
        ch_shortcut2_arc_path_rm_refiner_ = new CHShortcut2ArcPathRMRefiner(
            reachability_relation_, ch_shortcut2_arc_graph_->GetForwardGraph(),
            ch_shortcut2_arc_graph_->GetBackwardGraph(), &level_);
        num_reachable_shortcuts_ = ch_shortcut2_arc_path_rm_refiner_
            ->MarkReachableArcs();
      }
    }
  }

  // R-CH
  // For R-contractions, create a graph with ReachableArcs and initialize the
  // related search components.
  else if (p_.contraction_type == kRContraction) {

    // Search
    if (p_.use_astar) {
      ch_bi_astar_reachable_oct_ = new CHBiAStarReachableOct(
          ch_reachable_arc_graph_->GetForwardGraph(),
          ch_reachable_arc_graph_->GetBackwardGraph(), h_, h_,
          p_.use_stall_on_demand);
      ch_bi_astar_reachable_oct_->SetRCHSearch(&level_);
    } else {
      reachable_ch_bidij_ = new ReachableCHBidij(
          ch_reachable_arc_graph_->GetForwardGraph(),
          ch_reachable_arc_graph_->GetBackwardGraph(), use_ch_search,
          p_.use_stall_on_demand);
      reachable_ch_bidij_->SetRCHSearch(&level_);
    }
    // Refine
    // FIXME: Right now, using octile reachability for refinement.
    // But octile reachability uses clearances, which is not necessary for
    // refinement.
    assert(reachability_relation_ != NULL);
    //reachability_relation_ = sgm_->GetReachabilityRelation();
    node_id_sg_path_refiner_ = new NodeIdSGPathRefiner(reachability_relation_);
  }
}

// TODO: Directed
template<class SGM>
void SGCHQuery<SGM>::GenerateShortcut2ArcGraph(
    ShortcutArcGraph* shortcut, Shortcut2ArcGraph* shortcut2,
    std::vector<ShortcutArc> & shortcut_arcs) {
  // Create shortcut2 graph from shortcut1 graph, where first and second fields
  // are not initialized.

  int num_nodes;
  ExplicitGraphExtractor graph_extractor;
  graph_extractor.ExtractGraph(shortcut, num_nodes, shortcut_arcs);
  std::vector < Shortcut2Arc > shortcut2_arcs;
  for (auto a : shortcut_arcs)
    shortcut2_arcs.push_back(
        Shortcut2Arc(a.source, a.target, a.weight, kNonArc, kNonArc));
  shortcut2->CreateGraph(num_nodes, shortcut2_arcs);
}
template<class SGM>
void SGCHQuery<SGM>::ConvertCHGraphToShortcut2() {
  assert(ch_shortcut_arc_graph_ != NULL);
  assert(ch_shortcut2_arc_graph_ == NULL);
  //assert(ch_shortcut_arc_graph_->IsUndirected()); //FIXME: necessary?

  ch_shortcut2_arc_graph_ = new CHShortcut2ArcGraph();
  ch_shortcut2_arc_graph_->SetIsUndirected(
      ch_shortcut_arc_graph_->IsUndirected());

  std::vector<ShortcutArc> forward_shortcut_arcs, backward_shortcut_arcs;

  // Copy the forward and backward graphs to shortcut2_arc_graph
  GenerateShortcut2ArcGraph(ch_shortcut_arc_graph_->GetForwardGraph(),
                            ch_shortcut2_arc_graph_->GetForwardGraph(),
                            forward_shortcut_arcs);
  if (!ch_shortcut2_arc_graph_->IsUndirected()) {
    GenerateShortcut2ArcGraph(ch_shortcut_arc_graph_->GetBackwardGraph(),
                              ch_shortcut2_arc_graph_->GetBackwardGraph(),
                              backward_shortcut_arcs);
  }

  // The forward and backward shortcut2 graphs.
  auto shortcut2_fg = ch_shortcut2_arc_graph_->GetForwardGraph();
  auto shortcut2_bg = ch_shortcut2_arc_graph_->GetBackwardGraph();

  // The list of arcs in the forward and backward shortcut2 graphs (internal
  // representation, so arcIds are consistent with the interal representation).
  auto fg_arclist = shortcut2_fg->GetArcList();
  auto bg_arclist = shortcut2_bg->GetArcList();

  // Each forward shortcut arc (s,t) with midpoint m corresponds to a path
  // s-m-t. Since m's contraction created (s,t), m has a level lower than both
  // s and t in the hierarchy. That means, m translates into:
  // - The id of the arc (m,s) in the backward graph (first).
  // - The id of the arc (m,t) in the forward graph (second).

  // Go over all the midpoints in the forward shortcut graph and translate
  // the midpoints into first and second arcs.
  for (auto a: forward_shortcut_arcs) {
    arcId curr_arc = shortcut2_fg->GetArcId(a.source, a.target);
    assert(fg_arclist->at(curr_arc).target == a.target);
    arcId first_arc, second_arc;

    if (a.mid == kNonNode) {
      first_arc = kNonArc;
      second_arc = a.source;
    }
    else {
      first_arc = shortcut2_bg->GetArcId(a.mid, a.source);
      assert(bg_arclist->at(first_arc).target == a.source);

      second_arc = shortcut2_fg->GetArcId(a.mid, a.target);
      assert(fg_arclist->at(second_arc).target == a.target);
    }
    fg_arclist->at(curr_arc).first = first_arc;
    fg_arclist->at(curr_arc).second = second_arc;
  }

  // Go over all the midpoints in the backward shortcut graph and translate
  // the midpoints into first and second arcs.
  // List of backward arcs should be empty if the CH is undirected.
  assert(
      !ch_shortcut2_arc_graph_->IsUndirected()
          || backward_shortcut_arcs.empty());

  // Any upward backward arc (t,s) is a discarded downward forward arc (s,t).
  for (auto a: backward_shortcut_arcs) {
    arcId curr_arc = shortcut2_bg->GetArcId(a.source, a.target);
    assert(bg_arclist->at(curr_arc).target == a.target);
    arcId first_arc, second_arc;

    if (a.mid == kNonNode) {
      first_arc = kNonArc;
      second_arc = a.source;
    }
    else {
      // Source and target are swapped below, because we have (t,s), not (s,t).
      first_arc = shortcut2_fg->GetArcId(a.mid, a.source);
      assert(fg_arclist->at(first_arc).target == a.source);

      second_arc = shortcut2_bg->GetArcId(a.mid, a.target);
      assert(bg_arclist->at(second_arc).target == a.target);
    }
    bg_arclist->at(curr_arc).first = first_arc;
    bg_arclist->at(curr_arc).second = second_arc;
  }


  delete ch_shortcut_arc_graph_;
  ch_shortcut_arc_graph_ = NULL;
}

