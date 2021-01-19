#include <stdio.h>
#include "timer.h"

warthog::timer::timer()
{

#ifdef OS_MAC
    start_time = stop_time = 0;
    mach_timebase_info(&timebase);

#else
    start_time.tv_sec = 0;
    start_time.tv_nsec = 0;
    stop_time.tv_sec = 0;
    stop_time.tv_nsec = 0;
#endif

}

double
warthog::timer::get_time_nano()
{
#ifdef OS_MAC
    uint64_t raw_time = mach_absolute_time();
    return (double)(raw_time * timebase.numer / timebase.denom);
#else
    timespec raw_time;
    clock_gettime(CLOCK_MONOTONIC , &raw_time);
    return (double)(raw_time.tv_nsec);
#endif
}

void warthog::timer::start()
{
#ifdef OS_MAC
    start_time = mach_absolute_time();
    stop_time = start_time;
#else
    clock_gettime(CLOCK_MONOTONIC , &start_time);
    stop_time = start_time;
#endif
}

void warthog::timer::stop()
{
#ifdef OS_MAC
    stop_time = mach_absolute_time();
#else
    clock_gettime(CLOCK_MONOTONIC , &stop_time);
#endif
}


double warthog::timer::elapsed_time_nano()
{
#ifdef OS_MAC
    uint64_t elapsed_time = stop_time - start_time;
    return (double)(elapsed_time * timebase.numer / timebase.denom);
    //Nanoseconds nanosecs = AbsoluteToNanoseconds(*(AbsoluteTime*)&elapsed_time);
    //return (double) UnsignedWideToUInt64(nanosecs) ;

#else
    if ((stop_time.tv_nsec-start_time.tv_nsec)<0)
        return (double)(1000000000+stop_time.tv_nsec-start_time.tv_nsec);
    else
        return (double)(stop_time.tv_nsec - start_time.tv_nsec);
#endif
}

void warthog::timer::reset()
{
#ifdef OS_MAC
    start_time = stop_time = 0;
#else
    start_time.tv_sec = 0;
    start_time.tv_nsec = 0;
    stop_time.tv_sec = 0;
    stop_time.tv_nsec = 0;
#endif
}

double
warthog::timer::elapsed_time_micro()
{
    return elapsed_time_nano() / 1000.0;
}

