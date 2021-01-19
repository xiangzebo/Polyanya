#include <cassert>

template<class ArcHeadType>
template<class ArcType>
ArcListQueryGraph<ArcHeadType>::ArcListQueryGraph(
    int num_nodes, std::vector<ArcType> & input_arcs) {
  num_arcs_ = 0;
  num_nodes_ = 0;
  num_query_nodes_ = 0;
  CreateGraph(num_nodes, input_arcs);
}

template<class ArcHeadType>
ArcListQueryGraph<ArcHeadType>::~ArcListQueryGraph() {
}

template<class ArcHeadType>
Distance ArcListQueryGraph<ArcHeadType>::GetWeight(nodeId n, ArcHeadType & h) {
  return h.weight;
}
// This seems to work for now.
template<> inline
Distance ArcListQueryGraph<ArcHead>::GetWeight(nodeId n, ArcHead & m) {
  std::cout<<"Get weight called for ArcHead!\n";
  return kMaxDistance;
}

template<class ArcHeadType>
template<class ArcType>
void ArcListQueryGraph<ArcHeadType>::CreateGraph(
    int num_nodes, std::vector<ArcType> & input_arcs) {

  num_nodes_ = num_nodes;
  num_query_nodes_ = num_nodes;
  num_arcs_ = 0;
  arcs_.clear();
  nodes_with_query_arcs_.clear();

  // Sort the arcs wrt their heads.
  std::sort(input_arcs.begin(), input_arcs.end());

  // +2 for query nodes, +1 for first-out implementation.
  first_out_.resize(num_nodes + 3, 0);
  no_extra_arc_.resize(num_nodes + 3, true);

  // + 1000 to leave room for connecting the start to the graph.
  arcs_.resize(input_arcs.size() + num_nodes_ + 100000);

  int read = 0;
  int write = 0;

  for (nodeId n = 0; n < num_nodes_; n++) {
    // +1 for the extra arc.
    first_out_[n+1] = first_out_[n] + 1;
    while (read < input_arcs.size() && input_arcs[read].source == n) {
      arcs_[write] = get_head(input_arcs[read]);
      read++;
      write++;
      first_out_[n+1]++;
      num_arcs_++;
    }
    write++;
    assert(write < arcs_.size());
  }

  // Leave slots for the extra arc flags for the query nodes (which are
  // sometimes necessary to connect start and goal nodes).
  first_out_[num_nodes_+1] = first_out_[num_nodes_]+1;
  first_out_[num_nodes_+2] = first_out_[num_nodes_+1]+1;
  //  first_out_[num_nodes_+1] = first_out_[num_nodes_];
  //  first_out_[num_nodes_+2] = first_out_[num_nodes_];

  assert(num_arcs_ == input_arcs.size());
  assert(first_out_[num_nodes_] == num_arcs_+ num_nodes_);

  //std::cout<<"First out[num_nodes] = "<<first_out_[num_nodes_]<<std::endl;
  //std::cout<<"Num arcs = "<<num_arcs_<<std::endl;
  //std::cout<<"Num nodes = "<<num_nodes_<<std::endl;
}

template<class ArcHeadType>
inline void ArcListQueryGraph<ArcHeadType>::AddQueryNode(nodeId n) {
  // Assumes that each query node has an id higher than all the existing nodes
  // in the graph (and the previous query nodes).
  assert(n >= num_query_nodes_);
  num_query_nodes_ = n+1;
  assert(n < first_out_.size()-1);
}

template<class ArcHeadType>
void ArcListQueryGraph<ArcHeadType>::AddQueryArcs(
    nodeId n, std::vector<ArcHeadType> & arcs, bool reverse) {

  // Reverse means we are adding the arcs to the extra arc slots.
  if (reverse) {
    for (unsigned int i = 0; i < arcs.size(); i++) {
      assert(no_extra_arc_[arcs[i].target]);
      arcs_[first_out_[arcs[i].target+1]-1] = arcs[i];
      arcs_[first_out_[arcs[i].target+1]-1].target = n;
      no_extra_arc_[arcs[i].target] = false;
      nodes_with_query_arcs_.push_back(arcs[i].target);

      /*
      nodeId source = n;
      ArcHeadType arc = arcs[i];
      std::swap(source, arc.target);

      if (!no_extra_arc_[source]) {
        std::cout << "Trying to add an arc (" << source << ", " << arc.target
            << ", " << arc.weight << ")" << std::endl;
        auto existing_arc = arcs_[first_out_[source+1]-1];
        std::cout << "But there is already an extra arc to "
            << existing_arc.target << " with weight " << existing_arc.weight
            << std::endl;
        assert(false);
      }
      assert(no_extra_arc_[source]);

      arcs_[first_out_[source+1]-1] = arc;
      no_extra_arc_[source] = false;
      nodes_with_query_arcs_.push_back(source);
      */
    }
  }

  // We are adding the arcs at the end of the arcs array.
  else {
    // +1 for the extra arc.
    assert (first_out_[n] + 1 == first_out_[n+1]);

//    first_out_[n+1] = first_out_[n]+1;
//    ArcHeadType possible_extra_arc = arcs_[first_out_[n]];
    for (unsigned int i = 0; i < arcs.size(); i++) {
      arcs_[first_out_[n] + i] = arcs[i];
      first_out_[n+1]++;
    }
//    arcs_[first_out_[n+1]] = possible_extra_arc;

    // FIXME: Update the rest of the array as well.
    for (unsigned int i = n + 2; i < first_out_.size(); i++)
      first_out_[i] = first_out_[i-1] + 1;
  }
}

template<class ArcHeadType>
void ArcListQueryGraph<ArcHeadType>::ClearQuery() {
  while (!nodes_with_query_arcs_.empty()) {
    nodeId n = nodes_with_query_arcs_.back();
    assert(!no_extra_arc_[n]);
    no_extra_arc_[n] = true;
    nodes_with_query_arcs_.pop_back();
  }

  // Clear the start/goal query arcs.
  first_out_[num_nodes_+1] = first_out_[num_nodes_]+1;
  first_out_[num_nodes_+2] = first_out_[num_nodes_+1]+1;
//  first_out_[first_out_.size()-2] = first_out_[first_out_.size()-3];
//  first_out_[first_out_.size()-1] = first_out_[first_out_.size()-3];
  num_query_nodes_ = num_nodes_;
  return;
}
