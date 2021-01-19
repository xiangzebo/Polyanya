/*
 * Grid3DGraph.h
 *
 *  Created on: Oct 8, 2016
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH2DGRID_GRID2DGRAPH_H_
#define APPS_SUBGOALGRAPH2DGRID_GRID2DGRAPH_H_

#include <cassert>
#include <vector>
#include <string>
#include <sstream>

#include "GraphDefinitions.h"
#include "Grid2D.h"
#include "GraphConnectedComponentAnalyzer.h"
#include "CommonHeuristics.h"
#include "../../Utils/range.h"

#include "GPPCGrid.h"

class Grid2DGraph {
 public:
  typedef WeightedArcHead SuccessorType;

  Grid2DGraph(GPPCGrid* grid, bool direction_extended = false)
      : grid_(grid),
        direction_extended_(direction_extended) {
    ConstructGraph();

#ifndef SG_QUIET
    std::cout << "Grid graph contains " << num_valid_nodes_ << " nodes and "
              << num_directed_edges_ << " directed edges." << std::endl;
#endif
  }
  ~Grid2DGraph() {
  }

  bool IsDirectionExtended() const {return direction_extended_;}

  unsigned int GetNumAllNodes() const {
    if (direction_extended_)
      return grid_->GetNumPaddedCells()*9;
    else
      return grid_->GetNumPaddedCells();
  }

  unsigned int GetNumValidNodes() const {
    return num_valid_nodes_;
  }
  unsigned int GetNumArcs() const {
    return num_directed_edges_;
  }

  nodeId ToNodeId(xyLoc l) const {
    return grid_->ToXYLin(l);
  }
  xyLoc ToXYLoc(nodeId n) const {
    if (direction_extended_)
      return grid_->ToXYLoc(grid_->ExtractXYLin(n));
    else
      return grid_->ToXYLoc(n);
  }
  xyLoc ToState(nodeId n) const {
    return ToXYLoc(n);
  }

  bool IsValidNode(nodeId n) const {
    if (direction_extended_)
      return grid_->IsTraversable(grid_->ExtractXYLin(n));
    else
      return grid_->IsTraversable(n) && in_largest_connected_component_[n];
  }

//*
  // FIXME:
  bool HasNoSuccessors(nodeId n) {
    return false;
  }
  void GetSuccessors(nodeId n, std::vector<WeightedArcHead> & neighbors) const {
    if (direction_extended_)
      grid_->GetDirectedNeighbors(n, neighbors);
    else
      grid_->GetNeighbors(n, neighbors);
  }
  void GetPredecessors(nodeId n, std::vector<WeightedArcHead> & neighbors) const {
    grid_->GetNeighbors(n, neighbors);
  }

  void GetSuccessors(nodeId n, WeightedArcHead* & neighbors,
                     int & num_neighbors) {
    GetSuccessors(n, temp_neighbors_);
    neighbors = &temp_neighbors_[0];
    num_neighbors = temp_neighbors_.size();
  }
  Range<typename std::vector<WeightedArcHead>::iterator> GetSuccessors(
      nodeId n) {
    GetSuccessors(n, temp_neighbors_);
    return vector_to_reference_range(temp_neighbors_);
  }
  Range<typename std::vector<WeightedArcHead>::iterator> GetPredecessors(
      nodeId n) {
    GetPredecessors(n, temp_neighbors_);
    return vector_to_reference_range(temp_neighbors_);
  }

  Distance GetWeight(nodeId n, WeightedArcHead & h) {
    return h.weight;
  }

  Distance GetEdgeLength(xyLoc p1, xyLoc p2) {
    int dx = abs(p1.x - p2.x);
    int dy = abs(p1.y - p2.y);

    if (dx > 1 || dy > 1)
      return kMaxDistance;
    else if (dx + dy == 1)
      return CARD_COST;
    else if (dx + dy == 2)
      return DIAG_COST;
    else
      return 0;
  }

  std::string GetNodeName(nodeId n) const {
    // FIXME: Direction-extended
    xyLoc l = grid_->ToXYLoc(n);
    std::stringstream ss;
    ss << "(" << l.x << ", " << l.y << ")";
    return ss.str();
  }

  GPPCGrid* GetGrid() const {
    return grid_;
  }


#ifndef NO_HOG
  /*
  void DrawNode(const MapEnvironment *env, xyLoc pos,
                double priority = 0) const {
    env->OpenGLPriorityDraw(xyLoc(pos.x, pos.y), priority);
  }
  */
  void DrawNode(const MapEnvironment *env, nodeId n,
                double priority = 0) const {
    if (direction_extended_) {
      auto l = grid_->ToXYLoc(grid_->ExtractXYLin(n));
      auto d = grid_->ExtractDirection2D(n);

      if (d == dAll)
        grid_->DrawNode(env, l.x, l.y, priority);
      else
        grid_->DrawArrowNode(env, n, 0.5);
        //grid_->DrawArrowNode(env, l.x, l.y, d, 0.5);
    }
    else {
      auto l = ToXYLoc(n);
      grid_->DrawNode(env, l.x, l.y, priority);
    }
  }

  void DrawNode(const MapEnvironment *env, xyLoc l, Direction2D d, double prioirty = 0) {
    grid_->DrawArrowNode(env, l.x, l.y, d, 0.5);
    /*
    auto c = grid_->ToLinearizedPaddedCoordinate(l);
    if (direction_extended_)
      DrawNode(env, grid_->ToLinearizedXYDCoordinate(c, d), prioirty);
    else
      DrawNode(env, c, prioirty);
    */
  }

  void DrawEdge(const MapEnvironment *env, xyLoc from, xyLoc to) const {
    //*
    if (true)
    /*/
    if (direction_extended_)
    //*/
      grid_->DrawDiagonalFirstEdge(env, from.x, from.y, to.x, to.y);
    else
      grid_->DrawDiagonalFirstGoingRightEdge(env, from.x, from.y, to.x, to.y);

//      env->GLDrawColoredLine(from.x, from.y, to.x, to.y);
  }
  void DrawEdge(const MapEnvironment *env, nodeId n1, nodeId n2) const {
    xyLoc l1 = ToXYLoc(n1);
    xyLoc l2 = ToXYLoc(n2);
/*
    if (direction_extended_)
      grid_->DrawDiagonalFirstEdge(env, l1.x, l1.y, l2.x, l2.y,
                                   grid_->ExtractDirection2D(n1));
    else
*/
//      DrawEdge(env, ToXYLoc(n1), ToXYLoc(n2));
      env->GLDrawColoredLine(l1.x, l1.y, l2.x, l2.y);
  }


  void DrawLevelNode(const MapEnvironment *env, nodeId n, int level,
                double priority = 0) const {
    // TODO
    env->OpenGLPriorityDrawLevel(ToXYLoc(n), level, priority);
  }
  void DrawLevelEdge(const MapEnvironment *env, nodeId n1, int level1,
                     nodeId n2, int level2) const {
    // TODO
    env->GLDrawColoredLineLevel(ToXYLoc(n1), level1, ToXYLoc(n2), level2);
  }

  // HACKY EXTENSION TO DIRECTED
  // TODO
  nodeId ToDirectionExtendedNodeId(xyLoc l, Direction2D d = dAll) const {
    return grid_->ToXYLin(l)*9 + d;
  }
  xyLoc ToXYLocWithoutDirection(nodeId n) const {
    return grid_->ToXYLoc(n/9);
  }



