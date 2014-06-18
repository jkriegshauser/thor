/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * basetypes.h
 *
 * ** THOR INTERNAL FILE - NOT FOR APPLICATION USE **
 *
 * This file defines base types needed by all THOR containers.  It should only need to be
 * included by THOR files, never by application code.
 */

#ifndef THOR_BASETYPES_H
#define THOR_BASETYPES_H
#pragma once

#ifdef _DEBUG
#define THOR_DEBUG 1
#endif

// GCC and Visual Studio apparently want this different
#ifdef _MSC_VER
#define THOR_TYPENAME typename
#define THOR_ALIGN_OF(type) __alignof(type)
#define THOR_ALIGN(align_, declaration_) __declspec(align(align_)) declaration_
#define THOR_NOTHROW throw()
#pragma warning(disable:4324) // warning: structure was padded due to __declspec(align())
#else
#define THOR_TYPENAME
#define THOR_ALIGN_OF(type) __alignof__(type)
#define THOR_ALIGN(align_, declaration_) declaration_ __attribute__ ((aligned (align_)))
#define THOR_NOTHROW /*nothrow for linux does nothing currently*/
#endif

#include <stddef.h>

typedef size_t        thor_size_type;
typedef ptrdiff_t     thor_diff_type;
typedef unsigned char thor_byte;

typedef unsigned __int8     byte;
typedef unsigned   int      uint;
typedef unsigned __int8     uint8;
typedef unsigned __int16    uint16;
typedef unsigned __int32    uint32;
typedef unsigned __int64    uint64;
typedef signed   __int8     int8;
typedef signed   __int16    int16;
typedef signed   __int32    int32;
typedef signed   __int64    int64;

namespace thor
{

typedef thor_size_type size_type;
typedef thor_diff_type difference_type;

enum { THOR_GUARANTEED_ALIGNMENT = (2 * sizeof(thor_size_type)) };

}; // namespace thor

#ifndef THOR_ASSERT
    #ifdef THOR_DEBUG
        #define THOR_ASSERT(expr) (void)((!!(expr))||(__debugbreak(), 1))
    #else
        #define THOR_ASSERT(expr) static_cast<void>(0)
    #endif
#endif

#ifdef THOR_DEBUG
#define THOR_DEBUG_ASSERT(expr) THOR_ASSERT(expr)
#else
#define THOR_DEBUG_ASSERT(expr) static_cast<void>(0)
#endif

#if defined(THOR_CHECKED_BUILD) || defined(THOR_DEBUG)
#define THOR_CHECKED(expr)  (void)((!!(expr))||(__debugbreak(), 1))
#else
#define THOR_CHECKED(expr)  static_cast<void>(0)
#endif

#define THOR_COMPILETIME_ASSERT(expr,msg) typedef char ERROR_##msg[1][(expr)]

inline int SuppressWarning( int i ) { return i; }
#define THOR_SUPPRESS_WARNING(expr) SuppressWarning((expr))

#define THOR_UNUSED(expr) static_cast<void>(expr)

#define THOR_OFFSET_OF(__struct, __member) (size_t)((thor_byte*)&(((__struct*)1)->__member) - (thor_byte*)1)

#define THOR_INIT_MEM(__addr, __bytes, __pattern) memset((__addr), (__pattern), (__bytes))

#define THOR_MATCH_TYPE(t1, t2) THOR_ASSERT((t1*)0 == (t2*)0)

#ifdef THOR_DEBUG
#define THOR_DEBUG_INIT_MEM(__addr, __bytes, __pattern) THOR_INIT_MEM(__addr, __bytes, __pattern)
#else
#define THOR_DEBUG_INIT_MEM(__addr, __bytes, __pattern) static_cast<void>(0)
#endif

#define THOR_DECLARE_NOCOPY(__class) \
    private: \
    __class(const __class&); \
    __class& operator = (const __class&)

// Block some specific warnings
#ifdef _MSC_VER
#pragma warning(disable:4345) /* behavior change: an object of POD type constructed with an initializer of the form() will be default-initialized */
#pragma warning(disable:4180) /* qualifier applied to function type has no meaning; ignored */
#endif

#endif

