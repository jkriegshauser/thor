/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * named_semaphore.h
 *
 * Defines a platform-agnostic semaphore class. This mutex can be given a system-wide
 * name to synchronize between multiple processes.
 */

#ifndef THOR_NAMED_SEMAPHORE_H
#define THOR_NAMED_SEMAPHORE_H
#pragma once

#ifndef THOR_BASETYPES_H
#include "basetypes.h"
#endif

#if defined(THOR_DEBUG) && !defined(THOR_BASIC_STRING_H)
#include "basic_string.h"
#endif

#if defined(_WIN32)
#include "win/semaphore_win.inl"
#else
#error Unsupported Platform!
#endif

namespace thor
{

class named_semaphore : public internal::semaphore_base
{
    THOR_DECLARE_NOCOPY(named_semaphore);
public:
    named_semaphore(const char* name, size_type released = 0, size_type max_count = size_type(-1));
    named_semaphore(const wchar_t* name, size_type released = 0, size_type max_count = size_type(-1));
    ~named_semaphore();

    bool wait(size_type timeout_ms = size_type(-1));
    bool try_wait();
    bool release(size_type count = 1);

private:
#ifdef THOR_DEBUG
    wstring debug_name_;
    void set_debug_name(const wchar_t* name) { debug_name_ = name; }
#else
    void set_debug_name(const wchar_t*) {}
#endif
};

}

#endif