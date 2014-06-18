/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * auto_closer.h
 *
 * Template class for automatically closing resources (i.e. Windows HANDLE objects, etc)
 */

#ifndef THOR_AUTO_CLOSER_H
#define THOR_AUTO_CLOSER_H
#pragma once

#ifndef THOR_BASETYPES_H
#include "basetypes.h"
#endif

namespace thor
{
namespace internal
{

template<class T, T INVALID, typename T_CLOSEFUNC> class auto_closer
{
    THOR_DECLARE_NOCOPY(auto_closer);
    struct wrapper : public T_CLOSEFUNC
    {
        T t_;
        wrapper(T t) : t_(t) {};
        wrapper(T t, T_CLOSEFUNC func) : T_CLOSEFUNC(func), t_(t) {}
    } wrapper_;
public:
    auto_closer(T t) : wrapper_(t) {}
    auto_closer(T t, T_CLOSEFUNC func) : wrapper_(t, func) {}
    ~auto_closer()
    {
        close();
    }

    void close()
    {
        if (wrapper_.t_ != INVALID)
        {
            static_cast<T_CLOSEFUNC&>(wrapper_)(wrapper_.t_);
            wrapper_.t_ = INVALID;
        }
    }

    bool valid() const
    {
        return wrapper_.t_ != INVALID;
    }

    operator T () const
    {
        return wrapper_.t_;
    }
};

} // namespace internal
} // namespace thor

#endif