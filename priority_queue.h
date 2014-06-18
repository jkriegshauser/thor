/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * priority_queue.h
 *
 * This file defines the STL-compatible container adapter priority_queue.
 * Note that stack is not an actual container, it provides a subset of functionality for an underlying container
 */

#ifndef THOR_PRIORITY_QUEUE_H
#define THOR_PRIORITY_QUEUE_H
#pragma once

#ifndef THOR_VECTOR_H
#include "vector.h"
#endif

#ifndef THOR_FUNCTION_H
#include "function.h"
#endif

#ifndef THOR_HEAP_H
#include "heap.h"
#endif

namespace thor
{

template<typename T, typename Sequence = vector<T>, typename Compare = less<T> > class priority_queue : private Sequence, private Compare
{
public:
    typedef T               value_type;
    typedef thor_size_type  size_type;
    typedef Sequence        sequence_type;
    typedef Compare         compare_type;

    priority_queue() {}
    priority_queue(const priority_queue& rhs) : Sequence(rhs), Compare(rhs) {}
    priority_queue(const Compare& comp) : Compare(comp) {}
    priority_queue(const value_type* begin, const value_type* end);
    priority_queue(const value_type* begin, const value_type* end, const Compare& comp);
    ~priority_queue() {}

    priority_queue& operator = (const priority_queue& rhs);

    bool empty() const              { return Sequence::empty(); }
    size_type size() const          { return Sequence::size(); }
    const value_type& top() const   { THOR_ASSERT(!empty()); return Sequence::at(0); }

    void push();
    void push(const value_type& t);
    template<class T1> void push(const T1& t1);
    template<class T1, class T2> void push(const T1& t1, const T2& t2);
    template<class T1, class T2, class T3> void push(const T1& t1, const T2& t2, const T3& t3);
    template<class T1, class T2, class T3, class T4> void push(const T1& t1, const T2& t2, const T3& t3, const T4& t4);

    void pop();
};

template<typename T, typename Sequence, typename Compare> priority_queue<T,Sequence,Compare>::priority_queue(const value_type* begin, const value_type* end)
{
    size_type count = distance(begin, end);
    if (count)
    {
        Sequence::assign(begin, end);
        make_heap(Sequence::begin(), Sequence::end(), static_cast<Compare&>(*this));
    }
}

template<typename T, typename Sequence, typename Compare> priority_queue<T,Sequence,Compare>::priority_queue(const value_type* begin, const value_type* end, const Compare& comp)
    : Compare(comp)
{
    size_type count = distance(begin, end);
    if (count)
    {
        Sequence::assign(begin, end);
        make_heap(Sequence::begin(), Sequence::end(), static_cast<Compare&>(*this));
    }
}

template<typename T, typename Sequence, typename Compare> priority_queue<T,Sequence,Compare>& priority_queue<T, Sequence, Compare>::operator=(const priority_queue& rhs)
{
    static_cast<Sequence&>(*this) = rhs;
    static_cast<Compare&>(*this) = rhs;
    return *this;
}

template<typename T, typename Sequence, typename Compare> void priority_queue<T,Sequence,Compare>::push()
{
    static_cast<Sequence&>(*this).push_back();
    push_heap(begin(), end(), static_cast<Compare&>(*this));
}

template<typename T, typename Sequence, typename Compare> void priority_queue<T,Sequence,Compare>::push(const value_type& v)
{
    static_cast<Sequence&>(*this).push_back(v);
    push_heap(begin(), end(), static_cast<Compare&>(*this));
}

template<typename T, typename Sequence, typename Compare> template<class T1> void priority_queue<T,Sequence,Compare>::push(const T1& t1)
{
    static_cast<Sequence&>(*this).push_back(t1);
    push_heap(begin(), end(), static_cast<Compare&>(*this));
}

template<typename T, typename Sequence, typename Compare> template<class T1, class T2> void priority_queue<T,Sequence,Compare>::push(const T1& t1, const T2& t2)
{
    static_cast<Sequence&>(*this).push_back(t1, t2);
    push_heap(begin(), end(), static_cast<Compare&>(*this));
}

template<typename T, typename Sequence, typename Compare> template<class T1, class T2, class T3> void priority_queue<T,Sequence,Compare>::push(const T1& t1, const T2& t2, const T3& t3)
{
    static_cast<Sequence&>(*this).push_back(t1, t2, t3);
    push_heap(begin(), end(), static_cast<Compare&>(*this));
}

template<typename T, typename Sequence, typename Compare> template<class T1, class T2, class T3, class T4> void priority_queue<T,Sequence,Compare>::push(const T1& t1, const T2& t2, const T3& t3, const T4& t4)
{
    static_cast<Sequence&>(*this).push_back(t1, t2, t3, t4);
    push_heap(begin(), end(), static_cast<Compare&>(*this));
}

template<typename T, typename Sequence, typename Compare> void priority_queue<T,Sequence,Compare>::pop()
{
    THOR_ASSERT(!empty());
    pop_heap(begin(), end(), static_cast<Compare&>(*this));
    Sequence::pop_back();
}

}

#endif