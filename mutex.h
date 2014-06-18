/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * mutex.h
 *
 * Defines a platform-agnostic mutex class
 */

#ifndef THOR_MUTEX_H
#define THOR_MUTEX_H
#pragma once

#ifndef THOR_BASE_TYPES_H
#include "basetypes.h"
#endif

#if defined(_WIN32)
#include "win/mutex_win.inl"
#else
#error Unsupported platform!
#endif

namespace thor
{

class mutex : private internal::mutex_base
{
    THOR_DECLARE_NOCOPY(mutex);
public:
    mutex(size_type spin_count = 4000);
    ~mutex();

    bool lock();
    bool try_lock();
    bool unlock();
};

///////////////////////////////////////////////////////////////////////////////

template<class T> class scope_locker
{
    THOR_DECLARE_NOCOPY(scope_locker);
    T& lockable_;
    bool locked_;
public:
    scope_locker(T& lockable)
        : lockable_(lockable)
        , locked_(false)
    {
        lock();
    }

    ~scope_locker()
    {
        unlock();
    }

    void lock()
    {
        if (!locked_)
        {
            lockable_.lock();
            locked_ = true;
        }
    }

    void unlock()
    {
        if (locked_)
        {
            lockable_->unlock();
            locked_ = false;
        }
    }
};

}

#endif