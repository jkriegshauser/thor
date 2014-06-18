/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * string_util.h
 *
 * This file defines general-purpose string functions.
 */

#ifndef THOR_STRING_UTIL_H
#define THOR_STRING_UTIL_H
#pragma once

#ifndef THOR_BASETYPES_H
#include "basetypes.h"
#endif

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <locale>

namespace thor
{

// Returns c-style string length in characters (not bytes)
thor_size_type string_length(const char* s);
thor_size_type string_length(const wchar_t* s);

const char*    string_find(const char* s, char find);
const wchar_t* string_find(const wchar_t* s, wchar_t find);
const char*    string_find_right(const char* s, char find);
const wchar_t* string_find_right(const wchar_t* s, wchar_t find);

// String formatting
thor_size_type string_format_count(const char* s, ...);
thor_size_type string_format_count(const wchar_t* s, ...);
thor_size_type string_format_count_v(const char* s, va_list va);
thor_size_type string_format_count_v(const wchar_t* s, va_list va);
thor_size_type string_format(char* d, thor_size_type dlen, const char* s, ...);
thor_size_type string_format(wchar_t* d, thor_size_type dlen, const wchar_t* s, ...);
thor_size_type string_format_v(char* d, thor_size_type dlen, const char* s, va_list va);
thor_size_type string_format_v(wchar_t* d, thor_size_type dlen, const wchar_t* s, va_list va);

int string_compare(const char* lhs, const char* rhs);
int string_compare(const wchar_t* lhs, const wchar_t* rhs);
int string_compare_i(const char* lhs, const char* rhs);
int string_compare_i(const wchar_t* lhs, const wchar_t* rhs);

// Memory compare
template<typename T> int memory_compare(const T* lhs, const T* rhs, thor_size_type len);
int memory_compare_i(const char* lhs, const char* rhs, thor_size_type len);
int memory_compare_i(const wchar_t* lhs, const wchar_t* rhs, thor_size_type len);

///////////////////////////////////////////////////////////////////////////////
// Inline implementations
///////////////////////////////////////////////////////////////////////////////

inline thor_size_type string_length(const char* s)
{
    return s ? ::strlen(s) : 0;
}

inline thor_size_type string_length(const wchar_t* s)
{
    return s ? ::wcslen(s) : 0;
}

///////////////////////////////////////////////////////////////////////////////

inline const char* string_find(const char* s, char val)
{
    return s ? ::strchr(s, val) : 0;
}

inline const wchar_t* string_find(const wchar_t* s, wchar_t val)
{
    return s ? ::wcschr(s, val) : 0;
}

inline const char* string_find_right(const char* s, char val)
{
    return s ? ::strrchr(s, val) : 0;
}

inline const wchar_t* string_find_right(const wchar_t* s, wchar_t val)
{
    return s ? ::wcsrchr(s, val) : 0;
}

///////////////////////////////////////////////////////////////////////////////

inline thor_size_type string_format_count(const char* s, ...)
{
    va_list va;
    va_start(va, s);
    thor_size_type count = (thor_size_type)_vscprintf(s, va);
    va_end(va);
    return count;
}

inline thor_size_type string_format_count(const wchar_t* s, ...)
{
    va_list va;
    va_start(va, s);
    thor_size_type count = (thor_size_type)_vscwprintf(s, va);
    va_end(va);
    return count;
}

inline thor_size_type string_format_count_v(const char* s, va_list va)
{
    return (thor_size_type)_vscprintf(s, va);
}

inline thor_size_type string_format_count_v(const wchar_t* s, va_list va)
{
    return (thor_size_type)_vscwprintf(s, va);
}

inline thor_size_type string_format(char* d, thor_size_type dlen, const char* s, ...)
{
    va_list va;
    va_start(va, s);
    thor_size_type count = string_format_v(d, dlen, s, va);
    va_end(va);
    return count;
}

inline thor_size_type string_format(wchar_t* d, thor_size_type dlen, const wchar_t* s, ...)
{
    va_list va;
    va_start(va, s);
    thor_size_type count = string_format_v(d, dlen, s, va);
    va_end(va);
    return count;
}

inline thor_size_type string_format_v(char* d, thor_size_type dlen, const char* s, va_list va)
{
    return (thor_size_type)vsprintf_s(d, dlen, s, va);
}

inline thor_size_type string_format_v(wchar_t* d, thor_size_type dlen, const wchar_t* s, va_list va)
{
    return (thor_size_type)vswprintf_s(d, dlen, s, va);
}


///////////////////////////////////////////////////////////////////////////////

inline int string_compare(const char* lhs, const char* rhs)
{
    THOR_DEBUG_ASSERT(lhs && rhs);
    return ::strcmp(lhs, rhs);
}

inline int string_compare(const wchar_t* lhs, const wchar_t* rhs)
{
    THOR_DEBUG_ASSERT(lhs && rhs);
    return ::wcscmp(lhs, rhs);
}

inline int string_compare_i(const char* lhs, const char* rhs)
{
    THOR_DEBUG_ASSERT(lhs && rhs);
    return ::_stricmp(lhs, rhs);
}

inline int string_compare_i(const wchar_t* lhs, const wchar_t* rhs)
{
    THOR_DEBUG_ASSERT(lhs && rhs);
    return ::_wcsicmp(lhs, rhs);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T> inline int memory_compare(const T* lhs, const T* rhs, thor_size_type len)
{
    return ::memcmp(lhs, rhs, len * sizeof(T));
}

inline int memory_compare_i(const char* lhs, const char* rhs, thor_size_type len)
{
    const char* lhs_end = lhs + len;
    while (lhs != lhs_end)
    {
        int lc = towlower(*lhs), rc = towlower(*rhs);
        if (lc != rc)
        {
            return lc - rc;
        }
        ++lhs, ++rhs;
    }
    return 0;
}

inline int memory_compare_i(const wchar_t* lhs, const wchar_t* rhs, thor_size_type len)
{
    const wchar_t* lhs_end = lhs + len;
    while (lhs != lhs_end)
    {
        int lc = towlower(*lhs), rc = towlower(*rhs);
        if (lc != rc)
        {
            return lc - rc;
        }
        ++lhs, ++rhs;
    }
    return 0;
}

}

#endif