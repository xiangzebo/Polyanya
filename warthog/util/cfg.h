#ifndef WARTHOG_UTIL_CFG_H
#define WARTHOG_UTIL_CFG_H

#include "getopt.h"

#include <iostream>
#include <map>

// cfg.h
//
// A centralised place for dealing with configuration parameters.
//
// @author: dharabor
// @created: 10/09/2012
//

namespace warthog
{

namespace util
{

typedef struct option param;

class cfg
{
	public:
		cfg();
		~cfg();

		void 
		parse_args(int argc, char** argv, warthog::util::param params[]);

		std::string
		get_param_value(std::string);


		void
		print(std::ostream& out);

	private:
		// no copy
		cfg(const cfg& other) {}
		cfg& operator=(const cfg& other) { return *this; }

		std::map<std::string, std::string> params_;
};

}
}

#endif

