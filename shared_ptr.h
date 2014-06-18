/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * shared_ptr.h
 *
 * Defines an externally-managed reference counting mechanism through a smart pointer
 */
#ifndef THOR_SHARED_PTR
#define THOR_SHARED_PTR
#pragma once

#include "atomic_integer.h"
#include "policy.h"

namespace thor
{

namespace internal
{

template
<
    class T_REFCOUNT_POLICY = policy::thread_safe_ref_count
>
struct shared_ptr_info
{
private:
    T_REFCOUNT_POLICY refcount_;
    T_REFCOUNT_POLICY strong_refcount_;
public:
    shared_ptr_info() : refcount_(1), strong_refcount_(1) {}
    ~shared_ptr_info()
    {
        THOR_ASSERT(refcount_.refcount_ == 0);
        THOR_ASSERT(strong_refcount_.refcount_ == 0);
    }

    size_type strong_count() const { return strong_refcount_.refcount_; }
    bool expired() const { return strong_refcount_.refcount_ == 0; }

    void add_refcount_and_strong() { ++refcount_.refcount_, ++strong_refcount_.refcount_; }
    void add_refcount_and_weak()   { ++refcount_.refcount_; }
    bool promote_weak_to_strong()
    {
        if (strong_refcount_.increment_if_not_zero())
        {
            ++refcount_.refcount_;
            return true;
        }
        return false;
    }
    bool release_strong() { THOR_ASSERT(strong_refcount_.refcount_ > 0); return --strong_refcount_.refcount_ == 0; }
    void release_weak()   { }
    bool release()        { THOR_ASSERT(refcount_.refcount_ > 0); return --refcount_.refcount_ == 0; }
};

} // namespace internal

///////////////////////////////////////////////////////////////////////////////
// shared_ptr
///////////////////////////////////////////////////////////////////////////////
template
<
    class T,
    class T_REFCOUNT_POLICY = policy::thread_safe_ref_count,
    template <class> class T_DELETE_POLICY = policy::default_delete
>
class shared_ptr : private T_DELETE_POLICY<T>
{
    template <class U, class T_REFCOUNT_POLICY, template <class> class U_DELETE_POLICY> friend class weak_ptr;
    template <class U, class T_REFCOUNT_POLICY, template <class> class U_DELETE_POLICY> friend class shared_ptr;

    typedef internal::shared_ptr_info<T_REFCOUNT_POLICY> shared_ptr_info_type;
public:
    typedef T        value_type;
    typedef T*             pointer;
    typedef const T* const_pointer;
    typedef T&             reference;
    typedef const T& const_reference;

    explicit shared_ptr() : info_(0), value_(0) {}
    explicit shared_ptr(pointer value) : info_(0), value_(value) { if (value_) info_ = new shared_ptr_info_type; }
    shared_ptr(const shared_ptr& rhs) : info_(rhs.info_), value_(rhs.value_) { if (info_) { info_->add_refcount_and_strong(); } }
    template <class U, template <class> class U_DELETE_POLICY> shared_ptr(const shared_ptr<U, T_REFCOUNT_POLICY, U_DELETE_POLICY>& rhs) : info_(rhs.info_), value_(rhs.value_) { if (info_) { info_->add_refcount_and_strong(); } }

    ~shared_ptr()
    {
        release_();
    }

    shared_ptr& operator = (pointer value) { return assign(value); }
    shared_ptr& operator = (const shared_ptr& rhs) { return assign(rhs); }
    template <class U> shared_ptr& operator = (const shared_ptr<U>& rhs) { return assign(rhs); }

    const_pointer   operator -> () const { THOR_DEBUG_ASSERT(value_); return value_; }
    pointer         operator -> ()       { THOR_DEBUG_ASSERT(value_); return value_; }
    const_reference operator * () const  { THOR_DEBUG_ASSERT(value_); return *value_; }
    reference       operator * ()        { THOR_DEBUG_ASSERT(value_); return *value_; }

    bool            operator ! () const { return value_ == 0; }
    operator const void* () const { return value_; }
    operator void* () { return value_; }

    shared_ptr& assign(pointer value)
    {
        // We should only ever be given an object that is not tracked elsewhere. Since we're using external
        // reference counting, objects must ONLY be passed through copying the shared_ptr object
        THOR_ASSERT(value != value_);

        release_();
        value_ = value;
        if (value_)
        {
            info_ = new shared_ptr_info_type;
        }
        return *this;
    }
    
    template <class U> shared_ptr& assign(const shared_ptr<U>& rhs)
    {
        if (rhs.info_)
        {
            rhs.info_->add_refcount_and_strong();
        }
        release_();
        info_ = rhs.info_;
        value_ = rhs.value_;
        return *this;
    }