#endif

 private:
  GPPCGrid* grid_;
  //std::vector<std::vector<WeightedArcHead> > neighbors_;
  std::vector<bool> in_largest_connected_component_;

  bool direction_extended_;

  int num_valid_nodes_;
  int num_directed_edges_;

  std::vector<WeightedArcHead> temp_neighbors_;

  void ConstructGraph() {
    // FIXME
    if (direction_extended_) {
      num_valid_nodes_ = 0;
      num_directed_edges_ = 0;
      for (xyLin l = 0; l < grid_->GetNumPaddedCells(); l++) {
        for (Direction2D d = -1; d < 8; d++) {
          auto ld = grid_->ToXYDLin(l,d);
          assert(l == grid_->ExtractXYLin(ld));
          assert(d == grid_->ExtractDirection2D(ld));
        }
      }
      return;
    }
    num_valid_nodes_ = 0;
    num_directed_edges_ = 0;
    in_largest_connected_component_.resize(GetNumAllNodes(), true);

    for (nodeId n = 0; n < GetNumAllNodes(); n++) {
      if (!IsValidNode(n))
        continue;
      num_valid_nodes_++;
      std::vector<WeightedArcHead> neighbors;
      grid_->GetNeighbors(n, neighbors);
      num_directed_edges_ += neighbors.size();
    }

    /*
    neighbors_.resize(GetNumAllNodes());

    for (nodeId id = 0; id < GetNumAllNodes(); id++) {
      if (!IsValidNode(id))
        continue;

      num_valid_nodes_++;
      std::vector<xyLoc> neighbors;
      grid_->GetNeighbors(ToXYLoc(id), neighbors, eight_neighbor_,
                          diagonal_rule_);
      num_directed_edges_ += neighbors.size();

      for (int i = 0; i < neighbors.size(); i++) {
        nodeId succ = grid_->ToLinearizedPaddedCoordinate(neighbors[i]);
        neighbors_[id].push_back(
            WeightedArcHead(succ, OctileDistance(ToState(id), ToState(succ))));
      }
    }
    */

    // Connected component analysis.
    //GraphConnectedComponentAnalyzer<Grid2DGraph> cc_analyzer(this);
    //in_largest_connected_component_ =
     //   *(cc_analyzer.GetLargestConnectedComponent());

  }
};

#endif /* APPS_SUBGOALGRAPH2DGRID_GRID2DGRAPH_H_ */
