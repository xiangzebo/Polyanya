/*
 * DirectoryParam.h
 *
 *  Created on: Feb 19, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_PARAMETERS_DIRECTORYPARAM_H_
#define APPS_SUBGOALGRAPH_PARAMETERS_DIRECTORYPARAM_H_
#include <cassert>
#include <string>

// For each map, we can have different types of graphs (grid, lattice, etc).
// For each map and graph pair, we can have diferent algorithms.

// Mapname:       maps_dir/relative_dir/mapname.map
// Instance name: instances_dir/{base_graph_name}/relative_dir/mapname.map.scen
// Statistics:    stat_dir/{base_graph_name}/relative_dir/mapname/{filename}
// Save/Load:     save_load_dir/{base_graph_name}/relative_dir/mapname/{filename}

const std::string kDefaultMapsDir      = "maps/";
const std::string kDefaultBaseDir      = "/home/idm-lab/HOG2Experiment/";
const std::string kDefaultInstancesDir = "instances/";
const std::string kDefaultStatDir      = "results/";
const std::string kDefaultInstStatDir  = "results-instances/";
const std::string kDefaultSaveLoadDir  = "save/";

struct DirectoryParam {
  DirectoryParam() {
    maps_dir = kDefaultMapsDir;
    base_dir = kDefaultBaseDir;
    instances_dir = kDefaultInstancesDir;
    stat_dir = kDefaultStatDir;
    inst_stat_dir = kDefaultInstStatDir;
    save_load_dir = kDefaultSaveLoadDir;
  }

  void SetMapPath(std::string map_path) {
    // FIXME: A little hacky solution for the moment:
    // Assume that the mapsdir is correct and mappath contains it.

    // Breakdown map_path into maps_dir/map_path
    // Find the first occurence of maps_dir in the map_path.
    const size_t map_pos = map_path.find(maps_dir);
    assert(
        map_pos != std::string::npos
            || "Map should be contained in some directory named \"maps/\". Alternative: change the maps\_dir value of DirectoryParam.");  // Make sure our assumption is correct.

    // Update maps_dir if necessary.
    maps_dir = map_path.substr(0, map_pos + maps_dir.length());
    map_path = map_path.substr(maps_dir.length());

    // Breakdown map_path which is now relative_dir/mapname.ext into
    // relative_dir, mapname, and extension.
    const size_t last_slash = map_path.find_last_of("/");
    if (last_slash == std::string::npos) {
      mapname = map_path;
      relative_dir = "";
    }
    else {
      mapname = map_path.substr(last_slash+1);
      relative_dir = map_path.substr(0, last_slash+1);
    }

    // map_name contains extension. Separate the extension.
    const size_t last_dot = mapname.find_last_of(".");
    if (last_dot == std::string::npos) {
      mapname_extension = "";
    }
    else {
      mapname_extension = mapname.substr(last_dot);
      mapname = mapname.substr(0, last_dot);
    }
  }

  std::string GetInstancesDirectory(std::string base_graph_name) {
//    std::string s = base_dir + (base_graph_name + "/") + instances_dir + relative_dir;
    std::string s = base_dir + instances_dir + (base_graph_name + "/") + relative_dir;
    if (s.back() == '/')
      s.pop_back();
    return s;
  }
  std::string GetStatisticsDirectory(std::string base_graph_name) {
//    std::string s = base_dir + (base_graph_name + "/") + stat_dir + relative_dir
//        + mapname;
    std::string s = base_dir + stat_dir + (base_graph_name + "/") + relative_dir
        + mapname;
    return s;
  }

  std::string GetInstanceStatisticsDirectory(std::string base_graph_name) {
//    std::string s = base_dir + (base_graph_name + "/") + inst_stat_dir
//        + relative_dir + mapname;
    std::string s = base_dir + inst_stat_dir + (base_graph_name + "/")
        + relative_dir + mapname;
    return s;
  }

  std::string GetSaveLoadDirectory(std::string base_graph_name) {
//    std::string s = base_dir + (base_graph_name + "/") + save_load_dir + relative_dir
//        + mapname;
    std::string s = base_dir + save_load_dir + (base_graph_name + "/") + relative_dir
        + mapname;
    return s;
  }

  std::string GetMapFilename() {
    return maps_dir  + relative_dir + mapname + mapname_extension;
  }
  std::string GetInstanceFilename(std::string base_graph_name) {
    return GetInstancesDirectory(base_graph_name) + "/" + mapname
        + mapname_extension + ".scen";
  }
  void MakeDirectories(std::string base_graph_name) {
    std::string make_instance_dir_cmd = "mkdir -p "
        + GetInstancesDirectory(base_graph_name);
    std::string make_statistics_dir_cmd = "mkdir -p "
        + GetStatisticsDirectory(base_graph_name);
    std::string make_instance_statistics_dir_cmd = "mkdir -p "
        + GetInstanceStatisticsDirectory(base_graph_name);
    std::string make_save_load_dir_cmd = "mkdir -p "
        + GetSaveLoadDirectory(base_graph_name);

    system(make_instance_dir_cmd.c_str());
    system(make_statistics_dir_cmd.c_str());
    system(make_instance_statistics_dir_cmd.c_str());
    system(make_save_load_dir_cmd.c_str());
  }

  std::string maps_dir;
  std::string base_dir;
  std::string instances_dir;
  std::string stat_dir;
  std::string inst_stat_dir;
  std::string save_load_dir;

  std::string relative_dir;
  std::string mapname;
  std::string mapname_extension;
};


#endif /* APPS_SUBGOALGRAPH_PARAMETERS_DIRECTORYPARAM_H_ */
