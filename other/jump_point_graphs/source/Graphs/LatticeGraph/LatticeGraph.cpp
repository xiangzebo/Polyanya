#include <LatticeGraph.h>

using namespace std;

LatticeGraph::LatticeGraph(Lattice* lattice)
    : xyt_l_(lattice) {
  width_ = xyt_l_->GetGrid()->GetOriginalWidth();
  height_ = xyt_l_->GetGrid()->GetOriginalHeight();
  num_angles_ = xyt_l_->GetNumAngles();

  // Calculate bits to represent num angles and the bit mask.
  int temp_num_angles_ = num_angles_;
  num_angle_bits_ = 0;
  while (temp_num_angles_ >>= 1)
    num_angle_bits_ ++;

  uint32_t bit = 1;
  angle_mask_ = 0;
  for (int i = 0; i < num_angle_bits_; i++) {
    angle_mask_ = angle_mask_ | bit;
    bit = bit << 1;
  }

//  num_linearized_nodes_ = width_ * height_ * num_angles_;
  num_linearized_nodes_ = width_ * height_ * pow(2, num_angle_bits_);
  is_undirected_ = xyt_l_->IsUndirected();
  show_direction_ = true;

  IdentifyValidNodes();
  GenerateNeighbors();
  //IdentifyLargestConnectedComponent();

  num_valid_nodes_ = 0;
  num_valid_edges_ = 0;
  for (nodeId n = 0; n < num_linearized_nodes_; n++) {
    if (IsValidNode(n)) {
      num_valid_nodes_++;
      std::vector<WeightedArcHead> neighbors;
      GetSuccessors(n, neighbors);
      num_valid_edges_ += neighbors.size();
    }
  }

  cout << "Lattice graph has " << num_valid_nodes_ << " nodes and "
       << num_valid_edges_
       << (string) (is_undirected_ ? " undirected" : " directed") << " edges."
       << endl;
  cout << "Number of discrete poses: " << num_angles_ << "; bits: "
       << num_angle_bits_ << "; bit mask: "<<angle_mask_<<endl;

  // TODO: Move to function
  // Calculate delta nodeIds for primitives.
  for (unsigned int i = 0; i < num_angles_; i++) {
    xyThetaPos p(0,0,i);
    vector<MotionPrimitive>* prims = NULL;

    prims = xyt_l_->GetAllForwardPrimitives(i);
    assert(prims->size() <= kInvalidPrimId); // Assuming InvalidPrimId is the largest value.
    assert(prims->size() <= sizeof(ExecutablePrimitiveFlags)*8);
    for (unsigned int j = 0; j < prims->size(); j++)
      prims->at(j).SetDeltaNodeId(ToNodeId(prims->at(j).GetResultingPos(p)) - ToNodeId(p));

    prims = xyt_l_->GetAllReversePrimitives(i);
    assert(prims->size() <= kInvalidPrimId);
    assert(prims->size() <= sizeof(ExecutablePrimitiveFlags)*8);
    for (unsigned int j = 0; j < prims->size(); j++)
      prims->at(j).SetDeltaNodeId(ToNodeId(prims->at(j).GetResultingPos(p)) - ToNodeId(p));
  }

  // TODO: Move somewhere else

  if (kLatticeLargestConnectedComponent) {
    IdentifyLargestConnectedComponent();
//    GraphConnectedComponentAnalyzer<LatticeGraph> gcca(this);
//    valid_node_ = *gcca.GetLargestConnectedComponent();
//    GenerateNeighbors();
  }

  Debug();
}
LatticeGraph::~LatticeGraph() {
}

void LatticeGraph::Debug() {


  auto DebugPrimitive = [&](MotionPrimitive & p) {
    cout<<p.GetResultingPos(xyThetaPos(0, 0, p.GetDiscreteStartAngle()));
    cout<<"\t"<<p.GetCost()<<"\t"<<(int)p.delta_node_id_<<endl;
    assert(p.GetCost() > kEpsDistance);
    assert(
      p.delta_x_ != 0 || p.delta_y_ != 0
          || p.disc_start_orientation_ != p.disc_end_orientation_);
    assert(abs(p.delta_x_) <= p.cost_ + kEpsDistance);
    assert(abs(p.delta_y_) <= p.cost_ + kEpsDistance);
  };

  for (int a = 0; a < num_angles_; a++) {
    cout << "Orientation: " << a
        << "\tnum forward prims: " << xyt_l_->GetAllForwardPrimitives(a)->size()
        << "\tnum reverse prims: " << xyt_l_->GetAllReversePrimitives(a)->size()
        << endl;

//    for (auto p : *(xyt_l_->GetAllForwardPrimitives(a)))
//      DebugPrimitive(p);
//    for (auto p: *(xyt_l_->GetAllReversePrimitives(a)))
//      DebugPrimitive(p);
  }
}

