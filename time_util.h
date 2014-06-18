/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * time_util.h
 *
 * Time utilities
 */

#ifndef THOR_TIME_UTIL_H
#define THOR_TIME_UTIL_H
#pragma once

#ifndef THOR_STRONG_TYPE_H
#include "strong_type.h"
#endif

namespace thor
{

namespace time
{

THOR_DECLARE_STRONG_TYPE(uint64, seconds);
seconds seconds_now() THOR_NOTHROW;
difference_type seconds_diff(const seconds& earlier) THOR_NOTHROW;  // gets the difference in seconds between earlier and now
difference_type seconds_diff(seconds* earlier) THOR_NOTHROW; // gets the difference in seconds between earlier and now; updates earlier to now
difference_type seconds_diff(const seconds& earlier, const seconds& later) THOR_NOTHROW; // gets the difference in seconds between two time periods. Allows negative results
seconds seconds_adjust(const seconds& t, difference_type adjustment) THOR_NOTHROW;

THOR_DECLARE_STRONG_TYPE(uint64, milliseconds);
milliseconds milliseconds_now() THOR_NOTHROW;
difference_type milliseconds_diff(const milliseconds& earlier) THOR_NOTHROW;
difference_type milliseconds_diff(milliseconds* earlier) THOR_NOTHROW;
difference_type milliseconds_diff(const milliseconds& earlier, const milliseconds& later) THOR_NOTHROW;
milliseconds milliseconds_adjust(const milliseconds& t, difference_type adjustment) THOR_NOTHROW;

THOR_DECLARE_STRONG_TYPE(uint64, microseconds);
microseconds microseconds_now() THOR_NOTHROW;
difference_type microseconds_diff(const microseconds& earlier) THOR_NOTHROW;
difference_type microseconds_diff(microseconds* earlier) THOR_NOTHROW;
difference_type microseconds_diff(const microseconds& earlier, const microseconds& later) THOR_NOTHROW;
microseconds microseconds_adjust(const microseconds& t, difference_type adjustment) THOR_NOTHROW;

THOR_DECLARE_STRONG_TYPE(uint64, nanoseconds);
nanoseconds nanoseconds_now() THOR_NOTHROW;
difference_type nanoseconds_diff(const nanoseconds& earlier) THOR_NOTHROW;
difference_type nanoseconds_diff(nanoseconds* earlier) THOR_NOTHROW;
difference_type nanoseconds_diff(const nanoseconds& earlier, const nanoseconds& later) THOR_NOTHROW;
nanoseconds nanoseconds_adjust(const nanoseconds& t, difference_type adjustment) THOR_NOTHROW;

}

}

#include "time_util.inl"

#endif