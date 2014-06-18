/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * win/debug_win.h
 *
 * Provides platform-abstracted debug facilities
 */

#include "../debug.h"

#define WIN32_EXTRA_LEAN
#include <Windows.h>

namespace thor
{
namespace debug
{

void crash()
{
    volatile size_type* p = 0;
    *p = 0xbadc0de;
}

void debugbreak()
{
    __debugbreak();
}

// http://msdn.microsoft.com/en-us/library/xcb2z8hs(v=vs.100).aspx
enum { MS_VC_EXCEPTION = 0x406d1388 };

#pragma pack(push,8)
struct THREADNAME_INFO
{
    DWORD dwType;
    LPCSTR szName;
    DWORD dwThreadID;
    DWORD dwFlags;

    THREADNAME_INFO(LPCSTR name, DWORD threadId)
        : dwType(0x1000)
        , szName(name)
        , dwThreadID(threadId)
        , dwFlags(0)
    {}
};
#pragma pack(pop)

void set_thread_name(const char* name, thread_id tid)
{
#ifdef THOR_DEBUG
    if (name && *name)
    {
        THREADNAME_INFO info(name, tid.thread_id_);
        __try
        {
            ::RaiseException(MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info);
        }
        __except(EXCEPTION_CONTINUE_EXECUTION)
        {
        }
    }
#endif
}

void debug_output(const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    debug_output_v(fmt, va);
    va_end(va);
}

void debug_output_v(const char* fmt, va_list va)
{
#ifdef THOR_DEBUG
    basic_string<char, 2048> str;
    str.format_v(fmt, va);
    ::OutputDebugStringA(str.c_str());
#endif
}

void debug_output(const wchar_t* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    debug_output_v(fmt, va);
    va_end(va);
}

void debug_output_v(const wchar_t* fmt, va_list va)
{
#ifdef THOR_DEBUG
    basic_string<wchar_t, 2048> str;
    str.format_v(fmt, va);
    ::OutputDebugStringW(str.c_str());
#endif
}

}
}