/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * atomic_pointer.h
 *
 * This file defines a pointer wrapper that allows for thread-safe atomic operations.
 */

#ifndef THOR_ATOMIC_POINTER_H
#define THOR_ATOMIC_POINTER_H
#pragma once

#ifndef THOR_BASETYPES_H
#include "basetypes.h"
#endif

namespace thor
{

template <typename T> class atomic_pointer
{
public:
    typedef T  value_type;
    typedef T* pointer;
    typedef T& reference;
    typedef const T* const_pointer;
    typedef const T& const_reference;
    typedef thor_size_type size_type;
    typedef thor_diff_type difference_type;

    // Constructors
    explicit atomic_pointer();
    atomic_pointer(pointer p);
    atomic_pointer(const atomic_pointer& rhs);

    // Destructor
    ~atomic_pointer();

    // Operator overloading
    atomic_pointer& operator = (pointer p);
    atomic_pointer& operator = (const atomic_pointer& rhs);
    pointer         operator -- ();     /* --atomic_pointer - correctly decrements sizeof(T) */
    pointer         operator -- (int);  /* atomic_pointer-- - correctly decrements sizeof(T) */
    pointer         operator ++ ();     /* ++atomic_pointer - correctly increments sizeof(T) */
    pointer         operator ++ (int);  /* atomic_pointer++ - correctly increments sizeof(T) */
    pointer         operator += (difference_type t);
    pointer         operator -= (difference_type t);

    operator pointer ();
    operator const_pointer () const;

    pointer set(pointer p); // returns the value passed in
    pointer get();
    const_pointer get() const;

    // Atomically equivalent to:
    //   T* temp = *this;
    //   *this = p;
    //   return temp;
    pointer exchange(pointer p);

    // Atomically equivalent to:
    //   T* temp = *this;
    //   if (*this == comp) *this = rhs;
    //   return temp;
    // (returns comp if successfully set to rhs,
    //  current value otherwise)
    pointer compare_exchange(pointer rhs, pointer comp);

private:
    pointer p_;
};

}

// Inline implementations
#if defined(WIN32)
#include "win/atomic_pointer_win.inl"
#else
#error Unsupported system!
#endif

#endif