/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * hash_map.h
 *
 * This file defines (mostly) STL-compatible hash_map and hash_multimap associative containers
 *
 * Changes/Extensions:
 * - Unlike STL, the ordering of items in hash_map and hash_multimap are treated like a linked list.
 *   * Iteration order will not change when the bucket count changes.
 *   * New items inserted will be added to the back of the list (i.e. end()).
 *   * Items can be reorganized within the list using move().
 *   * Since the list is independent of hashing, matching keys (for hash_multimap) will not necessarily be
 *     encountered sequentially when iterating. If grouped matching keys are desired, call begin as such: begin(true).
 *     Note that all other ordering aside from grouped keys is not guaranteed.
 * - There is no template parameter for EqualsFunc. Therefore, an operator == MUST be defined
 *   for types used as keys.
 *   * Subsequently, key_equals, key_eq() and the constructors that pass key_equals are not implemented.
 * - bucket_count() will always return powers-of-two whereas some other implementations use
 *   prime numbers. The power-of-two implementation is faster.
 * - The insert(InputIterator, InputIterator) function has been renamed insert_range.
 * - While insert(value_type) is supported, it is not the best way to insert elements.
 *   Consider using the templatized insert() functions. These functions pass up to four
 *   parameters directly to the constructor meaning that there is no copy construction
 *   of the Value. Additionally, if more than four parameters are required, insert_placement()
 *   can be used with placement new to construct the item.
 *   * In the case of hash_map, if the key/value pair already exists, the key/value pair is destroyed
 *     and re-constructed.
 *   * In the case of hash_map, the insert() functions return an iterator, so it is impossible
 *     to tell whether the key previously existed from the insert() function call alone.
 * - Raw pointer support:
 *   * delete_all() will delete the Value only (not the Key) for all items in the container, followed by a clear().
 * - equal_range() supports an optional count parameter
 * - A PartitionPolicy can be used to control the bucketizing scheme (base2, prime, etc)
 *
 * hash_map/hash_multimap - Non-ordered associative containers
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
 *     hash_map - Use for storing/retrieving unique key/value pairs
 *     hash_multimap - Use for storing/retrieving non-unique key/value pairs
 *         * If sorted order is desired, consider map/multimap which have worse time characteristics
 */

#ifndef THOR_HASH_MAP_H
#define THOR_HASH_MAP_H
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

#ifndef THOR_PAIR_H
#include "pair.h"
#endif

#ifndef THOR_LIST_H
#include "list.h"
#endif

#ifndef THOR_POLICY_H
#include "policy.h"
#endif

