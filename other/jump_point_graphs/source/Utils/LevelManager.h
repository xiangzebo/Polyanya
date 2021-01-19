/*
 * Level.h
 *
 *  Created on: Nov 14, 2017
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_UTILS_LEVELMANAGER_H_
#define APPS_SUBGOALGRAPH_UTILS_LEVELMANAGER_H_
#include <vector>

#include "FileReadWrite.h"
// Maintains the levels of every node, and the highest level.
// Can answer level-related questions.

typedef int level;
const level kInvalidLevel = -1;
const level kQueryNodeLevel = 0;
const level kBaseLevel = 1;
const level kHighLevel = std::numeric_limits<level>::max()/2;

class LevelManager {
 public:
  LevelManager(int num_nodes = 0) {
    GenerateInitialLevels(num_nodes);
  }
  ~LevelManager() {}


  void Write(FileReadWrite & rw) {
    rw.Write(num_nodes_);
    rw.WriteVector(levels_);
    rw.Write(max_level_);
    rw.Write(is_initialized_);
    rw.Write(query_mode_);
  }
  void Read(FileReadWrite & rw) {
    rw.Read(num_nodes_);
    rw.ReadVector(levels_);
    rw.Read(max_level_);
    rw.Read(is_initialized_);
    rw.Read(query_mode_);
  }

  void Write(std::string filename) {
    FileReadWrite rw;
    rw.StartWrite(filename);
    Write(rw);
    rw.CloseWrite();
  }
  void Read(std::string filename) {
    FileReadWrite rw;
    rw.StartRead(filename);
    Read(rw);
    rw.CloseRead();
  }


  void GenerateInitialLevels(int num_nodes, level l = kBaseLevel) {
    num_nodes_ = num_nodes;
    levels_.clear();
    levels_.resize(num_nodes, l);
    is_initialized_ = num_nodes > 0;
    max_level_ = l;
    query_mode_ = false;
    AddQueryNodeLevels();
  }

  bool IsInitialized() const {
    return is_initialized_;
  }

  void AddQueryNodeLevels() {
    if (!query_mode_) {
      query_mode_ = true;
      levels_.push_back(kQueryNodeLevel);
      levels_.push_back(kQueryNodeLevel);
    }
  }


  void SetLevel(nodeId n, level l) {
    levels_[n] = l;
    IncreaseHighestLevel(l);
  }
  void SetToMaxLevel(nodeId n) {
    levels_[n] = max_level_;
  }
  void SetToInvalidLevel(nodeId n) {
    levels_[n] = kInvalidLevel;
  }
  void IncreaseLevel(nodeId n, level l) {
    if (levels_[n] < l)
      SetLevel(n,l);
  }
  void DecrementLevel(nodeId n) {
    levels_[n]--;
  }
  void IncrementMaxLevel() {
    max_level_++;
  }

  void IncrementLevelsOfAllHighestLevelNodes() {
    for (nodeId n = 0; n < num_nodes_; n++)
      if (levels_[n] == max_level_)
        levels_[n]++;
    max_level_++;
  }
  void DecrementLevelsOfAllHighestLevelNodes() {
    for (nodeId n = 0; n < num_nodes_; n++)
      if (levels_[n] == max_level_)
        levels_[n]--;
      else
        assert(
            levels_[n] != max_level_ - 1
                || "Shouldn't decrement highest level if it results in ambiguous levels!");
    max_level_--;
  }

  int GetLevel(nodeId n) const {
    return levels_[n];
  }
  int GetMaxLevel() const {
    return max_level_;
  }
  bool IsHighestLevel(nodeId n) const {
    return levels_[n] == max_level_;
  }
  bool IsStrictlyLowerLevel(nodeId n, nodeId m) const {
    return levels_[n] < levels_[m];
  }
  bool SameLevel(nodeId n, nodeId m) const {
    return levels_[n] == levels_[m];
  }

  // Mostly used for visualization.
  double GetRatioToMaxLevel(nodeId n) const {
    if (max_level_ == kBaseLevel)
      return 0;
    if (levels_[n] == kInvalidLevel)
      return 0;

    return (levels_[n]-kBaseLevel) / (double) (max_level_ - kBaseLevel);
  }

 private:
  int num_nodes_;
  std::vector<level> levels_;
  level max_level_;
  bool is_initialized_;
  bool query_mode_;

  void IncreaseHighestLevel(level l) {
    if (l > max_level_)
      max_level_ = l;
  }

};



#endif /* APPS_SUBGOALGRAPH_UTILS_LEVELMANAGER_H_ */
