/*
 * SGLattice.cpp
 *
 *  Created on: Oct 18, 2017
 *      Author: idm-lab
 */

#include <GraphConnectedComponentAnalyzer.h>
#include <GraphUtils.h>
#include <LatticeSG.h>
#include <ReachabilityRelation.h>
#include <SGArcConstructor.h>
#include <SubgoalGenerator.h>
#include <iostream>

LatticeSG::LatticeSG(std::string mapname, SGLatticeParam p)
    : p_(p),
      lattice_(mapname, p.l),
      lattice_graph_(&lattice_),
      search_heuristic_(&lattice_graph_, p.h)
{
  freespace_manager_ = NULL;
  freespace_heuristic_ = NULL;

  sg_ = NULL;
  r_ = NULL;
  sm_ = NULL;

  subgoals_generated_ = false;

  freespace_time_ = 0;
  subgoal_time_ = 0;
  edge_time_ = 0;
  redundancy_elimination_time_ = 0;
  num_connecting_subgoals_ = 0;
}
LatticeSG::~LatticeSG() {
  if (freespace_manager_)
    delete freespace_manager_;
  if (freespace_heuristic_)
    delete freespace_heuristic_;

  if (sm_)
    delete sm_;
  if (sg_)
    delete sg_;
  if (r_)
    delete r_;
}

void LatticeSG::GenerateR() {
  freespace_manager_ = new LatticeFreespace(&lattice_graph_, p_.r);
  freespace_heuristic_ = new LatticeFreespaceHeuristic(&lattice_graph_,
                                                       freespace_manager_);

  sm_ = new SubgoalIdMapper(lattice_graph_.GetNumAllNodes());
  if (p_.r.type == kBoundedDistanceReachability)
    r_ = new BD(&lattice_graph_, sm_, &search_heuristic_, p_.r.bound, p_.r.conn);
  else if (p_.r.type == kFreespaceReachability)
    r_ = new FR(&lattice_graph_, sm_, freespace_heuristic_, p_.r.bound, p_.r.conn);
  else if (p_.r.type == kCanonicalFreespaceReachability)
    r_ = new CR(&lattice_graph_, sm_, freespace_heuristic_, p_.r.bound);
  else if (p_.r.type == kSafeFreespaceReachability)
    r_ = new SFR(&lattice_graph_, sm_, freespace_heuristic_, p_.r.bound);

  freespace_time_ = freespace_manager_->GetComputationTime();
}

void LatticeSG::GenerateSubgoals() {
  if (r_ == NULL)
    GenerateR();
  SubgoalGenerator<SubgoalIdMapper, LatticeGraph, ReachabilityType> subgoal_generator(
      sm_, &lattice_graph_, (ReachabilityType*) r_, p_.s);
  subgoal_time_ = subgoal_generator.IdentifySubgoals();
  num_connecting_subgoals_ = subgoal_generator.GetNumConnectingSubgoals();
  subgoals_generated_ = true;
}

void LatticeSG::GenerateSubgoalGraph() {
  if (subgoals_generated_ == false)
    GenerateSubgoals();
  SGArcConstructor arc_constructor_;
  sg_ = new ArcListQueryGraph<WeightedArcHead>();

  bool superset_then_prune = false;
  if (p_.s.construction_type == kConnectedRSPC
      || p_.s.construction_type == kRandomRSPC) {
    if(!r_->CanConstructSCSGEdgesExactly())
        superset_then_prune = true;
  }
  else if (!r_->CanConstructSGEdgesExactly())
    superset_then_prune = true;

  if (superset_then_prune)
    arc_constructor_.ConstructWeightedArcGraph_RReachableThenPrune(
        sm_, sg_, r_, p_.s.spanner_suboptimality);
  else
    arc_constructor_.ConstructWeightedArcGraph_DirectRReachable(
        sm_, sg_, r_, p_.s.spanner_suboptimality);

  edge_time_ = arc_constructor_.GetEdgeTime();
  redundancy_elimination_time_ =
      arc_constructor_.GetRedundancyEliminationTime();

  // Verification: If G is undirected, SG must be undirected as well.
  if (p_.s.verify_undirected && IsUndirected())
    VerifyUndirectedSG();

  // Verification: Assuming that G is strongly connected, SG should be strongly
  // connected as well.
  if (true) {
    VerifySingleConnectedComponentSG();
  }
}

void LatticeSG::VerifyUndirectedSG() {
  UndirectedGraphCheck undirected_check;
  bool undirected_lattice = undirected_check.IsUndirected(&lattice_graph_);
  bool undirected_sg = undirected_check.IsUndirected(sg_);

  std::cout << "Lattice is ";
  if (undirected_lattice)
    std::cout<<"undirected"<<std::endl;
  else
    std::cout<<"directed"<<std::endl;

  std::cout << "SG is ";
  if (undirected_sg)
    std::cout<<"undirected"<<std::endl;
  else
    std::cout<<"directed"<<std::endl;

  assert(undirected_lattice && undirected_sg);
}

void LatticeSG::VerifySingleConnectedComponentSG() {
  DirectedGraph<ArcListQueryGraph<WeightedArcHead>> d(sg_);
  GraphConnectedComponentAnalyzer<DirectedGraph<ArcListQueryGraph<WeightedArcHead>>> analyzer(
      &d);
  if (analyzer.GetNumConnectedComponents() == 1)
    std::cout<<"ONLY ONE CONNECTED COMPONENT!"<<std::endl;
  else
    analyzer.ListSmallerConnectedCompmonents();
}

/*
void LatticeSG::EstimateReachableAreaSize(double & num_reachable,
                                          double & num_direct_reachable,
                                          int trials) {
  std::vector<nodeId> order;
  for (nodeId n = 0; n < g_lattice_->GetNumAllNodes(); n++) {
    if (g_lattice_->IsValidNode(n))
      order.push_back(n);
  }
  srand(1337);
  std::random_shuffle(order.begin(), order.end());

  if (trials > order.size())
    trials = order.size();

  int total_num_reachable = 0;
  int total_num_direct_reachable = 0;
  for (unsigned int i = 0; i < trials; i++) {
    int r, dr;
    explorer_->GetReachableAreaSize(order[i], r, dr);
    total_num_reachable += r;
    total_num_direct_reachable += dr;
  }

  num_reachable = total_num_reachable / (double) trials;
  num_direct_reachable = total_num_direct_reachable / (double) trials;
}
*/
