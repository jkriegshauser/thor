/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * time_util.inl
 *
 * Time utilities
 */

namespace thor
{
namespace time
{

///////////////////////////////////////////////////////////////////////////////Inline functions
inline difference_type seconds_diff(const seconds& earlier) THOR_NOTHROW
{
    return (difference_type)(seconds_now().cvalue() - earlier.cvalue());
}

inline difference_type seconds_diff(seconds* earlier) THOR_NOTHROW
{
    THOR_ASSERT(earlier != 0);
    seconds old = *earlier;
    *earlier = seconds_now();
    return (difference_type)(earlier->cvalue() - old.cvalue());
}

inline difference_type seconds_diff(const seconds& earlier, const seconds& later) THOR_NOTHROW
{
    return (difference_type)(later.cvalue() - earlier.cvalue());
}

inline seconds seconds_adjust(const seconds& t, difference_type adjustment) THOR_NOTHROW
{
    return seconds(t.cvalue() + adjustment);
}

inline difference_type milliseconds_diff(const milliseconds& earlier) THOR_NOTHROW
{
    THOR_ASSERT(earlier.cvalue() != 0); // Zero is a meaningless time; all comparisons should be relative
    return (difference_type)(milliseconds_now().cvalue() - earlier.cvalue());
}

inline difference_type milliseconds_diff(milliseconds* earlier) THOR_NOTHROW
{
    THOR_ASSERT(earlier != 0);
    THOR_ASSERT(earlier->cvalue() != 0); // Zero is a meaningless time; all comparisons should be relative
    milliseconds old = *earlier;
    *earlier = milliseconds_now();
    return (difference_type)(earlier->cvalue() - old.cvalue());
}

inline difference_type milliseconds_diff(const milliseconds& earlier, const milliseconds& later) THOR_NOTHROW
{
    THOR_ASSERT(earlier.cvalue() != 0); // Zero is a meaningless time; all comparisons should be relative
    THOR_ASSERT(later.cvalue() != 0); // Zero is a meaningless time; all comparisons should be relative
    return (difference_type)(later.cvalue() - earlier.cvalue());
}

inline milliseconds milliseconds_adjust(const milliseconds& t, difference_type adjustment) THOR_NOTHROW
{
    return milliseconds(t.cvalue() + adjustment);
}

inline difference_type microseconds_diff(const microseconds& earlier) THOR_NOTHROW
{
    THOR_ASSERT(earlier.cvalue() != 0); // Zero is a meaningless time; all comparisons should be relative
    return (difference_type)(microseconds_now().cvalue() - earlier.cvalue());
}

inline difference_type microseconds_diff(microseconds* earlier) THOR_NOTHROW
{
    THOR_ASSERT(earlier != 0);
    THOR_ASSERT(earlier->cvalue() != 0); // Zero is a meaningless time; all comparisons should be relative
    microseconds old = *earlier;
    *earlier = microseconds_now();
    return (difference_type)(earlier->cvalue() - old.cvalue());
}

inline difference_type microseconds_diff(const microseconds& earlier, const microseconds& later) THOR_NOTHROW
{
    THOR_ASSERT(earlier.cvalue() != 0); // Zero is a meaningless time; all comparisons should be relative
    THOR_ASSERT(later.cvalue() != 0); // Zero is a meaningless time; all comparisons should be relative
    return (difference_type)(later.cvalue() - earlier.cvalue());
}

inline microseconds microseconds_adjust(const microseconds& t, difference_type adjustment) THOR_NOTHROW
{
    return microseconds(t.cvalue() + adjustment);
}

inline difference_type nanoseconds_diff(const nanoseconds& earlier) THOR_NOTHROW
{
    THOR_ASSERT(earlier.cvalue() != 0); // Zero is a meaningless time; all comparisons should be relative
    return (difference_type)(nanoseconds_now().cvalue() - earlier.cvalue());
}

inline difference_type nanoseconds_diff(nanoseconds* earlier) THOR_NOTHROW
{
    THOR_ASSERT(earlier != 0);
    THOR_ASSERT(earlier->cvalue() != 0); // Zero is a meaningless time; all comparisons should be relative
    nanoseconds old = *earlier;
    *earlier = nanoseconds_now();
    return (difference_type)(earlier->cvalue() - old.cvalue());
}

inline difference_type nanoseconds_diff(const nanoseconds& earlier, const nanoseconds& later) THOR_NOTHROW
{
    THOR_ASSERT(earlier.cvalue() != 0); // Zero is a meaningless time; all comparisons should be relative
    THOR_ASSERT(later.cvalue() != 0); // Zero is a meaningless time; all comparisons should be relative
    return (difference_type)(later.cvalue() - earlier.cvalue());
}

inline nanoseconds nanoseconds_adjust(const nanoseconds& t, difference_type adjustment) THOR_NOTHROW
{
    return nanoseconds(t.cvalue() + adjustment);
}

}
}