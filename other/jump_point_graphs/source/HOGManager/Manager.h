#ifndef MANAGER_H
#define MANAGER_H

#include <iostream>
#include <string>
#include <vector>

#include "../../environments/Map2DEnvironment.h"
#include "../SubgoalGraph/Grid2D/Grid2DSG.h"
#include "Graphs/GraphDefinitions.h"
#include "Graphs/LatticeGraph/Lattice/LatticeDefinitions.h"
#include "Parameters/CHParam.h"
#include "QueryMethods/SGCHQuery.h"
#include "ReachabilityRelation/ReachabilityRelation.h"
#include "SearchMethods/Dijkstra.h"
#include "SubgoalGraph/Lattice/LatticeSG.h"
#include "Utils/Statistics/ConnectionStatistics.h"
#include "Utils/Statistics/AggregateQueryStatistics.h"
#include "Utils/Experiments/InstanceManager.h"
#include "Utils/Experiments/SGExperiment.h"

#ifdef INCLUDE_GPPC_CH
#include "../CH_GPPC/CHQueryGPPC.h"
#endif


enum PathDisplayType {
  kDontDisplayPath,
  kDisplayPath,
  kDisplayPathWithNodes,
  kDisplayAllPaths,
  kDisplayAllPathsWithNodes,
  kDisplaySGPath,
  kDisplayBothPaths
};
enum SGDisplayType {
  kDontDisplayGraph,
  kDisplayGraphNodes,
  kDisplaySGNodes,
  kDisplaySGEdges,
  kDisplaySGNodesEdges
};

enum SearchTreeDisplayType {
  kDontDisplaySearchTree,
  kDisplayForwardSearchTree,
  kDisplayBackwardSearchTree,
  kDisplayBidirectionalSearchTree
};

// Interface with HOG.
// Works either in the lattice mode or the grid mode.
// Can process keyboard / mouse commands from the GUI.
// See the command handlers in sample.cpp

class ManagerBase {
 public:
  ManagerBase() {}

  virtual void Initialize(std::string mapname) = 0;
  virtual void ProcessKeyboardCommand(char key, bool shift, bool ctrl) = 0;
  virtual const char* GetScreenshotName() {return "";}
  virtual void Visualize(const MapEnvironment *env) = 0;
  virtual void SinglePointQuery(xyLoc l) = 0;
  virtual void DoublePointQuery(xyLoc l1, xyLoc l2) = 0;

  virtual std::string GetName() = 0;
};

template <class SGM>
class Manager : public ManagerBase {
  typedef SGCHQuery<SGM> QueryMethod;
  typedef typename SGM::GraphType Graph;
  typedef typename SGM::StateType State;
  typedef InstanceManager<State,Graph> InstanceGenerator;
  typedef typename SGM::ParamType SGParam;

 public:
  Manager();
  ~Manager();

  void Initialize(std::string mapname);

  void ProcessKeyboardCommand(char key, bool shift, bool ctrl);
  const char* GetScreenshotName() {return "";}
  void Visualize(const MapEnvironment *env);

  void DoublePointQuery(); // Runs a query with the current start/goal positions.
  void Preprocess();
  void Experiment();

  void SinglePointQuery();  // With the start point.
  void SinglePointQuery(xyLoc l);
  void DoublePointQuery(xyLoc l1, xyLoc l2);

  std::string GetName() {return "Manager";}

  void ExploreRReachableArea();


  void PrintSearchStatistic(std::string name, QueryStatistic statistics,
                            std::ostream & out = std::cout);
  void PrintSearchStatistic(int method_id, std::ostream & out = std::cout) {
    PrintSearchStatistic(method_names_[method_id], statistics_[method_id], out);
  }

  void PrintConnectionStatistic(std::string name, AggregateConnectionStatistics statistics,
                            std::ostream & out = std::cout);
  void PrintConnectionStatistic(int method_id, std::ostream & out = std::cout) {
    PrintConnectionStatistic(
        method_names_[method_id],
        methods_[method_id]->GetAggregateConnectionStatistics(), out);
  }

 private:
  bool is_preprocess_complete_;
  DirectoryParam dp_;

  Graph* g_graph_;
  InstanceGenerator* g_ins_man_;

  std::vector<CHParam> c;
  std::vector<SGParam> s;

  std::vector<QueryStatistic> statistics_;
  std::vector<std::vector<State> > grid_paths_;

  std::vector<QueryMethod*> methods_;
  std::vector<std::string> method_names_;
  std::vector<std::string> override_method_names_;  // FIXME
  std::vector<SGExperiment<QueryMethod,InstanceGenerator>* > experiments_;

  int method_name_length_;

  int method_id_;
  std::vector<int> display_levels_;

  // For displaying all, symmetric, shortest paths.
  Dijkstra<Graph>* dij_;
  std::vector<nodeId> sp_arc_from_;
  std::vector<nodeId> sp_arc_to_;

