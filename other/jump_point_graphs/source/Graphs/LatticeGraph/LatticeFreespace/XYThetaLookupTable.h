/*
 * LookupTable3D.h
 *
 *  Created on: Mar 28, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_UTILS_XYTHETALOOKUPTABLE_H_
#define APPS_SUBGOALGRAPH_UTILS_XYTHETALOOKUPTABLE_H_
#include <cstdint>
#include <vector>

#include "../Lattice/LatticeDefinitions.h"
#include "FileReadWrite.h"
// Allows look up of values of type T, indexed by x, y, theta, where x and y
// can be negative values as well.


template<class T>
class XYThetaLookupTable {
 public:
  XYThetaLookupTable() {}

  void InitializeTable(int x_min, int x_max, int y_min, int y_max,
                       int num_angles, T def_val);
  void InitializeTableByExtendingFromOrigin(int width, int height,
                                            int num_angles, T def_val);

  template<class CanDropEntry>
  void ShrinkToFit(CanDropEntry can_drop_entry);
  void ShrinkToFit() {
    ShrinkToFit([&](T e)->bool {return e == uninitialized_value_;});
  }

  void SetValue(int x, int y, int ea, T val) {
    table_[Linearize(x, y, ea)] = val;
  }
  T GetValue(int x, int y, int ea) const {
    if (x_min_ <= x && x <= x_max_ && y_min_ <= y && y <= y_max_)
      return table_[Linearize(x, y, ea)];
    else
      return uninitialized_value_;
  }

  T GetValueDirectly(int x, int y, int ea) const {
    return table_[Linearize(x, y, ea)];
  }

  T GetValue(xyThetaPos p) const {
    return GetValue(p.x, p.y, p.o);
  }

  T operator [] (xyThetaPos & p) const {
    return table_[Linearize(p.x, p.y, p.o)];
  }
  T& operator [] (xyThetaPos & p) {
    return table_[Linearize(p.x, p.y, p.o)];
  }

  long long GetTableSize() const {
    return table_.size();
  }

  void Read(FileReadWrite & rw);
  void Write(FileReadWrite & rw) const ;

 private:
  std::vector<T> table_;

  T uninitialized_value_;
  int x_min_, x_max_, y_min_, y_max_;
  int width_, height_, num_angles_;
  int num_angle_bits_;

  uint32_t Linearize(int x, int y, int ea) const {
    return ((height_*(x - x_min_) + (y - y_min_)) << num_angle_bits_) | ea;
  }
};

#include "XYThetaLookupTable.inc"

#endif /* APPS_SUBGOALGRAPH_UTILS_XYTHETALOOKUPTABLE_H_ */
