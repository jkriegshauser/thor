/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * map.h
 *
 * This file defines (mostly) STL-compatible map and multimap associative containers
 *
 * Extensions/Changes to map and multimap:
 * - The insert(pos, value_type) functions that support an insert hint are not implemented.
 * - The value_comp() functions are not implemented.
 * - The insert(InputIterator, InputIterator) function has been renamed insert_range.
 * - While insert(value_type) is supported, it is not the best way to insert elements.
 *   Consider using the templatized insert() functions. These functions pass up to four
 *   parameters directly to the constructor meaning that there is no copy construction
 *   of the Value. Additionally, if more than four parameters are required, insert_placement()
 *   can be used with placement new to construct the item.
 *   * In the case of map, if the key/value pair already exists, the value is destroyed
 *     and re-constructed.
 *   * In the case of map, the insert() functions return an iterator, so it is impossible
 *     to tell whether the key previously existed from the insert() function call alone.
 */

#ifndef THOR_MAP_H
#define THOR_MAP_H
#pragma once

#ifndef THOR_TREE_H
#include "tree.h"
#endif

#ifndef THOR_FUNCTION_H
#include "function.h"
#endif

#ifndef THOR_SORT_H
#include "sort.h"
#endif

namespace thor
{

// thor::map
template <class Key, class Value, class Compare = less<Key> > class map
{
public:
    typedef Key key_type;
    typedef Value data_type;
    typedef pair<const Key, Value> value_type;
    typedef Compare key_compare;

private:
    typedef red_black_tree<key_type, value_type, select1st<value_type>, Compare> tree_type;
    tree_type m_tree;

public:
    typedef typename tree_type::pointer pointer;
    typedef typename tree_type::reference reference;
    typedef typename tree_type::const_pointer const_pointer;
    typedef typename tree_type::const_reference const_reference;
    typedef typename tree_type::size_type size_type;
    typedef typename tree_type::difference_type difference_type;
    typedef typename tree_type::iterator iterator;
    typedef typename tree_type::const_iterator const_iterator;
    typedef typename tree_type::reverse_iterator reverse_iterator;
    typedef typename tree_type::const_reverse_iterator const_reverse_iterator;

    // constructors
    map()
    {}

    map(const key_compare& comp) :
        m_tree(comp)
    {}
    
    template <class InputIterator> map(InputIterator first, InputIterator last)
    {
        m_tree.insert_unique(first, last);
    }

    template <class InputIterator> map(InputIterator first, InputIterator last, const key_compare& comp) :
        m_tree(comp)
    {
        m_tree.insert_unique(first, last);
    }
    
    map(const map& m) :
        m_tree(m.m_tree)
    {}

    ~map()
    {}

    map& operator = (const map& m)
    {
        m_tree = m.m_tree;
        return *this;
    }

    // members
    iterator begin()                                { return m_tree.begin(); }
    const_iterator begin() const                    { return m_tree.begin(); }
    iterator end()                                  { return m_tree.end(); }
    const_iterator end() const                      { return m_tree.end(); }
    
    reverse_iterator rbegin()                       { return m_tree.rbegin(); }
    const_reverse_iterator rbegin() const           { return m_tree.rbegin(); }
    reverse_iterator rend()                         { return m_tree.rend(); }
    const_reverse_iterator rend() const             { return m_tree.rend(); }

    bool empty() const                              { return m_tree.empty(); }
    size_type size() const                          { return m_tree.size(); }
    size_type max_size() const                      { return m_tree.max_size(); }

    const key_compare& key_comp() const             { return m_tree.key_comp(); }

    void swap(map& m)                               { m_tree.swap(m.m_tree); }

    pair<iterator, bool> insert(const value_type& v) { return m_tree.insert_unique(v); }
    template <class InputIterator> void insert_range(InputIterator first, InputIterator last) { m_tree.insert_unique(first, last); }

    // Extended insert():
    // These insert extension functions work more like operator[] than insert(value_type) in that
    // they always reconstruct the value object if it already exists.
    iterator insert(const Key& key)
    {
        value_type* v = m_tree.key_insert_unique(key);
        typetraits<value_type>::construct(v, key);
        return m_tree.iterator_from_value_type(*v);
    }
    template <class T1> iterator insert(const Key& key, const T1& t1)
    {
        value_type* v = m_tree.key_insert_unique(key);
        new (v) value_type(key, t1);
        return m_tree.iterator_from_value_type(*v);
    }
    template <class T1, class T2> iterator insert(const Key& key, const T1& t1, const T2& t2)
    {
        value_type* v = m_tree.key_insert_unique(key);
        new (v) value_type(key, t1, t2);
        return m_tree.iterator_from_value_type(*v);
    }
    template <class T1, class T2, class T3> iterator insert(const Key& key, const T1& t1, const T2& t2, const T3& t3)
    {
        value_type* v = m_tree.key_insert_unique(key);
        new (v) value_type(key, t1, t2, t3);
        return m_tree.iterator_from_value_type(*v);
    }
    template <class T1, class T2, class T3, class T4> iterator insert(const Key& key, const T1& t1, const T2& t2, const T3& t3, const T4& t4)
    {
        value_type* v = m_tree.key_insert_unique(key);
        new (v) value_type(key, t1, t2, t3, t4);
        return m_tree.iterator_from_value_type(*v);
    }
    // Requires the use of placement new to construct the Value.
    // Example: new (l.insert_placement(key)) Value(arg1, arg2);
    void* insert_placement(const Key& key)
    {
        value_type* v = m_tree.key_insert_unique(key);
        typetraits<Key>::construct(const_cast<Key*>(&v->first), key);
        return &v->second;
    }

    void erase(iterator pos)                        { m_tree.erase(pos); }
    
    size_type erase(const key_type& k)
    {
        iterator i(find(k));
        if (i != end()) { m_tree.erase(i); return 1; }
        return 0;
    }
    
    void erase(iterator first, iterator last)       { m_tree.erase(first, last); }

    void clear()                                    { m_tree.clear(); }

    iterator find(const key_type& k)                { return m_tree.find(k); }
    const_iterator find(const key_type& k) const    { return m_tree.find(k); }

    size_type count(const key_type& k) const        { return find(k) == end() ? 0 : 1; }

    iterator lower_bound(const key_type& k)         { return m_tree.lower_bound(k); }
    const_iterator lower_bound(const key_type& k) const { return m_tree.lower_bound(k); }
    
    iterator upper_bound(const key_type& k)         { return m_tree.upper_bound(k); }
    const_iterator upper_bound(const key_type& k) const { return m_tree.upper_bound(k); }
    
    pair<iterator,iterator> equal_range(const key_type& k) { return m_tree.equal_range(k); }
    pair<const_iterator,const_iterator> equal_range(const key_type& k) const { return m_tree.equal_range(k); }

    // Using this involves default-constructing the value and copying the key.
    data_type& operator [] (const key_type& k)
    {
        return (*insert(value_type(k, data_type())).first).second;
    }
};

// thor::multimap
template <class Key, class Value, class Compare = less<Key> > class multimap
{
public:
    typedef Key key_type;
    typedef Value data_type;
    typedef pair<const Key, Value> value_type;
    typedef Compare key_compare;

private:
    typedef red_black_tree<key_type, value_type, select1st<value_type>, Compare> tree_type;
    tree_type m_tree;

public:
    typedef typename tree_type::pointer pointer;
    typedef typename tree_type::reference reference;
    typedef typename tree_type::const_pointer const_pointer;
    typedef typename tree_type::const_reference const_reference;
    typedef typename tree_type::size_type size_type;
    typedef typename tree_type::difference_type difference_type;
    typedef typename tree_type::iterator iterator;
    typedef typename tree_type::const_iterator const_iterator;
    typedef typename tree_type::reverse_iterator reverse_iterator;
    typedef typename tree_type::const_reverse_iterator const_reverse_iterator;

    // constructors
    multimap()
    {}

    multimap(const key_compare& comp) :
        m_tree(comp)
    {}
    
    template <class InputIterator> multimap(InputIterator first, InputIterator last)
    {
        m_tree.insert_unique(first, last);
    }
    
    template <class InputIterator> multimap(InputIterator first, InputIterator last, const key_compare& comp) :
        m_tree(comp)
    {
        m_tree.insert_unique(first, last);
    }
    
    multimap(const multimap& m) :
        m_tree(m.m_tree)
    {}

    ~multimap()
    {}

    multimap& operator = (const multimap& m)
    {
        m_tree = m.m_tree;
        return *this;
    }

    // members
    iterator begin()                            { return m_tree.begin(); }
    const_iterator begin() const                { return m_tree.begin(); }
    iterator end()                              { return m_tree.end(); }
    const_iterator end() const                  { return m_tree.end(); }
    
    reverse_iterator rbegin()                   { return m_tree.rbegin(); }
    const_reverse_iterator rbegin() const       { return m_tree.rbegin(); }
    reverse_iterator rend()                     { return m_tree.rend(); }
    const_reverse_iterator rend() const         { return m_tree.rend(); }

    bool empty() const                          { return m_tree.empty(); }
    size_type size() const                      { return m_tree.size(); }
    size_type max_size() const                  { return m_tree.max_size(); }

    key_compare key_comp() const                { return tree.key_comp(); }

    void swap(multimap& m)                      { m_tree.swap(m.m_tree); }

    iterator insert(const value_type& v)        { return m_tree.insert_equal(v); }
    template <class InputIterator> void insert_range(InputIterator first, InputIterator last) { m_tree.insert_equal(first, last); }

    iterator insert(const Key& key)
    {
        value_type* v = m_tree.key_insert_equal(key);
        typetraits<value_type>::construct(v, key);
        return m_tree.iterator_from_value_type(*v);
    }
    template <class T1> iterator insert(const Key& key, const T1& t1)
    {
        value_type* v = m_tree.key_insert_equal(key);
        new (v) value_type(key, t1);
        return m_tree.iterator_from_value_type(*v);
    }
    template <class T1, class T2> iterator insert(const Key& key, const T1& t1, const T2& t2)
    {
        value_type* v = m_tree.key_insert_equal(key);
        new (v) value_type(key, t1, t2);
        return m_tree.iterator_from_value_type(*v);
    }
    template <class T1, class T2, class T3> iterator insert(const Key& key, const T1& t1, const T2& t2, const T3& t3)
    {
        value_type* v = m_tree.key_insert_equal(key);
        new (v) value_type(key, t1, t2, t3);
        return m_tree.iterator_from_value_type(*v);
    }
    template <class T1, class T2, class T3, class T4> iterator insert(const Key& key, const T1& t1, const T2& t2, const T3& t3, const T4& t4)
    {
        value_type* v = m_tree.key_insert_equal(key);
        new (v) value_type(key, t1, t2, t3, t4);
        return m_tree.iterator_from_value_type(*v);
    }
    // Requires the use of placement new to construct the Value.
    // Example: new (l.insert_placement(key)) Value(arg1, arg2);
    void* insert_placement(const Key& key)
    {
        value_type* v = m_tree.key_insert_equal(key);
        typetraits<Key>::construct(const_cast<Key*>(&v->first), key);
        return &v->second;
    }

    void erase(iterator pos)                    { m_tree.erase(pos); }
    size_type erase(const key_type& k)          { return m_tree.erase(k); }
    void erase(iterator first, iterator last)   { m_tree.erase(first, last); }

    void clear()                                { m_tree.clear(); }

    iterator find(const key_type& k)            { return m_tree.find(k); }
    const_iterator find(const key_type& k) const { return m_tree.find(k); }

    size_type count(const key_type& k) const    { return m_tree.count(k); }

    iterator lower_bound(const key_type& k)     { return m_tree.lower_bound(k); }
    const_iterator lower_bound(const key_type& k) const { return m_tree.lower_bound(k); }

    iterator upper_bound(const key_type& k)     { return m_tree.upper_bound(k); }
    const_iterator upper_bound(const key_type& k) const { return m_tree.upper_bound(k); }

    pair<iterator,iterator> equal_range(const key_type& k) { return m_tree.equal_range(k); }
    pair<const_iterator,const_iterator> equal_range(const key_type& k) const { return m_tree.equal_range(k); }
};

// Swap specializations
template <class Key, class Value, class Compare> void swap(map<Key, Value, Compare>& lhs, map<Key, Value, Compare>& rhs)
{
    lhs.swap(rhs);
}

template <class Key, class Value, class Compare> void swap(multimap<Key, Value, Compare>& lhs, multimap<Key, Value, Compare>& rhs)
{
    lhs.swap(rhs);
}

} // namespace thor

