/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * mutex.h
 *
 * Defines a platform-agnostic mutex class. This mutex can be given a system-wide
 * name to synchronize between multiple processes.
 */

#ifndef THOR_NAMED_MUTEX_H
#define THOR_NAMED_MUTEX_H
#pragma once

#ifndef THOR_BASETYPES_H
#include "basetypes.h"
#endif

#if defined(THOR_DEBUG) && !defined(THOR_BASIC_STRING_H)
#include "basic_string.h"
#endif

#if defined(_WIN32)
#include "win/named_mutex_win.inl"
#else
#error Unsupported platform!
#endif

namespace thor
{

class named_mutex : public internal::named_mutex_base
{
    THOR_DECLARE_NOCOPY(named_mutex);
public:
    explicit named_mutex(const char* name);
    explicit named_mutex(const wchar_t* name);
    ~named_mutex();

    bool lock(size_type timeout_ms = size_type(-1));
    bool try_lock();
    bool unlock();

private:
#ifdef THOR_DEBUG
    thor::wstring debug_name_;
    void set_debug_name(const wchar_t* name) { debug_name_ = name; }
#else
    void set_debug_name(const wchar_t*) {}
#endif
};

}

#endif