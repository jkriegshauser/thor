/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * win/mutex_win.inl
 *
 * Inline definitions for Windows portion of mutex
 */

#ifndef THOR_BASETYPES_H
#include "../basetypes.h"
#endif

// Mimicked from Windows.h (Winnt.h)
struct _RTL_CRITICAL_SECTION;

namespace thor
{

namespace internal
{

struct mutex_base
{
#ifdef THOR_DEBUG
    mutex_base() : debug_critical_section_(*get_critical_section()) {}
    struct _RTL_CRITICAL_SECTION& debug_critical_section_;
#endif

    // Enough space for CRITICAL_SECTION
    thor_byte space_[(4 * sizeof(size_type)) + (2 * 4)];

    struct _RTL_CRITICAL_SECTION* get_critical_section()
    {
        return (_RTL_CRITICAL_SECTION*)space_;
    }
    THOR_DECLARE_NOCOPY(mutex_base);
};

}

}