namespace thor
{

// thor::hash_map
template
<
    class Key,
    class Data,
    class HashFunc = hash<Key>,
    class PartitionPolicy = policy::base2_partition
> class hash_map
{
public:
    typedef Key key_type;
    typedef Data data_type;
    typedef pair<const key_type, data_type> value_type;
    typedef HashFunc hasher;

private:
    typedef hashtable<key_type, value_type, hasher, select1st<value_type>, PartitionPolicy> hashtable_type;
    hashtable_type m_hashtable;

public:
    typedef typename hashtable_type::pointer pointer;
    typedef typename hashtable_type::const_pointer const_pointer;
    typedef typename hashtable_type::reference reference;
    typedef typename hashtable_type::const_reference const_reference;
    typedef thor_size_type size_type;
    typedef thor_diff_type difference_type;

    typedef typename hashtable_type::iterator iterator;
    typedef typename hashtable_type::const_iterator const_iterator;
    typedef typename hashtable_type::reverse_iterator reverse_iterator;
    typedef typename hashtable_type::const_reverse_iterator const_reverse_iterator;

    // constructors
    hash_map()
    {}
    
    hash_map(size_type n) :
        m_hashtable(n)
    {}

    hash_map(size_type n, const hasher& h) :
        m_hashtable(n, h)
    {}
    
    template <class InputIterator> hash_map(InputIterator first, InputIterator last)
    {
        m_hashtable.insert_unique(first, last);
    }
    
    template <class InputIterator> hash_map(InputIterator first, InputIterator last, size_type n) :
        m_hashtable(n)
    {
        m_hashtable.insert_unique(first, last);
    }

    template <class InputIterator> hash_map(InputIterator first, InputIterator last, size_type n, const hasher& h) :
        m_hashtable(n, h)
    {
        m_hashtable.insert_unique(first, last);
    }

    hash_map(const hash_map& rhs) :
        m_hashtable(rhs.m_hashtable)
    {}

    ~hash_map()
    {}

    // iteration
    iterator begin(bool mode_hash=false)                { return m_hashtable.begin(mode_hash); }
    iterator end()                                      { return m_hashtable.end(); }
    const_iterator begin(bool mode_hash=false) const    { return m_hashtable.begin(mode_hash); }
    const_iterator end() const                          { return m_hashtable.end(); }
    reverse_iterator rbegin(bool mode_hash=false)       { return m_hashtable.rbegin(mode_hash); }
    reverse_iterator rend()                             { return m_hashtable.rend(); }
    const_reverse_iterator rbegin(bool mode_hash=false) const { return m_hashtable.rbegin(mode_hash); }
    const_reverse_iterator rend() const                 { return m_hashtable.rend(); }
    
    // size
    size_type size() const                              { return m_hashtable.size(); }
    size_type max_size() const                          { return m_hashtable.max_size(); }
    bool empty() const                                  { return m_hashtable.empty(); }
    size_type bucket_count() const                      { return m_hashtable.bucket_count(); }
    void resize(size_type n)                            { m_hashtable.resize(n); }
    const hasher& hash_funct() const                    { return m_hashtable.hash_funct(); }
    
    hash_map& operator = (const hash_map& rhs)          { m_hashtable = rhs.m_hashtable; return *this; }
    void swap(hash_map& rhs)                            { m_hashtable.swap(rhs.m_hashtable); }

    // insertion
    pair<iterator, bool> insert(const value_type& x)    { return m_hashtable.insert_unique(x); }
    template <class InputIterator> void insert_range(InputIterator first, InputIterator last) { m_hashtable.insert_unique(first, last); }

    // insert extensions
    iterator insert(const Key& k)
    {
        value_type* v = m_hashtable.key_insert_unique(k);
        typetraits<value_type>::construct(v, k);
        return m_hashtable.iterator_from_value_type(*v);
    }
    template <class T1> iterator insert(const Key& k, const T1& t1)
    {
        value_type* v = m_hashtable.key_insert_unique(k);
        new (v) value_type(k, t1);
        return m_hashtable.iterator_from_value_type(*v);
    }
    template <class T1, class T2> iterator insert(const Key& k, const T1& t1, const T2& t2)
    {
        value_type* v = m_hashtable.key_insert_unique(k);
        new (v) value_type(k, t1, t2);
        return m_hashtable.iterator_from_value_type(*v);
    }
    template <class T1, class T2, class T3> iterator insert(const Key& k, const T1& t1, const T2& t2, const T3& t3)
    {
        value_type* v = m_hashtable.key_insert_unique(k);
        new (v) value_type(k, t1, t2, t3);
        return m_hashtable.iterator_from_value_type(*v);
    }
    template <class T1, class T2, class T3, class T4> iterator insert(const Key& k, const T1& t1, const T2& t2, const T3& t3, const T4& t4)
    {
        value_type* v = m_hashtable.key_insert_unique(k);
        new (v) value_type(k, t1, t2, t3, t4);
        return m_hashtable.iterator_from_value_type(*v);
    }
    // Requires the use of placement new to construct the Value.
    // Example: new (l.insert_placement(key)) Value(arg1, arg2);
    void* insert_placement(const Key& k)
    {
        value_type* v = m_hashtable.key_insert_unique(k);
        typetraits<Key>::construct(&const_cast<Key&>(v->first), k);
        return &v->second;
    }

    void move(iterator which, iterator pos)             { m_hashtable.move(which, pos); }

    // erasing
    void erase(iterator pos)                            { m_hashtable.erase(pos); }
    size_type erase(const key_type& k)                  { return m_hashtable.erase(k); }
    void erase(iterator first, iterator last)           { m_hashtable.erase(first, last); }
    void clear()                                        { m_hashtable.clear(); }
    void delete_all()
    {
        for (iterator iter(begin()); iter != end(); ++iter)
        {
            delete (*iter).second;
        }
        clear();
    }

    // search
    const_iterator find(const key_type& k) const        { return m_hashtable.find(k); }
    iterator find(const key_type& k)                    { return m_hashtable.find(k); }
    size_type count(const key_type& k) const            { return find(k) == end() ? 0 : 1; }

    pair<const_iterator, const_iterator> equal_range(const key_type& k) const { return m_hashtable.equal_range(k); }
    pair<iterator, iterator> equal_range(const key_type& k) { return m_hashtable.equal_range(k); }

    data_type& operator[](const key_type& k) { return (*insert(value_type(k)).first).second; }
};

// thor::hash_multimap
template
<
    class Key,
    class Data,
    class HashFunc = hash<Key>,
    class PartitionPolicy = policy::base2_partition
> class hash_multimap
{
public:
    typedef Key key_type;
    typedef Data data_type;
    typedef pair<const key_type, data_type> value_type;
    typedef HashFunc hasher;

private:
    typedef hashtable<key_type, value_type, hasher, select1st<value_type>, PartitionPolicy> hashtable_type;
    hashtable_type m_hashtable;

public:
    typedef typename hashtable_type::pointer pointer;
    typedef typename hashtable_type::const_pointer const_pointer;
    typedef typename hashtable_type::reference reference;
    typedef typename hashtable_type::const_reference const_reference;
    typedef thor_size_type size_type;
    typedef thor_diff_type difference_type;

    typedef typename hashtable_type::iterator iterator;
    typedef typename hashtable_type::const_iterator const_iterator;
    typedef typename hashtable_type::reverse_iterator reverse_iterator;
    typedef typename hashtable_type::const_reverse_iterator const_reverse_iterator;

    // constructors
    hash_multimap()
    {}
    
    hash_multimap(size_type n) :
        m_hashtable(n)
    {}

    hash_multimap(size_type n, const hasher& h) :
        m_hashtable(n, h)
    {}

    template <class InputIterator> hash_multimap(InputIterator first, InputIterator last)
    {
        m_hashtable.insert_equal(first, last);
    }

    template <class InputIterator> hash_multimap(InputIterator first, InputIterator last, size_type n) :
        m_hashtable(n)
    {
        m_hashtable.insert_equal(first, last);
    }

    template <class InputIterator> hash_multimap(InputIterator first, InputIterator last, size_type n, const hasher& h) :
        m_hashtable(n, h)
    {
        m_hashtable.insert_equal(first, last);
    }

    hash_multimap(const hash_multimap& rhs) :
        m_hashtable(rhs.m_hashtable)
    {}

    ~hash_multimap()
    {}

    // iteration
    iterator begin(bool mode_hash=false)                    { return m_hashtable.begin(mode_hash); }
    iterator end()                                          { return m_hashtable.end(); }
    const_iterator begin(bool mode_hash=false) const        { return m_hashtable.begin(mode_hash); }
    const_iterator end() const                              { return m_hashtable.end(); }
    reverse_iterator rbegin(bool mode_hash=false)           { return m_hashtable.rbegin(mode_hash); }
    reverse_iterator rend()                                 { return m_hashtable.rend(); }
    const_reverse_iterator rbegin(bool mode_hash=false) const { return m_hashtable.rbegin(mode_hash); }
    const_reverse_iterator rend() const                     { return m_hashtable.rend(); }

    // size
    size_type size() const                                  { return m_hashtable.size(); }
    size_type max_size() const                              { return m_hashtable.max_size(); }
    bool empty() const                                      { return m_hashtable.empty(); }
    size_type bucket_count() const                          { return m_hashtable.bucket_count(); }
    void resize(size_type n)                                { m_hashtable.resize(n); }
    const hasher& hash_funct() const                        { return m_hashtable.hash_funct(); }

    hash_multimap& operator = (const hash_multimap& rhs)    { m_hashtable = rhs.m_hashtable; return *this; }
    void swap(hash_multimap& rhs)                           { m_hashtable.swap(rhs.m_hashtable); }

    // insertion
    iterator insert(const value_type& x)                    { return m_hashtable.insert_equal(x); }
    template <class InputIterator> void insert_range(InputIterator first, InputIterator last) { m_hashtable.insert_equal(first, last); }

    // insert extensions
    iterator insert(const Key& k)
    {
        value_type* v = m_hashtable.key_insert_equal(k);
        typetraits<value_type>::construct(v, k);
        return m_hashtable.iterator_from_value_type(*v);
    }
    template <class T1> iterator insert(const Key& k, const T1& t1)
    {
        value_type* v = m_hashtable.key_insert_equal(k);
        new (v) value_type(k, t1);
        return m_hashtable.iterator_from_value_type(*v);
    }
    template <class T1, class T2> iterator insert(const Key& k, const T1& t1, const T2& t2)
    {
        value_type* v = m_hashtable.key_insert_equal(k);
        new (v) value_type(k, t1, t2);
        return m_hashtable.iterator_from_value_type(*v);
    }
    template <class T1, class T2, class T3> iterator insert(const Key& k, const T1& t1, const T2& t2, const T3& t3)
    {
        value_type* v = m_hashtable.key_insert_equal(k);
        new (v) value_type(k, t1, t2, t3);
        return m_hashtable.iterator_from_value_type(*v);
    }
    template <class T1, class T2, class T3, class T4> iterator insert(const Key& k, const T1& t1, const T2& t2, const T3& t3, const T4& t4)
    {
        value_type* v = m_hashtable.key_insert_equal(k);
        new (v) value_type(k, t1, t2, t3, t4);
        return m_hashtable.iterator_from_value_type(*v);
    }
    // Requires the use of placement new to construct the Value.
    // Example: new (l.insert_placement(key)) Value(arg1, arg2);
    void* insert_placement(const Key& k)
    {
        value_type* v = m_hashtable.key_insert_equal(k);
        typetraits<Key>::construct(&const_cast<Key&>(v->first), k);
        return &v->second;
    }

    void move(iterator which, iterator pos)                 { m_hashtable.move(which, pos); }

    // erasing
    void erase(iterator pos)                                { m_hashtable.erase(pos); }
    size_type erase(const key_type& k)                      { return m_hashtable.erase(k); }
    void erase(iterator first, iterator last)               { m_hashtable.erase(first, last); }
    void clear()                                            { m_hashtable.clear(); }
    void delete_all()
    {
        for (iterator iter(begin()); iter != end(); ++iter)
        {
            delete (*iter).second;
        }
        clear();
    }

    // searching
    const_iterator find(const key_type& k) const            { return m_hashtable.find(k); }
    iterator find(const key_type& k)                        { return m_hashtable.find(k); }
    size_type count(const key_type& k) const                { return m_hashtable.count(k); }

    pair<const_iterator, const_iterator> equal_range(const key_type& k, size_type* count = 0) const { return m_hashtable.equal_range(k, count); }
    pair<iterator, iterator> equal_range(const key_type& k, size_type* count = 0) { return m_hashtable.equal_range(k, count); }
};

// Swap specializations
template <class Key, class Data, class HashFunc> void swap(hash_map<Key, Data, HashFunc>& lhs, hash_map<Key, Data, HashFunc>& rhs)
{
    lhs.swap(rhs);
}

template <class Key, class Data, class HashFunc> void swap(hash_multimap<Key, Data, HashFunc>& lhs, hash_multimap<Key, Data, HashFunc>& rhs)
{
    lhs.swap(rhs);
}

} // namespace thor

