/*
 * RExploreExperiment.h
 *
 *  Created on: Jan 1, 2019
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_UTILS_REXPLOREEXPERIMENT_H_
#define APPS_SUBGOALGRAPH_UTILS_REXPLOREEXPERIMENT_H_
#include "../GraphNodeOrderGenerator.h"
#include "../Parameters/DirectoryParam.h"
#include "../Parameters/LatticeParam.h"
#include "../Parameters/RParam.h"
#include "../Statistics/ExplorationStatistics.h"
#include "../Statistics/StatisticsUtil.h"

template<class SGM>
class RExploreExperiment {

  typedef typename SGM::StateType State;

 public:
  RExploreExperiment(SGM* sgm) {
    sgm_ = sgm;
    num_nodes_ = 0;
  }
  ~RExploreExperiment() {}

  void Experiment(int num_instances) {
    if (sgm_->GetReachabilityRelationIfExists() == NULL)
      return;

    srand(0);
    std::vector<nodeId> order;
    GenerateRandomOrder(sgm_->GetOriginalGraph(), order);
    num_nodes_ = order.size();

    if (num_instances > order.size())
      num_instances = order.size();

    sgm_->GetReachabilityRelation()->Reset();
    for (int i = 0; i < num_instances; i++)
      EstimateDRRSize(order[i], true, forward_drr_);

    sgm_->GetReachabilityRelation()->Reset();
    for (int i = 0; i < num_instances; i++)
      EstimateDRRSize(order[i], false, backward_drr_);

    sgm_->GetReachabilityRelation()->Reset();
    for (int i = 0; i < num_instances; i++)
      CalculateRConnectStatistic(order[i], true, aes_forward_);

    sgm_->GetReachabilityRelation()->Reset();
    for (int i = 0; i < num_instances; i++)
      CalculateRConnectStatistic(order[i], false, aes_backward_);
  }

  void Report(StatisticsFile* sf) {
    sf->ReportInt("Number of nodes", num_nodes_);
    sf->ReportInt("Number of subgoals", sgm_->GetSubgoalIdMapper()->GetNumSubgoals());
    sf->ReportInt("Number of instances", forward_drr_.size());

    sf->AddRemark("");
    sf->ReportDouble("Freespace time (s)", sgm_->GetFreespaceTimeInSeconds());
    sgm_->ReportMemoryStatistics(sf,true,true);
    //sf->ReportDouble("Freespace memory (Mb)", sgm_->GetRMemoryMB(true, true));

    sf->AddRemark("");
    ReportAll("Forward direct reachable", sf, forward_drr_);
    ReportAll("Backward direct reachable", sf, backward_drr_);
    ReportStatistics(aes_forward_, sf, true);
    ReportStatistics(aes_backward_, sf, false);
  }

  void PrintOneLineSummary(std::ostream & out = std::cout) {
    bool sd = false;
    out << GetMeanAndSDString(forward_drr_, [&](int v) -> double {return v;}, sd);
//*
    out <<" ";
    out << GetMeanAndSDString(
        aes_forward_,
        [&](RConnectStatistic v) -> double {return v.time;}, sd, 1000000);
//*/
//*
    out <<" ";
    out << GetMeanAndSDString(
        aes_forward_,
        [&](RConnectStatistic v) -> double {return v.radius;}, sd);
//*/
//*
    out <<" ";
    out << GetMeanAndSDString(
        aes_forward_,
        [&](RConnectStatistic v) -> double {return v.num_popped;}, sd);
//*/
//*
    out <<" ";
    out << GetMeanAndSDString(
        aes_forward_,
        [&](RConnectStatistic v) -> double {return v.num_expanded;}, sd);
//*/
//*
    out <<" ";
    out << GetMeanAndSDString(
        aes_forward_,
        [&](RConnectStatistic v) -> double {return v.num_subgoals;}, sd);
//*/
//*
    out <<" ";
    out << GetMeanAndSDString(
        aes_forward_,
        [&](RConnectStatistic v) -> double {return v.num_covered;}, sd);
//*/
//*
    out <<" ";
    out << GetMeanAndSDString(
        aes_forward_,
        [&](RConnectStatistic v) -> double {return v.num_stalled;}, sd);
//*/
//*
    out <<" ";
    out << GetMeanAndSDString(
        aes_forward_,
        [&](RConnectStatistic v) -> double {return v.num_relaxed;}, sd);
//*/
//*
    out <<" ";
    out << GetMeanAndSDString(
        aes_forward_,
        [&](RConnectStatistic v) -> double {return v.num_stall_relaxed;}, sd);
//*/
  }

 private:
  SGM* sgm_;
  int num_nodes_;
  std::vector<int> forward_drr_, backward_drr_;
  std::vector<RConnectStatistic> aes_forward_, aes_backward_;

  void EstimateDRRSize(nodeId n, bool forward, std::vector<int> & vec) {
    auto r = sgm_->GetReachabilityRelation();
    if (forward)
      r->SetExploreForward();
    else
      r->SetExploreBackward();
    vec.push_back(r->GetDirectReachableAreaSize(n));
  }

  void CalculateRConnectStatistic(nodeId n, bool forward,
                                  std::vector<RConnectStatistic> & vec) {
    auto r = sgm_->GetReachabilityRelation();
    if (forward)
      r->SetExploreForward();
    else
      r->SetExploreBackward();

    r->RConnect(n, true);
    vec.push_back(r->GetRConnectStatistic());
  }
};

class LatticeRExploreExperiment {
 public:
  LatticeRExploreExperiment(DirectoryParam dp, SGLatticeParam lp) {
    dp_ = dp;
    lp_ = lp;
    sgm_ = new LatticeSG(dp_.GetMapFilename(), lp_);

    dp_.MakeDirectories(sgm_->GetGName());

    // Generate the reachability relation.
    sgm_->GetReachabilityRelation();

    // Read subgoals if possible, else, create them.
    bool read = false;
    FileReadWrite rw;
    std::string filename = dp_.GetSaveLoadDirectory(sgm_->GetGName()) + "/"
        + sgm_->GetRSPCName() + ".rspc";

    if (kCHParamDefaultLoad && !kCHParamDefaultClear
        && rw.StartRead(filename)) {
      sgm_->ReadRSPC(rw);
      rw.CloseRead();
      std::cout << "Loaded R-SPC from " << filename << std::endl;
      read = true;
    }
    sgm_->GetSubgoalIdMapper();

    // Write subgoals.
    if (kCHParamDefaultSave && !read && rw.StartWrite(filename)) {
      sgm_->WriteRSPC(rw);
      rw.CloseWrite();
      std::cout << "Saved R-SPC to " << filename << std::endl;
    }

    // Create the experiment.
    exp_ = new RExploreExperiment<LatticeSG>(sgm_);
  }

  ~LatticeRExploreExperiment(){
    if (exp_)
      delete exp_;
  }

  void Experiment(int num_instances) {
    exp_->Experiment(num_instances);

    std::string filename = dp_.GetStatisticsDirectory(sgm_->GetGName()) + "/"
        + sgm_->GetRSPCName() + "_" + lp_.r.GetConnectType() + ".expstat";
    StatisticsFile sf (filename);
    exp_->Report(&sf);
    sf.Close();
  }

 private:
  DirectoryParam dp_;
  SGLatticeParam lp_;
  LatticeSG* sgm_;
  RExploreExperiment<LatticeSG>* exp_;
};

#endif /* APPS_SUBGOALGRAPH_UTILS_REXPLOREEXPERIMENT_H_ */
