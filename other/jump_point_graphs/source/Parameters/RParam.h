/*
 * RParam.h
 *
 *  Created on: Feb 18, 2018
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_PARAMETERS_RPARAM_H_
#define APPS_SUBGOALGRAPH_PARAMETERS_RPARAM_H_

#include <sstream>
#include "../Graphs/GraphDefinitions.h"
#include "DefaultParams.h"

struct RParam {
  RParam(ReachabilityType t = kLatticeParamDefaultType, Distance b =
             kLatticeParamDefaultBound, RConnectType c = kRConnectDefault) {
    type = t;
    bound = b;
    conn = c;
    if (conn == kRConnectDefault) {
      conn = kRConnectAggressive;
    }
  }

  std::string GetName() {
    std::stringstream ss;
    if (type == kFreespaceReachability)
      ss << "FR";
    else if (type == kSafeFreespaceReachability)
      ss << "SFR";
    else if (type == kBoundedDistanceReachability)
      ss << "BD";
    else if (type == kCanonicalFreespaceReachability)
      ss << "CR";
    else
      assert(false && "Reachability type not specified");

    if (bound <= 10000) // FIXME? Who would use bound > 10000. You can quote
                        // me on this.
      ss << bound;
    return ss.str();
  }

  std::string GetConnectType () {
    assert (conn != kRConnectDefault);
    if (conn == kRConnectConservative) {
      return "C";
    }
    if (conn == kRConnectAggressive) {
      return "A";
    }
    if (conn == kRConnectStall) {
      return "S";
    }
    if (conn == kRConnectSucc) {
      return "X";
    }

    return "";
  }

  ReachabilityType type;
  RConnectType conn;
  double bound;
};

//const RParam kRParamDefFR(false);
//const RParam kRParamDefSFR(true);

#endif /* APPS_SUBGOALGRAPH_PARAMETERS_RPARAM_H_ */
