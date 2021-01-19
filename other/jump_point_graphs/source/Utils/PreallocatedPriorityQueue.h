#ifndef PREALLOCATED_PRIORITY_QUEUE_H
#define PREALLOCATED_PRIORITY_QUEUE_H

//#define PPQ_DEBUG
//#define PPQ_VERBOSE

// A binary heap that preallocates memory for a fixed number of states. It assumes that each state has a unique unsigned integer identifier.
// The number of states should be specified during construction.

// TODO: Return cost as well? (to prevent duplicate cost storage).
// TODO: Allow cost increases as well?
#include "GraphDefinitions.h"

#include <vector>
#include <cassert>
#include <iostream>
#include <stdint.h>
#include <limits>

typedef uint32_t stateId;

// Cost can be any class. Double to specify f-values, or a pair of doubles for specifying f- and g-values (for tiebreaking).
// Less should have an operator() (Cost lhs, Cost rhs) that returns true if lhs < rhs.
// Max should have a function max() that returns a cost that is to be used as the infinite cost.

template<class Key, class Less, class Max>
class PreallocatedPriorityQueue {
 public:
  PreallocatedPriorityQueue(int num_states = 0);
  ~PreallocatedPriorityQueue();

  void SetupQueue(int num_states);
  void Reset();
  void StartNewSearch();

  // If not in heap, insert; otherwise, decrease key.
  void InsertOrDecreaseKey(stateId id, Key c);

  // If not in heap, insert; otherwise, increase/decrease key.
  void InsertOrUpdateKey(stateId id, Key new_key);

  stateId PopMin();
  stateId GetMin() {
    return pq_[0];
  }
  Key GetMinCost() const {
    return keys_[pq_[0]];
  }

  bool IsEmpty() {
    return pq_.empty();
  }
  bool HasElement(stateId id) {
    return indices_[id] != -1;
  }

  int GetSize() const {
    return pq_.size();
  }

  void Debug();

  int GetNumPercolations() const {
    return num_percolations_;
  }
  int GetNumUpdateKey() const {
    return num_update_key_;
  }

  bool Contains(stateId id) {
    return indices_[id] != -1;
  }

 private:
  Key worst_key_;
  Less less_;
  int num_percolations_;
  int num_update_key_;

  std::vector<int> indices_;
  std::vector<Key> keys_;
  std::vector<stateId> pq_;

  void PercolateUp(int index);
  void PercolateDown(int index);
};

#include "PreallocatedPriorityQueue.inc"

// Some common cost definitions.
struct FLess {
  bool operator()(const Distance &x, const Distance &y) const {
    return x < y;
  }
};
struct FMax {
  Distance max() {
    return std::numeric_limits<Distance>::max();
  }
};
typedef PreallocatedPriorityQueue<Distance, FLess, FMax> F_Val_PPQ;

struct FG_Cost {
  Distance f_val;
  Distance g_val;

  FG_Cost(Distance f_val_ = 0, Distance g_val_ = 0)
      : f_val(f_val_),
        g_val(g_val_) {
  }
};
struct FG_Less_Prefer_Higher_G {
  bool operator()(const FG_Cost & x, const FG_Cost & y) const {
    if (x.f_val + 0.0001 < y.f_val)
      return true;
    if (x.f_val > y.f_val + 0.0001)
      return false;
    return x.g_val > y.g_val;
  }
};
struct FG_Less_Prefer_Lower_G {
  bool operator()(const FG_Cost & x, const FG_Cost & y) const {
    if (x.f_val + 0.0001 < y.f_val)
      return true;
    if (x.f_val > y.f_val + 0.0001)
      return false;
    return x.g_val < y.g_val;
  }
};
struct FG_Max {
  FG_Cost max() {
    return FG_Cost(kMaxDistance,
                   kMaxDistance);
  }
};
typedef PreallocatedPriorityQueue<FG_Cost, FG_Less_Prefer_Higher_G, FG_Max> FG_Val_PPQ_Higher_G;
typedef PreallocatedPriorityQueue<FG_Cost, FG_Less_Prefer_Lower_G, FG_Max> FG_Val_PPQ_Lower_G;

class FG_Val_PPQ_Higher_G_Tester {
 public:
  FG_Val_PPQ_Higher_G_Tester() {
    FG_Val_PPQ_Higher_G ppq(3);

    ppq.Reset();
    ppq.InsertOrDecreaseKey(0, FG_Cost(5.6, 3));
    ppq.InsertOrDecreaseKey(1, FG_Cost(8.7, 5));
    ppq.InsertOrDecreaseKey(2, FG_Cost(5.6, 4));

    assert(ppq.PopMin() == 2);
    assert(ppq.PopMin() == 0);
    assert(ppq.PopMin() == 1);
    assert(ppq.IsEmpty());

    std::cout << "Priority queue test completed successfully!" << std::endl;
  }

  ~FG_Val_PPQ_Higher_G_Tester() {
  }
  ;
 private:
};

#endif
