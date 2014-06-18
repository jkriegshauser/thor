/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * win/thread_local_base_win.h
 *
 * Windows-specific thread-local base implementation
 */

#ifndef THOR_BASETYPES_H
#include "../basetypes.h"
#endif

namespace thor
{

namespace internal
{

struct thread_local_base
{
    typedef void* base_value_type;
    unsigned long tls_index_;

    thread_local_base();
    ~thread_local_base();

    base_value_type get() const;
    base_value_type set(base_value_type t);
};

}

}