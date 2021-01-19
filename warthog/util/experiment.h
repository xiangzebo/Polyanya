#ifndef WARTHOG_EXPERIMENT_H
#define WARTHOG_EXPERIMENT_H

// experiment.h
//
// An object for holding experiments read from Nathan Sturtevant's 
// .scenario files.
// Based on an implementation from HOG by Renee Jansen.
//
// NB: This implementation makes use of an additional attribute,
// ::precision_, which can be used to indicate the accuracy with which
// ::distance_ should be interpreted. The hardcoded default is 4.
//
// NB2: The attributes ::mapwidth_ and ::mapheight_ refer to the x/y dimensions
// that ::map should be scaled to. The individial node
// coordinates (::startx_, ::starty_ etc.) are taken with respect to the 
// dimensions of the scaled map.
//
// @author: dharabor
// @created: 21/08/2012
//

#include <iostream>
#include <string>

namespace warthog
{

class experiment
{
	public:
		experiment(unsigned int sx, unsigned int sy, unsigned int gx, 
				unsigned int gy, unsigned int mapwidth, unsigned int mapheight,
			   	double d, std::string m) :
			startx_(sx), starty_(sy), goalx_(gx), goaly_(gy), 
			mapwidth_(mapwidth), mapheight_(mapheight), distance_(d), map_(m),
			precision_(4)
		{}
		~experiment() {}

		inline unsigned int
		startx() { return startx_; }

		inline unsigned int
		starty() { return starty_; }

		inline unsigned int
		goalx() { return goalx_; } 

		inline unsigned int
		goaly()  { return goaly_; }

		inline double
		distance() { return distance_; }

		inline std::string
		map() { return map_; } 

		inline unsigned int
		mapwidth() { return mapwidth_; }

		inline unsigned int
		mapheight() { return mapheight_; }

		inline int 
		precision() { return precision_; }

		inline void
		set_precision(unsigned int prec) { precision_ = prec; }

		inline void 
		set_precision(int prec) { precision_ = prec; }

		void
		print(std::ostream& out);

	private:
		unsigned int startx_, starty_, goalx_, goaly_;
		unsigned int mapwidth_, mapheight_;
		double distance_;
		std::string map_;
		unsigned int precision_;

		// no copy
		experiment(const experiment& other) {} 
		experiment& operator=(const experiment& other) { return *this; }
};

}

#endif

