/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * win/semaphore_win.inl
 *
 * Defines a platform-agnostic semaphore class
 *
 * This is the Windows platform implementation
 */

#ifndef THOR_BASETYPES_H
#include "../basetypes.h"
#endif

namespace thor
{

namespace internal
{

struct semaphore_base
{
    void* handle_;

    semaphore_base() : handle_(0) {}
};

}

}