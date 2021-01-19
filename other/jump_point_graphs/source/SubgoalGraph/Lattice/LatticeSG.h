/*
 * SubgoalGraphXYThetaLatticeManager.h
 *
 *  Created on: Feb 21, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_SUBGOALGRAPH_SGLATTICE_H_
#define APPS_SUBGOALGRAPH_SUBGOALGRAPH_SGLATTICE_H_

#include <cassert>
#include <string>

#include "../../Graphs/ArcListQueryGraph.h"
#include "../../Graphs/GraphDefinitions.h"
#include "../../Graphs/LatticeGraph/Heuristic/LatticeHeuristicManager.h"
#include "../../Graphs/LatticeGraph/Lattice/Lattice.h"
#include "../../Graphs/LatticeGraph/Lattice/LatticeDefinitions.h"
#include "../../Graphs/LatticeGraph/LatticeFreespace/LatticeFreespace.h"
#include "../../Graphs/LatticeGraph/LatticeFreespace/LatticeFreespaceHeuristic.h"
#include "../../Graphs/LatticeGraph/LatticeGraph.h"
#include "../../Parameters/LatticeParam.h"
#include "../../Parameters/ParamEnums.h"
#include "../../Parameters/RParam.h"
#include "../../Parameters/SGConstructionParam.h"
#include "../../Parameters/SGLatticeParam.h"
#include "../../ReachabilityRelation/Lattice/BoundedDistanceReachability.h"
#include "../../ReachabilityRelation/Lattice/CanonicalReachability.h"
#include "../../ReachabilityRelation/Lattice/HReachability.h"
#include "../../ReachabilityRelation/Lattice/SafeFreespaceReachability.h"
#include "../../Utils/Statistics/ConnectionStatistics.h"
#include "../../Utils/FileReadWrite.h"
#include "../../Utils/GroupingMapper.h"
#include "../../Utils/Statistics/AggregateQueryStatistics.h"
#include "../SubgoalIdMapper.h"


class LatticeSG {
 public:
  typedef SGLatticeParam ParamType;

  typedef xyThetaPos StateType;
  typedef LatticeGraph GraphType;

  typedef LatticeHeuristicManager HeuristicType;
  typedef SubgoalIdMapper SubgoalIdMapperType;

  typedef HReachability<LatticeGraph, SubgoalIdMapper, LatticeFreespaceHeuristic> FR;
  typedef SafeFreespaceReachability<LatticeGraph, SubgoalIdMapper, LatticeFreespaceHeuristic> SFR;
  typedef CanonicalReachability<LatticeGraph, SubgoalIdMapper, LatticeFreespaceHeuristic> CR;

  // TODO: Change heuristic to freespace reachability?
  typedef BoundedDistanceReachability<LatticeGraph, SubgoalIdMapper, HeuristicType> BD;

  typedef ReachabilityRelationExplore ReachabilityType;

  typedef ArcListQueryGraph<WeightedArcHead> SGGraphType;

  LatticeSG(std::string mapname, SGLatticeParam p);
  ~LatticeSG();

  bool UniqueSUB() {return false;}
  bool RSPCNameIncludesRName() {
    return true;
  }
  std::string GetGName() {
    return "L-" + p_.l.GetName();
  }
  std::string GetRName() {
    return p_.r.GetName();
  }
  std::string GetRSPCName() {
    return GetRName() + "_" + p_.s.GetRSPCConstructionName();
  }
  std::string GetSubName() {
    return GetRName() + "_" + p_.s.GetSubConstructionName();
  }
  std::string GetSearchSuffix() {
    std::stringstream ss;

    ss<<p_.r.GetConnectType();

    if (p_.h == kLatticeEuclideanHeuristic)
      ss<< "E";
    if (p_.h == kLattice2DProjectionHeuristic)
      ss<< "2D";

    return ss.str();
  }

  bool IsUndirected() {
    return lattice_graph_.IsUndirected();
  }
  LatticeGraph* GetOriginalGraph() {
    return &lattice_graph_;
  }
  HeuristicType* GetHeuristic() {
    return &search_heuristic_;
  }

  ReachabilityType* GetReachabilityRelation() {
    if (r_ == NULL)
      GenerateR();
    return r_;
  }

  ReachabilityType* GetReachabilityRelationIfExists() {
    return r_;
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
        freespace_time_ + subgoal_time_ + edge_time_
            + redundancy_elimination_time_);
    st->ReportDouble("Freespace distance computation time (s)",
                     freespace_time_);
    st->ReportDouble("Identify subgoal time (s)", subgoal_time_);
    st->ReportDouble("Identify edge time (s)", edge_time_);
    st->ReportDouble("Redundant edge elimination time (s)",
                     redundancy_elimination_time_);
    st->ReportInt("Number of connecting subgoals", num_connecting_subgoals_);

    /*
    // Reachable area size
    double avg_reachable_area_size, avg_direct_reachable_area_size;
    EstimateReachableAreaSize(avg_reachable_area_size,
                              avg_direct_reachable_area_size, 1000);
    st->ReportDouble("Estimate average reachable area size",
                      avg_reachable_area_size);
    st->ReportDouble(
        "Estimate average ratio of reachable area size to graph",
        avg_reachable_area_size / (double) g_lattice_->GetNumValidNodes());
    st->ReportDouble("Estimate average direct reachable area size",
                      avg_direct_reachable_area_size);
    st->ReportDouble(
        "Estimate average ratio of direct reachable area size to graph",
        avg_direct_reachable_area_size / (double) g_lattice_->GetNumValidNodes());
    st->AddRemark("");
    */

  }

  double GetFreespaceTimeInSeconds() {
    return freespace_time_;
  }

  void ReportMemoryStatistics(StatisticsFile* st, bool connect, bool refine) {
    bool fewer_tables = true;
    bool smaller_tables = true;
    if (freespace_manager_ != NULL) {
      st->ReportDouble(
          "Memory for freespace information default (Mb)",
          freespace_manager_->EstimateStorageMB(connect, refine, !fewer_tables,
                                                !smaller_tables));
      st->ReportDouble(
          "Memory for freespace information fewer tables (Mb)",
          freespace_manager_->EstimateStorageMB(connect, refine, fewer_tables,
                                                !smaller_tables));
      st->ReportDouble(
          "Memory for freespace information smaller tables (Mb)",
          freespace_manager_->EstimateStorageMB(connect, refine, !fewer_tables,
                                                smaller_tables));
      st->ReportDouble(
          "Memory for freespace information fewer and smaller tables (Mb)",
          freespace_manager_->EstimateStorageMB(connect, refine, fewer_tables,
                                                smaller_tables));
    }
    else {
      st->ReportDouble("Memory for freespace information default (Mb)", 0);
      st->ReportDouble("Memory for freespace information fewer tables (Mb)", 0);
      st->ReportDouble("Memory for freespace information smaller tables (Mb)",
                       0);
      st->ReportDouble(
          "Memory for freespace information fewer and smaller tables (Mb)", 0);

    }
  }

  double GetPreprocessingTime() {
    return freespace_time_ + subgoal_time_ + edge_time_
        + redundancy_elimination_time_;
  }

  double GetRMemoryMB(bool connect, bool refine) {
    double mem_freespace = 0;
    if (freespace_manager_ != NULL)
      mem_freespace = freespace_manager_->EstimateStorageMB(connect, refine);
    return mem_freespace;
  }

  double GetBytesPerEdge() {
    if (freespace_manager_ != NULL
        && freespace_manager_->UsingDistances()) {
      return 4;
    }
    else
      return 8;
  }

  void WriteR(FileReadWrite & rw) {
    assert(r_ != NULL);
    //freespace_manager_->Write(rw);
    rw.Write(freespace_time_);
  }

  void WriteRSPC(FileReadWrite & rw) {
    assert(sm_ != NULL);
    sm_->Write(rw);
    rw.Write(subgoal_time_);
    rw.Write(num_connecting_subgoals_);
  }

  void WriteSub(FileReadWrite & rw) {
    assert(sg_ != NULL);
    rw.WriteGraph(sg_);
    rw.Write(edge_time_);
    rw.Write(redundancy_elimination_time_);
  }

  void ReadR(FileReadWrite & rw) {
    assert(r_ == NULL);
    GenerateR();
    /*
    freespace_manager_ = new LatticeFreespace(&lattice_graph_, p_.r, rw);
    freespace_heuristic_ = new LatticeFreespaceHeuristic(&lattice_graph_,
                                                         freespace_manager_);
    sm_ = new SubgoalIdMapper(lattice_graph_.GetNumAllNodes());
    if (p_.r.type == kSafeFreespaceReachability)
      fr_ = new SFR(&lattice_graph_, sm_, freespace_heuristic_, p_.r.bound);
    else if (p_.r.type == kFreespaceReachability)
      fr_ = new FR(&lattice_graph_, sm_, freespace_heuristic_, p_.r.bound);
    else if (p_.r.type == kCanonicalFreespaceReachability)
      fr_ = new CR(&lattice_graph_, sm_, freespace_heuristic_, p_.r.bound);
    else if (p_.r.type == kBoundedDistanceReachability)
      fr_ = new BD(&lattice_graph_, sm_, &search_heuristic_, p_.r.bound);
    */
    rw.Read(freespace_time_);
  }
  void ReadRSPC(FileReadWrite & rw) {
    if (sm_ == NULL)
      sm_ = new SubgoalIdMapper(lattice_graph_.GetNumAllNodes());
    sm_->Read(rw);
    rw.Read(subgoal_time_);
    rw.Read(num_connecting_subgoals_);
    subgoals_generated_ = true;
  }
  void ReadSub(FileReadWrite & rw) {
    assert(sg_ == NULL);
    sg_ = new ArcListQueryGraph<WeightedArcHead>();
    rw.ReadGraph(sg_);
    rw.Read(edge_time_);
    rw.Read(redundancy_elimination_time_);
  }

  LatticeFreespace* GetFreespaceManager() {
    return freespace_manager_;
  }

  // Bad coding... Grid specific functions.
  bool UseJPSearch() {return false;}
  AvoidanceTable* GetForwardAvoidanceTable() {return NULL;}
  AvoidanceTable* GetBackwardAvoidanceTable() {return NULL;}
  GroupingMapper* GetGroupingMapper() {return NULL;}
  bool AvoidRedundantEdgesDuringContraction() {return false;}

 private:
  SGLatticeParam p_;

  Lattice lattice_;
  LatticeGraph lattice_graph_;
  HeuristicType search_heuristic_;

  LatticeFreespace* freespace_manager_;
  LatticeFreespaceHeuristic* freespace_heuristic_;

  ReachabilityType* r_;
  SubgoalIdMapper* sm_;
  ArcListQueryGraph<WeightedArcHead>* sg_;

  bool subgoals_generated_;
  double freespace_time_, subgoal_time_, edge_time_,
      redundancy_elimination_time_;

  int num_connecting_subgoals_;

  void GenerateR();
  void GenerateSubgoals();
  void GenerateSubgoalGraph();

  void VerifyUndirectedSG();
  void VerifySingleConnectedComponentSG();

  AggregateConnectionStatistics ConnectionTest(int trials);


  //  void EstimateReachableAreaSize(double & num_reachable,
//                                 double & num_direct_reachable,
//                                 int trials = 1000);
};

#endif /* APPS_SUBGOALGRAPH_SUBGOALGRAPH_SGLATTICE_H_ */
