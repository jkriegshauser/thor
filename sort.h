/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * sort.h
 *
 * ** THOR INTERNAL FILE - NOT FOR APPLICATION USE **
 *
 * This file contains STL-compatible sort algorithms and commonly used predicates.
 */

#ifndef THOR_SORT_H
#define THOR_SORT_H
#pragma once

#ifndef THOR_FUNCTION_H
#include "function.h"
#endif

#ifndef THOR_TEMPORARY_BUFFER_H
#include "temporary_buffer.h"
#endif

namespace thor
{

// sort() implementation (using heap sort). 

template <class T> less<T> __less_factory(T*)
{
    return less<T>();
}

template<class T, class RandomAccessIterator, class Compare>
void __sort_inner(RandomAccessIterator first, thor_diff_type i, thor_diff_type count, Compare comp, T*)
{
    thor_diff_type k = i * 2 + 1;

    T save = *(first + i);

    while (k < count)
    {
        if (k + 1 < count && comp(*(first + k), *(first + k + 1)))
        {
            ++k;
        }

        if (!comp(save, *(first + k)))
        {
            break;
        }

        *(first + i) = *(first + k);
        i = k;
        k = i * 2 + 1;
    }

    *(first + i) = save;
}

template<class T, class RandomAccessIterator, class Compare>
void __sort_internal(RandomAccessIterator first, RandomAccessIterator last, Compare comp, T*)
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // well optimized heap sort - special thanks to eternallyconfuzzled.com (Julienne Walker)
    //////////////////////////////////////////////////////////////////////////////////////////

    thor_diff_type sortCount = last - first;
    THOR_DEBUG_ASSERT(sortCount >= 0);

    thor_diff_type i = sortCount / 2;
    while (i-- > 0)
    {
        __sort_inner(first, i, sortCount, comp, (T*)0);
    }

    while (--sortCount > 0)
    {
        thor::swap(*first, *(first + sortCount));
        __sort_inner(first, 0, sortCount, comp, (T*)0);
    }    
}

template <class RandomAccessIterator>
void sort(RandomAccessIterator first, RandomAccessIterator last)
{
    if (first != last)
    {
        __sort_internal(first, last, __less_factory(THOR_GET_VALUE_TYPE(first, RandomAccessIterator)), THOR_GET_VALUE_TYPE(first, RandomAccessIterator));
    }
}

template <class RandomAccessIterator, class Compare>
void sort(RandomAccessIterator first, RandomAccessIterator last, Compare comp)
{
    if (first != last)
    {
        __sort_internal(first, last, comp, THOR_GET_VALUE_TYPE(first, RandomAccessIterator));
    }
}

// stable_sort() implementation (using merge sort). 
template <class RandomAccessIterator, class T, class Compare>
void __unguarded_linear_insert(RandomAccessIterator last, T val, Compare comp)
{
    RandomAccessIterator next = last;
    --next;
    while (comp(val, *next))
    {
        *last = *next;
        last = next;
        --next;
    }
    *last = val;
}

template <class RandomAccessIterator, class T, class Compare>
inline void __linear_insert(RandomAccessIterator first, RandomAccessIterator last, T val, Compare comp)
{
    if (comp(val, *first))
    {
        thor::copy_backward(first, last, last + 1);
        *first = val;
    }
    else
    {
        __unguarded_linear_insert(last, val, comp);
    }
}

template <class RandomAccessIterator, class Compare>
void __insertion_sort(RandomAccessIterator first, RandomAccessIterator last, Compare comp)
{
    if (first == last)
    {
        return;
    }
    for (RandomAccessIterator iter = first + 1; iter != last; ++iter)
    {
        __linear_insert(first, iter, *iter, comp);
    }
}

template <class RandomAccessIterator1, class RandomAccessIterator2, class Compare>
void __merge_sort_loop(RandomAccessIterator1 first, RandomAccessIterator1 last, RandomAccessIterator2 result, difference_type step_size, Compare comp)
{
    difference_type two_step = 2 * step_size;

    while (last - first >= two_step)
    {
        result = thor::merge(first, first + step_size, first + step_size, first + two_step, result, comp);
        first += two_step;
    }
    
    difference_type diff = last - first;
    if (diff < step_size)
    {
        step_size = diff;
    }

    thor::merge(first, first + step_size, first + step_size, last, result, comp);
}

const int __default_chunk_size = 7;

template <class RandomAccessIterator, class Compare>
void __chunk_insertion_sort(RandomAccessIterator first, RandomAccessIterator last, difference_type chunk_size, Compare comp)
{
    while (last - first >= chunk_size)
    {
        __insertion_sort(first, first + chunk_size, comp);
        first += chunk_size;
    }
    __insertion_sort(first, last, comp);
}

template <class RandomAccessIterator, class T, class Compare>
void __merge_sort_buffer(RandomAccessIterator first, RandomAccessIterator last, T* buffer, Compare comp)
{
    difference_type len = last - first;
    T* buffer_last = buffer + len;

    difference_type step_size = __default_chunk_size;
    __chunk_insertion_sort(first, last, step_size, comp);

    while (step_size < len)
    {
        __merge_sort_loop(first, last, buffer, step_size, comp);
        step_size *= 2;
        __merge_sort_loop(buffer, buffer_last, first, step_size, comp);
        step_size *= 2;
    }
}

