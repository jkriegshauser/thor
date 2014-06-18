/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * ref_counted.h
 *
 * This file defines a base class that can be used for reference counted classes
 */

#ifndef THOR_REF_COUNTED_H
#define THOR_REF_COUNTED_H
#pragma once

#ifndef THOR_BASETYPES_H
#include "basetypes.h"
#endif

#ifndef THOR_POLICY_H
#include "policy.h"
#endif

namespace thor
{

///////////////////////////////////////////////////////////////////////////////
// ref_counted
//  A base class for reference counted types.
///////////////////////////////////////////////////////////////////////////////
template <typename T_REFCOUNT_POLICY = policy::thread_safe_ref_count,
          typename T_DELETE_POLICY = policy::delete_this_on_zero_refcount,
          size_type initial_refcount = 0>
class ref_counted : private T_REFCOUNT_POLICY, protected T_DELETE_POLICY
{
    typedef T_REFCOUNT_POLICY refcount_policy;
    typedef T_DELETE_POLICY delete_policy;
    static const thor_size_type not_referenced = thor_size_type(-1);
    
    THOR_DECLARE_NOCOPY(ref_counted);
public:
    typedef size_type thor_size_type;

    ref_counted() : refcount_policy(initial_refcount) {}
    virtual ~ref_counted()
    {
        THOR_DEBUG_ASSERT(refcount_ == 0 || refcount_ == not_referenced); // Should have a zero refcount when destroyed
    }

    void add_ref()
    {
        THOR_DEBUG_ASSERT(refcount_ != not_referenced);
        ++refcount_;
    }

    void release()
    {
        THOR_DEBUG_ASSERT(refcount_ != not_referenced && refcount_ > 0);
        if (--refcount_ == 0)
        {
            on_zero_refcount(); // Could potentially delete this
        }
    }

    size_type get_ref_count() const
    {
        return refcount_;
    }

    void make_not_referenced()
    {
        THOR_DEBUG_ASSERT(refcount_ == initial_refcount);
        refcount_ = not_referenced;
    }
};

///////////////////////////////////////////////////////////////////////////////
// ref_pointer
//  a smart pointer type to automatically handle reference counts
///////////////////////////////////////////////////////////////////////////////
template <class T> class ref_pointer
{
public:
    typedef T        value_type;
    typedef T*             pointer;
    typedef const T* const_pointer;
    typedef T&             reference;
    typedef const T& const_reference;

    explicit ref_pointer() : value_(0) {}
    ref_pointer(pointer value, bool addref = true) : value_(value) { if (value_ && addref) value_->add_ref(); }
    ref_pointer(const ref_pointer& rhs) : value_(rhs.value_) { if (value_) value_->add_ref(); }
    template <class U> ref_pointer(const ref_pointer<U>& rhs) : value_(rhs.value_) { if (value_) value_->add_ref(); }
    
    ~ref_pointer() { if (value_) value_->release(); value_ = 0; }

    ref_pointer& operator = (reference value) { return assign(&value, false); }
    ref_pointer& operator = (pointer value) { return assign(value); }
    ref_pointer& operator = (const ref_pointer& rhs) { return assign(rhs.value_); }
    template <class U> ref_pointer& operator = (const ref_pointer<U>& rhs) { return assign(rhs.value_); }

    const_pointer   operator -> () const { THOR_DEBUG_ASSERT(value_); return value_; }
    pointer         operator -> ()       { THOR_DEBUG_ASSERT(value_); return value_; }
    const_reference operator * () const  { THOR_DEBUG_ASSERT(value_); return *value_; }
    reference       operator * ()        { THOR_DEBUG_ASSERT(value_); return *value_; }

    operator       pointer ()       { return value_; }
    operator const_pointer () const { return value_; }

    ref_pointer& assign(pointer value, bool addref = true)
    {
        if (value && addref)
        {
            value->add_ref();
        }
        if (value_)
        {
            value_->release();
        }
        value_ = value;
        return *this;
    }

    pointer       get()       { return value_; }
    const_pointer get() const { return value_; }

private:
    pointer value_;
};

}

#endif