#include <Linearize3D.h>
#include <PreallocatedPriorityQueue.h>
#include <algorithm>
#include "Lattice.h"

using namespace std;

void Lattice::ReadPrimitivesFromFile(string mprim_filename,
		bool is_undirected) {
	is_undirected_ = is_undirected; // TODO: Determine this on the fly?

	cout << "Reading primitive file: " << mprim_filename << endl;
	bool verbose = false;

	FILE *f;
	f = fopen(mprim_filename.c_str(), "r");
	if (f) {
		int num_prims;
		double grid_resolution;

		fscanf(f,
				"resolution\_m: %lf\nnumberofangles: %d\ntotalnumberofprimitives: %d\n",
				&grid_resolution, &num_discrete_angles_, &num_prims);
		cout << "resolution: " << grid_resolution << "\tnumber of angles: "
				<< num_discrete_angles_ << "\tnumber of primitives: "
				<< num_prims << endl;

		prims_.resize(num_discrete_angles_);
		orientation_to_radian_.resize(num_discrete_angles_);

		int i = 0;
		while (i < num_prims) {
			// Read prim info. (ip = number of intermediate poses)
			int id, sa, ex, ey, ea, ip;
			double cm; // additional cost multiplier.

			fscanf(f,
					"primID: %d\nstartangle\_c: %d\nendpose\_c: %d %d %d\nadditionalactioncostmult: %lf\nintermediateposes: %d",
					&id, &sa, &ex, &ey, &ea, &cm, &ip);

			if (sa < 0)
				sa += num_discrete_angles_;
			if (ea < 0)
				ea += num_discrete_angles_;

			//printf("%d (%d): (0, 0, %d) -> (%d, %d, %d), *%lf, %d", i, id, sa, ex, ey, ea, cm, ip);

			vector<xyThetaPosCont> intermediate_poses;

			// Add intermediate poses for the prim.
			for (int j = 0; j < ip; j++) {
				double ix, iy, ia; // intermediate pose.
				fscanf(f, "\n%lf %lf %lf", &ix, &iy, &ia);

				intermediate_poses.push_back(
            xyThetaPosCont(ix / grid_resolution, iy / grid_resolution, ia));

				// Extract the start radian
				if (j == 0)
					orientation_to_radian_[sa] = ia;
			}

			// Generate the motion primitive.
			MotionPrimitive p(sa, ex, ey, ea, intermediate_poses, cm, agent_radius_);
			prims_[sa].push_back(p);

			i++;
			if (i < num_prims)
				fscanf(f, "\n");

			if (verbose) {
				p.Print();
			}
		}
		fclose(f);
	}
#ifndef SG_QUIET
	for (int a = 0; a < num_discrete_angles_; a++)
		cout << "Orientation: " << a << "\tRadian: "
				<< orientation_to_radian_[a] << endl;
#endif
}

