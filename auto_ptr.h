/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * auto_ptr.h
 *
 * Defines a smart pointer class that can be used to ensure that an object is
 * destroyed when no longer referenced. Unlike the C++ std auto_ptr class, this
 * implementation allows copying. The resource is destroyed when all references
 * are destroyed.
 *
 * Use of this class is not thread-safe.
 */
#ifndef THOR_AUTO_PTR_H
#define THOR_AUTO_PTR_H
#pragma once

#include "policy.h"

namespace thor
{

template
<
    class T,
    template <class> class T_DELETE_POLICY = policy::default_delete
>
class auto_ptr : private T_DELETE_POLICY<T>
{
public:
    typedef T           value_type;
    typedef T*          pointer;
    typedef const T*    const_pointer;
    typedef T&          reference;
    typedef const T&    const_reference;

    explicit auto_ptr(pointer value = 0)
    {
        prev_ = next_ = this;
        value_ = value;
    }
    auto_ptr(const auto_ptr& rhs)
    {
        prev_ = rhs.prev_;
        prev_->next_ = this;
        next_ = &rhs;
        next_->prev_ = this;
        value_ = rhs.value_;
    }
    ~auto_ptr()
    {
        reset(0);
    }

    auto_ptr& operator = (pointer value)
    {
        reset(value);
        return *this;
    }
    auto_ptr& operator = (const auto_ptr& rhs)
    {
        if (value_ != rhs.value_)
        {
            reset(rhs.value_);
            prev_ = rhs.prev_;
            prev_->next_ = this;
            next_ = &rhs;
            next_->prev_ = this;
        }
        return *this;
    }

    reference operator * () { THOR_ASSERT(value_); return *value_; }
    const_reference operator * () const { THOR_ASSERT(value_); return *value_; }
    pointer operator -> () { THOR_ASSERT(value_); return value_; }
    const_pointer operator -> () const { THOR_ASSERT(value_); return value_; }

    pointer get() { return value_; }
    const_pointer get() const { return value_; }

    void reset(pointer p)
    {
        if (p != value_)
        {
            if (prev_ == this)
            {
                THOR_ASSERT(next_ == this);
                static_cast<T_DELETE_POLICY<T>&>(*this)(value_);
            }
            else
            {
                prev_->next_ = next_;
                next_->prev_ = prev_;
                next_ = prev_ = this;
            }
            value_ = p;
        }
    }

private:
    mutable const auto_ptr*   prev_;
    mutable const auto_ptr*   next_;
    pointer             value_;
};

}

#endif