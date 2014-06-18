/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * win/semaphore_win.cpp
 *
 * Defines a platform-agnostic semaphore class. This mutex can be given a system-wide
 * name to synchronize between multiple processes.
 *
 * This is the Windows platform implementation
 */

#include "../named_semaphore.h"

#include "../math_util.h"
#include "../basic_string.h"

#define WIN32_EXTRA_LEAN 1
#include <Windows.h>

#pragma warning (disable : 4307) // '+' integral constant overflow

const static LONG MAX_LONG = ((LONG)-1) ^ (1 << ((8*sizeof(LONG))-1));

namespace thor
{

named_semaphore::named_semaphore(const char* name, size_type released /*=0*/, size_type max_count /*=size_type(-1)*/)
{
    THOR_COMPILETIME_ASSERT(MAX_LONG > 0 && (MAX_LONG + 1) < 0, InvalidAssumption);
    THOR_ASSERT(name);
    basic_string<wchar_t, 256> temp;
    utf8_to_wide(name, temp);
    set_debug_name(temp.c_str());
    temp.insert(0, L"Global\\"); // Use Global namespace
    LONG maximumCount = (LONG)thor::math::clamp<size_type>(max_count, 0U, MAX_LONG);
    LONG initialCount = (LONG)thor::math::clamp<size_type>(released,  0U, maximumCount);
    handle_ = ::CreateSemaphoreW(NULL, initialCount, maximumCount, temp.c_str());
    THOR_ASSERT(handle_);
}

named_semaphore::named_semaphore(const wchar_t* name, size_type released /*=0*/, size_type max_count /*=size_type(-1)*/)
{
    THOR_COMPILETIME_ASSERT(MAX_LONG > 0 && (MAX_LONG + 1) < 0, InvalidAssumption);
    THOR_ASSERT(name);
    basic_string<wchar_t, 256> temp;
    temp.format(L"Global\\%s", name);
    set_debug_name(name);
    LONG maximumCount = (LONG)thor::math::clamp<size_type>(max_count, 0U, MAX_LONG);
    LONG initialCount = (LONG)thor::math::clamp<size_type>(released,  0U, maximumCount);
    handle_ = ::CreateSemaphoreW(NULL, initialCount, maximumCount, temp.c_str());
    THOR_ASSERT(handle_);
}
named_semaphore::~named_semaphore()
{
    ::CloseHandle(handle_);
}

bool named_semaphore::wait(size_type timeout_ms /*=-1*/)
{
    DWORD timeout = (timeout_ms == size_type(-1) ? INFINITE : (DWORD)timeout_ms);
    return ::WaitForSingleObject(handle_, timeout) == WAIT_OBJECT_0;
}

bool named_semaphore::try_wait()
{
    return ::WaitForSingleObject(handle_, 0) == WAIT_OBJECT_0;
}

bool named_semaphore::release(size_type count /*=1*/)
{
    LONG releaseCount = (LONG)thor::math::clamp<size_type>(count, 0U, MAX_LONG);
    return ::ReleaseSemaphore(handle_, releaseCount, 0) == TRUE;
}

}