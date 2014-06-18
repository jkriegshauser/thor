/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * hash_set.h
 *
 * This file defines (mostly) STL-compatible hash_set and hash_multiset containers
 * Changes/Extensions:
 * - Unlike STL, the ordering of items in hash_set and hash_multiset are treated like a linked list.
 *   * Iteration order will not change when the bucket count changes.
 *   * New items inserted will be added to the back of the list (i.e. end()).
 *   * Items can be reorganized within the list using move().
 *   * Since the list is independent of hashing, matching keys (for hash_multiset) will not necessarily be
 *     encountered sequentially when iterating. If grouped matching keys are desired, call begin as such: begin(true).
 *     Note that all other ordering aside from grouped keys is not guaranteed.
 * - There is no template parameter for EqualsFunc. Therefore, an operator == MUST be defined
 *   for types used as keys.
 *   * Subsequently, key_equals, key_eq() and the constructors that pass key_equals are not implemented.
 * - bucket_count() will always return powers-of-two whereas some other implementations use
 *   prime numbers. The power-of-two implementation is faster.
 * - equal_range() supports an optional count parameter
 * - A PartitionPolicy can be used to control the bucketizing scheme (base2, prime, etc)
 *
 * hash_set/hash_multiset - Non-ordered simple associative containers
 *   Time:
 *     insert - constant (average; linear worst case)
 *     find   - constant (average; linear worst case)
 *     erase (iterator) - constant (average; linear worst case)
 *     erase (key) - linear on count(k) average, linear on size() worst case
 *     insert (single value) - amortized constant time; linear worst case.  May cause a resize()
 *     resize - linear
 *     iteration - linear
 *   Iterator invalidation:
 *     erase - invalidates only erased iterators
 *     insert - no iterators are invalidated
 *     resize - same as insert
 *   Usage suggestions:
 *     hash_set - Use for storing/retrieving unique keys
 *     hash_multiset - Use for storing/retrieving non-unique keys
 *         * If sorted order is desired, consider map/multimap which have worse time characteristics
 */

#ifndef THOR_HASH_SET_H
#define THOR_HASH_SET_H
#pragma once

#ifndef THOR_HASHTABLE_H
#include "hashtable.h"
#endif

#ifndef THOR_FUNCTION_H
#include "function.h"
#endif

#ifndef THOR_HASH_FUNCS_H
#include "hash_funcs.h"
#endif

#ifndef THOR_SORT_H
#include "sort.h"
#endif

#ifndef THOR_POLICY_H
#include "policy.h"
#endif

