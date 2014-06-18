/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * typetraits.h
 *
 * ** THOR INTERNAL FILE - NOT FOR APPLICATION USE **
 *
 * This file defines type traits needed by all THOR containers.  It should only need to be
 * included by THOR files, never by application code.
 */

#ifndef THOR_TYPETRAITS_H
#define THOR_TYPETRAITS_H
#pragma once

#ifndef THOR_BASETYPES_H
#include "basetypes.h"
#endif

#ifndef THOR_ITERATOR_H
#include "iterator.h"
#endif

#include <memory.h>
#include <string.h>
#include <new>

#pragma warning(push)
#pragma warning(disable:4100) // unreferenced formal parameter

namespace thor
{

// typetraits notes
// range_move() must construct at the destination and destroy at the source
// copy() must keep source in a known state
// copy_overlap() and copy_backwards() can leave the source in a modified state, but they must not destruct the source

// Template specialization for non-plain-old-data types
template <class T> struct typetraits
{
    static void construct(T* p){ new (p) T(); }
    template<class T1> static void construct(T* p, const T1& t1){ new (p) T(t1); }
    template<class T1, class T2> static void construct(T* p, const T1& t1, const T2& t2){ new (p) T(t1, t2); }
    template<class T1, class T2, class T3> static void construct(T* p, const T1& t1, const T2& t2, const T3& t3){ new (p) T(t1, t2, t3); }
    template<class T1, class T2, class T3, class T4> static void construct(T* p, const T1& t1, const T2& t2, const T3& t3, const T4& t4){ new (p) T(t1, t2, t3, t4); }
    static void destruct(T* p){ p->~T(); }
    static void range_destruct(T* p1, T* p2){ for(; p1 != p2; ++p1) p1->~T(); }
    static void range_construct(T* p1, T* p2){ for(; p1 != p2; ++p1) new (p1) T(); }
    static void range_construct(T* p1, T* p2, const T& t){ for(; p1 != p2; ++p1) new (p1) T(t); }
    static void range_construct(T* p1, T* p2, const T* s){ for(; p1 != p2; ++p1, ++s) new (p1) T(*s); }
    static void range_move(T* p1, T* p2, T* s) { for(; p1 != p2; ++p1, ++s) { new (p1) T(*s); s->~T(); } }
    static void range_copy(T* p1, T* p2, const T& t) { for (; p1 != p2; ++p1) { *p1 = t; } }
    static void copy(T* d, const T* s, size_t n){ for(; n; --n, ++d, ++s) *d = *s; }
    static void copy_overlap(T* d, T* s, size_t n){ for(; n; --n, ++d, ++s) *d = *s; }
    static void copy_backwards(T* d, T* s, size_t n){ for(d += n, s += n; n; --n) *--d = *--s; }
};

// Template specialization for pointer types
template <class T> struct typetraits<T*>
{
    static void construct(T** p){ *p = 0; }
    static void construct(T** p, T* t){*p=t;}
    static void range_destruct(T** , T**) {}
    static void range_construct(T** p1, T** p2) { memset(p1, 0, (p2 - p1) * sizeof(T*)); }
    static void range_construct(T** p1, T** p2, T* t){ for(; p1 != p2; ++p1) *p1 = t; }
    static void range_construct(T** p1, T** p2, T** s){ memcpy(p1, s, (p2 - p1) * sizeof(T*)); }
    static void range_move(T** p1, T** p2, T** s) { memcpy(p1, s, (p2 - p1) * sizeof(T*)); }
    static void range_copy(T** p1, T** p2, T* t) { for(; p1 != p2; ++p1) *p1 = t; }
    static void destruct(T**){}
    static void copy(T** d, T** s, size_t n){ memcpy(d, s, n * sizeof(T*)); }
    static void copy_overlap(T** d, T** s, size_t n){ memmove(d, s, n * sizeof(T*)); }
    static void copy_backwards(T** d, T** s, size_t n){ memmove(d, s, n * sizeof(T*)); }
};

// Template specializations for plain-old-data types
#define DEFINE_POD_TYPETRAITS(T) \
    template <> struct typetraits<T> \
    { \
        static void construct(T* p){ *p = 0; } \
        static void construct(T* p, const T &t){*p=t;} \
        static void range_destruct(T* , T*) {} \
        static void range_construct(T* p1, T* p2) { memset(p1, 0, (p2 - p1) * sizeof(T)); } \
        static void range_construct(T* p1, T* p2, const T& t){ for(; p1 != p2; ++p1) *p1 = t; } \
        static void range_construct(T* p1, T* p2, const T* s){ memcpy(p1, s, (p2 - p1) * sizeof(T)); } \
        static void range_move(T* p1, T* p2, T* s) { memcpy(p1, s, (p2 - p1) * sizeof(T)); } \
        static void range_copy(T* p1, T* p2, const T& t) { for(; p1 != p2; ++p1) *p1 = t; } \
        static void destruct(T*){} \
        static void copy(T* d, const T* s, size_t n){ memcpy(d, s, n * sizeof(T)); } \
        static void copy_overlap(T* d, T* s, size_t n){ memmove(d, s, n * sizeof(T)); } \
        static void copy_backwards(T* d, T* s, size_t n){ memmove(d, s, n * sizeof(T)); } \
    }

DEFINE_POD_TYPETRAITS(bool);
DEFINE_POD_TYPETRAITS(char);
DEFINE_POD_TYPETRAITS(unsigned char);
DEFINE_POD_TYPETRAITS(short);
DEFINE_POD_TYPETRAITS(unsigned short);
DEFINE_POD_TYPETRAITS(int);
DEFINE_POD_TYPETRAITS(unsigned int);
DEFINE_POD_TYPETRAITS(long);
DEFINE_POD_TYPETRAITS(unsigned long);
DEFINE_POD_TYPETRAITS(long long);
DEFINE_POD_TYPETRAITS(unsigned long long);
DEFINE_POD_TYPETRAITS(float);
DEFINE_POD_TYPETRAITS(double);
DEFINE_POD_TYPETRAITS(long double);
#ifdef _NATIVE_WCHAR_T_DEFINED
DEFINE_POD_TYPETRAITS(wchar_t);
#endif

#undef DEFINE_POD_TYPETRAITS

} // namespace thor

#pragma warning( pop )

#endif /*THOR_TYPETRAITS_H*/

