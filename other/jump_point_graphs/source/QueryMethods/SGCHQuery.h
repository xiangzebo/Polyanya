/*
 * Grid2DSG.h
 *
 *  Created on: Oct 22, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_QUERYMETHODS_SGCHQUERY_H_
#define APPS_SUBGOALGRAPH_QUERYMETHODS_SGCHQUERY_H_

#include <cstdio>
#include <cstdlib>
#include <string>

#include "../ContractionHierarchy/CHArcPathRefiner.h"
#include "../ContractionHierarchy/CHConstructor.h"
#include "../ContractionHierarchy/CHPathRefiner.h"
#include "../Graphs/ArcListQueryGraph.h"
#include "../Graphs/CommonHeuristics.h"
#include "../Parameters/CHParam.h"
#include "../Parameters/DirectoryParam.h"
#include "../SearchMethods/AStar.h"
#include "../SearchMethods/AStarDirectionExtended.h"
#include "../SearchMethods/CHBidirectionalSearch.h"
#include "../SearchMethods/CHBidirectionalSearchDirectionExtended.h"
#include "../SearchMethods/Dijkstra.h"
#include "../SubgoalGraph/Search/DummyPathRefiner.h"
#include "../SubgoalGraph/Search/QuerySubgoalGraph.h"
#include "../SubgoalGraph/Search/SequentialPathRefiner.h"
#include "../SubgoalGraph/Search/SGPathRefiner.h"
#include "../Utils/CPUTimer.h"
#include "../Utils/LevelManager.h"
#include "../Utils/Experiments/InstanceManager.h"
#include "../Utils/Statistics/QueryStatistic.h"
#include "../Utils/Statistics/ConnectionStatistics.h"

class recColor;


// Should have gone with inheritance. Kept growing out of control.

// SGM = Subgoal Manager
template <class SGM>
class SGCHQuery {
 public:
//  typedef Grid2DSGManager SGM;
  typedef typename SGM::ParamType SGParam;

  typedef typename SGM::StateType State;
  typedef typename SGM::GraphType G;
  typedef typename SGM::HeuristicType H;

  typedef typename SGM::SubgoalIdMapperType SM;
  typedef typename SGM::ReachabilityType R;
  typedef typename SGM::SGGraphType SG;


  // TODO: Make it specific to subgoal and/or R?
  typedef WeightedArcHead RA; // Reachable arc
  typedef ArcListQueryGraph<RA> ReachableArcGraph;
  typedef ArcListQueryGraph<ShortcutArcHead> ShortcutArcGraph;
  typedef ArcListQueryGraph<Shortcut2ArcHead> Shortcut2ArcGraph;

  typedef CHSearchGraph<ReachableArcGraph> CHReachableArcGraph;
  typedef CHSearchGraph<ShortcutArcGraph> CHShortcutArcGraph;
  typedef CHSearchGraph<Shortcut2ArcGraph> CHShortcut2ArcGraph;

  // TODO: Work out. Possibly takes as input three different params?
  // One for graph (for instance, lattice type),
  // One for subgoal graph (for instance, reachability relation type),
  // One for CH type.
  SGCHQuery(DirectoryParam dp, CHParam p = CHParam(), SGParam sg_p =
                SGParam());
  ~SGCHQuery();

  void Preprocess();

  // The following functions generate names for different methods.
  // GetName returns a unique name for the method, including search parameters.
  // That is, if two methods provide different results, they should have
  // different names.
  // GetSavefilename methods operate by

  std::string GetName() {
    std::string s = p_.GetMethodName();

    if (sgm_->UseJPSearch() && !sgm_->AvoidRedundantEdgesDuringContraction())
      s = s + "r";
    if (p_.use_subgoal_graph || p_.contract_subgoals_last)
      s = s + "_" + sgm_->GetSubName();
    if (!sgm_->RSPCNameIncludesRName() && (p_.r_refine_when_possible || p_.contraction_type == kRContraction
        || p_.contraction_type == kHeavyRContraction))
      s = s + "_" + sgm_->GetRName();

    s += "_" + p_.GetSearchName();
    s += sgm_->GetSearchSuffix();

    return s;
  }
  std::string GetInstanceFilename() {
    return dp_.GetInstanceFilename(sgm_->GetGName());
  }
  std::string GetStatisticsFilename() {
    return dp_.GetStatisticsDirectory(sgm_->GetGName()) + "/" + GetName()
        + ".stat";
  }
  std::string GetInstanceStatisticsFilename() {
    return dp_.GetInstanceStatisticsDirectory(sgm_->GetGName()) + "/"
        + GetName() + ".stat.ins";
  }

  std::string GetRSaveFilename() {
    return dp_.GetSaveLoadDirectory(sgm_->GetGName()) + "/" + sgm_->GetRName()
        + ".r";
  }
  std::string GetRSPCSaveFilename() {
    return dp_.GetSaveLoadDirectory(sgm_->GetGName()) + "/"
        + sgm_->GetRSPCName() + ".rspc";
  }
  std::string GetSubSaveFilename() {
    return dp_.GetSaveLoadDirectory(sgm_->GetGName()) + "/" + sgm_->GetSubName()
        + ".sub";
  }

  std::string GetCHSaveFilename() {
    std::string filename = dp_.GetSaveLoadDirectory(sgm_->GetGName()) + "/"
        + p_.GetGraphName();
    if (!sgm_->UniqueSUB()
        && (p_.use_subgoal_graph || p_.contract_subgoals_last))
      filename = filename + "-" + sgm_->GetRSPCName();
    if (p_.contraction_type == kRContraction
        || p_.contraction_type == kHeavyRContraction)
      filename = filename + "-" + sgm_->GetRName();

    if (sgm_->UseJPSearch() && !sgm_->AvoidRedundantEdgesDuringContraction())
      filename = filename + "-r";

    return filename;
  }

  void ClearFiles() {
   std::vector<std::string> commands;
   commands.push_back("rm -f " + GetRSaveFilename());
   commands.push_back("rm -f " + GetRSPCSaveFilename());
   commands.push_back("rm -f " + GetSubSaveFilename());
   commands.push_back("rm -f " + GetCHSaveFilename());
   for (auto cmd: commands)
     system(cmd.c_str());
  }

  CHParam* GetParam() {
    return &p_;
  }

  QueryStatistic Query(State start, State goal, std::vector<State> & path);
  QueryStatistic Query(ProblemInstance<State> ins, std::vector<State> & path) {
    return Query(ins.start, ins.goal, path);
  }
  QueryStatistic Query(ProblemInstance<State> ins) {
    std::vector<State> dummy_path;
    return Query(ins.start, ins.goal, dummy_path);
  }

  // Report any extra statistics for construction/search
  void ReportPreprocessStatistics(StatisticsFile* s);
  void ReportAdditionalQueryStatistics(StatisticsFile* s);

  // For visualization: Replace with get valid nodes / arcs?
  G* GetOriginalGraph() {
    return g_;
  }
  SM* GetSubgoalIdMapper() {
    return sm_;
  }

  LevelManager* GetLevelManager() {
    return &level_;
  }

  int GetMaxLevel() const {
    return level_.GetMaxLevel();
  }

  SG* GetSubgoalGraph() {
    return sg_;
  }

  SGM* GetSubgoalGraphManager() {
    return sgm_;
  }

  AggregateConnectionStatistics GetAggregateConnectionStatistics() {
    if (forward_sg_query_ != NULL)
      return forward_sg_query_->GetAggregateConnectionStatistics();
    if (bidirectional_ch_query_ != NULL)
      return bidirectional_ch_query_->GetAggregateConnectionStatistics();
    if (bidirectional_rch_query_ != NULL)
      return bidirectional_rch_query_->GetAggregateConnectionStatistics();
    if (bidirectional_ch2_query_ != NULL)
      return bidirectional_ch2_query_->GetAggregateConnectionStatistics();
    return AggregateConnectionStatistics();
  }


#ifndef NO_HOG
  void VisualizeSearch(const MapEnvironment* env, bool forward,
                       bool backward, bool nodes);

  void VisualizeGraph(const MapEnvironment* env, bool display_nodes,
                      bool display_arcs, int display_level);

  void PrintInfo(std::ostream & out = std::cout) {
    char buffer [1000];
    sprintf(buffer, "N: %7.0f A: %10.0f L: %3.0f | PT: %7.2f M: %5.2f + %5.2f = %5.2f",
            (double) num_nodes_, (double) num_arcs_, (double) GetMaxLevel(),
            preprocess_time_, graph_memory_, r_memory_, memory_);
    out << buffer;
    return;

    out<<num_nodes_<<" nodes, ";
    out<<num_arcs_<<" arcs, ";
    out<<GetMaxLevel()<<" levels | ";
    out<<"PT: "<<preprocess_time_;
    out<<", M: "<<memory_;
  }
#endif

 private:
  CHParam p_;
  DirectoryParam dp_;
  std::string map_;
  std::string save_load_path_;

  SGM* sgm_;

  // Original graph
  G* g_;
  H* h_;

  // Subgoal graph
  R* reachability_relation_;
  SM* sm_;
  SG* sg_;

  // SG heuristic
  typedef MappedHeuristic<SM, H > SGH;
  SGH* sg_h_;

  // Node levels in any of the hierarchies.
  LevelManager level_;

  // GraphTypes
  //ShortcutArcGraph* shortcut_arc_graph_;
  //ReachableArcGraph* reachable_arc_graph_;  // TODO: Remove weight;

  // TODO: Rename? It is not a graph, but a structure that can store 1 or 2
  // graphs, depending on whether the original graph is undirected.
  CHReachableArcGraph* ch_reachable_arc_graph_;  // TODO: Remove weight;
  CHShortcutArcGraph* ch_shortcut_arc_graph_;

  // Query subgoal graph formers (R-connect)
  // QuerySubgoalGraph :
  // - Require different types of arcs to add
  // - Forward/bidirectional distinction
  typedef QuerySubgoalGraph<SM, R, SG> ForwardSGQueryFormer;
  ForwardSGQueryFormer* forward_sg_query_;

  typedef QuerySubgoalGraph<SM, R, ShortcutArcGraph> BidirectionalCHQueryFormer;
  BidirectionalCHQueryFormer* bidirectional_ch_query_;

  typedef QuerySubgoalGraph<SM, R, ReachableArcGraph> BidirectionalRCHQueryFormer;
  BidirectionalRCHQueryFormer* bidirectional_rch_query_;

  // Search algorithms
  typedef Dijkstra<G> GraphDijkstra;
  GraphDijkstra* g_dij_;

  typedef AStar<G, H> GraphAStar;
  GraphAStar* g_astar_;

  typedef BidirectionalDijkstra<G, G> GraphBidij;
  GraphBidij* g_bidij_;

  typedef Dijkstra<SG> SGDijkstra;
  SGDijkstra* sg_dij_;

  typedef AStar<SG, SGH> SGAStar;
  SGAStar* sg_astar_;

  typedef AStarDirectionExtended<SG, SGH> JPAStar;
  JPAStar* jp_astar_;

  typedef CHBidirectionalSearchDirectionExtended<ShortcutArcGraph, SGH> JPCHBidirectional;
  JPCHBidirectional* jp_ch_bi_astar_;

  typedef CHBidirectionalSearchDirectionExtended<ShortcutArcGraph> JPCHBidirectionalDij;
  JPCHBidirectionalDij* jp_ch_bidij_;



  typedef BidirectionalDijkstra<ReachableArcGraph> ReachableCHBidij;
  ReachableCHBidij* reachable_ch_bidij_;
  typedef BidirectionalDijkstra<ShortcutArcGraph> ShortcutCHBidij;
  ShortcutCHBidij* shortcut_ch_bidij_;

  typedef CHBidirectionalSearch<ReachableArcGraph, H> CHBiAStarReachableOct;
  CHBiAStarReachableOct* ch_bi_astar_reachable_oct_;
  typedef CHBidirectionalSearch<ReachableArcGraph, SGH> CHBiAStarReachableSGOct;
  CHBiAStarReachableSGOct* ch_bi_astar_reachable_sg_oct_;
  typedef CHBidirectionalSearch<ShortcutArcGraph, H> CHBiAStarShortcutOct;
  CHBiAStarShortcutOct* ch_bi_astar_shortcut_oct_;
  typedef CHBidirectionalSearch<ShortcutArcGraph, SGH> CHBiAstarShortcutSGOct;
  CHBiAstarShortcutSGOct* ch_bi_astar_shortcut_sg_oct_;

  // Refinement algorithms (R-refine, CH-unpack, or combinations).
  typedef SGPathRefiner<R> NodeIdSGPathRefiner;
  NodeIdSGPathRefiner* node_id_sg_path_refiner_;

  typedef SGPathRefiner<R,SM> SubgoalIdSGPathRefiner;
  SubgoalIdSGPathRefiner* subgoal_id_sg_path_refiner_;

  typedef CHPathRefiner<ShortcutArcGraph> CHShortcutArcPathRefiner;
  CHShortcutArcPathRefiner* ch_shortcut_arc_path_refiner_;  // node_id_

  typedef SequentialPathRefiner<CHShortcutArcPathRefiner, SubgoalIdSGPathRefiner> SubgoalIdCHSGPathRefiner;
  SubgoalIdCHSGPathRefiner* subgoal_id_ch_sg_path_refiner_;

  typedef CH_R_PathRefiner<R, ShortcutArcGraph> CHShortcutArcPathRMRefiner;
  CHShortcutArcPathRMRefiner* ch_shortcut_arc_path_rm_refiner_;

  // Shortcut2 versions (unpacking with pointers).
  CHShortcut2ArcGraph* ch_shortcut2_arc_graph_;
  typedef QuerySubgoalGraph<SM, R, Shortcut2ArcGraph> BidirectionalCH2QueryFormer;
  BidirectionalCH2QueryFormer* bidirectional_ch2_query_;
  typedef BidirectionalDijkstra<Shortcut2ArcGraph> Shortcut2CHBidij;
  Shortcut2CHBidij* shortcut2_ch_bidij_;

  typedef CHBidirectionalSearch<Shortcut2ArcGraph, H> CHBiAStarShortcut2Oct;
  CHBiAStarShortcut2Oct* ch_bi_astar_shortcut2_oct_;
  typedef CHBidirectionalSearch<Shortcut2ArcGraph, SGH> CHBiAstarShortcut2SGOct;
  CHBiAstarShortcut2SGOct* ch_bi_astar_shortcut2_sg_oct_;
  typedef CHBidirectionalSearchDirectionExtended<Shortcut2ArcGraph, SGH> CHBiAstarShortcut2JPOct;
  CHBiAstarShortcut2JPOct* ch_bi_astar_shortcut2_jp_oct_;

  typedef CHArcPathRefiner<Shortcut2ArcGraph> CHShortcut2ArcPathRefiner;
  CHShortcut2ArcPathRefiner* ch_shortcut2_arc_path_refiner_;  // node_id_
  typedef SequentialPathRefiner<CHShortcut2ArcPathRefiner, SubgoalIdSGPathRefiner> SubgoalIdCH2SGPathRefiner;
  SubgoalIdCH2SGPathRefiner* subgoal_id_ch2_sg_path_refiner_;
  typedef CH_R_ArcPathRefiner<R, Shortcut2ArcGraph> CHShortcut2ArcPathRMRefiner;
  CHShortcut2ArcPathRMRefiner* ch_shortcut2_arc_path_rm_refiner_;

  // Visualization
  std::vector<Arc> f_exp, f_stall, f_gen, b_exp, b_stall, b_gen;
  nodeId last_start_, last_goal_;

  CHConstructionStatistics ch_construction_stat_;

  // Preprocessing statistics.
  double preprocess_time_, contract_time_;
  double memory_, graph_memory_, r_memory_;
  long long num_nodes_, num_arcs_;
  long num_reachable_shortcuts_;

  void ConstructSGComponents();
  void ConstructCH();
  void ConstructGridSearch();
  void ConstructSubgoalSearch();

  void GenerateShortcut2ArcGraph(ShortcutArcGraph* shortcut,
                                 Shortcut2ArcGraph* shortcut2,
                                 std::vector<ShortcutArc> & shortcut_arcs);
  void ConvertCHGraphToShortcut2();

  template <class CHGraph>
  bool WriteCH(CHGraph* ch, std::string filename);

  template <class CHGraph>
  bool ReadCH(CHGraph* & ch, std::string filename);

  void CalculateNodeAndArcStatistics();

#ifndef NO_HOG
  void DrawSearchTreeEdge(const MapEnvironment* env, nodeId s, nodeId t,
                          bool forward, bool draw_search_tree_nodes,
                          recColor c0, recColor cm);
  void DrawSearchTree(const MapEnvironment* env, std::vector<Arc> & exp,
                      std::vector<Arc> & stall, std::vector<Arc> & gen,
                      bool forward, bool draw_search_tree_nodes);

  int DetermineLevelForVisualization(nodeId n);

  template<class IM>
  void DrawLevelNode(const MapEnvironment* env, IM* im, nodeId n,
                     double priority = 0);

  template<class IM>
  void DrawLevelEdge(const MapEnvironment* env, IM* im, nodeId n1, nodeId n2);

  recColor DetermineLevelColor(nodeId n, recColor color_0, recColor color_max);

  template<class Gr, class IM>
  void VisualizeGraph(const MapEnvironment* env, Gr* g, IM* im,
                      bool display_nodes, bool display_arcs,
                      int display_level);
#endif

  template<class QueryManager, class SearchAlgorithm, class Refiner>
  QueryStatistic FindPath(QueryManager* qm, SearchAlgorithm sa, Refiner r,
                           nodeId n_start, nodeId n_goal, std::vector<nodeId> & path) {
    path.clear();
    QueryStatistic s;

    CPUTimer t;
    nodeId s_start = n_start;
    nodeId s_goal = n_goal;

    // Connect
    // If a query connecter is specified, connect the start and the goal
    // to the graph (and update s_start, s_goal if necessary).
    if (qm != NULL) {
      t.StartTimer();
      s.dist = qm->AddQueryPoints(n_start, n_goal, s_start, s_goal, path);
      s.connect_time = t.EndTimer();
      if(!path.empty())
        return s;
    }

    // Search
    // Rely on the search method's statistics for time.
    if (sa == NULL) {
      std::cout << "The required SGCH search algorithm is not initialized!"
                << std::endl;
      return s;
    }
    std::vector<nodeId> high_level_path;
    high_level_path.reserve(2000);
    s.dist = sa->FindPath(s_start, s_goal, high_level_path);
    sa->AddStatistics(s);

    // Refine
    path.reserve(2000);
    if (r == NULL) {
      path.swap(high_level_path);
//      path = high_level_path;
      s.refine_time = 0;
    }
    else
      s.refine_time = r->RefinePath(high_level_path, path);

    if (qm != NULL) {
      t.StartTimer();
      qm->ClearQuery();
      s.connect_time += t.EndTimer();
    }

#ifndef NO_HOG
    last_start_ = n_start;
    last_goal_ = n_goal;
    f_exp.clear();    f_stall.clear();    f_gen.clear();
    b_exp.clear();    b_stall.clear();    b_gen.clear();
    sa->GetSearchTree(f_exp, f_stall, f_gen, b_exp, b_stall, b_gen);
#endif

    return s;
  }

  template<class QueryManager, class SearchAlgorithm, class Refiner>
  QueryStatistic FindPathJP(QueryManager* qm, SearchAlgorithm sa, Refiner r,
                           nodeId n_start, nodeId n_goal, std::vector<nodeId> & path) {
    path.clear();
    QueryStatistic s;

    CPUTimer t;
    nodeId s_start = n_start;
    nodeId s_goal = n_goal;

    // Connect
    // If a query connecter is specified, connect the start and the goal
    // to the graph (and update s_start, s_goal if necessary).

    // FIXME: Grab the tables from the search?
    AvoidanceTable* forward_avoid = sgm_->GetForwardAvoidanceTable();
    AvoidanceTable* backward_avoid = sgm_->GetBackwardAvoidanceTable();
    if (qm != NULL) {
      t.StartTimer();
      s.dist = qm->AddQueryPoints(n_start, n_goal, s_start, s_goal, path,
                                  forward_avoid, backward_avoid);
      s.connect_time = t.EndTimer();
      if(!path.empty())
        return s;
    }

    // Search
    // Rely on the search method's statistics for time.
    if (sa == NULL) {
      std::cout << "The required SGCH search algorithm is not initialized!"
                << std::endl;
      return s;
    }
    std::vector<nodeId> high_level_path;
    high_level_path.reserve(2000);

    s.dist = sa->FindPath(s_start, s_goal, high_level_path);
    sa->AddStatistics(s);

    // TODO: Not reflected in the timings
    if (forward_avoid != NULL)
      forward_avoid->ClearExceptions();
    if (backward_avoid != NULL)
      backward_avoid->ClearExceptions();

    // Refine
    path.reserve(2000);
    if (r == NULL) {
      path = high_level_path;
      s.refine_time = 0;
    }
    else
      s.refine_time = r->RefinePath(high_level_path, path);

    if (qm != NULL) {
      t.StartTimer();
      qm->ClearQuery();
      s.connect_time += t.EndTimer();
    }

#ifndef NO_HOG
    last_start_ = n_start;
    last_goal_ = n_goal;
    f_exp.clear();    f_stall.clear();    f_gen.clear();
    b_exp.clear();    b_stall.clear();    b_gen.clear();
    sa->GetSearchTree(f_exp, f_stall, f_gen, b_exp, b_stall, b_gen);
#endif

    return s;
  }
};

#include "SGCHQuery.inc"
#include "SGCHQueryMisc.inc"

#endif /* APPS_SUBGOALGRAPH_QUERYMETHODS_SGCHQUERY_H_ */
