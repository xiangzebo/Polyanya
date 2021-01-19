/*
 * SGExperiment.h
 *
 *  Created on: Oct 18, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_UTILS_SGEXPERIMENT_H_
#define APPS_SUBGOALGRAPH_UTILS_SGEXPERIMENT_H_

#include "../Statistics/AggregateQueryStatistics.h"
#include "InstanceManager.h"

// Q: Query method
// I: Instances

template<class Q, class I>
class SGExperiment {

  typedef typename Q::State State;

 public:
  SGExperiment(Q* q, I* i, std::string statistics_filename,
               std::string instance_filename)
      : query_(q),
        instance_manager_(i),
        sf_(statistics_filename),
        qs_(instance_filename) {
  }
  ~SGExperiment() {
    CloseStatisticsFiles();
  }

  void Preprocess() {
    query_->Preprocess();
  }

  void RunInstances() {
    int valid_count = 0;
    for (int i = 0; i < instance_manager_->GetNumInstances(); i++) {
      auto ins = instance_manager_->GetInstance(i);

      std::vector<State> path;
      QueryStatistic s = query_->Query(ins, path);
      s.dist = ValidatePath(path, ins.start, ins.goal);

      s.valid = s.dist < kMaxDistance;
      s.suboptimality = s.dist/ins.dist;
      s.shortest = s.suboptimality < 1.001;
      qs_.AddQueryData(s);

      if (!s.shortest) {
        //std::cout<<"Suboptimal instance: "<<ins.start<<"\t"<<ins.goal<<std::endl;
      }

      if (!s.valid) {
        if (valid_count == 0) {
          std::cout<<"Invalid instance: "<<ins.start<<"\t"<<ins.goal<<std::endl;
        }
        valid_count ++;
      }
    }
    if (valid_count > 0) {
      std::cout<<"INVALID INSTANCE COUNT: "<<valid_count<<std::endl;
    }
  }

  void ReportPreprocessingStatistics() {
    query_->ReportPreprocessStatistics(&sf_);
  }

  void ReportQueryStatistics() {
    qs_.Report(&sf_);
    query_->ReportAdditionalQueryStatistics(&sf_);
  }
  void CloseStatisticsFiles() {
    sf_.Close();
    qs_.CloseInstanceFile();
  }

  AggregateQueryStatistics* GetStatistics() {
    return &qs_;
  }

  Q* GetQueryMethod() {
    return query_;
  }

 private:
  Q* query_;
  I* instance_manager_;
  StatisticsFile sf_;
  AggregateQueryStatistics qs_;

  Distance ValidatePath(std::vector<State> & path, State s, State t) {
    if (path.size() == 0)
      return kMaxDistance;

    if (!(s == path.front() && t == path.back()))
      return kMaxDistance;

    Distance d = 0;
    for (int i = 0; i < path.size()-1; i++) {
      Distance l = 0;
      if (!(path[i] == path[i+1]))
        l = query_->GetOriginalGraph()->GetEdgeLength(path[i], path[i + 1]);

      if (l >= kMaxDistance - kEpsDistance)
        return kMaxDistance;

      d += l;
    }
    return d;
  }
};


#endif /* APPS_SUBGOALGRAPH_UTILS_SGEXPERIMENT_H_ */
