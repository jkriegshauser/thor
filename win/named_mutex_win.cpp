/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * win/named_mutex_win.cpp
 *
 * Defines a platform-agnostic mutex class. This mutex can be given a system-wide
 * name to synchronize between multiple processes.
 *
 * This file defines the Windows base implementation
 */

#include "../named_mutex.h"

#include "../basic_string.h"

#define WIN32_EXTRA_LEAN 1
#include <Windows.h>

namespace thor
{

named_mutex::named_mutex(const char* name)
{
    THOR_ASSERT(name);
    // Convert to wide
    basic_string<wchar_t, 256> temp;
    bool b = utf8_to_wide(name, temp);
    THOR_UNUSED(b); THOR_ASSERT(b);
    set_debug_name(temp.c_str());
    temp.insert(0, L"Global\\");    // Use Windows Global\ namespace
    handle_ = ::CreateMutexW(0, FALSE, temp.c_str());
    THOR_ASSERT(handle_);
}

named_mutex::named_mutex(const wchar_t* name)
{
    THOR_ASSERT(name);
    basic_string<wchar_t, 256> temp;
    set_debug_name(name);
    temp.format(L"Global\\%s", name);   // Use Windows Global\ namespace
    handle_ = ::CreateMutexW(0, FALSE, temp.c_str());
    THOR_ASSERT(handle_);
}

named_mutex::~named_mutex()
{
    ::CloseHandle(handle_);
}

bool named_mutex::lock(size_type timeout_ms /*=-1*/)
{
    DWORD timeout = timeout_ms == size_type(-1) ? INFINITE : (DWORD)timeout_ms;

    return ::WaitForSingleObject(handle_, timeout) == WAIT_OBJECT_0;
}

bool named_mutex::try_lock()
{
    return ::WaitForSingleObject(handle_, 0) == WAIT_OBJECT_0;
}

bool named_mutex::unlock()
{
    return ::ReleaseMutex(handle_) == TRUE;
}


}