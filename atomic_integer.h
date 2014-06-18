/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * atomic_integer.h
 *
 * This file defines a number where operations performed against
 * it are threadsafe. It is valid to define it for any integer
 * type that is 8-, 16-, 32- or 64-bits. Using the word-size of
 * the machine is most efficient.
 */

#ifndef THOR_ATOMIC_INTEGER_H
#define THOR_ATOMIC_INTEGER_H
#pragma once

#ifndef THOR_BASETYPES_H
#include "basetypes.h"
#endif

namespace thor
{

template <typename T> class atomic_integer
{
public:
    // STL-compatible typedefs
    typedef T value_type;
    typedef T* pointer;
    typedef T& reference;
    typedef const T* const_pointer;
    typedef const T& const_reference;
    typedef thor_size_type size_type;
    typedef thor_diff_type difference_type;

    // Constructors
    explicit atomic_integer();
    atomic_integer(const T t);
    atomic_integer(const atomic_integer& rhs);

    // Destructor
    ~atomic_integer();

    // Operator overloading
    T operator = (const T t);
    T operator = (const atomic_integer& t);
    T operator -- ();     /* --atomic_integer */
    T operator -- (int);  /* atomic_integer-- */
    T operator ++ ();     /* ++atomic_integer */
    T operator ++ (int);  /* atomic_integer++ */
    T operator += (T rhs);
    T operator -= (T rhs);
    
    operator T () const;

    // Explicit functions
    T set(const T t);    // returns the value passed in
    T get() const;

    // Atomically equivalent to:
    //   T temp = *this;
    //   *this = rhs;
    //   return temp;
    T exchange(const T rhs);
    
    // Atomically equivalent to:
    //   T temp = *this;
    //   if (*this == comp) *this = rhs;
    //   return temp;
    // (returns comp if successfully set to rhs,
    //  current value otherwise).
    T compare_exchange(const T rhs, const T comp);

private:
    volatile T integer_;
};

}

// Inline implementations
#if defined(WIN32)
#include "win/atomic_integer_win.inl"
#else
#error Unsupported system!
#endif

#endif