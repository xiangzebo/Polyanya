#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <cassert>

#include "SGCHQuery.h"
#include "Grid2DSG.h"
#include "InstanceManager.h"
#include "SGExperiment.h"
#include "ScenarioLoader.h"

using namespace std;


std::vector<CHParam> c;
std::vector<SGGrid2DParam> s;

void Experiment(DirectoryParam d, CHParam c, SGGrid2DParam s, bool only_print_name = false) {
  string mapfile = d.GetMapFilename();
  string scenfile = mapfile + ".scen";

  SGCHQuery<Grid2DSG> q(d, c, s);
  std::cout<<q.GetName()<<std::endl;  
  
  if (only_print_name)
    return;
    
  // Read the instances.
  InstanceManager<xyLoc, Grid2DGraph> im;
  ScenarioLoader sl(scenfile.c_str());
  for (unsigned int i = 0; i < sl.GetNumExperiments(); i++) {
    ProblemInstance<xyLoc> ins;
    auto scen = sl.GetNthExperiment(i);
    ins.start.x = scen.GetStartX();
    ins.start.y = scen.GetStartY();
    ins.goal.x = scen.GetGoalX();
    ins.goal.y = scen.GetGoalY();
    ins.dist = scen.GetDistance();
    im.AddInstance(ins);
  }
  SGExperiment<SGCHQuery<Grid2DSG>, InstanceManager<xyLoc, Grid2DGraph>> exp(
      &q, &im, q.GetStatisticsFilename(), q.GetInstanceStatisticsFilename());
  exp.Preprocess();
  exp.ReportPreprocessingStatistics();
  exp.RunInstances();
  exp.ReportQueryStatistics();
  exp.CloseStatisticsFiles();
}


void GenerateParams() {
  auto AddParam = [&](CHParam chp, SGGrid2DParam sgp = SGGrid2DParam(kGrid2DSGCanonical, false)) {
    c.push_back(chp);
    s.push_back(sgp);
  };

  // Always true (just for easy reading for the rest of the function).
  bool p_unpack = true;
  bool sg = true;
  bool r_refine = true;
  bool astar = true;
  bool stall = true;

  //SGGrid2DParam f_refine(kGrid2DSGFreespace, false);
  //SGGrid2DParam cf_refine(kGrid2DSGCanonical, false);

  // A*
  AddParam(MakeGParam(!sg, astar));
  
  // GPPC-CH
  AddParam(MakeCHParam(!sg, !r_refine, !p_unpack, !astar, stall)); 
  
  // GPPC-CH-AStar version
  //AddParam(MakeCHParam(!sg, !r_refine, !p_unpack, astar, stall)); 

  // CH (GPPC + AStar + pointer unpacking)
  AddParam(MakeCHParam(!sg, !r_refine, p_unpack, astar, stall));

  // CH + R
  //AddParam(MakeCHParam(!sg, r_refine, p_unpack, astar, stall), f_refine);
  //AddParam(MakeCHParam(!sg, r_refine, p_unpack, astar, stall), cf_refine);

  // RCH variants
  //AddParam(MakeRCHParam(!sg, astar, stall), f_refine);
  //AddParam(MakeRCHParam(!sg, astar, stall), cf_refine);

  // All SG/JP variants
  //for (int clearance = 1; clearance <=2; clearance ++) {  // Use a single set or a double set of clearances?
  for (int clearance = 2; clearance <=2; clearance ++) {
    bool dc = clearance == 2;
    SGGrid2DParam sub   = SGGrid2DParam(kGrid2DSGCanonical, dc);
    //SGGrid2DParam subr  = SGGrid2DParam(kGrid2DSGFreespace, dc);
    //SGGrid2DParam csg   = SGGrid2DParam(kGrid2DCSG, dc);
    //SGGrid2DParam jpdg  = SGGrid2DParam(kGrid2DJPDiagonalCorners, dc);
    SGGrid2DParam jpdgm = SGGrid2DParam(kGrid2DJPDiagonalCornersMerged, dc);
    SGGrid2DParam jp    = SGGrid2DParam(kGrid2DJP, dc);

    // For the CH variants of CH, don't do the specialized search?
    //SGGrid2DParam jpdg_ch = jpdg;
    SGGrid2DParam jpdgm_ch = jpdgm;
    SGGrid2DParam jp_ch = jp;
    //jpdg_ch.use_jp_search = false;
    jpdgm_ch.use_jp_search = false;
    jp_ch.use_jp_search = false;

    // SG variants
    AddParam(MakeGParam(sg), sub);
    //AddParam(MakeGParam(sg), csg);
    //AddParam(MakeGParam(sg), jpdg);
    AddParam(MakeGParam(sg), jpdgm);
    AddParam(MakeGParam(sg), jp);

    // SG-CH variants
    auto sgch = MakeCHParam(sg, !r_refine, p_unpack, astar, stall);
    AddParam(sgch, sub);
    //AddParam(sgch, csg);
    //AddParam(sgch, jpdg_ch);
    AddParam(sgch, jpdgm_ch);
    AddParam(sgch, jp_ch);
    
    // SG-CH-R variants
    //AddParam(MakeCHParam(sg, r_refine, p_unpack, astar, stall), sub);
    //AddParam(MakeCHParam(sg, r_refine, p_unpack, astar, stall), subr);
    
    // SG-RCH variants
    //AddParam(MakeRCHParam(sg, astar, stall), sub);
    //AddParam(MakeRCHParam(sg, astar, stall), subr);
    
    // N-SUB variants
    //AddParam(MakeNSUBParam(), sub);
    //AddParam(MakeNSUBParam(), subr);
  }

  // CH-SL variants
  //AddParam(MakeCHSLParam(p_unpack));
}


int main(int argc, char **argv)
{
  if (argc == 1) {
    cout << "Called with no filename.\n";
    return -1;
  }
  
  // Read the mapname.
  string mapname = argv[1];
  DirectoryParam dp;
  dp.base_dir = "./experiments/";
  
  dp.SetMapPath(mapname);

  GenerateParams();

  if (argc == 2) {
    cout<<"There are "<<c.size()<<" experiments to perform.\n";
    for (int i = 1; i <= c.size(); i++) {
      Experiment(dp, c[i-1], s[i-1], true);
      //cout<<i<<":\t";
      //cout<<c[i-1].GetMethodName()<<"\t"<<s[i-1].GetConfigName()<<"\t"<<s[i-1].s.GetSubConstructionName()<<"\t";
      //cout<<mapname<<endl;    
    }
    return c.size();
  }
  
  if (argc >= 3) {
    int i = atoi(argv[2]);
    if (i < 1 || i > c.size()) {
      cout<<"Please enter a number between 1 and "<<c.size()<<" (inclusive)"<<endl;
      return -2;
    }
    //cout<<"Performing experiment "<<i<<":\t";
    //cout<<c[i-1].GetMethodName()<<"\t"<<s[i-1].GetConfigName()<<"\t"<<s[i-1].s.GetSubConstructionName()<<"\t";
    //cout<<mapname<<endl;
    
    c[i-1].clear = false;
    c[i-1].save = true;
    c[i-1].load = true;
    Experiment(dp, c[i-1], s[i-1]); //, true
    
    if (i >= c.size()) 
      return 0;
    else
      return 1;
  }

  return 0;
}

