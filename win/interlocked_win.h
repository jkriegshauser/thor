/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * win/interlocked_win.h
 *
 * Windows interlocked wrapper implmentation
 */

#ifndef THOR_INTERLOCKED_WIN_H
#define THOR_INTERLOCKED_WIN_H
#pragma once

// #include <intrin.h>

// Definitions from intrin.h reproduced here so as to not carry intrin.h and windows.h everywhere.
extern "C" {
char _InterlockedIncrement8(char volatile *);
char _InterlockedDecrement8(char volatile *);
char _InterlockedCompareExchange8(char volatile *, char, char);
char _InterlockedExchange8(char volatile *, char);

short _InterlockedIncrement16(short volatile *);
short _InterlockedDecrement16(short volatile *);
short _InterlockedCompareExchange16(short volatile *, short, short);
short _InterlockedExchange16(short volatile *, short);

long __cdecl _InterlockedIncrement(long volatile *);
long __cdecl _InterlockedDecrement(long volatile *);
long __cdecl _InterlockedCompareExchange(long volatile *, long, long);
long __cdecl _InterlockedExchange(long volatile *, long);

__int64 _InterlockedCompareExchange64(__int64 volatile *, __int64, __int64);
__int64 _InterlockedIncrement64(__int64 volatile *);
__int64 _InterlockedDecrement64(__int64 volatile *);
__int64 _InterlockedExchange64(__int64 volatile *, __int64);
};

// inc/dec for 8-bit doesn't exist, so do our own:
inline char _InterlockedIncrement8(char volatile *p)
{
    char old;
    do
    {
        old = *p;
    } while (_InterlockedCompareExchange8(p, old + 1, old) != old);
    return old + 1;
}
inline char _InterlockedDecrement8(char volatile *p)
{
    char old;
    do
    {
        old = *p;
    } while (_InterlockedCompareExchange8(p, old - 1, old) != old);
    return old - 1;
}

#pragma intrinsic(_InterlockedCompareExchange8)
#pragma intrinsic(_InterlockedExchange8)

#pragma intrinsic(_InterlockedIncrement16)
#pragma intrinsic(_InterlockedDecrement16)
#pragma intrinsic(_InterlockedCompareExchange16)
#pragma intrinsic(_InterlockedExchange16)

#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(_InterlockedDecrement)
#pragma intrinsic(_InterlockedCompareExchange)
#pragma intrinsic(_InterlockedExchange)

#pragma intrinsic(_InterlockedCompareExchange64)
#if defined(_M_X64)
#pragma intrinsic(_InterlockedIncrement64)
#pragma intrinsic(_InterlockedDecrement64)
#pragma intrinsic(_InterlockedExchange64)
#else
// These functions are only intrinsics on 64-bit targets, so do our own for 32-bit targets.
inline __int64 _InterlockedIncrement64(__int64 volatile *p)
{
    __int64 old;
    do
    {
        old = *p;
    } while (_InterlockedCompareExchange64(p, old + 1, old) != old);
    return old + 1;
}
inline __int64 _InterlockedDecrement64(__int64 volatile *p)
{
    __int64 old;
    do
    {
        old = *p;
    } while (_InterlockedCompareExchange64(p, old - 1, old) != old);
    return old - 1;
}
inline __int64 _InterlockedExchange64(__int64 volatile *p, __int64 val)
{
    __int64 old;
    do 
    {
        old = *p;
    } while (_InterlockedCompareExchange64(p, val, old) != old);
    return old;
}
#endif

namespace thor
{

namespace internal
{

template<typename T, typename INT_T> struct convert_int_type
{
    static volatile INT_T* as_pointer(volatile T* t)
    {
        THOR_COMPILETIME_ASSERT(sizeof(INT_T) == sizeof(T), InvalidSizeAssumption);
        return (volatile INT_T*)(t);
    }
    static INT_T as_value(const T& t)
    {
        return *(INT_T*)(&t);
    }
    static T as_type(INT_T l)
    {
        return *(T*)(&l);
    }
};

// default interlocked structure. no functions defined as this should fail to compile
template<typename T, size_type T_SIZE = sizeof(T)> struct interlocked
{
};

// partial specializations for size to match to the correct interlocked function sets
template<typename T> struct interlocked<T, 1>
{
    typedef convert_int_type<T, char> cvt;
    static T increment(T volatile *a) { return cvt::as_type(_InterlockedIncrement8(cvt::as_pointer(a))); }
    static T decrement(T volatile *a) { return cvt::as_type(_InterlockedDecrement8(cvt::as_pointer(a))); }
    static T compare_exchange(T volatile *a, T b, T c) { return cvt::as_type(_InterlockedCompareExchange8(cvt::as_pointer(a), cvt::as_value(b), cvt::as_value(c))); }
    static T exchange(T volatile *a, T b) { return cvt::as_type(_InterlockedExchange8(cvt::as_pointer(a), cvt::as_value(b))); }
};

template<typename T> struct interlocked<T, 2>
{
    typedef convert_int_type<T, short> cvt;
    static T increment(T volatile *a) { return cvt::as_type(_InterlockedIncrement16(cvt::as_pointer(a))); }
    static T decrement(T volatile *a) { return cvt::as_type(_InterlockedDecrement16(cvt::as_pointer(a))); }
    static T compare_exchange(T volatile *a, T b, T c) { return cvt::as_type(_InterlockedCompareExchange16(cvt::as_pointer(a), cvt::as_value(b), cvt::as_value(c))); }
    static T exchange(T volatile *a, T b) { return cvt::as_type(_InterlockedExchange16(cvt::as_pointer(a), cvt::as_value(b))); }
};

template<typename T> struct interlocked<T, 4>
{
    typedef convert_int_type<T, long> cvt;
    static T increment(T volatile *a) { return cvt::as_type(_InterlockedIncrement(cvt::as_pointer(a))); }
    static T decrement(T volatile *a) { return cvt::as_type(_InterlockedDecrement(cvt::as_pointer(a))); }
    static T compare_exchange(T volatile *a, T b, T c) { return cvt::as_type(_InterlockedCompareExchange(cvt::as_pointer(a), cvt::as_value(b), cvt::as_value(c))); }
    static T exchange(T volatile *a, T b) { return cvt::as_type(_InterlockedExchange(cvt::as_pointer(a), cvt::as_value(b))); }
};

template<typename T> struct interlocked<T, 8>
{
    typedef convert_int_type<T, __int64> cvt;
    static T increment(T volatile *a) { return cvt::as_type(_InterlockedIncrement64(cvt::as_pointer(a))); }
    static T decrement(T volatile *a) { return cvt::as_type(_InterlockedDecrement64(cvt::as_pointer(a))); }
    static T compare_exchange(T volatile *a, T b, T c) { return cvt::as_type(_InterlockedCompareExchange64(cvt::as_pointer(a), cvt::as_value(b), cvt::as_value(c))); }
    static T exchange(T volatile *a, T b) { return cvt::as_type(_InterlockedExchange64(cvt::as_pointer(a), cvt::as_value(b))); }
};

}

}

#endif