void LatticeGraph::IdentifyValidNodes() {
  valid_node_.resize(num_linearized_nodes_);
  for (nodeId n = 0; n < num_linearized_nodes_; n++)
    valid_node_[n] = xyt_l_->IsValidPos(ToState(n));


  // TODO: Move to GraphConnectedComponentManager?
  if (kLatticeEliminateDeadEnds) {
    // Incrementally invalidates nodes that do not have valid successors or predecessors.
    std::vector<int> num_successors(num_linearized_nodes_, 0);
    std::vector<int> num_predecessors(num_linearized_nodes_, 0);
    std::vector<nodeId> nodes_to_invalidate;

    for (nodeId n = 0; n < num_linearized_nodes_; n++) {
      if (IsValidNode(n)) {
        std::vector<const MotionPrimitive*> prims;
        xyt_l_->GetExecutableForwardPrimitives(ToState(n), prims);
        num_successors[n] = prims.size();
        xyt_l_->GetExecutableReversePrimitives(ToState(n), prims);
        num_predecessors[n] = prims.size();

        if (num_successors[n] == 0 || num_predecessors[n] == 0) {
          nodes_to_invalidate.push_back(n);
          valid_node_[n] = false;
        }
      }
    }

    while (!nodes_to_invalidate.empty()) {
      nodeId n = nodes_to_invalidate.back();
      nodes_to_invalidate.pop_back();
      std::vector<const MotionPrimitive*> prims;

      xyt_l_->GetExecutableForwardPrimitives(ToState(n), prims);
      for (unsigned int i = 0; i < prims.size(); i++) {
        nodeId s = ToNodeId(prims[i]->GetResultingPos(ToState(n)));
        num_predecessors[s]--;
        if (valid_node_[s] && num_predecessors[s] == 0) {
          nodes_to_invalidate.push_back(s);
          valid_node_[s] = false;
        }
      }

      xyt_l_->GetExecutableReversePrimitives(ToState(n), prims);
      for (unsigned int i = 0; i < prims.size(); i++) {
        nodeId s = ToNodeId(prims[i]->GetResultingPos(ToState(n)));
        num_successors[s]--;
        if (valid_node_[s] && num_successors[s] == 0) {
          nodes_to_invalidate.push_back(s);
          valid_node_[s] = false;
        }
      }
    }
  }
}

ExecutablePrimitiveFlags LatticeGraph::GenerateExecutablePrimitiveFlags(
    nodeId node, bool forward) {

  xyThetaPos pos = ToState(node);
  ExecutablePrimitiveFlags flags = 0;

  if (!IsValidNode(node))
    return 0;

  vector<MotionPrimitive>* prims = NULL;

  if (forward)
    prims = xyt_l_->GetAllForwardPrimitives(pos.o);
  else
    prims = xyt_l_->GetAllReversePrimitives(pos.o);

  assert(prims != NULL && prims->size() <= sizeof(ExecutablePrimitiveFlags)*8);

  for (unsigned int i = 0; i < prims->size(); i++) {
    ExecutablePrimitiveFlags flag = 0;

    xyThetaPos res_pos = prims->at(i).GetResultingPos(pos);

    if (xyt_l_->CanExecutePrimitive(pos, &prims->at(i))
        && IsValidNode(ToNodeId(res_pos))) {
      flag = 1;
    }

    flags = flags | (flag << i);
  }

  return flags;
}
void LatticeGraph::GenerateNeighbors() {
  valid_forward_primitive_flags_.clear();
  valid_reverse_primitive_flags_.clear();
  for (nodeId n = 0; n < num_linearized_nodes_; n++) {
    valid_forward_primitive_flags_.push_back(
        GenerateExecutablePrimitiveFlags(n, true));
    if (!is_undirected_)
      valid_reverse_primitive_flags_.push_back(
          GenerateExecutablePrimitiveFlags(n, false));
  }
}

