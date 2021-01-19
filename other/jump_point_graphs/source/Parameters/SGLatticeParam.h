/*
 * SGLatticeParam.h
 *
 *  Created on: Oct 28, 2016
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_XYTHETALATTICESUBGOALPARAMETERS_H_
#define APPS_SUBGOALGRAPH_XYTHETALATTICESUBGOALPARAMETERS_H_

#include <cassert>
#include <string>
#include <sstream>
#include <fstream>
#include "DefaultParams.h"
#include "LatticeParam.h"
#include "RParam.h"
#include "SGConstructionParam.h"

enum LatticeHeuristicType {
  kLatticeEuclideanHeuristic,
  kLattice2DProjectionHeuristic
};

struct SGLatticeParam {
  SGLatticeParam(LatticeParam lp = LatticeParam(), RParam rp = RParam(),
                 SGConstructionParam sp = SGConstructionParam(),
                 LatticeHeuristicType hp = kLatticeEuclideanHeuristic) {
    l = lp;
    r = rp;
    s = sp;
    h = hp;
	}

  SGLatticeParam(LatticeParam lp, LatticeHeuristicType ht)
      : SGLatticeParam(lp, RParam(), SGConstructionParam(), ht) {
  }

  std::string GetConfigName() {
		return l.GetName() + "-" + r.GetName();
	}

  LatticeParam l;
  RParam r;
  SGConstructionParam s;
  LatticeHeuristicType h;
};


#endif /* APPS_SUBGOALGRAPH_XYTHETALATTICESUBGOALPARAMETERS_H_ */