template <class BidirectionalIterator1, class BidirectionalIterator2>
BidirectionalIterator1 __rotate_adaptive(BidirectionalIterator1 first,
                                         BidirectionalIterator1 middle,
                                         BidirectionalIterator1 last,
                                         difference_type len1, difference_type len2,
                                         BidirectionalIterator2 buffer,
                                         difference_type buffer_size)
{
    if (len1 > len2 && len2 <= buffer_size)
    {
        BidirectionalIterator2 buffer_end = thor::copy(middle, last, buffer);
        thor::copy_backward(first, middle, last);
        return thor::copy(buffer, buffer_end, first);
    }
    else if (len1 <= buffer_size)
    {
        BidirectionalIterator2 buffer_end = thor::copy(first, middle, buffer);
        thor::copy(middle, last, first);
        return thor::copy_backward(buffer, buffer_end, last);
    }
    else
    {
        return thor::rotate(first, middle, last);
    }
}

template <class BidirectionalIterator1, class BidirectionalIterator2, class BidirectionalIterator3, class Compare>
BidirectionalIterator3 __merge_backward(BidirectionalIterator1 first1, BidirectionalIterator1 last1,
                                        BidirectionalIterator2 first2, BidirectionalIterator2 last2,
                                        BidirectionalIterator3 result, Compare comp)
{
    if (first1 == last1)
    {
        return thor::copy_backward(first2, last2, result);
    }
    if (first2 == last2)
    {
        return thor::copy_backward(first1, last1, result);
    }
    --last1;
    --last2;
    for (;;)
    {
        if (comp(*last2, *last1))
        {
            *--result = *last1;
            if (first1 == last1)
            {
                return thor::copy_backward(first2, ++last2, result);
            }
            --last1;
        }
        else
        {
            *--result = *last2;
            if (first2 == last2)
            {
                return thor::copy_backward(first1, ++last1, result);
            }
            --last2;
        }
    }
}

template <class BidirectionalIterator, class T, class Compare>
void __merge_adaptive(BidirectionalIterator first, 
                      BidirectionalIterator middle, 
                      BidirectionalIterator last,
                      difference_type len1, difference_type len2,
                      T* buffer, difference_type buffer_size,
                      Compare comp)
{
    if (len1 <= len2 && len1 <= buffer_size)
    {
        T* buffer_end = thor::copy(first, middle, buffer);
        thor::merge(buffer, buffer_end, middle, last, first, comp);
    }
    else if (len2 <= buffer_size)
    {
        T* buffer_end = thor::copy(middle, last, buffer);
        __merge_backward(first, middle, buffer, buffer_end, last, comp);
    }
    else
    {
        BidirectionalIterator first_cut = first;
        BidirectionalIterator second_cut = middle;
        difference_type len11 = 0;
        difference_type len22 = 0;
        if (len1 > len2)
        {
            len11 = len1 / 2;
            thor::advance(first_cut, len11);
            second_cut = thor::lower_bound(middle, last, *first_cut, comp);
            len22 += thor::distance(middle, second_cut);   
        }
        else
        {
            len22 = len2 / 2;
            thor::advance(second_cut, len22);
            first_cut = thor::upper_bound(first, middle, *second_cut, comp);
            len11 += thor::distance(first, first_cut);
        }
        BidirectionalIterator new_middle =
            __rotate_adaptive(first_cut, middle, second_cut, len1 - len11,
                              len22, buffer, buffer_size);
        
        __merge_adaptive(first, first_cut, new_middle, len11,
                         len22, buffer, buffer_size, comp);
        __merge_adaptive(new_middle, second_cut, last, len1 - len11,
                         len2 - len22, buffer, buffer_size, comp);
    }
}

template<class T, class RandomAccessIterator, class Compare>
void __stable_sort_adaptive(RandomAccessIterator first, RandomAccessIterator last, T* p, difference_type s, Compare comp)
{
    difference_type len = (last - first + 1) / 2;
    RandomAccessIterator middle = first + len;
    __merge_sort_buffer(first, middle, p, comp);
    __merge_sort_buffer(middle, last, p, comp);
    __merge_adaptive(first, middle, last, middle - first, last - middle, p, s, comp);
}

template<class T, class RandomAccessIterator, class Compare>
void __stable_sort_internal(RandomAccessIterator first, RandomAccessIterator last, Compare comp, T*)
{
    __TemporaryBuffer<RandomAccessIterator, T> buf(first, last);

    __stable_sort_adaptive(first, last, buf.begin(), buf.size(), comp);
}

template <class RandomAccessIterator>
void stable_sort(RandomAccessIterator first, RandomAccessIterator last)
{
    if (first != last)
    {
        __stable_sort_internal(first, last, __less_factory(THOR_GET_VALUE_TYPE(first, RandomAccessIterator)), THOR_GET_VALUE_TYPE(first, RandomAccessIterator));
    }
}

template <class RandomAccessIterator, class Compare>
void stable_sort(RandomAccessIterator first, RandomAccessIterator last, Compare comp)
{
    if (first != last)
    {
        __stable_sort_internal(first, last, comp, THOR_GET_VALUE_TYPE(first, RandomAccessIterator));
    }
}

}; // namespace thor

#endif