// TODO: Set of valid nodes do not contain odd poses.
// TODO: A bit hardcoded for Max's urban challenge primitives.
void Lattice::Read4DPrimitivesFromFile(string mprim_filename,
    bool is_undirected) {
  is_undirected_ = is_undirected; // TODO: Determine this on the fly?
  int reverse_multiplier = 3;

  cout << "Reading primitive 4D file: " << mprim_filename << endl;
  bool verbose = false;
  ifstream in (mprim_filename.c_str());

  if (in) {
    string s;
    int num_prims;
    double grid_resolution;

    // FIXME: Hardcoded 64
    num_discrete_angles_ = 32;
    prims_.resize(num_discrete_angles_);
    orientation_to_radian_.resize(num_discrete_angles_);

    in >> s >> grid_resolution;
    in >> s >> num_prims;
    getline(in, s);
//    cout << "Resolution: " << grid_resolution << ", number of prims: "
//         << num_prims << endl;

    vector<MotionPrimitive> prims;

    auto get_discrete_pose = [&] (int t, int v) -> int {
      // Assume that velocity is either 0 or 1.
      return t;//*2 + v;
    };

    for (int i = 0; i < num_prims; i++) {
      int x, y, t, v, num_intermediate_poses, num_intermediate_cells,
          num_intersecting_cells;
      MotionPrimitive p;

      getline(in, s);
      getline(in, s);

      in >> s >> x >> y >> t >> v;
      p.disc_start_orientation_ = get_discrete_pose(t,v);

      in >> s >> p.delta_x_ >> p.delta_y_ >> t >> v;
      p.disc_end_orientation_ = get_discrete_pose(t,v);

      in >> s >> p.length_;

      // FIXME: Hardcoded to make sure cost 1 travels at most 1 cell
      p.cost_multiplier_ = 4*((v == 1) ? 1 : reverse_multiplier);
      p.cost_ = p.length_ * p.cost_multiplier_;

      getline(in, s);
      getline(in, s);
      getline(in, s);
      getline(in, s);
      in >> s >> num_intermediate_poses;

      double fx, fy, ft, fv;
      double x_off, y_off;
      //cout<<"Num intermediate poses: "<<num_intermediate_poses<<endl;
      for (int j = 0; j < num_intermediate_poses; j++) {
        in >> fx >> fy >> ft >> fv;
        // Extract the start radian
        if (j == 0) {
          orientation_to_radian_[p.disc_start_orientation_] = ft;
          x_off = fx / grid_resolution;
          y_off = fy / grid_resolution;
        }
        p.intermediate_poses_.push_back(
            xyThetaPosCont(fx / grid_resolution - x_off,
                           fy / grid_resolution - y_off, ft));
      }
      getline(in, s);

      in >> s >> num_intermediate_cells;
      //cout<<"Num intermediate cells: "<<num_intermediate_cells<<endl;
      for (int j = 0; j < num_intermediate_cells; j++) {
        getline(in, s);
        getline(in, s);
      }

      in >> s >> num_intersecting_cells;
      //cout<<"Num intersecting cells: "<<num_intersecting_cells<<endl;
      for (int j = 0; j < num_intersecting_cells; j++) {
        in >> x >> y;
        p.swept_cells_.push_back(xyPos(x,y));
      }
      getline(in, s);

      p.agent_radius_ = agent_radius_;
      p.CalculateSweptCellsGivenRadius(agent_radius_);

//      if (i < 31)
        prims_[p.disc_start_orientation_].push_back(p);
    }
  }

  in.close();
#ifndef SG_QUIET
  for (int a = 0; a < num_discrete_angles_; a++) {
    cout << "Orientation: " << a << "\tRadian: " << orientation_to_radian_[a]
         << "\tnum prims: " << prims_[a].size() << endl;
  }
#endif
}

void Lattice::RoundPrimitiveCosts(double d) {
  for (int start_a = 0; start_a < num_discrete_angles_; start_a++) {
    for (auto &p : prims_[start_a]) {
      double c = p.GetCost();
      int i = round(c/d);
      p.cost_ = d*i;

      //cout << "Rounding " << c << " to "<<p.cost_<<"; i = "<<i<<endl;
    }
  }
}


void Lattice::ShrinkPrimitives() {
  for (int start_a = 0; start_a < num_discrete_angles_; start_a++) {
    for (auto &p: prims_[start_a]) {

//      auto calculate_gcd = [&](int a, int b)->int {
//        return b == 0 ? a : calculate_gcd(b, a % b);
//      };

      if (p.disc_end_orientation_ == start_a) {
        int gcd = __gcd(abs(p.delta_x_), abs(p.delta_y_));
        if (p.delta_x_ == 0)
          gcd = abs(p.delta_y_);
        if (p.delta_y_ == 0)
          gcd = abs(p.delta_x_);

        for (int i = 1; i < p.intermediate_poses_.size(); i++) {
          if (fabs(
              p.intermediate_poses_[i - 1].orientation
                  - p.intermediate_poses_[i].orientation) > 0.001)
            gcd = 1;
        }

        if (gcd > 1) {
          std::cout << "Shrinking primitive with end loc (" << p.delta_x_
                    << ", " << p.delta_y_ << ")." << std::endl;
          p.delta_x_ /= gcd;
          p.delta_y_ /= gcd;
          for (auto &ip : p.intermediate_poses_) {
            ip.x  /= gcd;
            ip.y  /= gcd;
          }
          p.length_ /= gcd;
          p.cost_ /= gcd;
          p.CalculateSweptCellsGivenRadius(agent_radius_);
        }
      }
    }
  }
}


