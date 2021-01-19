#ifndef H_REACHABLE_AREA_EXPLORATION_DATA_H
#define H_REACHABLE_AREA_EXPLORATION_DATA_H

/*
 * AreaExplorer uses this data to for storing search related data.
 * Can be passed to SubgoalSelector so that it can try to cover paths to fringe vertices with subgoals.
 */

#include <vector>
#include <stdint.h>
#include "Flags.h"
#include "GraphDefinitions.h"

const uint8_t kIsGenerated = 1;
const uint8_t kIsReachable = 2;
const uint8_t kIsCovered = 4;
const uint8_t kHasReachableParent = 8;
const uint8_t kHasFringeParent = 16;

const uint8_t kIsOpen = 32;
const uint8_t kIsClosed = 64;

const uint8_t kIsStalled = 128;

class AreaExplorationData {
 public:
  AreaExplorationData() {
    num_nodes_ = 0;
  }
  ~AreaExplorationData() {
  }

  void SetupData(int num_nodes) {
    num_nodes_ = num_nodes;
    flags_.resize(num_nodes_);
    g_val_.resize(num_nodes_);
    num_pred_.resize(num_nodes_);

    for (int i = 0; i < num_nodes_; i++)
      CleanData(i);
  }
  void CleanData(nodeId n) {
    flags_[n].ResetFlags();
    g_val_[n] = std::numeric_limits<double>::max();
    num_pred_[n] = 0;
  }
  void Reset() {
    for (unsigned int i = 0; i < generated_nodes_.size(); i++) {
      CleanData(generated_nodes_[i]);
    }
    generated_nodes_.clear();
  }

  bool GenerateNode(nodeId n) {
    if (!GetFlag(n, kIsGenerated)) {
      SetFlag(n, kIsGenerated);
      generated_nodes_.push_back(n);
      return true;
    }
    return false;
  }
  bool IsGenerated(nodeId n) const {
    return flags_[n].GetFlag(kIsGenerated);
  }

  void SetGVal(nodeId n, double g_val) {
    g_val_[n] = g_val;
  }
  double GetGVal(nodeId n) const {
    return g_val_[n];
  }

  void SetNumPred(nodeId n, int num_pred) {
    num_pred_[n] = num_pred;
  }
  void IncrementNumPred(nodeId n) {
    num_pred_[n]++;
  }
  int GetNumPred(nodeId n) const {
    return num_pred_[n];
  }

  void SetReachable(nodeId n) {
    flags_[n].SetFlag(kIsReachable);
  }
  bool IsReachable(nodeId n) const {
    return flags_[n].GetFlag(kIsReachable);
  }

  bool IsDirectReachable(nodeId n) const {
    return IsReachable(n) && !IsCovered(n);
  }
  bool IsFringeNode(nodeId n) const {
    return !IsReachable(n) && !IsCovered(n) && HasReachableParent(n);
  }

  void SetCovered(nodeId n) {
    flags_[n].SetFlag(kIsCovered);
  }
  void UnsetCovered(nodeId n) {
    flags_[n].UnsetFlag(kIsCovered);
  }
  bool IsCovered(nodeId n) const {
    return flags_[n].GetFlag(kIsCovered);
  }

  void SetStalled(nodeId n) {
    flags_[n].SetFlag(kIsStalled);
  }
  bool IsStalled(nodeId n) const {
    return flags_[n].GetFlag(kIsStalled);
  }

  void SetHasReachableParent(nodeId n) {
    flags_[n].SetFlag(kHasReachableParent);
  }
  void UnsetHasReachableParent(nodeId n) {
    flags_[n].UnsetFlag(kHasReachableParent);
  }
  bool HasReachableParent(nodeId n) const {
    return flags_[n].GetFlag(kHasReachableParent);
  }

  void SetFlag(nodeId n, uint8_t flag_mask) {
    flags_[n].SetFlag(flag_mask);
  }
  void UnsetFlag(nodeId n, uint8_t flag_mask) {
    flags_[n].UnsetFlag(flag_mask);
  }
  bool GetFlag(nodeId n, uint8_t flag_mask) const {
    return flags_[n].GetFlag(flag_mask);
  }
  void SetFlagIf(nodeId n, uint8_t flag_mask, bool is_true) {
    flags_[n].SetFlagIf(flag_mask, is_true);
  }

 private:
  int num_nodes_;

  std::vector<double> g_val_;
  std::vector<uint8_t> num_pred_;  // TODO: Specific for safe-freespace-reachability: Move somewhere else.
  std::vector<Flags<uint8_t> > flags_;
  std::vector<nodeId> generated_nodes_;
};

#endif
