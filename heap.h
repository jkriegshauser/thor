/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * heap.h
 *
 * ** THOR INTERNAL FILE - NOT FOR APPLICATION USE **
 *
 * This file defines STL-compatible functions for managing an array configured as a heap:
 *  is_heap(first,last[,comp]) - returns true if [first, last) is a heap.  Linear time: O(n)
 *  make_heap(first,last[,comp]) - turns [first,last) into a heap.  Linear time: O(3*n) worst case
 *  push_heap(first,last[,comp]) - pushes the value at (last-1) onto an existing heap.  Logarithmic time: O(log(n)) worst case
 *  pop_heap(first,last[,comp]) - moves the largest value to (last-1) and maintains the heap.  Logarithmic time: O(2*log(n)) worst case
 *  sort_heap(first,last[,comp]) - turns the heap at [first,last) into a sorted list.  Exponential logarithmic time: O(n*log(n)) worst case
 */

#ifndef THOR_HEAP_H
#define THOR_HEAP_H
#pragma once

#ifndef THOR_BASETYPES_H
#include "basetypes.h"
#endif

#ifndef THOR_ITERATOR_H
#include "iterator.h"
#endif

#ifndef THOR_SORT_H
#include "sort.h"
#endif

namespace thor
{

template <class RandomAccessIterator, class T>
void __push_heap_internal(RandomAccessIterator first, thor_diff_type holeindex, thor_diff_type topindex, T val)
{
    thor_diff_type parent = (holeindex - 1) / 2;
    while (holeindex > topindex && *(first + parent) < val)
    {
        *(first + holeindex) = *(first + parent);
        holeindex = parent;
        parent = (holeindex - 1) / 2;
    }
    *(first + holeindex) = val;
}

template <class RandomAccessIterator, class T>
void __push_heap_type(RandomAccessIterator first, RandomAccessIterator last, T*) 
{
    __push_heap_internal(first, (last - first) - 1, 0, T(*(last - 1)));
}

template <class RandomAccessIterator, class T>
void __adjust_heap_internal(RandomAccessIterator first, thor_diff_type holeindex, thor_diff_type length, T val)
{
    thor_diff_type topindex = holeindex;
    thor_diff_type secondchild = 2 * holeindex + 2;
    while (secondchild < length)
    {
        if (*(first + secondchild) < *(first + (secondchild - 1)))
        {
            --secondchild;
        }
        *(first + holeindex) = *(first + secondchild);
        holeindex = secondchild;
        secondchild = 2 * (secondchild + 1);
    }
    
    if (secondchild == length)
    {
        *(first + holeindex) = *(first + (secondchild - 1));
        holeindex = secondchild - 1;
    }
    __push_heap_internal(first, holeindex, topindex, val);
}

template <class RandomAccessIterator, class T>
void __make_heap_internal(RandomAccessIterator first, RandomAccessIterator last, T*)
{
    if (last - first < 2)
    {
        return;
    }

    thor_diff_type length = last - first;
    thor_diff_type parent = (length - 2)/2;

    for (;;)
    {
        __adjust_heap_internal(first, parent, length, T(*(first + parent)));
        if ( parent-- == 0 )
        {
            return;
        }
    }
}

template <class RandomAccessIterator, class T>
void __pop_heap_internal(RandomAccessIterator first, RandomAccessIterator last, RandomAccessIterator result, T val)
{
    *result = *first;
    __adjust_heap_internal(first, 0, last - first, val);
}

template <class RandomAccessIterator, class T>
void __pop_heap_type(RandomAccessIterator first, RandomAccessIterator last, T*)
{
    __pop_heap_internal(first, last - 1, last - 1, T(*(last - 1)));
}

template <class RandomAccessIterator, class StrictWeakOrdering>
bool __is_heap_internal(RandomAccessIterator first, StrictWeakOrdering comp, thor_diff_type length)
{
    thor_diff_type parent = 0;
    for (thor_diff_type child = 1; child < length; ++child)
    {
        if (comp(*(first+parent), *(first+child)))
            return false;
        if ((child & 1) == 0)
            ++parent;
    }
    return true;
}

template <class T>
less<T> __less_selector(T*)
{
    return less<T>();
}

template <class RandomAccessIterator>
void make_heap(RandomAccessIterator first, RandomAccessIterator last)
{
    __make_heap_internal(first, last, THOR_GET_VALUE_TYPE(first, RandomAccessIterator));
}

template <class RandomAccessIterator>
void push_heap(RandomAccessIterator first, RandomAccessIterator last) 
{
    __push_heap_type(first, last, THOR_GET_VALUE_TYPE(first, RandomAccessIterator));
}

template <class RandomAccessIterator>
void pop_heap(RandomAccessIterator first, RandomAccessIterator last)
{
    __pop_heap_type(first, last, THOR_GET_VALUE_TYPE(first, RandomAccessIterator));
}

template <class RandomAccessIterator>
bool is_heap(RandomAccessIterator first, RandomAccessIterator last)
{
    THOR_DEBUG_ASSERT(last - first >= 0);
    return __is_heap_internal(first, __less_selector(THOR_GET_VALUE_TYPE(first, RandomAccessIterator)), last - first);
}

template <class RandomAccessIterator>
void sort_heap(RandomAccessIterator first, RandomAccessIterator last)
{
    while (last - first > 1)
    {
        pop_heap(first, last--);
    }
}

//-----------------------------------------------------------------------------
// Compare versions

template <class RandomAccessIterator, class Compare, class T>
void __push_heap_internal(RandomAccessIterator first, thor_diff_type holeindex, thor_diff_type topindex, T val, Compare comp)
{
    thor_diff_type parent = (holeindex - 1) / 2;
    while (holeindex > topindex && comp(*(first + parent), val))
    {
        *(first + holeindex) = *(first + parent);
        holeindex = parent;
        parent = (holeindex - 1) / 2;
    }
    *(first + holeindex) = val;
}

template <class RandomAccessIterator, class Compare, class T>
void __push_heap_type(RandomAccessIterator first, RandomAccessIterator last, Compare comp, T*) 
{
    __push_heap_internal(first, (last - first) - 1, 0, T(*(last - 1)), comp);
}

template <class RandomAccessIterator, class Compare, class T>
void __adjust_heap_internal(RandomAccessIterator first, thor_diff_type holeindex, thor_diff_type length, T val, Compare comp)
{
    thor_diff_type topindex = holeindex;
    thor_diff_type secondchild = 2 * holeindex + 2;
    while (secondchild < length)
    {
        if (comp(*(first + secondchild), *(first + (secondchild - 1))))
        {
            --secondchild;
        }
        *(first + holeindex) = *(first + secondchild);
        holeindex = secondchild;
        secondchild = 2 * (secondchild + 1);
    }

    if (secondchild == length)
    {
        *(first + holeindex) = *(first + (secondchild - 1));
        holeindex = secondchild - 1;
    }
    __push_heap_internal(first, holeindex, topindex, val, comp);
}

template <class RandomAccessIterator, class Compare, class T>
void __make_heap_internal(RandomAccessIterator first, RandomAccessIterator last, Compare comp, T*)
{
    if (last - first < 2)
    {
        return;
    }

    thor_diff_type length = last - first;
    thor_diff_type parent = (length - 2)/2;

    for (;;)
    {
        __adjust_heap_internal(first, parent, length, T(*(first + parent)), comp);
        if ( parent-- == 0 )
        {
            return;
        }
    }
}

template <class RandomAccessIterator, class Compare, class T>
void __pop_heap_internal(RandomAccessIterator first, RandomAccessIterator last, RandomAccessIterator result, T val, Compare comp)
{
    *result = *first;
    __adjust_heap_internal(first, 0, last - first, val, comp);
}

template <class RandomAccessIterator, class Compare, class T>
void __pop_heap_type(RandomAccessIterator first, RandomAccessIterator last, Compare comp, T*)
{
    __pop_heap_internal(first, last - 1, last - 1, T(*(last - 1)), comp);
}

template <class RandomAccessIterator, class Compare>
void make_heap(RandomAccessIterator first, RandomAccessIterator last, Compare comp)
{
    __make_heap_internal(first, last, comp, THOR_GET_VALUE_TYPE(first, RandomAccessIterator));
}

template <class RandomAccessIterator, class Compare>
void push_heap(RandomAccessIterator first, RandomAccessIterator last, Compare comp) 
{
    __push_heap_type(first, last, comp, THOR_GET_VALUE_TYPE(first, RandomAccessIterator));
}

template <class RandomAccessIterator, class Compare>
void pop_heap(RandomAccessIterator first, RandomAccessIterator last, Compare comp)
{
    __pop_heap_type(first, last, comp, THOR_GET_VALUE_TYPE(first, RandomAccessIterator));
}

template <class RandomAccessIterator, class Compare>
bool is_heap(RandomAccessIterator first, RandomAccessIterator last, Compare comp)
{
    THOR_DEBUG_ASSERT(last - first >= 0);
    return __is_heap_internal(first, comp, last - first);
}

template <class RandomAccessIterator, class Compare>
void sort_heap(RandomAccessIterator first, RandomAccessIterator last, Compare comp)
{
    while (last - first > 1)
    {
        pop_heap(first, last--, comp);
    }
}

} // namespace thor

#endif

