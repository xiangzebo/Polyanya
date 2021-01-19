// timer.h
//
// A cross-platform monotonic wallclock timer.
// Currently supports nanoseconds resolution.
//
// Reference doco for timers on OSX:
// https://developer.apple.com/library/mac/qa/qa1398/_index.html
// https://developer.apple.com/library/mac/technotes/tn2169/_index.html#//apple_ref/doc/uid/DTS40013172-CH1-TNTAG5000
//
// @author: dharabor
//
// @created: September 2012
//

#ifndef WARTHOG_TIMER_H
#define WARTHOG_TIMER_H

#ifdef OS_MAC
//#include <CoreServices/CoreServices.h>
#include <mach/mach.h>
#include <mach/mach_time.h>

#else
#include <time.h>
#endif

namespace warthog
{

class timer
{

#ifdef OS_MAC
  uint64_t start_time;
  uint64_t stop_time;
  mach_timebase_info_data_t timebase;
#else
	timespec stop_time;
	timespec start_time;
#endif

public:
	timer();
	void reset();
	void start();
	void stop();
	double elapsed_time_nano();
	double elapsed_time_micro();
	double get_time_nano();
};

}

#endif 