void Lattice::EliminateRedundantPrimitives(bool eliminate_if_same_cost) {
  // For each starting angle:
  for (int start_a = 0; start_a < num_discrete_angles_; start_a++) {
    int d = GetMaxPrimitiveReach();

    Linearize3D l(-d, d, -d, d, 0, num_discrete_angles_-1);
    auto ToNodeId = [&](xyThetaPos p) -> nodeId {
      return l.ToLinear(p.x, p.y, p.o);
    };
    auto ToXYThetaPos = [&](nodeId & n) -> xyThetaPos {
      xyThetaPos p;
      l.ExtractXYZ(n, p.x, p.y, p.o);
      return p;
    };

    int num_entries = (d*2 + 1) * (d*2 + 1) * num_discrete_angles_;
    F_Val_PPQ ppq_(num_entries);
    std::vector<Distance> dist(num_entries, kMaxDistance);
    std::vector<int> parent_count(num_entries, 0);

    xyThetaPos start(0, 0, start_a);

    //Dijkstra
    ppq_.Reset();
    nodeId start_id = ToNodeId(start);
    ppq_.InsertOrDecreaseKey(start_id, 0);
    dist[start_id] = 0;

    while (!ppq_.IsEmpty()) {
      nodeId curr_id = ppq_.PopMin();
      xyThetaPos curr_pos = ToXYThetaPos(curr_id);
      Distance curr_g = dist[curr_id];

      const std::vector<MotionPrimitive>* prims = GetAllForwardPrimitives(
          curr_pos.o);

      for (int i = 0; i < prims->size(); i++) {
        const MotionPrimitive* prim = &prims->at(i);
        xyThetaPos succ_pos = prim->GetResultingPos(curr_pos);

        if (abs(succ_pos.x) <= d && abs(succ_pos.y) <= d) {
          nodeId succ_id = ToNodeId(succ_pos);
          Distance new_g = curr_g + prim->GetCost();
          if (new_g + kEpsDistance  < dist[succ_id]) {
            dist[succ_id] = new_g;
            parent_count[succ_id] = 1;
            ppq_.InsertOrDecreaseKey(succ_id, new_g);
          }
          else if (new_g - kEpsDistance <= dist[succ_id]) {
            parent_count[succ_id]++;
          }
        }
      }
    }

    auto prims = GetAllForwardPrimitives(start_a);
    int num_eliminated_prims = 0;
    for (int i = 0; i < prims->size(); i++) {
      nodeId n = ToNodeId(prims->at(i).GetResultingPos(start));
      bool eliminate = false;
      if (prims->at(i).GetCost() > dist[n] + kEpsDistance)
        eliminate = true;

      else if (eliminate_if_same_cost
          && prims->at(i).GetCost() >= dist[n] - kEpsDistance
          && parent_count[n] > 1) {
        eliminate = true;
        parent_count[n]--;
      }

      if (eliminate) {
        //*
        prims->at(i) = prims->back();
        prims->pop_back();
        i--;
        num_eliminated_prims++;
        /*/
        cout << "Changed primitive cost from "<<prims->at(i).cost_;
        prims->at(i).cost_ = dist[n];
        cout << " to "<<prims->at(i).cost_<<endl;
        //*/
      }
    }
#ifndef SG_QUIET
    if (num_eliminated_prims > 0)
      cout << "Eliminated " << num_eliminated_prims << " primitives for pose "
           << start_a << endl;
#endif
  }
}

void Lattice::SortPrimitivesInDescendingCostOrder() {
  for (int start_a = 0; start_a < num_discrete_angles_; start_a++) {
    std::sort(prims_[start_a].begin(), prims_[start_a].end(),
              [&](MotionPrimitive & p1, MotionPrimitive & p2) -> bool {
      return p1.GetCost() > p2.GetCost();
    });
  }
}


void Lattice::LoadMap(std::string mapname) {
	// Calculate the padding thickness.
	int max_reach = GetMaxPrimitiveReach();

#ifndef SG_QUIET
	cout << "Borders should be " << max_reach << " cells thick!" << endl;
#endif

	// Generate the grid.
	grid_.GenerateGrid(mapname, max_reach);

#ifndef SG_QUIET
	cout << "Motion lattice: map loaded!" << endl;
#endif

	// Calculate linearized padded coordinates offsets for both the primitives
	// and the reverse primitives.
	for (int a = 0; a < num_discrete_angles_; a++)
		for (unsigned int i = 0; i < prims_[a].size(); i++)
			CalculateLinearizedPaddedCoordinateOffsets(prims_[a][i]);

	for (int a = 0; a < num_discrete_angles_; a++)
		for (unsigned int i = 0; i < reverse_prims_[a].size(); i++)
			CalculateLinearizedPaddedCoordinateOffsets(reverse_prims_[a][i]);

	// Calculate the agent footprint.
	// TODO: Move somewhere else?
	LatticeGeometry geom;
  agent_footprint_ = geom.GetPositionIntersectedCells(xyPosCont(0, 0),
                                                      agent_radius_);
/*
  for (auto prims_for_orientation: prims_) {
    for (auto prim: prims_for_orientation) {
      prim.Print(std::cout, true, true, false);
    }
  }
  std::cout<<"Agent footprint: \n";
  for (auto offset: agent_footprint_)
    std::cout<<offset.x<<" "<<offset.y<<std::endl;
//*/
}

