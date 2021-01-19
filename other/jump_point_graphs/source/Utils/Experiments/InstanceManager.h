/*
 * SGBenchmark.h
 *
 *  Created on: Oct 18, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_UTILS_SGBENCHMARK_H_
#define APPS_SUBGOALGRAPH_UTILS_SGBENCHMARK_H_

#include <cstdlib>
#include <iostream>
#include "Dijkstra.h"
#include "GraphDefinitions.h"

// S: State (xyLoc / xyThetaPos)
// Assumes that << and >> operators are overloaded for writing/reading State
// G: Original graph (for generating valid instances)
// Q: Query algorithm (for testing)

template <class S>
struct ProblemInstance {
 public:
  ProblemInstance() {
    start = S();
    goal = S();
    dist = kMaxDistance;
  }
  ProblemInstance(S s, S g, double c) {
    start = s; goal = g; dist = c;
  }

  S start;
  S goal;
  Distance dist;
};

inline
void Read(ProblemInstance<xyLoc> & ins, std::istream & in) {
  in >> ins.start.x;
  in >> ins.start.y;

  in >> ins.goal.x;
  in >> ins.goal.y;

  in >> ins.dist;
}

inline
void Read(ProblemInstance<xyThetaPos> & ins, std::istream & in) {
  in >> ins.start.x;
  in >> ins.start.y;
  in >> ins.start.o;

  in >> ins.goal.x;
  in >> ins.goal.y;
  in >> ins.goal.o;

  in >> ins.dist;
}

inline
void Write(ProblemInstance<xyLoc> ins, std::ostream & out) {
  out << ins.start.x << "\t";
  out << ins.start.y << "\t";

  out << ins.goal.x << "\t";
  out << ins.goal.y << "\t";

  out << ins.dist << std::endl;
}

inline
void Write(ProblemInstance<xyThetaPos> ins, std::ostream & out) {
  out << ins.start.x << "\t";
  out << ins.start.y << "\t";
  out << ins.start.o << "\t";

  out << ins.goal.x << "\t";
  out << ins.goal.y << "\t";
  out << ins.goal.o << "\t";

  out << ins.dist << std::endl;
}

template <class S, class G>
class InstanceManager{
public:
  InstanceManager(G* g = NULL)
      : g_(g),
        dijkstra_(NULL) {
  }

  ~InstanceManager() {
    if (dijkstra_ != NULL)
      delete dijkstra_;
  }

  void InitializeDijkstra() {
    assert(
        g_ != NULL && "Instance manager does not have graph for generating instances!");
    if (dijkstra_ == NULL)
      dijkstra_ = new Dijkstra<G> (g_);
  }

  ProblemInstance<S> GenerateValidInstance(int max_trials = 1000) {
    InitializeDijkstra();

    while (max_trials > 0) {
      nodeId start = GetRandomNodeId();
      nodeId goal = GetRandomNodeId();
      std::vector<nodeId> dummy_path;
      Distance d = dijkstra_->FindPath(start, goal, dummy_path);
      if (d < kMaxDistance)
        return ProblemInstance<S> (g_->ToState(start), g_->ToState(goal), d);
      max_trials --;
    }
    std::cout << "InstanceGenerator couldn't find a valid instance after "
              << max_trials << " trials." << std::endl;
    return ProblemInstance<S> (S(), S(), std::numeric_limits<double>::max());
  }

  void GenerateValidInstances(int num_instances, std::string filename) {
    if (ReadInstances(filename)) {
      // FIXME: Maybe an assertion is too much?
      assert(num_instances == instances_.size());
    }
    else {
      GenerateValidInstances(num_instances);
      WriteInstances(filename);
    }
  }

  void GenerateValidInstances(int num_instances) {
    srand(0);
    for (int i = 0; i < num_instances; i++)
      instances_.push_back(GenerateValidInstance());
    std::cout << "Generated " << instances_.size() << " instances."
        << std::endl;
  }

  void AddInstance(ProblemInstance<S> s) {
    instances_.push_back(s);
  }

  int GetNumInstances() {
    return instances_.size();
  }

  ProblemInstance<S> GetInstance(int n) {
    return instances_[n];
  }

private:
  G* g_;
  Dijkstra<G>* dijkstra_;
  std::vector<ProblemInstance<S> > instances_;

  nodeId GetRandomNodeId() {
    int max_node_id_ = g_->GetNumAllNodes();
    nodeId n = rand() % max_node_id_;
    while (!g_->IsValidNode(n))
      n = rand() % max_node_id_;
    return n;
  }

  bool ReadInstances(std::string filename) {
    std::ifstream in(filename.c_str());
    if (in.fail())
      return false;

    int size;
    in>>size;
    instances_.resize(size);
    for (int i = 0; i < size; i++) {
      Read(instances_[i], in);
    }

    std::cout << "Read " << instances_.size() << " instances from " << filename
        << std::endl;
    return true;
  }

  bool WriteInstances(std::string filename) {
    std::ofstream out(filename.c_str());
    if (!out.is_open())
      return false;

    out << instances_.size() << std::endl;
    for (int i = 0; i < instances_.size(); i++) {
      Write(instances_[i], out);
    }

    std::cout << "Wrote " << instances_.size() << " instances to " << filename
        << std::endl;
    return true;
  }
};


#endif /* APPS_SUBGOALGRAPH_UTILS_SGBENCHMARK_H_ */
