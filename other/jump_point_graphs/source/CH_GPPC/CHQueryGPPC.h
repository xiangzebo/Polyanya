#ifndef APPS_SUBGOALGRAPH_METHODMANAGERS_CHQUERYGPPC_H_
#define APPS_SUBGOALGRAPH_METHODMANAGERS_CHQUERYGPPC_H_
#include "../../../environments/Map2DEnvironment.h"
#include "../Utils/Experiments/InstanceManager.h"
#include "../Utils/Statistics/StatisticsFile.h"

#define USE_STALL_ON_DEMAND
//#define USE_NODE_REORDER
//#define QUICK_CONTRACT

#include <algorithm>
#include <cstdio>
#include <vector>

#include "adj_graph.h"
#include "arc_gppc.h"
#include "bidirectional_dijkstra_gppc.h"
#include "contraction_hierarchy.h"
#include "dijkstra_gppc.h"
#include "mapper.h"
#include "range_gppc.h"
#include "timestamp_flags.h"


namespace ch_gppc {

class CHQueryGPPC {

 public:
  typedef xyLoc State;
  CHQueryGPPC(std::string filename = "") {
    bidij = NULL;
    filename_ = filename;
  }
  ~CHQueryGPPC() {
    if (bidij != NULL)
      delete bidij;
  }

  void SetFilename(std::string filename) {
    filename_ = filename;
  }

  void ReportPreprocessStatistics(StatisticsFile* s) {
    s->ReportDouble("Preprocessing time (s)", preprocess_time_);
    s->ReportInt("Number of arcs", search_graph.arc_count());
    s->ReportDouble(
        "Memory",
        sizeof(ShortcutArcHeadGPPC) * search_graph.arc_count()
            / (1024.0 * 1024.0));
  }

  void ReportAdditionalQueryStatistics(StatisticsFile* s) {

  }

  QueryStatistic Query(xyLoc start, xyLoc goal, std::vector<xyLoc> & path) {
    QueryStatistic s;
    s.dist = FindPath(start, goal, path);
    s.search_time = search_time_;
    s.refine_time = refine_time_;
    s.num_expanded = bidij->GetNumExpansions();
    return s;
  }
  QueryStatistic Query(ProblemInstance<xyLoc> ins, std::vector<xyLoc> & path) {
    return Query(ins.start, ins.goal, path);
  }

  void Preprocess() {
    std::vector<bool> mapData;
    int width, height;
    LoadMap(filename_.c_str(), mapData, width, height);

    CPUTimer t;
    t.StartTimer();
    Preprocess(mapData, width, height);
    preprocess_time_ = t.EndTimer();
  }

  AdjGraph<ShortcutArcHeadGPPC>* GetOriginalGraph() {
    return &search_graph;
  }

 private:
  Mapper mapper;
  AdjGraph<ShortcutArcHeadGPPC> search_graph;
  BidirectionalDijkstraGPPC<TimestampFlags, AdjGraph<ShortcutArcHeadGPPC>,
      AdjGraph<ShortcutArcHeadGPPC>>*bidij;
  std::string filename_;
  double preprocess_time_;
  double search_time_, refine_time_;

  void Preprocess(std::vector<bool> &bits, int width, int height) {
    mapper = Mapper(bits, width, height);
    //printf("width = %d, height = %d\n", width, height);
    auto arc_list = extract_graph(mapper);
    //printf("node count: %d, arc count: %d\n", mapper.node_count(),
    //       arc_list.size());

    const int node_count = mapper.node_count();
    std::vector<ShortcutArcGPPC> search_arc_list;
    auto order = compute_online_contraction_order(
        node_count, arc_list, 100000000, [&](WeightedArcGPPC a, int m) {
          // T: On new forward arc: add to search_arc_list
        search_arc_list.push_back( {a.source, a.target, a.weight, m});
      },
        [&](WeightedArcGPPC, int) {}
        // T: On new backward arc: do nothing (This might be because of undirected graphs?)
        );

#ifdef USE_NODE_REORDER
    auto rank = inverse_permutation(order);

    for(auto&x:search_arc_list) {
      x.source = rank[x.source];
      x.target = rank[x.target];
      if(x.mid != -1)
      x.mid = rank[x.mid];
    }
#endif

    search_graph = AdjGraph<ShortcutArcHeadGPPC>(node_count, search_arc_list);

    bidij = new BidirectionalDijkstraGPPC<TimestampFlags,
        AdjGraph<ShortcutArcHeadGPPC>, AdjGraph<ShortcutArcHeadGPPC>>(search_graph,
                                                              search_graph);

    //printf("search graph arc count = %d\n", (int) search_graph.arc_count());
  }

