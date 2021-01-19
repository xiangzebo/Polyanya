#include "MotionPrimitive.h"

using namespace std;

MotionPrimitive::MotionPrimitive(int sa, int ex, int ey, int ea,
                                 vector<xyThetaPosCont> & intermediate_poses,
                                 double cost_multiplier, double radius)
    : disc_start_orientation_(sa),
      delta_x_(ex),
      delta_y_(ey),
      disc_end_orientation_(ea),
      intermediate_poses_(intermediate_poses),
      cost_multiplier_(cost_multiplier),
      agent_radius_(radius) {
  // Compute the length.
  length_ = 0;
  for (unsigned int i = 1; i < intermediate_poses_.size(); i++) {
    double dx = intermediate_poses_[i].x - intermediate_poses_[i - 1].x;
    double dy = intermediate_poses_[i].y - intermediate_poses_[i - 1].y;

    length_ += sqrt(dx * dx + dy * dy);
  }

  CalculateCost();

  delta_node_id_ = 0;

  // Calculate the swept cells.
//	CalculateSweptCells();
  CalculateSweptCellsGivenRadius(agent_radius_);
}

// Calculates swept cells with a hacky method:
// If any of the discrete poses (start, intermediate, goal) is within the
// boundaries of a cell: include it in the list of swept cells.
void MotionPrimitive::CalculateSweptCells() {
  // TODO: Right now, we assume that the vehicle is a point.
  // For more realistic scenarios, this function should also take the
  // dimensions of the vehicle into account.
  // Assumes 0,0 is the center of the cell 0,0.

  // TODO: Move this to another class that generates the primitives?
  // Especially if the math gets too complex.

  swept_cells_.clear();

  for (unsigned int i = 0; i < intermediate_poses_.size(); i++) {
    double fx = intermediate_poses_[i].x;
    double fy = intermediate_poses_[i].y;

    // TODO: very hacky initial implementation below.
    int ix = round(fx);
    int iy = round(fy);

    //if ((double)ix - 0.5 < fx && fx < (double)ix + 0.5 &&
    //    (double)iy - 0.5 < fy && fy < (double)iy + 0.5)
    {
      bool duplicate = false;
      for (unsigned int i = 0; i < swept_cells_.size(); i++) {
        if (swept_cells_[i].x == ix && swept_cells_[i].y == iy) {
          duplicate = true;
          break;
        }
      }

      if (!duplicate)
        swept_cells_.push_back(xyPos(ix, iy));
    }
  }
}

void MotionPrimitive::CalculateSweptCellsGivenRadius(double radius) {

  LatticeGeometry geom;

  swept_cells_.clear();

  for (unsigned int i = 0; i < intermediate_poses_.size() - 1; i++) {
    xyPosCont start(intermediate_poses_[i].x, intermediate_poses_[i].y);
    xyPosCont end(intermediate_poses_[i + 1].x, intermediate_poses_[i + 1].y);

    std::vector<xyPos> new_swept_cells = geom.GetLineSweepIntersectedCells(
        start, end, radius);

    for (unsigned int j = 0; j < new_swept_cells.size(); j++) {
      xyPos new_cell = new_swept_cells[j];
      bool duplicate = false;
      for (unsigned int k = 0; k < swept_cells_.size(); k++) {
        if (swept_cells_[k].x == new_cell.x
            && swept_cells_[k].y == new_cell.y) {
          duplicate = true;
          break;
        }
      }
      if (!duplicate)
        swept_cells_.push_back(new_cell);
    }
  }
}

MotionPrimitive MotionPrimitive::GetReversePrimitive() {
  MotionPrimitive r;
  r.disc_start_orientation_ = disc_end_orientation_;
  r.delta_x_ = -delta_x_;
  r.delta_y_ = -delta_y_;
  r.disc_end_orientation_ = disc_start_orientation_;
  r.length_ = length_;
  r.cost_ = cost_;
  r.cost_multiplier_ = cost_multiplier_;
  r.agent_radius_ = agent_radius_;

  double x = delta_x_;
  double y = delta_y_;

  for (int i = intermediate_poses_.size() - 1; i >= 0; i--)
    r.intermediate_poses_.push_back(
        xyThetaPosCont(intermediate_poses_[i].x - x,
                       intermediate_poses_[i].y - y,
                       intermediate_poses_[i].orientation));

  for (int i = swept_cells_.size() - 1; i >= 0; i--)
    r.swept_cells_.push_back(
        xyPos(swept_cells_[i].x - delta_x_, swept_cells_[i].y - delta_y_));

  /*
   for (int i = linearized_padded_coordinate_offsets_.size() - 1; i >= 0; i--)
   r.linearized_padded_coordinate_offsets_.push_back(-linearized_padded_coordinate_offsets_[i]);
   */

  return r;
}

void MotionPrimitive::Print(ostream & out, bool print_intermediate_poses,
                            bool print_swept_cells,
                            bool print_swept_cell_offsets) {
  out << "Start pose: 0 0 " << disc_start_orientation_ << endl;
  out << "Goal pose: " << delta_x_ << " " << delta_y_ << " "
      << disc_end_orientation_ << endl;
  out << "Cost: " << cost_ << endl;

  if (print_intermediate_poses) {
    out << "Intermediate poses:" << endl;
    for (unsigned int i = 0; i < intermediate_poses_.size(); i++)
      out << intermediate_poses_[i].x << " " << intermediate_poses_[i].y << " "
          << intermediate_poses_[i].orientation << endl;
  }

  if (print_swept_cells) {
    out << "Swept cells:" << endl;
    for (unsigned int i = 0; i < swept_cells_.size(); i++)
      out << swept_cells_[i].x << " " << swept_cells_[i].y << endl;
  }

  if (print_swept_cell_offsets) {
    out << "Linearized padded coordinate offsets:" << endl;
    for (unsigned int i = 0;
        i < swept_cell_linearized_padded_coordinate_offsets_.size(); i++)
      out << swept_cell_linearized_padded_coordinate_offsets_[i] << endl;
  }
}

#ifndef NO_HOG
void MotionPrimitive::DrawPrimitive(const MapEnvironment *env, int x,
                                    int y) const {
  for (unsigned int i = 1; i < intermediate_poses_.size(); i++) {
    env->GLDrawColoredLine(
        fmax(intermediate_poses_[i - 1].x + x, 0.001),
        fmax(intermediate_poses_[i - 1].y + y, 0.001),
        fmax(intermediate_poses_[i].x + x, 0.001),
        fmax(intermediate_poses_[i].y + y, 0.001));
  }
}
#endif
