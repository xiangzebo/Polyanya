/*
 * ThetaXYThetaLookupTable.h
 *
 *  Created on: Mar 28, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_GRAPHS_LATTICE_THETAXYTHETALOOKUPTABLE_H_
#define APPS_SUBGOALGRAPH_GRAPHS_LATTICE_THETAXYTHETALOOKUPTABLE_H_
#include "XYThetaLookupTable.h"

// Allows lookup of values of type T, indexed by theta1, x, y, theta2.
// Can be used to store various information relating (0,0,theta1) to (x,y,theta2).

template<class T>
class ThetaXYThetaLookupTable {
 public:
  ThetaXYThetaLookupTable() {
    initialized_ = false;
    num_angles_ = 0;
  }

  bool IsInitialized() const {return initialized_;}

  void InitializeTableByExtendingFromOrigin(int width, int height,
                                            int num_angles, T def_val) {
    num_angles_ = num_angles;
    table_.resize(num_angles);
    for (auto& t : table_)
      t.InitializeTableByExtendingFromOrigin(width, height, num_angles,
                                              def_val);
    initialized_ = true;
    uninitialized_value_ = def_val;
  }
  void InitializeTableByExtendingFromOrigin(int bound, int num_angles,
                                            T def_val) {
    InitializeTableByExtendingFromOrigin(bound, bound, num_angles, def_val);
  }

  void DeleteTable() {
    table_.clear();
    table_.shrink_to_fit();
    initialized_ = false;
  }

  template<class CanDropEntry>
  void ShrinkToFit(CanDropEntry can_drop_entry) {
    if (initialized_)
      for (auto& t : table_)
        t.ShrinkToFit(can_drop_entry);
  }
  void ShrinkToFit() {
    ShrinkToFit([&](T e)->bool {return e == uninitialized_value_;});
  }

  void SetValue(int sa, int x, int y, int ea, T val) {
    table_[sa].SetValue(x, y, ea, val);
  }
  void SetValue(int sa, xyThetaPos & p, T val) {
    SetValue(sa, p.x, p.y, p.o, val);
  }
  T GetValue(int sa, int x, int y, int ea) const {
    if (initialized_)
      return table_[sa].GetValue(x, y, ea);
    else
      return uninitialized_value_;
  }
  T GetValue(int sa, xyThetaPos & p) const {
    return GetValue(sa, p.x, p.y, p.o);
  }
  XYThetaLookupTable<T> operator [] (int sa) const {
    return table_[sa];
  }
  XYThetaLookupTable<T>& operator [] (int sa) {
    return table_[sa];
  }

  long long GetTableSize(bool reduce_num_tables = false, bool reduce_table_size = false) const {
    int num_tables_to_sum = table_.size();

    if (reduce_num_tables) {      // 90 degree rotational symmetry
      if (num_angles_ == 4)
        num_tables_to_sum = 1;
      if (num_angles_ == 8)
        num_tables_to_sum = 2;
      if (num_angles_ == 16)
        num_tables_to_sum = 3;
      if (num_angles_ == 32)
        num_tables_to_sum = 5;
      if (num_angles_ == 64)
        num_tables_to_sum = 9;
    }

    long long table_size = 0;
    for (int i = 0; i < num_tables_to_sum && i < table_.size(); i++)
      table_size += GetSubTableSize(i, reduce_table_size);

    //for (auto& t : table_)
    //  table_size += t.GetTableSize();

    return table_size;
  }

  long long GetSubTableSize(int o, bool reduce_table_size = false) const {
    double reduction_factor = 1;
    if (reduce_table_size) {
      if (num_angles_ == 4)
        reduction_factor = 0.5;
      else if (num_angles_ % 8 == 0) {
        if (o % (num_angles_/8) == 0)
          reduction_factor = 0.5;
      }
    }
    return table_[o].GetTableSize() * reduction_factor;
  }

  void Read(FileReadWrite & rw) {
    rw.Read(initialized_);
    if (initialized_) {
      rw.Read(uninitialized_value_);
      rw.Read(num_angles_);
      table_.resize(num_angles_);
      for (auto& t : table_)
        t.Read(rw);
    }
  }
  void Write(FileReadWrite & rw) const {
    rw.Write(initialized_);
    if (initialized_) {
      rw.Write(uninitialized_value_);
      rw.Write(num_angles_);
      for (auto& t : table_)
        t.Write(rw);
    }
  }

 private:
  bool initialized_;
  int num_angles_;
  T uninitialized_value_;
  std::vector<XYThetaLookupTable<T> > table_;
};

#endif /* APPS_SUBGOALGRAPH_GRAPHS_LATTICE_THETAXYTHETALOOKUPTABLE_H_ */