// Global comparators
template <class Key, class Data, class HashFunc> bool operator == (const thor::hash_map<Key,Data,HashFunc>& lhs,
                                                                   const thor::hash_map<Key,Data,HashFunc>& rhs)
{
    typedef thor::hash_map<Key,Data,HashFunc> hashmaptype;
    if (!(lhs.size() == rhs.size()))
    {
        // Early out if size doesn't match
        return false;
    }

    for (typename hashmaptype::const_iterator iter(lhs.begin(true));
         iter != lhs.end();
         ++iter)
    {
        typename hashmaptype::const_iterator rhs_iter(rhs.find((*iter).first));
        if (rhs_iter == rhs.end())
        {
            return false;
        }

        if (!((*iter).second == (*rhs_iter).second))
        {
            return false;
        }
    }

    return true;
}

template <class Key, class Data, class HashFunc> bool operator != (const thor::hash_map<Key,Data,HashFunc>& lhs,
                                                                   const thor::hash_map<Key,Data,HashFunc>& rhs)
{
    return !(lhs == rhs);
}

template <class Key, class Data, class HashFunc> bool operator == (const thor::hash_multimap<Key,Data,HashFunc>& lhs,
                                                                   const thor::hash_multimap<Key,Data,HashFunc>& rhs)
{
    typedef thor::hash_multimap<Key,Data,HashFunc> hashmaptype;
    if (!(lhs.size() == rhs.size()))
    {
        // Early out if size doesn't match
        return false;
    }

    typedef thor::list<typename hashmaptype::const_iterator, 64> listtype;
    listtype l;

    typename hashmaptype::const_iterator iter(lhs.begin(true));
    while(iter != lhs.end())
    {
        // This is somewhat nightmarish.  There's nothing to guarantee the values for a given key are in the same order
        // in both hash_multimaps.  Therefore, we have a bit of an O(n^2) problem to verify.
        thor::pair<typename hashmaptype::const_iterator, typename hashmaptype::const_iterator> range = rhs.equal_range((*iter).first);
        // Convert to a vector of iterators
        THOR_DEBUG_ASSERT(l.empty());
        while (range.first != range.second)
        {
            l.push_back(range.first);
            ++range.first;
        }

        for (;;)
        {
            // Find the value in the vector and remove it.
            if (l.empty())
            {
                // No more matching keys in rhs
                return false;
            }
            
            bool found = false;
            for (typename listtype::iterator listiter = l.begin(); listiter != l.end(); ++listiter)
            {
                if ((**listiter).second == (*iter).second)
                {
                    l.erase(listiter);
                    found = true;
                    break;
                }
            }
            
            if (!found)
            {
                // Key/value pair not found in rhs; hash_multimaps aren't equal
                return false;
            }

            // Break loop when out of keys
            const typename hashmaptype::key_type& lhs_key = (*iter++).first;
            if (iter == lhs.end() || !(lhs_key == (*iter).first))
            {
                break;
            }
        }

        if (!l.empty())
        {
            // Remaining key/value pairs in rhs; hash_multimaps aren't equal
            return false;
        }
    }

    return true;
}

template <class Key, class Data, class HashFunc> bool operator != (const thor::hash_multimap<Key,Data,HashFunc>& lhs,
                                                                   const thor::hash_multimap<Key,Data,HashFunc>& rhs)
{
    return !(lhs == rhs);
}

#endif

