#include "../time_util.h"

#include "../atomic_integer.h"

#include "../thread_local.h"

#define WIN32_EXTRA_LEAN 1
#include <Windows.h>

#pragma comment(lib, "winmm.lib")

namespace
{
    // Number of 100ns increments from 1/1/1601 to 1/1/1970. This is because seconds_now() is based on
    // 1/1/1970 whereas GetSystemTimeAsFileTime gives us 100ns increments since 1/1/1601.
    const uint64 EpochAdjustment = 116444736000000000ULL;

    static thor::atomic_integer<uint64> ms_val(0);
    static thor::atomic_integer<DWORD>  ms_last(0);

    uint64 get_frequency()
    {
        LARGE_INTEGER li;
        ::QueryPerformanceFrequency(&li);
        return li.QuadPart;
    }
    
    static uint64 frequency = get_frequency();
    static uint64 frequency_d1k = get_frequency() / 1000ULL;
}

namespace thor
{

namespace time
{

seconds seconds_now() THOR_NOTHROW
{
    static DWORD lastUpdateTick = 0;
    static uint64 lastTime = 0;
    const DWORD updateTick = ::GetTickCount();
    if ((updateTick - lastUpdateTick) > 10)
    {
        lastUpdateTick = updateTick;
        THOR_COMPILETIME_ASSERT(sizeof(FILETIME) == sizeof(uint64), InvalidAssumption);
        ::GetSystemTimeAsFileTime((LPFILETIME)&lastTime);
        lastTime -= EpochAdjustment;
        lastTime /= 10000000ULL; // Convert 100ns units to seconds
    }
    return seconds(lastTime);
}

milliseconds milliseconds_now() THOR_NOTHROW
{
    DWORD now = timeGetTime();

    // If we successfully updated ms_last to the now value, we can add the difference to ms_val.
    // If not, another thread beat us to it, so just use the ms_last value
    DWORD l = ms_last.get();
    if (now > l && ms_last.compare_exchange(now, l) == l)
    {
        uint64 newval = (ms_val += (now - l));
        THOR_UNUSED(newval);
        THOR_ASSERT(newval == uint64(now));
    }

    if (l == 0)
    {
        // First call; set timer resolution high (1ms)
        timeBeginPeriod(1);
    }

    return milliseconds(ms_val.get());
}

microseconds microseconds_now() THOR_NOTHROW
{
    LARGE_INTEGER li;
    BOOL b = ::QueryPerformanceCounter(&li);
    THOR_UNUSED(b); THOR_ASSERT(b);

    li.QuadPart *= 1000ULL;
    return microseconds(li.QuadPart / frequency_d1k);
}

nanoseconds nanoseconds_now() THOR_NOTHROW
{
    LARGE_INTEGER li;
    BOOL b = ::QueryPerformanceCounter(&li);
    THOR_UNUSED(b); THOR_ASSERT(b);

    li.QuadPart *= 1000000ULL;
    return nanoseconds(li.QuadPart / frequency_d1k);
}

}

}