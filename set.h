/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * set.h
 *
 * This file defines (mostly) STL-compatible set and multiset containers.
 *
 * Extensions/Changes to set and multiset:
 * - The insert(pos, value_type) functions that support an insert hint are not implemented.
 * - The value_comp() functions are not implemented.
 */

#ifndef THOR_SET_H
#define THOR_SET_H
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

// thor::set
template <class Key, class Compare = less<Key> > class set
{
    typedef red_black_tree<Key, Key, identity<Key>, Compare> tree_type;
    typedef typename tree_type::iterator mutable_iterator;
    mutable_iterator make_mutable(typename tree_type::const_iterator pos) const { return *(mutable_iterator*)&pos; }
    tree_type m_tree;

public:
    typedef Key key_type;
    typedef Key value_type;
    typedef Compare key_compare;
    typedef typename tree_type::pointer pointer;
    typedef typename tree_type::reference reference;
    typedef typename tree_type::const_pointer const_pointer;
    typedef typename tree_type::const_reference const_reference;
    typedef typename tree_type::size_type size_type;
    typedef typename tree_type::difference_type difference_type;
    
    // iterator and const_iterator are the same since the value can never be modified.
    typedef typename tree_type::const_iterator          iterator;
    typedef typename tree_type::const_iterator          const_iterator;
    typedef typename tree_type::const_reverse_iterator  reverse_iterator;
    typedef typename tree_type::const_reverse_iterator  const_reverse_iterator;

    // constructors
    set()
    {}

    set(const key_compare& comp) :
        m_tree(comp)
    {}
    
    template <class InputIterator> set(InputIterator first, InputIterator last)
    {
        m_tree.insert_unique(first, last);
    }

    template <class InputIterator> set(InputIterator first, InputIterator last, const key_compare& comp) :
        m_tree(comp)
    {
        m_tree.insert_unique(first, last);
    }

    set(const set& S) :
        m_tree(S.m_tree)
    {}

    ~set()
    {}

    set& operator = (const set& S)
    {
        m_tree = S.m_tree;
        return *this;
    }

    // iteration
    iterator begin() const                              { return m_tree.begin(); }
    iterator end() const                                { return m_tree.end(); }
    
    reverse_iterator rbegin() const                     { return m_tree.rbegin(); }
    reverse_iterator rend() const                       { return m_tree.rend(); }

    // size
    bool empty() const                                  { return m_tree.empty(); }
    size_type size() const                              { return m_tree.size(); }
    size_type max_size() const                          { return m_tree.max_size(); }

    const key_compare& key_comp() const                 { return m_tree.key_comp(); }

    void swap(set& m)                                   { m_tree.swap(m.m_tree); }

    // insertion
    pair<iterator, bool> insert(const value_type& v)    { return m_tree.insert_unique(v); }
    template <class InputIterator> void insert(InputIterator first, InputIterator last) { m_tree.insert_unique(first, last); }

    // erasing
    void erase(iterator pos)                            { m_tree.erase(make_mutable(pos)); }
    size_type erase(const key_type& k)
    {
        iterator i(find(k));
        if (i != end())
        {
            m_tree.erase(make_mutable(i));
            return 1;
        }
        return 0;
    }
    void erase(iterator first, iterator last)           { m_tree.erase(make_mutable(first), make_mutable(last)); }

    void clear() { m_tree.clear(); }

    // searching
    iterator find(const key_type& k) const              { return m_tree.find(k); }

    size_type count(const key_type& k) const            { return find(k) == end() ? 0 : 1; }

    iterator lower_bound(const key_type& k) const       { return m_tree.lower_bound(k); }
    iterator upper_bound(const key_type& k) const       { return m_tree.upper_bound(k); }
    pair<iterator,iterator> equal_range(const key_type& k) const { return m_tree.equal_range(k); }
};

