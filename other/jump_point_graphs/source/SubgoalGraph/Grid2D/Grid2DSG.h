/*
 * Grid2DSG.h
 *
 *  Created on: Feb 5, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_SUBGOALGRAPH_GRID2D_GRID2DSG_H_
#define APPS_SUBGOALGRAPH_SUBGOALGRAPH_GRID2D_GRID2DSG_H_

#include <cassert>
#include <string>

#include "../../Parameters/SGGrid2DParam.h"
#ifndef NO_HOG
#include "../../../../environments/Map2DEnvironment.h"
#endif

#include "../../Graphs/ArcListQueryGraph.h"
#include "../../Graphs/CommonHeuristics.h"
#include "../../Graphs/GraphDefinitions.h"
#include "../../Graphs/Grid2D/Grid2D.h"
#include "../../Graphs/Grid2D/Grid2DGraph.h"
#include "../../ReachabilityRelation/Grid2D/CanonicalReachabilityGrid2D.h"
#include "../../ReachabilityRelation/Grid2D/DiagonalFirstReachability.h"
#include "../../ReachabilityRelation/Grid2D/DiagonalFirstReachabilityAllCorners.h"
#include "../../ReachabilityRelation/Grid2D/DiagonalFirstReachabilityDiagonalCorners.h"
#include "../../ReachabilityRelation/Grid2D/DiagonalFirstReachabilityMergedDiagonalCorners.h"
#include "../../ReachabilityRelation/Grid2D/DiagonalFirstReachabilityStraightJP.h"
#include "../../ReachabilityRelation/Grid2D/FreespaceReachabilityGrid2D.h"
#include "../../Utils/AvoidanceTable.h"
#include "../../Utils/FileReadWrite.h"
#include "../../Utils/GroupingMapper.h"
#include "../../Utils/Statistics/StatisticsFile.h"
#include "../Construction/SGArcConstructor.h"
#include "../SubgoalIdMapper.h"
#include "Grid2DClearanceManager.h"

// TODO: Should Subgoal graph be responsible for the original graph as well?

// TODO: Save/load avoidance tables? Currently computing them from the SG.
// TODO: Same for clearances.
// TODO: Somehow distinguish between different searches wrt using the JP search
// when naming algorithms.

// TODO: Clearances not yet fully detached from IFDEFs

class Grid2DSG {
 public:
  typedef SGGrid2DParam ParamType; // TODO: Change Param type

  typedef xyLoc StateType;
  typedef GPPCGrid GridType;
  typedef Grid2DGraph GraphType;
  typedef OctileDistanceHeuristic<GraphType> HeuristicType; // TODOHeuristicType

  typedef SubgoalIdMapper SubgoalIdMapperType;
  typedef GridClearanceReachabilityRelation<GridType,SubgoalIdMapperType> ReachabilityType;
  typedef ArcListQueryGraph<WeightedArcHead> SGGraphType;

  Grid2DSG(std::string mapname, ParamType param)
      : grid_(mapname),
        grid_graph_(
            &grid_,
            param.IsDirectionExtended()),
        octile_heuristic_(&grid_graph_),
        param_(param) {

    subgoals_generated_ = false;
    or_ = NULL;
    sm_ = NULL;
    sg_ = NULL;

    gm_= NULL;
    forward_avoidance_table_ = NULL;
    backward_avoidance_table_ = NULL;

    subgoal_time_ = 0;
    clearance_time_ = 0;
    edge_time_ = 0;
    redundancy_elimination_time_ = 0;
  }
  ~Grid2DSG() {
    if (sm_)
      delete sm_;
    if (or_)
      delete or_;
    if (sg_)
      delete sg_;
    if (gm_)
      delete gm_;
    if (forward_avoidance_table_)
      delete forward_avoidance_table_;
    if (backward_avoidance_table_)
      delete backward_avoidance_table_;
  }

  bool UniqueSUB() {
    return false;
  }
  bool RSPCNameIncludesRName() {
    return false;
  }

  std::string GetGName() {
    return "Grid";
  }
  std::string GetRName() {
    return param_.GetRName();
  }
  std::string GetRSPCName () {
    return param_.GetRSPCName();
  }
  std::string GetSubName() {
    return param_.GetSubName();
  }
  std::string GetSearchSuffix() {
    if (param_.double_clearance)
      return "dC";
    else
      return "";
  }

  bool IsUndirected() {
    return param_.IsUndirected();
  }
  bool IsDirectionExtended() {
    return param_.IsDirectionExtended();
  }

  bool UseJPSearch() {
    return param_.use_jp_search;
  }
  bool UseAvoidanceTable() {
    return param_.use_avoidance_table;
  }
  bool AvoidRedundantEdgesDuringContraction () {
    return param_.avoid_redundant_edges_during_contraction;
  }
  AvoidanceTable* GetForwardAvoidanceTable() {
    return forward_avoidance_table_;
  }
  AvoidanceTable* GetBackwardAvoidanceTable() {
    return backward_avoidance_table_;
  }

  Grid2DGraph* GetOriginalGraph() {
    return &grid_graph_;
  }
  OctileDistanceHeuristic<GraphType>* GetHeuristic() {
    return &octile_heuristic_;
  }

  ReachabilityType* GetReachabilityRelation() {
    if (or_ == NULL)
      GenerateR();
    return or_;
  }
  ReachabilityType* GetReachabilityRelationIfExists() {
    return or_;
  }

  SubgoalIdMapper* GetSubgoalIdMapper() {
    if (subgoals_generated_ == false)
      GenerateSubgoals();
    return sm_;
  }
  ArcListQueryGraph<WeightedArcHead>* GetSubgoalGraph() {
    if (sg_ == NULL)
      GenerateSubgoalGraph();
    return sg_;
  }
  void ReportPreprocessStatistics(StatisticsFile* st) {
    st->ReportDouble(
        "Subgoal graph construction time (s)",
        clearance_time_ + subgoal_time_ + edge_time_
            + redundancy_elimination_time_);
    st->ReportDouble("Identify subgoal time (s)", subgoal_time_);
    st->ReportDouble("Clearance computation time (s)", clearance_time_);
    st->ReportDouble("Identify edge time (s)", edge_time_);
    st->ReportDouble("Redundant edge elimination time (s)",
                     redundancy_elimination_time_);
  }

  void ReportMemoryStatistics(StatisticsFile* st, bool connect, bool refine) {
    double mem_freespace = 0;
    if (connect)
      mem_freespace = or_->EstimateStorageMB();

    st->ReportDouble("Freespace memory (MB)", mem_freespace);
  }

  double GetPreprocessingTime() {
    return clearance_time_ + subgoal_time_ + edge_time_
        + redundancy_elimination_time_;
  }

  double GetRMemoryMB(bool connect, bool refine) {
    double m = 0;
    if (connect)
      m += or_->EstimateStorageMB();
    return m;
  }

  double GetBytesPerEdge() {
    return 4;
  }


  void Write(FileReadWrite & rw) {
    assert(or_ != NULL && sm_ != NULL && sg_ != NULL);
    sm_->Write(rw);
    rw.WriteGraph(sg_);
    rw.Write(subgoal_time_);
    rw.Write(clearance_time_);
    rw.Write(edge_time_);
  }

  void WriteR(FileReadWrite & rw) {
    assert(or_ != NULL);
    rw.Write(clearance_time_);
  }
  void WriteRSPC(FileReadWrite & rw) {
    assert(sm_ != NULL);
    sm_->Write(rw);
    rw.Write(subgoal_time_);
  }
  void WriteSub(FileReadWrite & rw) {
    assert(sg_ != NULL);
    rw.WriteGraph(sg_);
    rw.Write(edge_time_);
  }

  void ReadR(FileReadWrite & rw) {
    assert(or_ == NULL);
    GenerateR();  // Not saved.
    rw.Read(clearance_time_);
  }
  void ReadRSPC(FileReadWrite & rw) {
    if (sm_ == NULL)
      sm_ = new SubgoalIdMapper(grid_.GetNumPaddedCells());
    sm_->Read(rw);
    rw.Read(subgoal_time_);
    subgoals_generated_ = true;
  }
  void ReadSub(FileReadWrite & rw) {
    assert(sg_ == NULL);
    sg_ = new ArcListQueryGraph<WeightedArcHead>();
    rw.ReadGraph(sg_);
    rw.Read(edge_time_);
    clearance_time_ = or_->InitializeGivenSubgoals();
    if (param_.use_avoidance_table) { // previously: UseJPSearch()
      forward_avoidance_table_ = new AvoidanceTable(sg_, true);
      backward_avoidance_table_ = new AvoidanceTable(sg_, false);
    }
  }

  void PrintNodeInfo(xyLoc l, Direction2D d = dAll) {
    if (or_ != NULL)
      or_->PrintNodeInfo(grid_graph_.ToNodeId(l));
  }

  // FIXME: Not included in preprocessing time.
  GroupingMapper* GetGroupingMapper() {
    if (gm_ == NULL) {
      gm_ = new GroupingMapper();
      int num_subgoals = sm_->GetNumSubgoals();
      gm_->SetDomain(num_subgoals + 2);

      std::vector<std::vector<subgoalId> > groups(grid_.GetNumPaddedCells(),
                                                  std::vector<subgoalId>(0));
      for (subgoalId s = 0; s < num_subgoals; s++) {
        groups[grid_.ExtractXYLin(sm_->ToNodeId(s))].push_back(s);
      }
      int num_groups = 0;
      int num_groups_with_2_elements = 0;
      for (xyLin l = 0; l < groups.size(); l++) {
        gm_->AddGroup(groups[l]);
        if (groups[l].size() > 0)
          num_groups++;
        if (groups[l].size() > 1)
          num_groups_with_2_elements++;
      }
      /*
      std::cout << "Subgoals: " << num_subgoals << "\tGroups: " << num_groups
                << "\tGroups of 2:" << num_groups_with_2_elements << std::endl;

      std::cout << "Mapping: " << gm_->GetDomainSize() << "\t"
                << gm_->GetNumGroups() << std::endl;
      */

      // Groups for query nodes.
      std::vector<subgoalId> temp(1);
      temp[0] = num_subgoals;
      gm_->AddGroup(temp);
      temp[0] = num_subgoals + 1;
      gm_->AddGroup(temp);
    }
    return gm_;
  }

  GroupingMapper* GetGroupingMapperBaseGraph() {
    GroupingMapper* gm = new GroupingMapper();
    int num_nodes = grid_graph_.GetNumAllNodes();
    int num_groups = grid_.GetNumPaddedCells();
    gm->SetDomain(num_nodes);
    std::vector<std::vector<subgoalId> > groups(num_groups,
                                                std::vector<subgoalId>(0));
    for (nodeId n = 0; n < num_nodes; n++) {
      groups[grid_.ExtractXYLin(n)].push_back(n);
    }
    for (xyLin l = 0; l < groups.size(); l++) {
      gm->AddGroup(groups[l]);
    }
    return gm;
  }

 private:
  GPPCGrid grid_;
  Grid2DGraph grid_graph_;
  OctileDistanceHeuristic<GraphType> octile_heuristic_;

  ReachabilityType* or_;
  SubgoalIdMapper* sm_;
  ArcListQueryGraph<WeightedArcHead>* sg_;
  AvoidanceTable* forward_avoidance_table_;
  AvoidanceTable* backward_avoidance_table_;

  GroupingMapper* gm_;

  bool subgoals_generated_;

  SGGrid2DParam param_;

  double subgoal_time_, clearance_time_, edge_time_,
      redundancy_elimination_time_;

  void GenerateR() {
    sm_ = new SubgoalIdMapper(grid_graph_.GetNumAllNodes());
    if (param_.sg_type == kGrid2DSGFreespace)
      or_ = new FreespaceReachabilityGrid2D<GridType>(&grid_, sm_,
                                                      param_.double_clearance);
    if (param_.sg_type == kGrid2DSGCanonical)
      or_ = new CanonicalReachabilityGrid2D<GridType>(&grid_, sm_,
                                                      param_.double_clearance);
    if (param_.sg_type == kGrid2DCSG)
      or_ = new DiagonalFirstReachabilityAllCorners<GridType>(
          &grid_, sm_, param_.double_clearance);
    if (param_.sg_type == kGrid2DJPDiagonalCorners)
      or_ = new DiagonalFirstReachabilityDiagonalCorners<GridType>(
          &grid_, sm_, param_.double_clearance);
    if (param_.sg_type == kGrid2DJPDiagonalCornersMerged)
      or_ = new DiagonalFirstReachabilityMergedDiagonalCorners<GridType>(
          &grid_, sm_, param_.double_clearance);
    if (param_.sg_type == kGrid2DJP)
      or_ = new DiagonalFirstReachabilityStraightJP<GridType>(
          &grid_, sm_, param_.double_clearance);
  }

  void GenerateSubgoals() {
    if (or_ == NULL)
      GenerateR();

    subgoal_time_ = or_->IdentifySubgoals();
#ifndef SG_QUIET
    std::cout << sm_->GetNumSubgoals() << " subgoals identified in "
              << subgoal_time_ << " seconds." << std::endl;
#endif
    subgoals_generated_ = true;
  }

  void GenerateSubgoalGraph() {
    if (subgoals_generated_ == false)
      GenerateSubgoals();
    clearance_time_ = or_->InitializeGivenSubgoals();
    SGArcConstructor arc_constructor_;
    sg_ = new ArcListQueryGraph<WeightedArcHead>();
    arc_constructor_.ConstructWeightedArcGraph_DirectRReachable(
        sm_, sg_, or_, 1.0, param_.eliminate_redundant_arcs);
    edge_time_ = arc_constructor_.GetEdgeTime();
    redundancy_elimination_time_ =
        arc_constructor_.GetRedundancyEliminationTime();

    if (param_.use_avoidance_table) {
      forward_avoidance_table_ = new AvoidanceTable(sg_, true);
      backward_avoidance_table_ = new AvoidanceTable(sg_, false);
    }
  }
};

#endif /* APPS_SUBGOALGRAPH_SUBGOALGRAPH_GRID2D_GRID2DSG_H_ */