namespace thor
{

// thor::hash_set
template
<
    class Key,
    class HashFunc = hash<Key>,
    class PartitionPolicy = policy::base2_partition
> class hash_set
{
    typedef hashtable<Key, Key, HashFunc, identity<Key>, PartitionPolicy> hashtable_type;
    typedef typename hashtable_type::iterator mutable_iterator;
    mutable_iterator make_mutable(typename hashtable_type::const_iterator pos) const { return *(mutable_iterator*)&pos; }
    hashtable_type m_hashtable;

public:
    typedef Key key_type;
    typedef Key value_type;
    typedef HashFunc hasher;
    typedef typename hashtable_type::pointer pointer;
    typedef typename hashtable_type::const_pointer const_pointer;
    typedef typename hashtable_type::reference reference;
    typedef typename hashtable_type::const_reference const_reference;
    typedef thor_size_type size_type;
    typedef thor_diff_type difference_type;

    // iterator and const_iterator are the same since the value can never be modified.
    typedef typename hashtable_type::const_iterator iterator;
    typedef typename hashtable_type::const_iterator const_iterator;
    typedef typename hashtable_type::const_reverse_iterator reverse_iterator;
    typedef typename hashtable_type::const_reverse_iterator const_reverse_iterator;

    hash_set()
    {}

    hash_set(size_type n) :
        m_hashtable(n)
    {}

    hash_set(size_type n, const hasher& h) :
        m_hashtable(n, h)
    {}

    template <class InputIterator> hash_set(InputIterator first, InputIterator last)
    {
        m_hashtable.insert_unique(first, last);
    }

    template <class InputIterator> hash_set(InputIterator first, InputIterator last, size_type n) :
        m_hashtable(n)
    {
        m_hashtable.insert_unique(first, last);
    }

    template <class InputIterator> hash_set(InputIterator first, InputIterator last, size_type n, const hasher& h) :
        m_hashtable(n, h)
    {
        m_hashtable.insert_unique(first, last);
    }

    hash_set(const hash_set& rhs) :
        m_hashtable(rhs.m_hashtable)
    {}

    ~hash_set()
    {}

    hash_set& operator = (const hash_set& rhs)
    {
        m_hashtable = rhs.m_hashtable;
        return *this;
    }

    // iteration
    iterator begin(bool hash_mode=false) const                      { return m_hashtable.begin(hash_mode); }
    iterator end() const                                            { return m_hashtable.end(); }
    reverse_iterator rbegin(bool hash_mode=false) const             { return m_hashtable.rbegin(hash_mode); }
    reverse_iterator rend() const                                   { return m_hashtable.rend(); }

    // size
    size_type size() const                                          { return m_hashtable.size(); }
    size_type max_size() const                                      { return m_hashtable.max_size(); }
    bool empty() const                                              { return m_hashtable.empty(); }
    size_type bucket_count() const                                  { return m_hashtable.bucket_count(); }
    void resize(size_type n)                                        { m_hashtable.resize(n); }
    const hasher& hash_funct() const                                { return m_hashtable.hash_funct(); }
    
    void swap(hash_set& rhs)                                        { m_hashtable.swap(rhs.m_hashtable); }

    // insertion
    pair<iterator, bool> insert(const value_type& x)                { return m_hashtable.insert_unique(x); }
    template <class InputIterator> void insert(InputIterator f, InputIterator l) { m_hashtable.insert_unique(f, l); }

    void move(iterator which, iterator pos)                         { m_hashtable.move(make_mutable(which), make_mutable(pos)); }

    // erasing
    void erase(iterator pos)                                        { m_hashtable.erase(make_mutable(pos)); }
    size_type erase(const key_type& k)                              { return m_hashtable.erase(k); }
    void erase(iterator first, iterator last)                       { m_hashtable.erase(make_mutable(first), make_mutable(last)); }
    void clear()                                                    { m_hashtable.clear(); }

    // searching
    iterator find(const key_type& k) const                          { return m_hashtable.find(k); }
    size_type count(const key_type& k) const                        { return find(k) == end() ? 0 : 1; }

    pair<iterator, iterator> equal_range(const key_type& k) const   { return m_hashtable.equal_range(k); }
};

// thor::hash_multiset
template
<
    class Key,
    class HashFunc = hash<Key>,
    class PartitionPolicy = policy::base2_partition
> class hash_multiset
{
    typedef hashtable<Key, Key, HashFunc, identity<Key>, PartitionPolicy> hashtable_type;
    typedef typename hashtable_type::iterator mutable_iterator;
    mutable_iterator make_mutable(typename hashtable_type::const_iterator pos) const { return *(mutable_iterator*)&pos; }
    hashtable_type m_hashtable;

public:
    typedef Key key_type;
    typedef Key value_type;
    typedef HashFunc hasher;
    typedef typename hashtable_type::pointer pointer;
    typedef typename hashtable_type::const_pointer const_pointer;
    typedef typename hashtable_type::reference reference;
    typedef typename hashtable_type::const_reference const_reference;
    typedef thor_size_type size_type;
    typedef thor_diff_type difference_type;

    typedef typename hashtable_type::const_iterator iterator;
    typedef typename hashtable_type::const_iterator const_iterator;
    typedef typename hashtable_type::const_reverse_iterator reverse_iterator;
    typedef typename hashtable_type::const_reverse_iterator const_reverse_iterator;

    // constructors
    hash_multiset()
    {}
    
    hash_multiset(size_type n) :
        m_hashtable(n)
    {}

    hash_multiset(size_type n, const hasher& h) :
        m_hashtable(n, h)
    {}

    template <class InputIterator> hash_multiset(InputIterator first, InputIterator last)
    {
        m_hashtable.insert_equal(first, last);
    }

    template <class InputIterator> hash_multiset(InputIterator first, InputIterator last, size_type n) :
        m_hashtable(n)
    {
        m_hashtable.insert_equal(first, last);
    }

    template <class InputIterator> hash_multiset(InputIterator first, InputIterator last, size_type n, const hasher& h) :
        m_hashtable(n, h)
    {
        m_hashtable.insert_equal(first, last);
    }

    hash_multiset(const hash_multiset& rhs) :
        m_hashtable(rhs.m_hashtable)
    {}

    ~hash_multiset()
    {}

    hash_multiset& operator=(const hash_multiset& rhs)
    {
        m_hashtable = rhs.m_hashtable;
        return *this;
    }

    // iteration
    iterator begin(bool hash_mode=false) const              { return m_hashtable.begin(hash_mode); }
    iterator end() const                                    { return m_hashtable.end(); }
    reverse_iterator rbegin(bool hash_mode=false) const     { return m_hashtable.rbegin(hash_mode); }
    reverse_iterator rend() const                           { return m_hashtable.rend(); }

    // size
    size_type size() const                                  { return m_hashtable.size(); }
    size_type max_size() const                              { return m_hashtable.max_size(); }
    bool empty() const                                      { return m_hashtable.empty(); }
    size_type bucket_count() const                          { return m_hashtable.bucket_count(); }
    void resize(size_type n)                                { m_hashtable.resize(n); }
    const hasher& hash_funct() const                        { return m_hashtable.hash_funct(); }

    void swap(hash_multiset& rhs)                           { m_hashtable.swap(rhs.m_hashtable); }

    // insertion
    iterator insert(const value_type& x)                    { return m_hashtable.insert_equal(x); }
    template <class InputIterator> void insert(InputIterator f, InputIterator l) { m_hashtable.insert_equal(f, l); }

    void move(iterator which, iterator pos)                 { m_hashtable.move(make_mutable(which), make_mutable(pos)); }

    // erasing
    void erase(iterator pos)                                { m_hashtable.erase(make_mutable(pos)); }
    size_type erase(const key_type& k)                      { return m_hashtable.erase(k); }
    void erase(iterator first, iterator last)               { m_hashtable.erase(make_mutable(first), make_mutable(last)); }
    void clear() { m_hashtable.clear(); }

    // searching
    iterator find(const key_type& k) const                  { return m_hashtable.find(k); }
    size_type count(const key_type& k) const                { return m_hashtable.count(k); }

    pair<iterator, iterator> equal_range(const key_type& k, size_type* count = 0) const { return m_hashtable.equal_range(k, count); }
};

// Swap specializations
template <class Key, class HashFunc> void swap(hash_set<Key, HashFunc>& lhs, hash_set<Key, HashFunc>& rhs)
{
    lhs.swap(rhs);
}

template <class Key, class HashFunc> void swap(hash_multiset<Key, HashFunc>& lhs, hash_multiset<Key, HashFunc>& rhs)
{
    lhs.swap(rhs);
}

} // namespace thor

