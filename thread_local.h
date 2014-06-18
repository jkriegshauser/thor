/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * thread_local.h
 *
 * Platform independent thread-local variable support
 */

#if defined(WIN32)
#include "win/thread_local_base_win.inl"
#else
#error Unsupported platform!
#endif

namespace thor
{

template <typename T> class thread_local : protected internal::thread_local_base
{
public:
    typedef T value_type;
    typedef T* pointer;
    typedef T& reference;
    typedef const T* const_pointer;
    typedef const T& const_reference;

    thread_local() THOR_NOTHROW;
    ~thread_local() THOR_NOTHROW;

    value_type get() const THOR_NOTHROW;
    value_type set(value_type t) THOR_NOTHROW;
};

}

#include "thread_local.inl"