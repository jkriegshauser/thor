/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * semaphore.h
 *
 * Defines a platform-agnostic semaphore class
 */

#ifndef THOR_SEMAPHORE_H
#define THOR_SEMAPHORE_H
#pragma once

#ifndef THOR_BASETYPES_H
#include "basetypes.h"
#endif

#if defined(_WIN32)
#include "win/semaphore_win.inl"
#else
#error Unsupported platform!
#endif

namespace thor
{

class semaphore : public internal::semaphore_base
{
    THOR_DECLARE_NOCOPY(semaphore);
public:
    semaphore(size_type released = 0, size_type max_count = size_type(-1));
    ~semaphore();

    bool wait(size_type timeout_ms = size_type(-1));
    bool try_wait();
    bool release(size_type count = 1);
};

}

#endif