// Global operators
template <class Key, class HashFunc> bool operator == (const thor::hash_set<Key,HashFunc>& lhs,
                                                       const thor::hash_set<Key,HashFunc>& rhs)
{
    typedef thor::hash_set<Key,HashFunc> hashsettype;
    if (!(lhs.size() == rhs.size()))
    {
        // Early out if size doesn't match
        return false;
    }

    for (typename hashsettype::const_iterator iter(lhs.begin(true));
         iter != lhs.end();
         ++iter)
    {
        typename hashsettype::const_iterator rhs_iter(rhs.find(*iter));
        if (rhs_iter == rhs.end())
        {
            return false;
        }
    }

    return true;
}

template <class Key, class HashFunc> bool operator != (const thor::hash_set<Key,HashFunc>& lhs,
                                                       const thor::hash_set<Key,HashFunc>& rhs)
{
    return !(lhs == rhs);
}

template <class Key, class HashFunc> bool operator == (const thor::hash_multiset<Key,HashFunc>& lhs,
                                                       const thor::hash_multiset<Key,HashFunc>& rhs)
{
    typedef thor::hash_multiset<Key,HashFunc> hashsettype;
    if (!(lhs.size() == rhs.size()))
    {
        // Early out if size doesn't match
        return false;
    }

    typename hashsettype::iterator iter(lhs.begin(true));
    while(iter != lhs.end())
    {
        // Count that the number of keys are the same
        thor::pair<typename hashsettype::iterator, typename hashsettype::iterator> range = rhs.equal_range(*iter);
        if (range.first != range.second)
        {
            for (;;)
            {
                const typename hashsettype::key_type& key = *iter;
                ++iter, ++range.first;

                const bool newkey = (iter == lhs.end() || !(key == *iter));
                const bool endrange = range.first == range.second;
                if (newkey && endrange)
                {
                    // Same keys in both
                    break;
                }
                else if (newkey || endrange)
                {
                    // Different keys
                    return false;
                }
            }
        }
        else
        {
            // Key doesn't exist for rhs
            return false;
        }
    }

    return true;
}

template <class Key, class HashFunc> bool operator != (const thor::hash_multiset<Key,HashFunc>& lhs,
                                                       const thor::hash_multiset<Key,HashFunc>& rhs)
{
    return !(lhs == rhs);
}

#endif

