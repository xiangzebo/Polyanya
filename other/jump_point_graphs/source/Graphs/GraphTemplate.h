#ifndef GRAPH_TEMPLATE_H
#define GRAPH_TEMPLATE_H

#include "GraphDefinitions.h"

// This template is not used as part of the code.
// It mainly identifies the common methods that any graph should implement
// which are required by search algorithms.
//
// Each node is identified by a nodeId.
// GetNumNodes() should return the maximum nodeId.
//
// For any n <= GetNumNodes(), it doesn't necessarily hold that n is actually
// a node. IsValidNode(n) should return whether n is part of the graph or not.
//
// This structure has several benefits:
// - Grids and state lattices can use linearized coordinates as nodeIds
//   - Convenient
//   - Doesn't require storing mappings between nodeId's and xyLoc/xyThetaPos'es
//   - Allows storing 'delta coordinates' for directions (on grids) and
//     primitives (on state lattices). This helps with successor generation.
//       - For state-lattices, we can use 'valid primitive flags' to store
//         neighbors (1 bit per primitive) to avoid sweep-checks.
//       - For grids, we can use delta-coordinates to quickly
// - We can post-process graphs without much hassle:
//   - Remove all but the nodes in the largest connected component

// For each node 'from' GetSuccessors/Predecessors returns a list of 'to'/cost
// pairs. This could be implemented as a pointer to such a list for more
// efficiency, but the current formulation makes it easier to deal with both
// implicit (grid or lattice, which generate neighbors on the fly)
// and explicit (SG, CH which actually store edges) graphs.

class GraphTemplate {
 public:
  GraphTemplate() {
  }
  virtual ~GraphTemplate() {
  }

  virtual int GetNumNodes() const = 0;
  virtual bool IsValidNode() const = 0;

  virtual void GetSuccessors(nodeId n,
                             std::vector<WeightedArcHead> & neighbors) const = 0;
  virtual void GetPredecessors(nodeId n,
                               std::vector<WeightedArcHead> & neighbors) const = 0;

  virtual std::string GetNodeName(nodeId n) const = 0;

  // Requires a method of conversion between nodeId and state
  // (xyLoc, xyThetaPos, etc).
  // This doesn't work with inheritance, unless we template the graph with
  // the state type, but we are not using inheritance anyway at this point.

  virtual void DrawNode(nodeId n) const = 0;
  virtual void DrawEdge(nodeId from, nodeId to) const = 0;

 private:

};
#endif