void Lattice::Debug() {
  auto DebugPrimitive = [&](MotionPrimitive & p) {
#ifndef SG_QUIET
    cout<<p.GetResultingPos(xyThetaPos(0, 0, p.GetDiscreteStartAngle()));
    cout<<"\t"<<p.GetCost()<<endl;
#endif
    assert(p.GetCost() > kEpsDistance);
    assert(
      p.delta_x_ != 0 || p.delta_y_ != 0
          || p.disc_start_orientation_ != p.disc_end_orientation_);
    assert(abs(p.delta_x_) <= p.cost_ + kEpsDistance);
    assert(abs(p.delta_y_) <= p.cost_ + kEpsDistance);
  };

  for (int a = 0; a < num_discrete_angles_; a++) {
#ifndef SG_QUIET
    cout << "Orientation: " << a << "\tRadian: " << orientation_to_radian_[a]
         << "\tnum prims: " << prims_[a].size() << endl;
#endif

    for (auto p: *GetAllForwardPrimitives(a))
      DebugPrimitive(p);
    //for (auto p: *GetAllReversePrimitives(a))
    //  DebugPrimitive(p);
  }
}

int Lattice::GetMaxPrimitiveReach() {
	int max_reach = 0;
	for (int a = 0; a < num_discrete_angles_; a++)
		for (unsigned int i = 0; i < prims_[a].size(); i++)
			max_reach = max(prims_[a][i].GetReach(), max_reach);
	return max_reach;
}
void Lattice::CalculateLinearizedPaddedCoordinateOffsets(
		MotionPrimitive & prim) {
	const vector<xyPos>* swept_cells = prim.GetSweptCells();
	vector<int> linearized_padded_coordinate_offsets;
	for (unsigned int i = 0; i < swept_cells->size(); i++) {
		int x = swept_cells->at(i).x;
		int y = swept_cells->at(i).y;
		linearized_padded_coordinate_offsets.push_back(
				x * grid_.GetXMult() + y * grid_.GetYMult());
	}
	prim.SetSweptCellLinearizedPaddedCoordinateOffsets(
			linearized_padded_coordinate_offsets);
}

bool Lattice::IsValidPos(xyThetaPos pos) const {
  for (auto offset: agent_footprint_) {
    if (!grid_.IsTraversable(pos.x + offset.x, pos.y + offset.y))
      return false;
  }
  return true;
}
bool Lattice::CanExecutePrimitive(
		xyThetaPos pos, const MotionPrimitive* prim) const {
	if (pos.o != prim->GetDiscreteStartAngle())
		return false;

	int linearized_padded_coordinate =
			grid_.ToXYLin(pos.x, pos.y);
	const vector<int>* linearized_padded_coordinate_offsets =
			prim->GetSweptCellLinearizedPaddedCoordinateOffsets();

	for (unsigned int i = 0; i < linearized_padded_coordinate_offsets->size();
			i++)
		if (!grid_.IsTraversable(
				linearized_padded_coordinate
						+ linearized_padded_coordinate_offsets->at(i)))
			return false;

	return true;
}

void Lattice::GetExecutableForwardPrimitives(xyThetaPos pos,
		std::vector<const MotionPrimitive*> & prims) const {
	int theta = pos.o;
	prims.clear();
	for (unsigned int i = 0; i < prims_[theta].size(); i++)
		if (CanExecutePrimitive(pos, &prims_[theta][i]))
			prims.push_back(&prims_[theta][i]);
}

void Lattice::GetExecutableReversePrimitives(xyThetaPos pos,
		std::vector<const MotionPrimitive*> & prims) const {
	int theta = pos.o;
	prims.clear();
	for (unsigned int i = 0; i < reverse_prims_[theta].size(); i++)
		if (CanExecutePrimitive(pos, &reverse_prims_[theta][i]))
			prims.push_back(&reverse_prims_[theta][i]);
}

const MotionPrimitive* Lattice::GetIntermediatePrimitive(
		const xyThetaPos & pos1, const xyThetaPos & pos2) const {
	for (unsigned int i = 0; i < prims_[pos1.o].size(); i++) {
		if (pos2 == prims_[pos1.o][i].GetResultingPos(pos1)) {
			return &prims_[pos1.o][i];
		}
	}
	return NULL;
}

