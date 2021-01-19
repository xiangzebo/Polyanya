/*
 * SGGrid2DParam.h
 *
 *  Created on: Nov 28, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_PARAMETERS_SGGRID2DPARAM_H_
#define APPS_SUBGOALGRAPH_PARAMETERS_SGGRID2DPARAM_H_

enum Grid2DSGType {
  kGrid2DSGFreespace,             // SG, freespace-reachability
  kGrid2DSGCanonical,             // SG, undirected canonical-freespace-reachability
  kGrid2DCSG,                     // CSG, diagonal-first reachability
  kGrid2DJPDiagonalCorners,       // CSG, diagonal-first reachability
  kGrid2DJPDiagonalCornersMerged, // CSG, diagonal-first reachability
  kGrid2DJP                       // JPG
};

struct SGGrid2DParam {
  SGGrid2DParam(Grid2DSGType sg_type_, bool double_clearance_, bool use_jp_search_,
              bool use_avoidance_table_, bool eliminate_redundant_arcs_) {
    sg_type = sg_type_;
    double_clearance = double_clearance_;
    use_jp_search = use_jp_search_;
    use_avoidance_table = use_avoidance_table_;
    eliminate_redundant_arcs = eliminate_redundant_arcs_;
    avoid_redundant_edges_during_contraction = false; // FIXME
  }

  SGGrid2DParam(Grid2DSGType sg_type_, bool double_clearance_) {
    sg_type = sg_type_;
    double_clearance = double_clearance_;
    if (sg_type == kGrid2DSGFreespace || sg_type == kGrid2DSGCanonical) {
      use_jp_search = false;
      use_avoidance_table = false;
      eliminate_redundant_arcs = false;
      avoid_redundant_edges_during_contraction = false;
    }
    else {
      use_jp_search = true;
      use_avoidance_table = true;
      eliminate_redundant_arcs = true;
      avoid_redundant_edges_during_contraction = true;
    }
  }

  std::string GetRName() {
    if (sg_type == kGrid2DSGFreespace)
      return "FR";
    if (sg_type == kGrid2DSGCanonical)
      return "CFR";
    if (sg_type == kGrid2DCSG)
      return "DFR";
    if (sg_type == kGrid2DJPDiagonalCorners)
      return "DFR";
    if (sg_type == kGrid2DJPDiagonalCornersMerged)
      return "DFR";
    if (sg_type == kGrid2DJP)
      return "DFR";
    return "NO-R-SPECIFIED";
  }
  std::string GetRSPCName () {
    if (sg_type == kGrid2DSGFreespace)
      return "Cor";
    if (sg_type == kGrid2DSGCanonical)
      return "Cor";
    if (sg_type == kGrid2DCSG)
      return "JPAll";
    if (sg_type == kGrid2DJPDiagonalCorners)
      return "DiagJP";
    if (sg_type == kGrid2DJPDiagonalCornersMerged)
      return "DiagJPM";
    if (sg_type == kGrid2DJP)
      return "StrJP";
    return "NO-RSPC-SPECIFIED";
  }
  std::string GetSubName() {
    if (sg_type == kGrid2DSGFreespace || sg_type == kGrid2DSGCanonical)
      return "SG";
    if (sg_type == kGrid2DCSG)
      return "CSG";
    if (sg_type == kGrid2DJPDiagonalCorners)
      return "JPDg";
    if (sg_type == kGrid2DJPDiagonalCornersMerged)
      return "JPDgM";
    if (sg_type == kGrid2DJP)
      return "JP";
    return "NO-SUB-SPECIFIED";
  }

  bool IsUndirected() {
    return sg_type == kGrid2DSGFreespace || sg_type == kGrid2DSGCanonical;
  }
  bool IsDirectionExtended() {
    return sg_type != kGrid2DSGFreespace && sg_type != kGrid2DSGCanonical;
  }

  Grid2DSGType sg_type;
  bool double_clearance;
  bool use_jp_search;
  bool use_avoidance_table;
  bool eliminate_redundant_arcs;
  bool avoid_redundant_edges_during_contraction;
};


#endif /* APPS_SUBGOALGRAPH_PARAMETERS_SGGRID2DPARAM_H_ */
