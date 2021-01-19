/*
 * XYThetaLookupTable.cpp
 *
 *  Created on: Apr 11, 2018
 *      Author: idm-lab
 */
#include "../../../../../utils/FPUtil.h"

template<class T>
void XYThetaLookupTable<T>::InitializeTable(int x_min, int x_max, int y_min,
                                         int y_max, int num_angles, T def_val) {
  assert(x_max >= x_min);
  assert(y_max >= y_min);
  x_min_ = x_min;
  x_max_ = x_max;
  y_min_ = y_min;
  y_max_ = y_max;
  width_ = x_max_ - x_min_ + 1;
  height_ = y_max_ - y_min_ + 1;

  assert(num_angles >= 1);
  num_angles_ = num_angles;
  num_angle_bits_ = 0;
  while (num_angles >>= 1)
    num_angle_bits_ ++;

  uninitialized_value_ = def_val;

  table_.resize(width_*height_*num_angles_, def_val);
}

template<class T>
void XYThetaLookupTable<T>::InitializeTableByExtendingFromOrigin(int x_stretch,
                                                                 int y_stretch,
                                                                 int num_angles,
                                                                 T def_val) {
  assert(x_stretch >= 0);
  assert(y_stretch >= 0);
  assert(num_angles >= 1);
  InitializeTable(-x_stretch, x_stretch, -y_stretch, y_stretch,
                  num_angles, def_val);
}

template<class T>
template<class CanDropEntry>
void XYThetaLookupTable<T>::ShrinkToFit(CanDropEntry can_drop_entry) {
  int new_x_min = std::numeric_limits<int>::max();
  int new_x_max = std::numeric_limits<int>::min();
  int new_y_min = std::numeric_limits<int>::max();
  int new_y_max = std::numeric_limits<int>::min();
  for (int x = x_min_; x <= x_max_; x++) {
    for (int y = y_min_; y <= y_max_; y++) {
      for (int a = 0; a < num_angles_; a++) {
        if (!can_drop_entry(GetValue(x, y, a))) {
          new_x_min = min(new_x_min, x);
          new_x_max = max(new_x_max, x);
          new_y_min = min(new_y_min, y);
          new_y_max = max(new_y_max, y);
        }
      }
    }
  }

  // If no value exists, create a 1x1 table.
  if (new_x_min > new_x_max) {
    InitializeTable(0,0,0,0,num_angles_, uninitialized_value_);
    return;
  }

  // Otherwise, shrink the table.
  int new_width = new_x_max - new_x_min + 1;
  int new_height = new_y_max - new_y_min + 1;
  auto NewLinearize =
      [&](int x, int y, int a) -> uint32_t {
        return ((new_height*(x - new_x_min) + (y - new_y_min)) << num_angle_bits_) | a;
      };
  for (int x = new_x_min; x <= new_x_max; x++)
    for (int y = new_y_min; y <= new_y_max; y++)
      for (int a = 0; a < num_angles_; a++)
        table_[NewLinearize(x,y,a)] = table_[Linearize(x,y,a)];
  x_min_ = new_x_min;
  x_max_ = new_x_max;
  y_min_ = new_y_min;
  y_max_ = new_y_max;
  width_ = new_width;
  height_ = new_height;
  table_.resize(width_*height_*num_angles_);
  table_.shrink_to_fit();
}

template<class T>
void XYThetaLookupTable<T>::Read(FileReadWrite & rw) {
  rw.Read(uninitialized_value_);
  rw.Read(x_min_);
  rw.Read(x_max_);
  rw.Read(y_min_);
  rw.Read(y_max_);
  rw.Read(width_);
  rw.Read(height_);
  rw.Read(num_angles_);
  rw.Read(num_angle_bits_);
  rw.ReadVector(table_);
}
template<class T>
void XYThetaLookupTable<T>::Write(FileReadWrite & rw) const {
  rw.Write(uninitialized_value_);
  rw.Write(x_min_);
  rw.Write(x_max_);
  rw.Write(y_min_);
  rw.Write(y_max_);
  rw.Write(width_);
  rw.Write(height_);
  rw.Write(num_angles_);
  rw.Write(num_angle_bits_);
  rw.WriteVector(table_);
}
