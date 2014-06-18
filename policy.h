/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * policy.h
 *
 * ** THOR INTERNAL FILE - NOT FOR APPLICATION USE **
 *
 * This file defines policy classes for use by internal THOR constructs.
 */

#ifndef THOR_POLICY_H
#define THOR_POLICY_H
#pragma once

#ifndef THOR_ATOMIC_INTEGER_H
#include "atomic_integer.h"
#endif

#ifndef THOR_ALGORITHM_H
#include "algorithm.h"
#endif

namespace thor
{

namespace policy
{

///////////////////////////////////////////////////////////////////////////////
// Reference counting policy types
///////////////////////////////////////////////////////////////////////////////

// Defines a thread-safe reference counting policy. Ref-counted objects using
// this policy can be used across multiple threads.
struct thread_safe_ref_count
{
    atomic_integer<thor_size_type> refcount_;
    thread_safe_ref_count(size_type initial_count = 0) : refcount_(initial_count) {}
    bool increment_if_not_zero();
};

// Defines a fast, but thread-unsafe reference counting policy. Ref-counted objects
// using this policy should not be used across multiple threads.
struct unsafe_ref_count
{
    thor_size_type refcount_;
    unsafe_ref_count(size_type initial_count = 0) : refcount_(initial_count) {}
    bool increment_if_not_zero();
};

struct delete_this_on_zero_refcount
{
protected:
    virtual ~delete_this_on_zero_refcount() {}
    void on_zero_refcount() { delete this; }
};

struct notify_on_zero_refcount
{
protected:
    virtual ~notify_on_zero_refcount() {}
    virtual void on_zero_refcount() = 0;
};

///////////////////////////////////////////////////////////////////////////////
// shared_ptr policy types
///////////////////////////////////////////////////////////////////////////////
template <class T> struct default_delete
{
    default_delete() {}
    template <class Other> default_delete(const default_delete<Other>&) {}
    void operator () (T* p) const { delete p; }
};

template <class T> struct default_delete_array
{
    default_delete_array() {}
    template <class Other> default_delete_array(const default_delete_array<Other>&) {}
    void operator () (T* p) const { delete[] p; }
};


///////////////////////////////////////////////////////////////////////////////
// embedded_hash_multimap/hash_map/hash_multimap policy types
///////////////////////////////////////////////////////////////////////////////
struct base2_partition
{
    const static size_type initial_size = size_type(8);
    static size_type bucket_index(size_type hash, size_type num_buckets);
    static size_type resize(size_type current, size_type minimum);
};

struct prime_number_partition
{
    const static size_type initial_size = size_type(7);
    static size_type bucket_index(size_type hash, size_type num_buckets);
    static size_type resize(size_type current, size_type minimum);
};

} // namespace thor::policy

} // namespace thor


///////////////////////////////////////////////////////////////////////////////
// Function implementations
namespace thor
{
namespace policy
{

inline bool thread_safe_ref_count::increment_if_not_zero()
{
    thor_size_type current = refcount_;
    while (current != 0)
    {
        thor_size_type oldval = refcount_.compare_exchange(current + 1, current);
        if (oldval == current)
        {
            return true;
        }
        current = oldval;
    }
    return false;
}

inline bool unsafe_ref_count::increment_if_not_zero()
{
    if (refcount_ != 0)
    {
        ++refcount_;
        return true;
    }
    return false;
}

inline size_type base2_partition::bucket_index(size_type hash, size_type num_buckets)
{
    THOR_ASSERT(num_buckets > 0);
    THOR_ASSERT((num_buckets & (num_buckets - 1)) == 0); // Ensure power of two
    return hash & (num_buckets - 1);
}

inline size_type base2_partition::resize(size_type current, size_type minimum)
{
    // double bucket count until we've exceeded n or reached approximately half of the max of size_type (i.e. 2 billion on a 32-bit machine)
    THOR_ASSERT((current & (current - 1)) == 0); // Ensure power of two
    while (current < minimum && current < ~(size_type(-1)>>1))
    {
        current <<= 1;
    }
    return current;
}

inline size_type prime_number_partition::bucket_index(size_type hash, size_type num_buckets)
{
    THOR_ASSERT(num_buckets > 0);
    return hash % num_buckets;
}

inline size_type prime_number_partition::resize(size_type, size_type minimum)
{
    static const size_type sizes_[] = { 7ul, 23ul, 53ul, 97ul, 193ul, 389ul, 769ul, 1543ul, 3079ul, 6151ul, 12289ul, 24593ul, 49157ul, 98317ul, 196613ul, 393241ul, 786433ul, 1572869ul, 3145739ul, 6291469ul, 12582917ul, 25165843ul, 50331653ul, 100663319ul, 201326611ul, 402653189ul, 805306457ul, 1610612741ul, 3221225473ul, 4294967291ul };
    static const size_type count = sizeof(sizes_) / sizeof(sizes_[0]);
    const size_type* begin = sizes_;
    const size_type* end = begin + count;
    const size_type* pos = lower_bound(begin, end, minimum);
    return pos == end ? *(pos - 1) : *pos;
}

}
}

#endif