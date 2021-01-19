template<class ArcHeadType>
DynamicGraph<ArcHeadType>::DynamicGraph() {
  num_nodes_ = 0;
  num_arcs_ = 0;
  query_mode_ = false;
//  num_query_nodes_ = 0;
  bypass_mode_ = false;
}

template<class ArcHeadType>
DynamicGraph<ArcHeadType>::~DynamicGraph() {
}

template<class ArcHeadType>
Distance DynamicGraph<ArcHeadType>::GetWeight(nodeId n, ArcHeadType & h) {
  return h.weight;
}
// This seems to work for now.
template<> inline
Distance DynamicGraph<ArcHead>::GetWeight(nodeId n, ArcHead & m) {
  std::cout<<"Get weight called for ArcHead!\n";
  return kMaxDistance;
}

template<class ArcHeadType>
template<class ArcType>
void DynamicGraph<ArcHeadType>::CreateGraph(int num_nodes,
                                            std::vector<ArcType> & input_arcs) {

  assert(
      !query_mode_ && !bypass_mode_
          && "Cannot create Dynamic graph when it is in query/bypass mode");
  num_nodes_ = num_nodes;
//  num_query_nodes_ = num_nodes;
  num_arcs_ = 0;
  arcs_.clear();

  // Sort the arcs wrt their heads.
  std::sort(input_arcs.begin(), input_arcs.end());

  // Leave gap for query nodes.
  arcs_.resize(num_nodes + 2);

  int start = 0;
  while (start < input_arcs.size()) {
    // Find all arcs of the next node.
    nodeId id = input_arcs[start].source;
    int end = start;

    // Find where the arcs end.
    while (input_arcs[end].source == id)
      end++;

    // Reserve space for all arcs + an extra edge.
    arcs_[id].reserve(end - start + 1);

    while (start < end) {
      assert(input_arcs[start].source == id);
      arcs_[id].push_back(get_head(input_arcs[start]));
      start++;
      num_arcs_++;
    }
  }

  assert(num_arcs_ == input_arcs.size());
}

template<class ArcHeadType>
void DynamicGraph<ArcHeadType>::AddQueryNode(nodeId n,
                                             std::vector<ArcHeadType> & arcs,
                                             bool reverse) {
  query_mode_ = true;
  for (auto arc : arcs)
    AddQueryArc(n, arc, reverse);
}

template<class ArcHeadType>
inline void DynamicGraph<ArcHeadType>::HideArc(nodeId from, nodeId to) {
  bypass_mode_ = true;

  int i = FindArc(from, to);
  assert(i != -1 && "Dynamic graph does not hide an arc that does not exist");

  // Make note of the hidden arc.
  hidden_arc_sources_.push_back(from);
  hidden_arc_heads_.push_back(arcs_[from][i]);

  // Overwrite the arc with the last arc on the list and pop the last arc.
  arcs_[from][i] = arcs_[from].back();
  arcs_[from].pop_back();
}

template<class ArcHeadType>
void DynamicGraph<ArcHeadType>::RestoreHiddenArcs() {
  while(!hidden_arc_sources_.empty()) {
    nodeId source = hidden_arc_sources_.back();
    ArcHeadType arc = hidden_arc_heads_.back();
    arcs_[source].push_back(arc);
    hidden_arc_sources_.pop_back();
    hidden_arc_heads_.pop_back();
  }
  bypass_mode_ = false;
}

template<class ArcHeadType>
void DynamicGraph<ArcHeadType>::AddArc(nodeId n, ArcHeadType arc, bool reverse) {
  assert(!query_mode_ && "Dynamic graph cannot add a new arc in query mode");
  assert(!bypass_mode_ && "Dynamic graph cannot add a new arc in bypass mode");

  if (num_nodes_ < std::max(n, arc.target) + 1)
    num_nodes_ = std::max(n, arc.target) + 1;
  AddArcSubroutine(n, arc, reverse);
}


template<class ArcHeadType>
void DynamicGraph<ArcHeadType>::RemoveArc(nodeId from, nodeId to) {
  assert(!query_mode_ && "Dynamic graph cannot remove an arc in query mode");
  assert(!bypass_mode_ && "Dynamic graph cannot remove an  arc in bypass mode");
  RemoveArcSubroutine(from, to);
}

template<class ArcHeadType>
void DynamicGraph<ArcHeadType>::AddQueryArc(nodeId from, ArcHeadType to,
                                            bool reverse) {
  query_mode_ = true;
  AddArcSubroutine(from, to, reverse);
  if (!reverse)
    nodes_with_query_arcs_.push_back(from);
  else
    nodes_with_query_arcs_.push_back(to.target);
}

template<class ArcHeadType>
void DynamicGraph<ArcHeadType>::ClearQuery() {
  query_mode_ = false;
  while (!nodes_with_query_arcs_.empty()) {
    arcs_[nodes_with_query_arcs_.back()].pop_back();
    nodes_with_query_arcs_.pop_back();
    num_arcs_--;
  }
//  num_query_nodes_ = num_nodes_;
}

template<class ArcHeadType>
int DynamicGraph<ArcHeadType>::FindArc(nodeId from, nodeId to) {
  for (int i = 0; i < arcs_[from].size(); i++)
    if (arcs_[from][i].target == to)
      return i;
  return -1;
}

template<class ArcHeadType>
void DynamicGraph<ArcHeadType>::AddArcSubroutine(nodeId from, ArcHeadType to,
                                                 bool reverse) {
  if (reverse)
    std::swap(from, to.target);


  if (arcs_.size() < std::max(from, to.target) + 1)
    arcs_.resize(std::max(from, to.target) + 1);

  assert(
      FindArc(from, to.target) == -1
      && "Dynamic graph does not add an arc that already exists");

  arcs_[from].push_back(to);
  num_arcs_++;
}

template<class ArcHeadType>
void DynamicGraph<ArcHeadType>::RemoveArcSubroutine(nodeId from, nodeId to) {
  int i = FindArc(from, to);
  assert(i != -1 && "Dynamic graph does not remove an arc that does not exist");
  arcs_[from][i] = arcs_[from].back();
  arcs_[from].pop_back();
  num_arcs_--;
}