#ifndef NO_HOG
void Lattice::VisualizePrimitives(const MapEnvironment *env, int x,
		int y) const {
	for (unsigned int i = 0; i < prims_.size(); i++)
		for (unsigned int j = 0; j < prims_[i].size(); j++)
			if (CanExecutePrimitive(
					xyThetaPos(x, y, prims_[i][j].GetDiscreteStartAngle()),
					&prims_[i][j]))
				prims_[i][j].DrawPrimitive(env, x, y);
}
#endif

void Lattice::GenerateCircularPrimitives(LatticeParam p) {
  is_undirected_ = p.is_undirected;
  num_discrete_angles_ = 4;

  double angle_to_radian = 0.0174533;
  orientation_to_radian_.push_back(0);
  orientation_to_radian_.push_back(90 * angle_to_radian);
  orientation_to_radian_.push_back(180 * angle_to_radian);
  orientation_to_radian_.push_back(270 * angle_to_radian);

  prims_.resize(4);

  for (int start_orientation = 0; start_orientation < 4; start_orientation++) {
    bool cw = true;
    MotionPrimitive straight_prim = GenerateStraightPrimitive(
        start_orientation, 1);
    MotionPrimitive cw_prim = GenerateCircularPrimitive(
        start_orientation, cw, p.turning_radius,
        p.num_intermediate_poses, p.forward_turn_cost_multiplier);
    MotionPrimitive ccw_prim = GenerateCircularPrimitive(
        start_orientation, !cw, p.turning_radius,
        p.num_intermediate_poses, p.forward_turn_cost_multiplier);

    AddPrimitive(straight_prim);
    AddPrimitive(cw_prim);
    AddPrimitive(ccw_prim);

    if (p.is_undirected || p.reverse_straight_move) {
      MotionPrimitive r_straight_prim =
          straight_prim.GetReversePrimitive();
      if (!p.is_undirected)
        r_straight_prim.SetCostMultiplier(
            p.reverse_straight_move_cost_multiplier);
      AddPrimitive(r_straight_prim);
    }

    if (p.is_undirected || p.reverse_circular_move) {
      MotionPrimitive r_cw_prim = cw_prim.GetReversePrimitive();
      if (!p.is_undirected)
        r_cw_prim.SetCostMultiplier(p.reverse_turn_cost_multiplier);

      MotionPrimitive r_ccw_prim = ccw_prim.GetReversePrimitive();
      if (!p.is_undirected)
        r_ccw_prim.SetCostMultiplier(p.reverse_turn_cost_multiplier);

      AddPrimitive(r_cw_prim);
      AddPrimitive(r_ccw_prim);
    }
  }
  for (int a = 0; a < num_discrete_angles_; a++)
    cout << "Orientation: " << a << "\tRadian: "
        << orientation_to_radian_[a] << endl;
}
MotionPrimitive Lattice::GenerateCircularPrimitive(
    int start_orientation, bool clockwise, int turning_radius_in_cells,
    int num_intermediate_poses, double cost_multiplier_for_turns) {

  assert(num_intermediate_poses > 0);

  // Assumes orientations are: 0 = East, 1 = North, 2 = West, 3 = South
  // Calculations are done in the regular coordinate_system
  // (x increases as we go West, y increases as we go North).

  // Figure out the pivot point for turning, based on the start orientation of
  // the agent.
  // The counter clockwise multipliers are clockwise multipliers * -1
  static int pivot_x_clockwise_multiplier[] = { 0, 1, 0, -1 };
  static int pivot_y_clockwise_multiplier[] = { -1, 0, 1, 0 };

  double pivot_x = turning_radius_in_cells
      * pivot_x_clockwise_multiplier[start_orientation]
      * (clockwise ? 1 : -1);
  double pivot_y = turning_radius_in_cells
      * pivot_y_clockwise_multiplier[start_orientation]
      * (clockwise ? 1 : -1);

  // Info print..
  //string cw = clockwise ? "clockwise" : "counter clockwise";
  //cout<<"Generating "<<cw<<" primitive with start orientation "<<start_orientation<<" and turning radius of "<<turning_radius_in_cells<<" cells..."<<endl;
  //cout<<"Resolution: "<<resolution_<<endl;
  //cout<<"Pivot: "<<pivot_x<<", "<<pivot_y<<endl;

  // Calculate the relative orientations of the start and end positions from the pivot points.
  // That is, from the pivot point, in which orientations do the start and goal locations lie.
  double start_orientation_from_pivot = (start_orientation
      + (clockwise ? 1 : 3)) % 4;
  double goal_orientation_from_pivot = start_orientation_from_pivot
      + (clockwise ? -1 : 1);
  if (goal_orientation_from_pivot < 0) {
    start_orientation_from_pivot += 4;
    goal_orientation_from_pivot += 4;
  }

  // Calculate the intermediate poses.
  double angle_to_radian = 0.0174533;
  vector<xyThetaPosCont> intermediate_poses;
  for (int i = 0; i < num_intermediate_poses; i++) {
    double orientation_from_pivot =
      fmod(start_orientation_from_pivot
      + i * (goal_orientation_from_pivot - start_orientation_from_pivot)
        / (num_intermediate_poses - 1), 4.0);

    double radian_from_pivot = orientation_from_pivot * 90
        * angle_to_radian;

    double x = pivot_x + cos(radian_from_pivot) * turning_radius_in_cells;
    double y = pivot_y + sin(radian_from_pivot) * turning_radius_in_cells;
    double orientation_in_radians = fmod(
        orientation_from_pivot + (clockwise ? -1 : 1), 4.0) * 90
        * angle_to_radian;

    intermediate_poses.push_back(
        xyThetaPosCont(x, y, orientation_in_radians));
    //cout<<i<<"\t("<<x<<" "<<y<<" "<<orientation_in_radians<<")"<<endl;
  }

  // Calculate the goal pose.
  xyThetaPosCont end_pose = intermediate_poses.back();
  int end_x = round(end_pose.x);
  int end_y = round(end_pose.y);
  int end_orientation = (start_orientation + 4 + (clockwise ? -1 : 1)) % 4;

  // Generate the primitive.
  MotionPrimitive prim(start_orientation, end_x, end_y, end_orientation,
                       intermediate_poses, cost_multiplier_for_turns,
                       agent_radius_);
  return prim;
}
MotionPrimitive Lattice::GenerateStraightPrimitive(
    int start_orientation, int length_in_cells) {
  static int goal_x_sign[] = { 1, 0, -1, 0 };
  static int goal_y_sign[] = { 0, 1, 0, -1 };

  int end_x = length_in_cells * goal_x_sign[start_orientation];
  int end_y = length_in_cells * goal_y_sign[start_orientation];

  double angle_to_radian = 0.0174533; // TODO: Make this a global variable?
  double orientation_in_radians = start_orientation * 90 * angle_to_radian;

  vector<xyThetaPosCont> intermediate_poses;

  intermediate_poses.push_back(xyThetaPosCont(0, 0, orientation_in_radians));
  intermediate_poses.push_back(
      xyThetaPosCont(end_x, end_y, orientation_in_radians));

  MotionPrimitive prim(start_orientation, end_x, end_y, start_orientation,
                       intermediate_poses, 1, agent_radius_);

  //prim.Print();
  return prim;
}
void Lattice::GenerateReversePrimitives() {
  // Bit of a hack: Also calculate and report the max cost.
  Distance max_cost = 0;
  reverse_prims_.resize(num_discrete_angles_);
  for (int a = 0; a < num_discrete_angles_; a++) {
    for (int i = 0; i < prims_[a].size(); i++) {
      MotionPrimitive r = prims_[a][i].GetReversePrimitive();
      reverse_prims_[r.GetDiscreteStartAngle()].push_back(r);
      if (r.GetCost() > max_cost)
        max_cost = r.GetCost();
    }
  }
  std::cout<<"Maximum cost: "<<max_cost<<std::endl;
}

void Lattice::GenerateGridPrimitives(LatticeParam p) {
  is_undirected_ = true;
  num_discrete_angles_ = 1;
  orientation_to_radian_.push_back(0);
  prims_.resize(1);

  static int x[] = { 1,  1,  0, -1, -1, -1,  0,  1 };
  static int y[] = { 0,  1,  1,  1,  0, -1, -1, -1 };

  for (int i = 0; i < 8; i++) {
    vector<xyThetaPosCont> intermediate_poses;
    intermediate_poses.push_back(xyThetaPosCont(0, 0, 0));
    intermediate_poses.push_back(xyThetaPosCont(x[i], y[i], 0));

    MotionPrimitive prim(0, x[i], y[i], 0, intermediate_poses, 1,
                         agent_radius_);
    AddPrimitive(prim);
  }
}

