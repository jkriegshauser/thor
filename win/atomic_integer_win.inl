/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * win/atomic_integer_win.inl
 *
 * Windows atomic_integer implmentation
 */

#include "../atomic_integer.h"
#include "interlocked_win.h"

namespace thor
{

template<typename T> atomic_integer<T>::atomic_integer()
: integer_(0)
{
    // TODO: validate T is an integer POD type
    THOR_DEBUG_ASSERT(((thor_size_type)(&integer_) & (sizeof(T) - 1)) == 0); // Ensure we're properly aligned
}

template<typename T> atomic_integer<T>::atomic_integer(const T t)
: integer_(t)
{
    THOR_DEBUG_ASSERT(((thor_size_type)(&integer_) & (sizeof(T) - 1)) == 0); // Ensure we're properly aligned
}

template<typename T> atomic_integer<T>::atomic_integer(const atomic_integer& rhs)
: integer_(rhs.integer_)
{
    THOR_DEBUG_ASSERT(((thor_size_type)(&integer_) & (sizeof(T) - 1)) == 0); // Ensure we're properly aligned
}

// Destructor
template<typename T> atomic_integer<T>::~atomic_integer()
{}

    // Operator overloading
template<typename T> T atomic_integer<T>::operator = (const T t)
{
    exchange(t);
    return t;
}

template<typename T> T atomic_integer<T>::operator = (const atomic_integer& rhs)
{
    const T t = rhs.get();
    exchange(t);
    return t;
}
    
template<typename T> T atomic_integer<T>::operator -- ()
{
    typedef internal::interlocked<T> interlocked;
    return interlocked::decrement(&integer_);
}

template<typename T> T atomic_integer<T>::operator -- (int)
{
    typedef internal::interlocked<T> interlocked;
    T t = interlocked::decrement(&integer_);
    return ++t;
}

template<typename T> T atomic_integer<T>::operator ++ ()
{
    typedef internal::interlocked<T> interlocked;
    return interlocked::increment(&integer_);
}

template<typename T> T atomic_integer<T>::operator ++ (int)
{
    typedef internal::interlocked<T> interlocked;
    T t = interlocked::increment(&integer_);
    return --t;
}

template<typename T> T atomic_integer<T>::operator += (T rhs)
{
    typedef internal::interlocked<T> interlocked;
    T old;
    do {
        old = integer_;
    } while (interlocked::compare_exchange(&integer_, old + rhs, old) != old);
    return old + rhs;
}

template<typename T> T atomic_integer<T>::operator -= (T rhs)
{
    typedef internal::interlocked<T> interlocked;
    T old;
    do {
        old = integer_;
    } while (interlocked::compare_exchange(&integer_, old - rhs, old) != old);
    return old - rhs;
}
    
template<typename T> atomic_integer<T>::operator T () const
{
    return get();
}

    // Explicit functions
template<typename T> T atomic_integer<T>::set(const T t)
{
    exchange(t);
    return t;
}

template<typename T> T atomic_integer<T>::get() const
{
    return integer_;
}

template<typename T> T atomic_integer<T>::exchange(const T rhs)
{
    typedef internal::interlocked<T> interlocked;
    return interlocked::exchange(&integer_, rhs);
}

template<typename T> T atomic_integer<T>::compare_exchange(const T rhs, const T comp)
{
    typedef internal::interlocked<T> interlocked;
    return interlocked::compare_exchange(&integer_, rhs, comp);
}

};