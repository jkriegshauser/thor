/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * memory.h
 *
 * ** THOR INTERNAL FILE - NOT FOR APPLICATION USE **
 *
 * This file defines memory functions required by THOR containers. It is not
 * intended for use outside of THOR.
 */

#ifndef THOR_MEMORY_H
#define THOR_MEMORY_H
#pragma once

#ifndef THOR_BASETYPES_H
#include "basetypes.h"
#endif

namespace thor
{

namespace memory
{

// Function that allocates raw memory aligned on the T_ALIGN boundary
// T_ALIGN must:
// - be greater than THOR_GUARANTEED_ALIGNMENT
// - be 128 or less
// - be a power of two
template <size_type T_ALIGN> inline thor_byte* align_alloc_raw(size_type size)
{
    THOR_COMPILETIME_ASSERT(T_ALIGN > THOR_GUARANTEED_ALIGNMENT, NeedSpecialization);
    THOR_COMPILETIME_ASSERT(T_ALIGN <= 128, InvalidAlign);
    THOR_COMPILETIME_ASSERT((T_ALIGN & (T_ALIGN - 1)) == 0, NonPowerOf2);

    thor_byte* p = new thor_byte[size + T_ALIGN];
    thor_byte* ret = (thor_byte*)((((size_type)(p + 1) + (T_ALIGN - 1)) & ~(T_ALIGN - 1)));
    *(ret - 1) = (thor_byte)(ret - p);
    return ret;
}

// Specialization that uses the default system alignment
template <> inline thor_byte* align_alloc_raw<0>(size_type size)
{
    thor_byte* p = new thor_byte[size];
    THOR_DEBUG_ASSERT(((size_type)p & (THOR_GUARANTEED_ALIGNMENT - 1)) == 0);
    return p;
}

// Function that frees raw memory previously allocated with align_alloc_raw().
// NOTE! The T_ALIGN parameters must match between alloc and free.
template <size_type T_ALIGN> inline void align_free_raw(thor_byte* p)
{
    THOR_COMPILETIME_ASSERT(T_ALIGN > THOR_GUARANTEED_ALIGNMENT, NeedSpecialization);
    THOR_COMPILETIME_ASSERT(T_ALIGN < 255, InvalidAlign);
    THOR_COMPILETIME_ASSERT((T_ALIGN & (T_ALIGN - 1)) == 0, NonPowerOf2);

    if (p != 0)
    {
        thor_byte* del = p - *(p - 1);
        delete[] del;
    }
}

// Specialization that frees memory specifically allocated with align_alloc_raw<0>
template <> inline void align_free_raw<0>(thor_byte* p)
{
    delete[] p;
}

// A simple alignment selector object. If the alignment required by T is less than
// or equal to the guaranteed alignment by the system, the selected alignment is zero
// which uses the zero specializations of align_alloc_raw and align_free_raw, above.
template <class T> struct align_selector
{
    enum { alignment = THOR_ALIGN_OF(T) > THOR_GUARANTEED_ALIGNMENT ? THOR_ALIGN_OF(T) : 0 };
};

// Returns p at a possibly higher address that aligns with T_ALIGN
template <size_type T_ALIGN> inline thor_byte* align_forward(thor_byte* p)
{
    THOR_COMPILETIME_ASSERT((T_ALIGN & (T_ALIGN-1)) == 0, NotPowerOf2);
    return (thor_byte*)(((size_type)p + (T_ALIGN-1)) & ~(T_ALIGN-1));
}

template <size_type T_ALIGN> inline const thor_byte* align_forward(const thor_byte* p)
{
    THOR_COMPILETIME_ASSERT((T_ALIGN & (T_ALIGN-1)) == 0, NotPowerOf2);
    return (const thor_byte*)(((size_type)p + (T_ALIGN-1)) & ~(T_ALIGN-1));
}

// Specialization for zero: does nothing
template <> inline thor_byte* align_forward<0>(thor_byte* p)
{
    return p;
}

template <> inline const thor_byte* align_forward<0>(const thor_byte* p)
{
    return p;
}

// Returns p at a possibly lower address that aligns with T_ALIGN
template <size_type T_ALIGN> inline thor_byte* align_back(thor_byte* p)
{
    THOR_COMPILETIME_ASSERT((T_ALIGN & (T_ALIGN-1)) == 0, NotPowerOf2);
    return (thor_byte*)((size_type)p & ~(T_ALIGN-1));
}

// Specialization for zero: does nothing
template <> inline thor_byte* align_back<0>(thor_byte* p)
{
    return p;
}

// Allocates memory for, but does not construct, 'count' T objects
template <class T, size_type T_ALIGN = align_selector<T>::alignment > class align_alloc
{
public:
    inline static T* alloc(size_type count = 1)
    {
        return (T*)align_alloc_raw<T_ALIGN>(count * sizeof(T));
    }

    inline static void free(T* p)
    {
        align_free_raw<T_ALIGN>((thor_byte*)p);
    }

    // Returns true if aligned correctly, false otherwise
    inline static bool is_aligned(T* p)
    {
        return align_back<T_ALIGN>((thor_byte*)p) == (thor_byte*)p;
    }
};

} // namespace memory

} // namespace thor

#endif