  // Manage query points through the HOG interface.
  xyLoc xy_start, xy_goal;
  int t_start, t_goal;
  int num_angles_;
  xyThetaPos xyt_start, xyt_goal;
  xyThetaPos GetXYThetaPos(xyLoc l, int disc_orientation) {
    return xyThetaPos(l.x, l.y, disc_orientation);
  }
  void UpdateXYThetaPoses() {
    xyt_start = GetXYThetaPos(xy_start, t_start);
    xyt_goal = GetXYThetaPos(xy_goal, t_goal);
  }
  void SetStartXY(xyLoc l) {
    xy_start = l;
    UpdateXYThetaPoses();
  }
  void SetGoalXY(xyLoc l) {
    xy_goal = l;
    UpdateXYThetaPoses();
  }
  void SetStartOrientation(int i) {
    t_start = i;
    UpdateXYThetaPoses();
  }
  void SetGoalOrientation(int i) {
    t_goal = i;
    UpdateXYThetaPoses();
  }

  void SetStart(xyLoc l) {
   SetStartXY(l);
  }
  void SetGoal(xyLoc l) {
   SetGoalXY(l);
  }
  void SetStart(xyThetaPos p) {
    SetStartXY(xyLoc(p.x, p.y));
    SetStartOrientation(p.o);
  }
  void SetGoal(xyThetaPos p) {
    SetGoalXY(xyLoc(p.x, p.y));
    SetGoalOrientation(p.o);
  }


  // Visualization
  bool draw_search_tree_nodes_;

  bool explore_backward_;

  HRAreaDisplayType r_reachable_area_display_type_;
  std::vector<HRAreaDisplayType> r_reachable_area_display_types_;
  std::vector<std::string> r_reachable_area_display_type_messages_;

  PathDisplayType path_display_type_;
  std::vector<PathDisplayType> path_display_types_;
  std::vector<std::string> path_display_type_messages_;

  SGDisplayType sg_display_type_;
  std::vector<SGDisplayType> sg_display_types_;
  std::vector<std::string> sg_display_type_messages_;

  SearchTreeDisplayType st_display_type_;
  std::vector<SearchTreeDisplayType> st_display_types_;
  std::vector<std::string> st_display_type_messages_;

  void SetupVisuals();
  void AddRReachableAreaDisplayType(HRAreaDisplayType t, std::string text) {
    r_reachable_area_display_types_.push_back(t);
    r_reachable_area_display_type_messages_.push_back(text);
  }
  void AddPathDisplayType(PathDisplayType t, std::string text) {
    path_display_types_.push_back(t);
    path_display_type_messages_.push_back(text);
  }
  void AddSubgoalGraphDisplayType(SGDisplayType t, std::string text) {
      sg_display_types_.push_back(t);
      sg_display_type_messages_.push_back(text);
  }
  void AddSearchTreeDisplayType(SearchTreeDisplayType t, std::string text) {
      st_display_types_.push_back(t);
      st_display_type_messages_.push_back(text);
  }

  void VisualizePath(const MapEnvironment *env);
  void VisualizeRReachableArea(const MapEnvironment *env);
  void VisualizeSubgoalGraph(const MapEnvironment *env);
  void VisualizeSearchTree(const MapEnvironment *env);

  template<class GraphToDisplay, class IdMapper, class Visualizer>
  void VisualizeGraph(const MapEnvironment *env, GraphToDisplay* g, IdMapper* m,
                      Visualizer* v, bool display_nodes, bool display_edges);

  template<class S, class G>
  void DrawPath(const MapEnvironment *env, G* g, std::vector<S> path) {
    for (unsigned int i = 1; i < path.size(); i++) {
      g->DrawEdge(env, path[i - 1], path[i]);
    }
  }

  State GetStart();
  State GetGoal();

  // Template specific functions.
  void InitializeMethods() {}   // Fill the parameter vectors c and s.
  void InitializeNumAngles() {num_angles_ = 1;} // Number of different orientations.
  void ExploreRReachableAreaSpecific() {}
  void DoublePointQuerySpecific() {}
  void SinglePointQueryGraphInfo() {}
  void SetupVisualsDomainSpecific() {}

  void DrawStartNode(const MapEnvironment *env) {}
  void DrawGoalNode(const MapEnvironment *env) {}

#ifdef INCLUDE_GPPC_CH
  typedef SGExperiment<ch_gppc::CHQueryGPPC, InstanceGenerator> GppcCHExperiment;
  ch_gppc::CHQueryGPPC* gppc_ch_;
  GppcCHExperiment* gppc_ch_exp_;
#endif
};


#include "Manager.inc"
#include "ManagerDisplay.inc"
#include "ManagerGrid.inc"
#include "ManagerLattice.inc"

#endif