void LatticeGraph::IdentifyLargestConnectedComponent() {
  GraphConnectedComponentAnalyzer<LatticeGraph> cc_analyzer(this);
  const std::vector<bool>* in_largest_component = cc_analyzer
      .GetLargestConnectedComponent();
  assert(valid_node_.size() == in_largest_component->size());

  for (nodeId n = 0; n < in_largest_component->size(); n++) {
    valid_node_[n] = in_largest_component->at(n);
  }
  GenerateNeighbors();

  num_valid_nodes_ = 0;
  num_valid_edges_ = 0;

  for (nodeId n = 0; n < in_largest_component->size(); n++) {
    if (valid_node_[n]) {
      num_valid_nodes_++;
      std::vector<WeightedArcHead> neighbors;
      GetSuccessors(n, neighbors);
      num_valid_edges_ += neighbors.size();
    }
  }
}
void LatticeGraph::GetNeighbors(
    nodeId n, ExecutablePrimitiveFlags flags,
    vector<MotionPrimitive>* prims,
    std::vector<WeightedArcHead> & neighbors) const {

  neighbors.clear();
  ExecutablePrimitiveFlags mask = 1;
  for (unsigned int i = 0; i < prims->size(); i++) {
    if (flags & mask)
      neighbors.push_back(
          WeightedArcHead((prims->at(i).GetResultingNodeId(n)),
                          prims->at(i).GetCost()));
    mask = mask << 1;
  }
}
void LatticeGraph::GetSuccessors(
  nodeId n, std::vector<WeightedArcHead> & neighbors,
  ExecutablePrimitiveFlags prim_mask) const {
  GetNeighbors(n, valid_forward_primitive_flags_[n] & prim_mask,
               xyt_l_->GetAllForwardPrimitives(n & angle_mask_),
               neighbors);
}
void LatticeGraph::GetPredecessors(
    nodeId n, std::vector<WeightedArcHead> & neighbors,
    ExecutablePrimitiveFlags prim_mask) const {
  if (is_undirected_)
    GetSuccessors(n, neighbors, prim_mask);
  else {
    GetNeighbors(n, valid_reverse_primitive_flags_[n] & prim_mask,
                 xyt_l_->GetAllReversePrimitives(n & angle_mask_),
                 neighbors);
  }
}
string LatticeGraph::GetNodeName(nodeId n) const {
  stringstream ss;
  ss << ToState(n);
  return ss.str();
}

nodeId LatticeGraph::GetNodeIdFromName(std::string s) const {
  xyThetaPos p(0,0,0);
  sscanf(s.c_str(), "(%d, %d, %d)", &p.x, &p.y, &p.o);
  return ToNodeId(p);
}

#ifndef NO_HOG
void LatticeGraph::DrawDirectedNode(const MapEnvironment *env,
                                           xyThetaPos pos,
                                           double length) const {
  double r = xyt_l_->ToRadian(pos.o);
  xyt_l_->GetGrid()->DrawDirectedNode(env, pos.x, pos.y,
                                      xyt_l_->ToRadian(pos.o), length);
}


void LatticeGraph::DrawArrowNode(const MapEnvironment *env, xyThetaPos pos,
                                 double ratio, int priority) const {
  GLfloat current_width;
  glGetFloatv(GL_LINE_WIDTH, &current_width);
  glLineWidth(kDrawArrowNodeWidth);

  xyt_l_->GetGrid()->DrawArrowNode(env, pos.x, pos.y, xyt_l_->ToRadian(pos.o),
                                   ratio, priority);
  glLineWidth(current_width);
}


void LatticeGraph::DrawNode(const MapEnvironment *env, xyThetaPos pos,
                            double priority) const {
  if (show_direction_ && num_angles_ > 1) {
    if (fabs(priority - 0) < 0.01)
      priority = 0.5;
    //DrawDirectedNode(env, pos, 0.4);
    DrawArrowNode(env, pos, priority);
  }
  else {
    xyt_l_->GetGrid()->DrawNode(env, pos.x, pos.y, priority);
  }
}
void LatticeGraph::DrawEdge(const MapEnvironment *env, xyThetaPos from,
                            xyThetaPos to, double width) const {
  GLfloat current_width;
  glGetFloatv(GL_LINE_WIDTH, &current_width);
  glLineWidth(width);

  const MotionPrimitive* prim = xyt_l_->GetIntermediatePrimitive(from, to);
  if (prim != NULL)
    prim->DrawPrimitive(env, from.x, from.y);

  else
    env->GLDrawColoredLine(from.x, from.y, to.x, to.y);

  glLineWidth(current_width);
}

void LatticeGraph::DrawPath(const MapEnvironment *env,
                                   const std::vector<nodeId> & path) const {
  for (unsigned int i = 1; i < path.size(); i++) {
    DrawEdge(env, path[i - 1], path[i]);
  }
}

void LatticeGraph::VisualizeExecutablePrimitives(
    const MapEnvironment *env, xyThetaPos pos,
    bool show_predecessors_instead) const {
  SetVisualizeNodeDirection(true);
//  env->SetColor(1, 0, 0);
  env->SetColor(0, 0, 1);
//  DrawNode(env, pos);
  DrawArrowNode(env, pos);

  vector<WeightedArcHead> neighbors;
  if (!show_predecessors_instead) {
    GetSuccessors(ToNodeId(pos), neighbors);
    for (unsigned int i = 0; i < neighbors.size(); i++) {
      env->SetColor(0,0,0);
      DrawEdge(env, pos, ToState(neighbors[i].target));
      env->SetColor(1,0,0);
      DrawArrowNode(env, ToState(neighbors[i].target));
    }
  } else {
    GetPredecessors(ToNodeId(pos), neighbors);
    for (unsigned int i = 0; i < neighbors.size(); i++) {
      DrawEdge(env, ToState(neighbors[i].target), pos);
    }
  }
}
#endif

