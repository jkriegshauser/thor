/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * pair.h
 *
 * This file defines a red-black tree to be used as a base for tree-type containers (map, set, multimap, multiset)
 *
 * Extension:
 * - constructor that default-constructs the second object
 * - make_pair() that default-constructs the second object
 */

#ifndef THOR_PAIR_H
#define THOR_PAIR_H
#pragma once

#ifndef THOR_BASETYPES_H
#include "basetypes.h"
#endif

namespace thor
{

template <class T1, class T2>
class pair
{
public:
    typedef T1 first_type;
    typedef T2 second_type;

    first_type first;
    second_type second;

    pair() : first(), second() {}
    pair(const first_type& f) : first(f), second() {}
    pair(const first_type& f, const second_type& s) : first(f), second(s) {}
    pair(const pair& p) : first(p.first), second(p.second) {}
    template <class U1, class U2> pair(const pair<U1,U2>& p) : first(p.first), second(p.second) {}

    // Extended constructors: allow multiple arguments to second constructor
    template <class F1> pair(const F1& f1) : first(f1), second() {}
    template <class F1, class S1> pair(const F1& f1, const S1& s1) : first(f1), second(s1) {}
    template <class F1, class S1, class S2> pair(const F1& f1, const S1& s1, const S2& s2) : first(f1), second(s1, s2) {}
    template <class F1, class S1, class S2, class S3> pair(const F1& f1, const S1& s1, const S2& s2, const S3& s3) : first(f1), second(s1, s2, s3) {}
    template <class F1, class S1, class S2, class S3, class S4> pair(const F1& f1, const S1& s1, const S2& s2, const S3& s3, const S4& s4) : first(f1), second(s1, s2, s3, s4) {}

    pair& operator = (const pair& p)
    {
        first = p.first, second = p.second;
        return *this;
    }
};

template <class T1, class T2> pair<T1,T2> make_pair(const T1& x)
{
    return pair<T1,T2>(x);
}

template <class T1, class T2> pair<T1,T2> make_pair(const T1& x, const T2& y)
{
    return pair<T1,T2>(x,y);
}

}; // namespace thor

// Global operators
template <typename T1, typename T2>
bool operator == (const thor::pair<T1,T2>& lhs, const thor::pair<T1,T2>& rhs)
{
    return lhs.first == rhs.first && lhs.second == rhs.second;
}

template <typename T1, typename T2>
bool operator < (const thor::pair<T1,T2>& lhs, const thor::pair<T1,T2>& rhs)
{
    if (lhs.first < rhs.first)
    {
        return true;
    }
    if (rhs.first < lhs.first)
    {
        return false;
    }
    return lhs.second < rhs.second;
}

template <typename T1, typename T2>
bool operator > (const thor::pair<T1, T2>& lhs, const thor::pair<T1, T2>& rhs)
{
    if (rhs.first < lhs.first)
    {
        return true;
    }
    if (lhs.first < rhs.first)
    {
        return false;
    }
    return rhs.second < lhs.second;
}

#endif