  Distance FindPath(xyLoc s, xyLoc t, std::vector<xyLoc> & path) {
    search_time_ = 0;
    refine_time_ = 0;

    CPUTimer timer;
    timer.StartTimer();
    int source_node = mapper(s);
    int target_node = mapper(t);

    auto get_forward_weight = [&](ShortcutArcGPPC a) {
      return a.weight;
    };

    auto get_backward_weight = [&](ShortcutArcGPPC a) {
      return a.weight;
    };

#ifndef USE_STALL_ON_DEMAND
    auto should_forward_search_continue = [&](int v) {
      return true;
    };

    auto should_backward_search_continue = [&](int v) {
      return true;
    };
#else
    auto should_forward_search_continue =
        [&](int v) {
          for(auto a:search_graph.out(v)) {
            if(bidij->forward.extract_distance(a.target) < bidij->forward.extract_distance(v) - a.weight) {
              return false;
            }
          }
          return true;
        };
    auto should_backward_search_continue =
        [&](int v) {
          for(auto a:search_graph.out(v)) {
            if(bidij->backward.extract_distance(a.target) < bidij->backward.extract_distance(v) - a.weight) {
              return false;
            }
          }
          return true;
        };
#endif

    bidij->start(source_node, target_node);
    while (bidij->get_current_shortest_path_distance()
        > bidij->forward.get_current_radius()
        || bidij->get_current_shortest_path_distance()
            > bidij->backward.get_current_radius()) {
      bidij->settle_next(get_forward_weight, get_backward_weight,
                         should_forward_search_continue,
                         should_backward_search_continue);
    }

    auto up_down_path = bidij->get_current_shortest_path();
    search_time_ = timer.EndTimer();
    timer.StartTimer();


    if (up_down_path.empty())
      return kMaxDistance;

    auto get_mid = [&](int x, int y) {
#ifdef USE_NODE_REORDER
        if(y < x)
        std::swap(x, y);
        for(auto h:state->search_graph.out(x))
        if(h.target == y)
        return h.mid;
#else
        for(auto h:search_graph.out(x))
        if(h.target == y)
        return h.mid;
        for(auto h:search_graph.out(y))
        if(h.target == x)
        return h.mid;
#endif
        return -1;
      };

    while (up_down_path.size() != 1) {
      int last = up_down_path[up_down_path.size() - 1];
      int second_last = up_down_path[up_down_path.size() - 2];
      auto mid = get_mid(second_last, last);
      if (mid == -1) {
        up_down_path.pop_back();
        path.push_back(mapper(last));
      } else {
        up_down_path.back() = mid;
        up_down_path.push_back(last);
      }
    }

    path.push_back(s);

    std::reverse(path.begin(), path.end());

    refine_time_ = timer.EndTimer();

    return bidij->get_current_shortest_path_distance()/1000.0;
  }

  void LoadMap(const char *fname, std::vector<bool> &map, int &width,
               int &height) {
    FILE *f;
    f = fopen(fname, "r");
    if (f) {
      fscanf(f, "type octile\nheight %d\nwidth %d\nmap\n", &height, &width);
      map.resize(height * width);
      for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
          char c;
          do {
            fscanf(f, "%c", &c);
          } while (isspace(c));
          map[y * width + x] = (c == '.' || c == 'G' || c == 'S');
          //printf("%c", c);
        }
        //printf("\n");
      }
      fclose(f);
    }
  }

};
}
#endif /* APPS_SUBGOALGRAPH_METHODMANAGERS_CHQUERYGPPC_H_ */
