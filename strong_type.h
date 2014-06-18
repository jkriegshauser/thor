/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * strong_type.h
 *
 * Special type declaration that allows integer types to be classified as a particular
 * strong type that disallows automatic conversion.
 */

#ifndef THOR_STRONG_TYPE_H
#define THOR_STRONG_TYPE_H
#pragma once

#ifndef THOR_BASETYPES_H
#include "basetypes.h"
#endif

namespace thor
{

namespace policy
{

template <typename T>
struct zero_init
{
    void operator () (T& val) { val = T(0); }
};

}

template
<
    typename T,
    typename T_SIGNATURE,
    template <class> class T_INIT_POLICY = policy::zero_init
>
class strong_type : private T_INIT_POLICY<T>
{
    T value_;
public:
    typedef T value_type;
    typedef T& reference;
    typedef const T& const_reference;
    typedef strong_type<T, T_SIGNATURE, T_INIT_POLICY> self_type;

    strong_type() THOR_NOTHROW { static_cast<T_INIT_POLICY<T>&>(*this)(value_); }
    strong_type(const self_type& rhs) THOR_NOTHROW : value_(rhs.value_) {}
    explicit strong_type(const_reference rhs) THOR_NOTHROW : value_(rhs) {}
    ~strong_type() THOR_NOTHROW {}

    self_type& operator = (const self_type& rhs)  THOR_NOTHROW { value_ = rhs.value_; return *this; }
    bool operator == (const self_type& rhs) const THOR_NOTHROW { return value_ == rhs.value_; }
    bool operator != (const self_type& rhs) const THOR_NOTHROW { return value_ != rhs.value_; }
    bool operator <  (const self_type& rhs) const THOR_NOTHROW { return value_ <  rhs.value_; }
    bool operator <= (const self_type& rhs) const THOR_NOTHROW { return value_ <= rhs.value_; }
    bool operator >  (const self_type& rhs) const THOR_NOTHROW { return value_ >  rhs.value_; }
    bool operator >= (const self_type& rhs) const THOR_NOTHROW { return value_ >= rhs.value_; }

    reference value() THOR_NOTHROW { return value_; }
    const_reference value() const THOR_NOTHROW { return value_; }
    const_reference cvalue() const THOR_NOTHROW { return value_; }

    // The coerce() functions can be used to ensure that the strong_type matches the expected size.
    // Only convertable pointer types are allowed. Useful for things like binary serialization and format
    // specifiers where size is important.
    template <typename U> U& coerce()              THOR_NOTHROW { THOR_MATCH_TYPE(U, T); return static_cast<U&>(value_); }
    template <typename U> const U& coerce() const  THOR_NOTHROW { THOR_MATCH_TYPE(U, T); return static_cast<const U&>(value_); }
    template <typename U> const U& ccoerce() const THOR_NOTHROW { THOR_MATCH_TYPE(U, T); return static_cast<const U&>(value_); }
};

#define THOR_DECLARE_STRONG_TYPE(T, NAME) class __thor_signature_##NAME; typedef thor::strong_type<T, __thor_signature_##NAME> NAME
#define THOR_DECLARE_STRONG_TYPE_INIT(T, NAME, INIT_POLICY) class __thor_signature_##NAME; typedef thor::strong_type<T, __thor_signature_##NAME, INIT_POLICY> NAME

}

#endif