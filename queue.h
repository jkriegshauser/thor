/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * queue.h
 *
 * This file defines the STL-compatible container adapter queue.
 * Note that stack is not an actual container, it provides a subset of functionality for an underlying container
 */

#ifndef THOR_QUEUE_H
#define THOR_QUEUE_H
#pragma once

#ifndef THOR_DEQUE_H
#include "deque.h"
#endif

#ifndef THOR_VECTOR_H
#include "vector.h"
#endif

#ifndef THOR_HEAP_H
#include "heap.h"
#endif

namespace thor
{

// thor::queue
template <class T, class Sequence = deque<T> > class queue
{
public:
    typedef typename Sequence::value_type value_type;
    typedef typename Sequence::size_type  size_type;

    queue()
    {}

    queue(const queue& rhs) :
        m_container(rhs.m_container)
    {}
    
    queue& operator = (const queue& rhs)
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

    T& front()
    {
        THOR_DEBUG_ASSERT(!empty());
        return m_container.front();
    }

    const T& front() const
    {
        THOR_DEBUG_ASSERT(!empty());
        return m_container.front();
    }

    T& back()
    {
        THOR_DEBUG_ASSERT(!empty());
        return m_container.back();
    }

    const T& back() const
    {
        THOR_DEBUG_ASSERT(!empty());
        return m_container.back();
    }

    void push(const T& x)
    {
        m_container.push_back(x);
    }

    void pop()
    {
        THOR_DEBUG_ASSERT(!empty());
        m_container.pop_front();
    }

    // Extensions
    // Default-construct
    void push() { m_container.push_back(); }
    const Sequence& get_container() const { return m_container; }

private:
    Sequence m_container;
};

// thor::priority_queue
template <class T, class Sequence = vector<T>, class Compare = less<T> > class priority_queue
{
public:
    typedef typename Sequence::value_type value_type;
    typedef typename Sequence::size_type  size_type;

    priority_queue() {}
    priority_queue(const priority_queue& rhs) : m_container(rhs.m_container), m_root(rhs.m_root) {}
    priority_queue(const Compare& c) : m_root(c) {}

    priority_queue(const value_type* first, const value_type* last) : m_container(first, last)
    {
        m_root.m_pushcount = last - first;
    }

    priority_queue(const value_type* first, const value_type* last, const Compare& c) : m_container(first, last), m_root(c)
    {
        m_root.m_pushcount = last - first;
    }

    priority_queue& operator=(const priority_queue& rhs)
    {
        m_container = rhs.m_container;
        m_root = rhs.m_root;
        return *this;
    }

    bool empty() const { return m_container.empty(); }
    size_type size() const { return m_container.size(); }
    
    const value_type& top() const
    {
        THOR_DEBUG_ASSERT(!empty());
        if (m_root.m_pushcount != 0)
        {
            heapify();
        }
        return m_container.front();
    }
    void push(const value_type& x)
    {
        m_container.push_back(x);
        if (m_root.m_pushcount == 0)
        {
            push_heap(m_container.begin(), m_container.end(), comp());
        }
        else
        {
            ++m_root.m_pushcount;
        }
    }
    void pop()
    {
        THOR_DEBUG_ASSERT(!empty());
        if (m_root.m_pushcount != 0)
        {
            heapify();
        }
        pop_heap(m_container.begin(), m_container.end(), comp());
        m_container.pop_back();
        THOR_DEBUG_ASSERT(is_heap(m_container.begin(), m_container.end(), comp()));
    }

    // Extensions
    // Default-construct
    void push()
    {
        m_container.push_back();
        if (m_root.m_pushcount == 0)
        {
            push_heap(m_container.begin(), m_container.end(), comp());
        }
        else
        {
            ++m_root.m_pushcount;
        }
    }
    const Sequence& get_container() const
    {
        if (m_root.m_pushcount != 0)
        {
            heapify();
        }
        return m_container;
    }

private:
    void heapify() const
    {
        THOR_DEBUG_ASSERT(m_root.m_pushcount != 0);
        if (m_root.m_pushcount >= size() || m_root.m_pushcount > 2)
        {
            THOR_DEBUG_ASSERT(m_root.m_pushcount == size() || is_heap(m_container.begin(), m_container.end() - m_root.m_pushcount, comp()));

            // Just rebuild the heap
            make_heap(m_container.begin(), m_container.end(), c);
        }
        else
        {
            // Just push the queued up values
            THOR_DEBUG_ASSERT(is_heap(m_container.begin(), m_container.end() - m_root.m_pushcount, comp()));
            Sequence::iterator iter(m_container.end());
            iter -= m_root.m_pushcount;
            do
            {
                push_heap(m_container.begin(), ++iter, c);
            } while (iter != m_container.end());
        }

        m_root.m_pushcount = 0;
        THOR_DEBUG_ASSERT(is_heap(m_container.begin(), m_container.end(), comp()));
    }

    Compare& comp()
    {
        return static_cast<Compare&>(m_root);
    }

    const Compare& comp() const
    {
        return static_cast<const Compare&>(m_root);
    }

    struct empty_member_opt : public Compare
    {
        size_type m_pushcount;

        empty_member_opt() : m_pushcount(0) {}
        empty_member_opt(const Compare& c) : Compare(c), m_pushcount(0) {}
    };
    
    mutable Sequence m_container;
    mutable empty_member_opt m_root;
};

} // namespace thor

// Global operators
template <class T, class Sequence>
bool operator == (const queue<T,Sequence>& lhs, const queue<T,Sequence>& rhs)
{
    return lhs.get_container() == rhs.get_container();
}

template <class T, class Sequence>
bool operator != (const queue<T,Sequence>& lhs, const queue<T,Sequence>& rhs)
{
    return !(lhs == rhs);
}

template <class T, class Sequence>
bool operator < (const queue<T,Sequence>& lhs, const queue<T,Sequence>& rhs)
{
    return lhs.get_container() < rhs.get_container();
}

template <class T, class Sequence>
bool operator > (const queue<T,Sequence>& lhs, const queue<T,Sequence>& rhs)
{
    return rhs.get_container() < lhs.get_container();
}

template <class T, class Sequence>
bool operator <= (const queue<T,Sequence>& lhs, const queue<T,Sequence>& rhs)
{
    return !(lhs > rhs);
}

template <class T, class Sequence>
bool operator >= (const queue<T,Sequence>& lhs, const queue<T,Sequence>& rhs)
{
    return !(lhs < rhs);
}

#endif

