#include "../mutex.h"

#define WIN32_EXTRA_LEAN 1
#include <Windows.h>

namespace thor
{

mutex::mutex(size_type spin_count)
{
    THOR_COMPILETIME_ASSERT(sizeof(CRITICAL_SECTION) <= sizeof(space_), SizeTooSmall);
    ::InitializeCriticalSectionAndSpinCount(get_critical_section(), (DWORD)spin_count);
}

mutex::~mutex()
{
    ::DeleteCriticalSection(get_critical_section());
}

bool mutex::lock()
{
    ::EnterCriticalSection(get_critical_section());
    return true;
}

bool mutex::try_lock()
{
    return ::TryEnterCriticalSection(get_critical_section()) == TRUE;
}

bool mutex::unlock()
{
    ::LeaveCriticalSection(get_critical_section());
    return true;
}

}