// thor::multiset
template <class Key, class Compare = less<Key> > class multiset
{
    typedef red_black_tree<Key, Key, identity<Key>, Compare> tree_type;
    typedef typename tree_type::iterator mutable_iterator;
    mutable_iterator make_mutable(typename tree_type::const_iterator pos) const { return *(mutable_iterator*)&pos; }
    tree_type m_tree;

public:
    typedef Key key_type;
    typedef Key value_type;
    typedef Compare key_compare;
    typedef typename tree_type::pointer pointer;
    typedef typename tree_type::reference reference;
    typedef typename tree_type::const_pointer const_pointer;
    typedef typename tree_type::const_reference const_reference;
    typedef typename tree_type::size_type size_type;
    typedef typename tree_type::difference_type difference_type;
    
    // iterator and const_iterator are the same since the value can never be modified.
    typedef typename tree_type::const_iterator iterator;
    typedef typename tree_type::const_iterator const_iterator;
    typedef typename tree_type::const_reverse_iterator reverse_iterator;
    typedef typename tree_type::const_reverse_iterator const_reverse_iterator;

    // constructors
    multiset()
    {}

    multiset(const key_compare& comp) :
        m_tree(comp)
    {}

    template <class InputIterator> multiset(InputIterator first, InputIterator last)
    {
        m_tree.insert_equal(first, last);
    }

    template <class InputIterator> multiset(InputIterator first, InputIterator last, const key_compare& comp) :
        m_tree(comp)
    {
        m_tree.insert_equal(first, last);
    }

    multiset(const multiset& m) :
        m_tree(m.m_tree)
    {}

    ~multiset()
    {}

    multiset& operator = (const multiset& m)
    {
        m_tree = m.m_tree;
        return *this;
    }

    // iteration
    iterator begin() const                              { return m_tree.begin(); }
    iterator end() const                                { return m_tree.end(); }
    
    reverse_iterator rbegin() const                     { return m_tree.rbegin(); }
    reverse_iterator rend() const                       { return m_tree.rend(); }

    // size
    bool empty() const                                  { return m_tree.empty(); }
    size_type size() const                              { return m_tree.size(); }
    size_type max_size() const                          { return m_tree.max_size(); }

    const key_compare& key_comp() const                 { return m_tree.key_comp(); }
    
    void swap(multiset& m)                              { m_tree.swap(m.m_tree); }

    // insertion
    iterator insert(const value_type& v)                { return m_tree.insert_equal(v); }
    template <class InputIterator> void insert(InputIterator f, InputIterator l) { m_tree.insert_equal(f, l); }

    // erasing
    void erase(iterator pos)                            { m_tree.erase(make_mutable(pos)); }
    size_type erase(const key_type& k)                  { return m_tree.erase(k); }
    void erase(iterator first, iterator last)           { m_tree.erase(make_mutable(first), make_mutable(last)); }

    void clear() { m_tree.clear(); }

    // searching
    iterator find(const key_type& k) const              { return m_tree.find(k); }

    size_type count(const key_type& k) const            { return m_tree.count(k); }

    iterator lower_bound(const key_type& k) const       { return m_tree.lower_bound(k); }
    iterator upper_bound(const key_type& k) const       { return m_tree.upper_bound(k); }
    pair<iterator,iterator> equal_range(const key_type& k) const { return m_tree.equal_range(k); }
};

// Swap specialization
template <class Key, class Compare> void swap(set<Key, Compare>& lhs, set<Key, Compare>& rhs)
{
    lhs.swap(rhs);
}

template <class Key, class Compare> void swap(multiset<Key, Compare>& lhs, multiset<Key, Compare>& rhs)
{
    lhs.swap(rhs);
}

} // namespace thor

// Global operators
template <class Key, class Compare>
bool operator == (const thor::set<Key,Compare>& lhs, const thor::set<Key,Compare>& rhs)
{
    return lhs.size() == rhs.size() && thor::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <class Key, class Compare>
bool operator < (const thor::set<Key,Compare>& lhs, const thor::set<Key,Compare>& rhs)
{
    return thor::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <class Key, class Compare>
bool operator != (const thor::set<Key,Compare>& lhs, const thor::set<Key,Compare>& rhs)
{
    return !(lhs == rhs);
}

template <class Key, class Compare>
bool operator > (const thor::set<Key,Compare>& lhs, const thor::set<Key,Compare>& rhs)
{
    return thor::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), thor::greater<typename thor::set<Key,Compare>::value_type>());
}

template <class Key, class Compare>
bool operator <= (const thor::set<Key,Compare>& lhs, const thor::set<Key,Compare>& rhs)
{
    return !(lhs > rhs);
}

template <class Key, class Compare>
bool operator >= (const thor::set<Key,Compare>& lhs, const thor::set<Key,Compare>& rhs)
{
    return !(lhs < rhs);
}

template <class Key, class Compare>
bool operator == (const thor::multiset<Key,Compare>& lhs, const thor::multiset<Key,Compare>& rhs)
{
    return lhs.size() == rhs.size() && thor::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <class Key, class Compare>
bool operator < (const thor::multiset<Key,Compare>& lhs, const thor::multiset<Key,Compare>& rhs)
{
    return thor::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <class Key, class Compare>
bool operator != (const thor::multiset<Key,Compare>& lhs, const thor::multiset<Key,Compare>& rhs)
{
    return !(lhs == rhs);
}

template <class Key, class Compare>
bool operator > (const thor::multiset<Key,Compare>& lhs, const thor::multiset<Key,Compare>& rhs)
{
    return thor::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), thor::greater<typename thor::multiset<Key,Compare>::value_type>());
}

template <class Key, class Compare>
bool operator <= (const thor::multiset<Key,Compare>& lhs, const thor::multiset<Key,Compare>& rhs)
{
    return !(lhs > rhs);
}

template <class Key, class Compare>
bool operator >= (const thor::multiset<Key,Compare>& lhs, const thor::multiset<Key,Compare>& rhs)
{
    return !(lhs < rhs);
}

#endif

