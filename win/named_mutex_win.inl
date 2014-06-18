/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * win/named_mutex_win.inl
 *
 * Defines a platform-agnostic mutex class. This mutex can be given a system-wide
 * name to synchronize between multiple processes.
 *
 * This file defines the Windows base implementation
 */

namespace thor
{

namespace internal
{

struct named_mutex_base
{
    void* handle_;

    named_mutex_base() : handle_(0) {}
};

}

}