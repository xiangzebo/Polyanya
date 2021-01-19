/*
 * Linearize3D.h
 *
 *  Created on: Apr 11, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_UTILS_LINEARIZE3D_H_
#define APPS_SUBGOALGRAPH_UTILS_LINEARIZE3D_H_
#include <cstdint>


class Linearize3D {
 public:
  Linearize3D(int x_min, int x_max, int y_min, int y_max, int z_min,
              int z_max) {
    x_min_ = x_min;
    x_max_ = x_max;
    y_min_ = y_min;
    y_max_ = y_max;
    z_min_ = z_min;
    z_max_ = z_max;

    x_width_ = x_max_ - x_min_ + 1;
    y_width_ = y_max_ - y_min_ + 1;
    z_width_ = z_max_ - z_min_ + 1;

    x_mult_ = y_width_ * z_width_;
    y_mult_ = z_width_;
    offset_ = -(x_min_ * x_mult_ + y_min_ * y_mult_ + z_min_);
  }

  uint32_t ToLinear(int x, int y, int z) {
    return x_mult_*x + y_mult_*y + z + offset_;
  }
  void ExtractXYZ(uint32_t l, int & x, int & y, int & z) {
    x = x_min_ + l / x_mult_;
    y = y_min_ + (l % x_mult_) / y_mult_;
    z = z_min_ + (l % y_mult_);
  }

 private:
  int x_min_, x_max_, y_min_, y_max_, z_min_, z_max_;
  int x_width_, y_width_, z_width_;
  int x_mult_, y_mult_, offset_;
};


#endif /* APPS_SUBGOALGRAPH_UTILS_LINEARIZE3D_H_ */