// Global operators
template <class Key, class Value, class Compare>
bool operator == (const thor::map<Key,Value,Compare>& lhs, const thor::map<Key,Value,Compare>& rhs)
{
    return lhs.size() == rhs.size() && thor::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <class Key, class Value, class Compare>
bool operator < (const thor::map<Key,Value,Compare>& lhs, const thor::map<Key,Value,Compare>& rhs)
{
    return thor::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <class Key, class Value, class Compare>
bool operator != (const thor::map<Key,Value,Compare>& lhs, const thor::map<Key,Value,Compare>& rhs)
{
    return !(lhs == rhs);
}

template <class Key, class Value, class Compare>
bool operator > (const thor::map<Key,Value,Compare>& lhs, const thor::map<Key,Value,Compare>& rhs)
{
    return thor::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), thor::greater<typename thor::map<Key,Value,Compare>::value_type>());
}

template <class Key, class Value, class Compare>
bool operator <= (const thor::map<Key,Value,Compare>& lhs, const thor::map<Key,Value,Compare>& rhs)
{
    return !(lhs > rhs);
}

template <class Key, class Value, class Compare>
bool operator >= (const thor::map<Key,Value,Compare>& lhs, const thor::map<Key,Value,Compare>& rhs)
{
    return !(lhs < rhs);
}

