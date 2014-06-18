/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * win/thread_local_win.cpp;
 *
 * Windows-specific thread-local implementation
 */

#include "../thread_local.h"

#define WIN32_EXTRA_LEAN 1
#include <Windows.h>

namespace thor
{

namespace internal
{

thread_local_base::thread_local_base()
    : tls_index_(::TlsAlloc())
{
    THOR_CHECKED(tls_index_ != TLS_OUT_OF_INDEXES);
}

thread_local_base::~thread_local_base()
{
    ::TlsFree(tls_index_);
}

thread_local_base::base_value_type thread_local_base::get() const
{
    return ::TlsGetValue(tls_index_);
}

thread_local_base::base_value_type thread_local_base::set(base_value_type t)
{
    BOOL b = ::TlsSetValue(tls_index_, t);
    THOR_CHECKED(b == TRUE); THOR_UNUSED(b);
    return t;
}

}

}