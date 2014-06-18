/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * hash_funcs.h
 *
 * ** THOR INTERNAL FILE - NOT FOR APPLICATION USE **
 *
 * This file defines STL-compatible hash functors
 */

#ifndef THOR_HASH_FUNCS_H
#define THOR_HASH_FUNCS_H
#pragma once

#ifndef THOR_BASETYPES_H
#include "basetypes.h"
#endif

namespace thor
{

// Hash functions

// Default (no implementation)
template <class T> struct hash
{
};

// Pointer specializations
template <class T> struct hash<T*>
{
    size_type operator () (const T* p) const
    {
        size_type s = (size_type)(void*)p;
        if (THOR_SUPPRESS_WARNING(sizeof(p) == 4))
        {
            return (s >> 2);
        }
        else if (THOR_SUPPRESS_WARNING(sizeof(p) == 8))
        {
            return (s >> 3);
        }
        THOR_ASSERT(0); // Unknown pointer size
        return s;
    }
};

#if 0
template <class T> struct hash<const T*>
{
    size_type operator () (const T* p) const
    {
        size_type s = (size_t)(void*)p;
        if (THOR_SUPPRESS_WARNING(sizeof(p) == 4))
        {
            return (s >> 2);
        }
        else if (THOR_SUPPRESS_WARNING(sizeof(p) == 8))
        {
            return (s >> 3);
        }
        THOR_ASSERT(0); // Unknown pointer size
        return s;
    }
};
#endif

// Intrinsic specializations
#define INTRINSIC_HASH(__type) \
    template <> struct hash< __type > \
{ \
    size_type operator () (__type t) const { return (size_type)t; } \
}

INTRINSIC_HASH(char);
INTRINSIC_HASH(unsigned char);
INTRINSIC_HASH(short);
INTRINSIC_HASH(unsigned short);
INTRINSIC_HASH(int);
INTRINSIC_HASH(unsigned int);
INTRINSIC_HASH(long);
INTRINSIC_HASH(unsigned long);
#ifdef _NATIVE_WCHAR_T_DEFINED
INTRINSIC_HASH(wchar_t);
#endif

#undef INTRINSIC_HASH

template <class T> size_type __hashstring(T *s)
{
    // Jenkins One-at-a-time hash function
    size_type hash = 0;
    while (*s != 0)
    {
        hash += *s;
        hash += (hash << 10);
        hash ^= (hash >> 6);
        s++;
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

template <class T> size_type __hashstring(T *s, size_type len)
{
    // Jenkins One-at-a-time hash function
    size_type hash = 0;
    const T* end = s + len;
    while (s < end)
    {
        hash += *s;
        hash += (hash << 10);
        hash ^= (hash >> 6);
        s++;
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

// String types specialization
template <> struct hash<char*>
{
    size_type operator () (const char* p) const { return __hashstring(p); }
};

template <> struct hash<const char*>
{
    size_type operator () (const char* p) const { return __hashstring(p); }
};

template <> struct hash<wchar_t*>
{
    size_type operator () (const wchar_t* p) const { return __hashstring(p); }
};

template <> struct hash<const wchar_t*>
{
    size_type operator () (const wchar_t* p) const { return __hashstring(p); }
};

// 64-bit POD type specializations
template <> struct hash< long long >
{
    size_type operator () (long long t) const
    {
        THOR_COMPILETIME_ASSERT(sizeof(t) == 8, InvalidSizeAssumption);
        THOR_COMPILETIME_ASSERT((sizeof(t)/sizeof(size_type)) <= 2, InvalidSizeAssumption2);
        if (THOR_SUPPRESS_WARNING(sizeof(size_type) == 4))
        {
            // 64-bit t, 32-bit size_t
            return (size_type(t) & size_type(-1)) ^ size_type(t >> 32);
        }
        else 
        {
            // Both t and size_t are 64-bit
            THOR_ASSERT(sizeof(size_type) == sizeof(t));
            return size_type(t);
        }
    }
};

template <> struct hash< unsigned long long >
{
    size_type operator () (unsigned long long t) const
    {
        THOR_COMPILETIME_ASSERT(sizeof(t) == 8, InvalidSizeAssumption);
        THOR_COMPILETIME_ASSERT((sizeof(t)/sizeof(size_type)) <= 2, InvalidSizeAssumption2);
        if (THOR_SUPPRESS_WARNING(sizeof(size_type) == 4))
        {
            // 64-bit t, 32-bit size_t
            return (size_type(t) & size_type(-1)) ^ size_type(t >> 32);
        }
        else 
        {
            // Both t and size_t are 64-bit
            THOR_ASSERT(sizeof(size_type) == sizeof(t));
            return (size_type)t;
        }
    }
};

// floating point specializations
template <> struct hash<float>
{
    size_type operator () (float f) const
    {
        THOR_COMPILETIME_ASSERT(sizeof(f) == sizeof(unsigned), InvalidSizeAssumption);
        return hash<unsigned>() (*(unsigned*)&f);
    }
};

template <> struct hash<double>
{
    size_type operator () (double d) const
    {
        THOR_COMPILETIME_ASSERT(sizeof(d) == sizeof(unsigned long long), InvalidSizeAssumption);
        return hash<unsigned long long>() (*(unsigned long long*)&d);
    }
};

template <> struct hash<long double>
{
    size_type operator () (long double d) const
    {
        THOR_COMPILETIME_ASSERT(sizeof(d) == sizeof(unsigned long long), InvalidSizeAssumption);
        return hash<unsigned long long>() (*(unsigned long long*)&d);
    }
};

}; // namespace thor

#endif

