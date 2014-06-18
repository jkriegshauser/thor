/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * win/atomic_pointer_win.inl
 *
 * Windows atomic_pointer implmentation
 */

#include "../atomic_pointer.h"
#include "interlocked_win.h"

namespace thor
{

template<typename T> atomic_pointer<T>::atomic_pointer()
: p_(0)
{
    THOR_DEBUG_ASSERT(((thor_size_type)(&p_) & (sizeof(pointer) - 1)) == 0); // Ensure we're properly aligned
}

template<typename T> atomic_pointer<T>::atomic_pointer(pointer p)
: p_(p)
{
    THOR_DEBUG_ASSERT(((thor_size_type)(&p_) & (sizeof(pointer) - 1)) == 0); // Ensure we're properly aligned
}

template<typename T> atomic_pointer<T>::atomic_pointer(const atomic_pointer& rhs)
: p_(rhs.p_)
{
    THOR_DEBUG_ASSERT(((thor_size_type)(&p_) & (sizeof(pointer) - 1)) == 0); // Ensure we're properly aligned
}

// Destructor
template<typename T> atomic_pointer<T>::~atomic_pointer()
{}

    // Operator overloading
template<typename T> atomic_pointer<T>& atomic_pointer<T>::operator = (pointer p)
{
    exchange(p);
    return *this;
}

template<typename T> atomic_pointer<T>& atomic_pointer<T>::operator = (const atomic_pointer& rhs)
{
    exchange(rhs.p_);
    return *this;
}
    
template<typename T> typename atomic_pointer<T>::pointer atomic_pointer<T>::operator -- ()
{
    return operator -= (1); // Use operator-= so that the pointer advances sizeof(value_type)
}

template<typename T> typename atomic_pointer<T>::pointer atomic_pointer<T>::operator -- (int)
{
    pointer p = operator -= (1); // Use operator-= so that the pointer advances sizeof(value_type)
    return ++p;
}

template<typename T> typename atomic_pointer<T>::pointer atomic_pointer<T>::operator ++ ()
{
    return operator += (1); // Use operator+= so that the pointer advances sizeof(value_type)
}

template<typename T> typename atomic_pointer<T>::pointer atomic_pointer<T>::operator ++ (int)
{
    pointer p = operator += (1); // Use operator+= so that the pointer advances sizeof(value_type)
    return --p;
}

template<typename T> typename atomic_pointer<T>::pointer atomic_pointer<T>::operator += (difference_type t)
{
    typedef internal::interlocked<pointer> interlocked;
    pointer old;
    do {
        old = p_;
    } while (interlocked::compare_exchange(&p_, old + t, old) != old);
    return old + t;
}

template<typename T> typename atomic_pointer<T>::pointer atomic_pointer<T>::operator -= (difference_type t)
{
    typedef internal::interlocked<pointer> interlocked;
    pointer old;
    do {
        old = p_;
    } while (interlocked::compare_exchange(&p_, old - t, old) != old);
    return old - t;
}
    
template<typename T> atomic_pointer<T>::operator pointer ()
{
    return get();
}

template<typename T> atomic_pointer<T>::operator const_pointer () const
{
    return get();
}

    // Explicit functions
template<typename T> typename atomic_pointer<T>::pointer atomic_pointer<T>::set(pointer t)
{
    exchange(t);
    return t;
}

template<typename T> typename atomic_pointer<T>::pointer atomic_pointer<T>::get()
{
    return p_;
}

template<typename T> typename atomic_pointer<T>::const_pointer atomic_pointer<T>::get() const
{
    return p_;
}

template<typename T> typename atomic_pointer<T>::pointer atomic_pointer<T>::exchange(pointer p)
{
    typedef internal::interlocked<pointer> interlocked;
    return interlocked::exchange(&p_, p);
}

template<typename T> typename atomic_pointer<T>::pointer atomic_pointer<T>::compare_exchange(pointer rhs, pointer comp)
{
    typedef internal::interlocked<pointer> interlocked;
    return interlocked::compare_exchange(&p_, rhs, comp);
}

};