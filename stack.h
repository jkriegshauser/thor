/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * stack.h
 *
 * This file defines the STL-compatible container adapter stack.
 * Note that stack is not an actual container, it provides a subset of functionality for an underlying container
 */

#ifndef THOR_STACK_H
#define THOR_STACK_H
#pragma once

#ifndef THOR_VECTOR_H
#include "vector.h"
#endif

namespace thor
{

// extension: typically STL stacks are implemented with deque, but the default in this case is a vector
template <class T, class Sequence = vector<T> >
class stack
{
public:
    typedef typename Sequence::value_type value_type;
    typedef typename Sequence::size_type  size_type;

    stack()
    {}

    stack(const stack& rhs) :
        m_container(rhs.m_container)
    {}

    stack& operator = (const stack& rhs)
    {
        m_container = rhs.m_container;
        return *this;
    }
    
    bool empty() const
    {
        return m_container.empty();
    }

    size_type size() const
    {
        return m_container.size();
    }

    value_type& top()
    {
        THOR_DEBUG_ASSERT(!empty());
        return m_container.back();
    }

    const value_type& top() const
    {
        THOR_DEBUG_ASSERT(!empty());
        return m_container.back();
    }
    
    void push(const value_type& x)
    {
        m_container.push_back(x);
    }

    void pop()
    {
        THOR_DEBUG_ASSERT(!empty());
        m_container.pop_back();
    }

    // Extensions
    // Default-construct
    void push() { m_container.push_back(); }
    const Sequence& get_container() const { return m_container; }

private:
    Sequence m_container;
};

}

// Global operators
template <class T, class Sequence> bool operator == (const stack<T,Sequence>& lhs, const stack<T,Sequence>& rhs)
{
    return lhs.get_container() == rhs.get_container();
}

template <class T, class Sequence> bool operator != (const stack<T,Sequence>& lhs, const stack<T,Sequence>& rhs)
{
    return !(lhs == rhs);
}

template <class T, class Sequence> bool operator < (const stack<T,Sequence>& lhs, const stack<T,Sequence>& rhs)
{
    return lhs.get_container() < rhs.get_container();
}

template <class T, class Sequence> bool operator > (const stack<T,Sequence>& lhs, const stack<T,Sequence>& rhs)
{
    return rhs.get_container() < lhs.get_container();
}

template <class T, class Sequence> bool operator <= (const stack<T,Sequence>& lhs, const stack<T,Sequence>& rhs)
{
    return !(lhs > rhs);
}

template <class T, class Sequence> bool operator >= (const stack<T,Sequence>& lhs, const stack<T,Sequence>& rhs)
{
    return !(lhs < rhs);
}

#endif

