/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * win/semaphore_win.cpp
 *
 * Defines a platform-agnostic semaphore class
 *
 * This is the Windows platform implementation
 */

#include "../semaphore.h"

#include "../math_util.h"

#define WIN32_EXTRA_LEAN 1
#include <Windows.h>

#pragma warning (disable : 4307) // '+' integral constant overflow

const static LONG MAX_LONG = ((LONG)-1) ^ (1 << ((8*sizeof(LONG))-1));

namespace thor
{

semaphore::semaphore(size_type released /*=0*/, size_type max_count /*=size_type(-1)*/)
{
    THOR_COMPILETIME_ASSERT(MAX_LONG > 0 && (MAX_LONG + 1) < 0, InvalidAssumption);
    LONG maximumCount = (LONG)thor::math::clamp<size_type>(max_count, 0U, MAX_LONG);
    LONG initialCount = (LONG)thor::math::clamp<size_type>(released,  0U, maximumCount);
    handle_ = ::CreateSemaphoreW(NULL, initialCount, maximumCount, NULL);
    THOR_ASSERT(handle_);
}

semaphore::~semaphore()
{
    ::CloseHandle(handle_);
}

bool semaphore::wait(size_type timeout_ms /*=-1*/)
{
    DWORD timeout = (timeout_ms == size_type(-1) ? INFINITE : (DWORD)timeout_ms);
    return ::WaitForSingleObject(handle_, timeout) == WAIT_OBJECT_0;
}

bool semaphore::try_wait()
{
    return ::WaitForSingleObject(handle_, 0) == WAIT_OBJECT_0;
}

bool semaphore::release(size_type count /*=1*/)
{
    LONG releaseCount = (LONG)thor::math::clamp<size_type>(count, 0U, MAX_LONG);
    return ::ReleaseSemaphore(handle_, releaseCount, 0) == TRUE;
}

}