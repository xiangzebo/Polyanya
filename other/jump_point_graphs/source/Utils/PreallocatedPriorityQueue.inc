template<class Key, class Less, class Max>
PreallocatedPriorityQueue<Key, Less, Max>::PreallocatedPriorityQueue(
    int num_states) {
  Max m;
  worst_key_ = m.max();
  Less l;
  less_ = l;
  SetupQueue(num_states);
  num_percolations_ = 0;
  num_update_key_ = 0;
}

template<class Key, class Less, class Max>
PreallocatedPriorityQueue<Key, Less, Max>::~PreallocatedPriorityQueue() {

}

template<class Key, class Less, class Max>
void PreallocatedPriorityQueue<Key, Less, Max>::SetupQueue(int num_states) {

#ifdef PPQ_VERBOSE
  std::cout<<"Setting up PPQ"<<std::endl;
#endif

  indices_.resize(num_states);
  keys_.resize(num_states);
  pq_.reserve(num_states);

  for (int i = 0; i < num_states; i++) {
    indices_[i] = -1;
    keys_[i] = worst_key_;
  }

#ifdef PPQ_DEBUG
  Debug();
#endif
}

template<class Key, class Less, class Max>
void PreallocatedPriorityQueue<Key, Less, Max>::Reset() {
  for (unsigned int i = 0; i < pq_.size(); i++) {
    int s = pq_[i];
    indices_[s] = -1;
    keys_[s] = worst_key_;
  }
  pq_.clear();
  num_percolations_ = 0;
  num_update_key_ = 0;
}

template<class Key, class Less, class Max>
void PreallocatedPriorityQueue<Key, Less, Max>::InsertOrDecreaseKey(
    const stateId id, Key c) {
#ifdef PPQ_VERBOSE
  std::cout<<"Add to open: id: "<<id<<", cost: "<<c<<std::endl;
#endif
  num_update_key_++;
  int index = indices_[id];

  // If it is already in the open list, update its location.
  if (index != -1) {
    keys_[id] = c;
    PercolateUp(index);
  }

  // Otherwise, add it to the open list
  else {
    indices_[id] = pq_.size();
    keys_[id] = c;
    pq_.push_back(id);
    PercolateUp(pq_.size() - 1);
  }

#ifdef PPQ_DEBUG
  Debug();
#endif
}

template<class Key, class Less, class Max>
void PreallocatedPriorityQueue<Key, Less, Max>::InsertOrUpdateKey(
    const stateId id, Key c) {
#ifdef PPQ_VERBOSE
  std::cout<<"Add to open: id: "<<id<<", cost: "<<c<<std::endl;
#endif
  num_update_key_++;
  int index = indices_[id];

  // If it is already in the open list, update its location.
  if (index != -1) {
    if (less_(c, keys_[id])) {
      keys_[id] = c;
      PercolateUp(index);
    } else {
      keys_[id] = c;
      PercolateDown(index);
    }
  }

  // Otherwise, add it to the open list
  else {
    indices_[id] = pq_.size();
    keys_[id] = c;
    pq_.push_back(id);
    PercolateUp(pq_.size() - 1);
  }

#ifdef PPQ_DEBUG
  Debug();
#endif
}

template<class Key, class Less, class Max>
stateId PreallocatedPriorityQueue<Key, Less, Max>::PopMin() {
#ifdef PPQ_VERBOSE
  std::cout<<"Pop min: "<<std::endl;
#endif

  // Node is no longer in the open list.
  stateId s = pq_[0];
  indices_[s] = -1;
  keys_[s] = worst_key_;

  if (pq_.size() == 0)
    return -1;

  else if (pq_.size() == 1) {
    pq_.pop_back();
  } else {

    pq_[0] = pq_.back();
    indices_[pq_[0]] = 0;
    pq_.pop_back();
    PercolateDown(0);
  }

#ifdef PPQ_DEBUG
  Debug();
#endif
  return s;
}

template<class Key, class Less, class Max>
void PreallocatedPriorityQueue<Key, Less, Max>::PercolateUp(int index) {
#ifdef PPQ_VERBOSE
//    std::cout<<"Percolating up from index: "<<index<<std::endl;
#endif

  stateId id = pq_[index];
  int parent = (index - 1) >> 1;

  while (index > 0 && less_(keys_[id], keys_[pq_[parent]])) {
    pq_[index] = pq_[parent];
    indices_[pq_[index]] = index;

    index = parent;
    parent = (index - 1) >> 1;

    num_percolations_++;
  }
  pq_[index] = id;
  indices_[id] = index;

#ifdef PPQ_DEBUG
//    Debug();
#endif
}

template<class Key, class Less, class Max>
void PreallocatedPriorityQueue<Key, Less, Max>::PercolateDown(int index) {

#ifdef PPQ_VERBOSE
//    std::cout<<"Percolating down from index: "<<index<<std::endl;
#endif

  stateId id = pq_[index];
  int max_size = pq_.size();

  while (true) {
    int child1 = (index << 1) + 1;
    if (child1 >= max_size)
      break;

    int child2 = child1 + 1;

    // If the first child has smaller key
    if (child2 == max_size || less_(keys_[pq_[child1]], keys_[pq_[child2]])) {
      if (less_(keys_[pq_[child1]], keys_[id])) {
        pq_[index] = pq_[child1];
        indices_[pq_[index]] = index;
        index = child1;
        num_percolations_++;
      } else
        break;
    }

    else if (less_(keys_[pq_[child2]], keys_[id])) {
      pq_[index] = pq_[child2];
      indices_[pq_[index]] = index;
      index = child2;
      num_percolations_++;
    } else
      break;
  }

  pq_[index] = id;
  indices_[id] = index;

#ifdef PPQ_DEBUG
//     Debug();
#endif
}

template<class Key, class Less, class Max>
void PreallocatedPriorityQueue<Key, Less, Max>::Debug() {
  /*
   for (int i = 0; i < f_vals_.size(); i++) {
   if (indices_[i] != -1) {
   cout<<i<<":\t"<<indices_[i];
   if (f_vals_[i] != infinite_cost_)
   cout<<"\t"<<f_vals_[i];
   cout<<endl;
   }
   //        assert(f_vals_[i] == infinite_cost_);
   //        assert(indices_[i] == -1);
   }
   */
#ifdef PPQ_VERBOSE
  std::cout<<"PQ: ";
  for (int i = 0; i < pq_.size(); i++)
  std::cout<<pq_[i]<<"("<<keys_[pq_[i]]<<") ";

  std::cout<<std::endl;
#endif

  for (int i = 0; i < keys_.size(); i++) {
    if (indices_[i] != -1) {
      if (pq_.size() <= indices_[i]) {
        std::cout << "Index out of bounds for state " << i << ". Index: "
            << indices_[i] << ". Size of pq: " << pq_.size() << std::endl;
      } else if (pq_[indices_[i]] != i) {
        std::cout << "Non matching index for state " << i << ". Index: "
            << indices_[i] << ". Size of pq: " << pq_.size() << std::endl;
      }
    }
  }
}