template <class Key, class Value, class Compare>
bool operator == (const thor::multimap<Key,Value,Compare>& lhs, const thor::multimap<Key,Value,Compare>& rhs)
{
    return lhs.size() == rhs.size() && thor::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <class Key, class Value, class Compare>
bool operator < (const thor::multimap<Key,Value,Compare>& lhs, const thor::multimap<Key,Value,Compare>& rhs)
{
    return thor::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <class Key, class Value, class Compare>
bool operator != (const thor::multimap<Key,Value,Compare>& lhs, const thor::multimap<Key,Value,Compare>& rhs)
{
    return !(lhs == rhs);
}

template <class Key, class Value, class Compare>
bool operator > (const thor::multimap<Key,Value,Compare>& lhs, const thor::multimap<Key,Value,Compare>& rhs)
{
    return thor::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), thor::greater<typename thor::multimap<Key,Value,Compare>::value_type>());
}

template <class Key, class Value, class Compare>
bool operator <= (const thor::multimap<Key,Value,Compare>& lhs, const thor::multimap<Key,Value,Compare>& rhs)
{
    return !(lhs > rhs);
}

template <class Key, class Value, class Compare>
bool operator >= (const thor::multimap<Key,Value,Compare>& lhs, const thor::multimap<Key,Value,Compare>& rhs)
{
    return !(lhs < rhs);
}

#endif