    // Only explicit get() functions; no implicit conversion to pointer type.
    pointer       get()       { return value_; }
    const_pointer get() const { return value_; }

    size_type use_count() const { return info_ ? info_->strong_count() : 0; }
    bool unique() const { return use_count() == 1; }

private:
    shared_ptr(shared_ptr_info_type* info, pointer value) : info_(info), value_(value) {}
    void release_()
    {
        if (info_)
        {
            if (info_->release_strong())
            {
                static_cast<T_DELETE_POLICY<T>&>(*this)(value_);
            }
            value_ = 0;

            if (info_->release())
            {
                delete info_;
            }
            info_ = 0;
        }
        THOR_ASSERT(value_ == 0);
        THOR_ASSERT(info_ == 0);
    }

    shared_ptr_info_type* info_;
    pointer value_;
};

///////////////////////////////////////////////////////////////////////////////
// weak_ptr
///////////////////////////////////////////////////////////////////////////////

template
<
    class T,
    class T_REFCOUNT_POLICY = policy::thread_safe_ref_count,
    template <class> class T_DELETE_POLICY = policy::default_delete
>
class weak_ptr
{
    typedef internal::shared_ptr_info<T_REFCOUNT_POLICY> shared_ptr_info_type;
public:
    typedef T        value_type;
    typedef T*             pointer;
    typedef const T* const_pointer;
    typedef T&             reference;
    typedef const T& const_reference;

    typedef shared_ptr<T, T_REFCOUNT_POLICY, T_DELETE_POLICY> shared_ptr_type;

    explicit weak_ptr() : info_(0), value_(0) {}
    explicit weak_ptr(const shared_ptr_type& rhs)
        : info_(rhs.info_), value_(rhs.value_)
    {
        if (info_) info_->add_refcount_and_weak();
    }
    template <class U, template <class> class U_DELETE_POLICY> explicit weak_ptr(const shared_ptr<U, T_REFCOUNT_POLICY, U_DELETE_POLICY>& rhs)
        : info_(rhs.info_), value_(rhs.value_)
    {
        if (info_) info_->add_refcount_and_weak();
    }
    ~weak_ptr()
    {
        release_();
    }

    weak_ptr& operator = (const weak_ptr& rhs)
    {
        return assign(rhs);
    }
    template <class U, template <class> class U_DELETE_POLICY> weak_ptr& operator = (const weak_ptr<U, T_REFCOUNT_POLICY, U_DELETE_POLICY>& rhs)
    {
        return assign(rhs);
    }

    weak_ptr& operator = (const shared_ptr_type& rhs)
    {
        return assign(rhs);
    }
    template <class U, template <class> class U_DELETE_POLICY> weak_ptr& operator = (const shared_ptr<U, T_REFCOUNT_POLICY, U_DELETE_POLICY>& rhs)
    {
        return assign(rhs);
    }

    template <class U, template <class> class U_DELETE_POLICY> weak_ptr& assign(const weak_ptr<U, T_REFCOUNT_POLICY, U_DELETE_POLICY>& rhs)
    {
        if (rhs.info_) rhs.info_->add_refcount_and_weak();

        release_();
        info_ = rhs.info_;
        value_ = rhs.value_;
        return *this;
    }

    template <class U, template <class> class U_DELETE_POLICY> weak_ptr& assign(const shared_ptr<U, T_REFCOUNT_POLICY, U_DELETE_POLICY>& rhs)
    {
        if (rhs.info_) rhs.info_->add_refcount_and_weak();
        
        release_();
        info_ = rhs.info_;
        value_ = rhs.value_;
        return *this;
    }

    shared_ptr_type lock()
    {
        if (info_ && !info_->promote_weak_to_strong())
        {
            release_();
        }
        return shared_ptr_type(info_, value_);
    }

    shared_ptr_type lock() const
    {
        if (0 == info_ || !info_->promote_weak_to_strong())
        {
            return shared_ptr_type(0, 0);
        }
        else
        {
            return shared_ptr_type(info_, value_);
        }
    }

    size_type use_count() const { return info_ ? info_->strong_count() : 0; }
    bool expired() const { return info_ ? info->expired() : true; }

private:
    void release_()
    {
        if (info_)
        {
            info_->release_weak();
            value_ = 0;
            if (info_->release())
            {
                delete info_;
            }
            info_ = 0;
        }
        THOR_ASSERT(info_ == 0);
        THOR_ASSERT(value_ == 0);
    }

    shared_ptr_info_type* info_;
    pointer value_;
};

}

#endif