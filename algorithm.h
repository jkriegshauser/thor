/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * algorithm.h
 *
 * This file defines STL-compatible algorithms
 */

#ifndef THOR_ALGORITHM_H
#define THOR_ALGORITHM_H
#pragma once

#include <stdlib.h>

#ifndef THOR_SORT_H
#include "sort.h"
#endif

#ifndef THOR_SWAP_H
#include "swap.h"
#endif

#ifndef THOR_HEAP_H
#include "heap.h"
#endif

#ifndef THOR_PAIR_H
#include "pair.h"
#endif

namespace thor
{

template <class InputIterator1, class InputIterator2>
bool equal(InputIterator1 f1, InputIterator1 l1, InputIterator2 f2)
{
    for (; f1 != l1; ++f1, ++f2)
    {
        if (!(*f1 == *f2))
        {
            return false;
        }
    }
    return true;
}

template <class InputIterator1, class InputIterator2, class BinaryPredicate>
bool equal(InputIterator1 f1, InputIterator1 l1, InputIterator2 f2, BinaryPredicate pred)
{
    for (; f1 != l1; ++f1, ++f2)
    {
        if (!pred(*f1,*f2))
        {
            return false;
        }
    }
    return true;
}


template <class InputIterator1, class InputIterator2>
bool lexicographical_compare(InputIterator1 f1, InputIterator1 l1,
                             InputIterator2 f2, InputIterator2 l2)
{
    for (; f1 != l1 && f2 != l2; ++f1, ++f2)
    {
        if (*f1 < *f2)
        {
            return true;
        }
        if (*f2 < *f1)
        {
            return false;
        }
    }
    return f1 == l1 && f2 != l2;
}

template <class InputIterator1, class InputIterator2, class Compare>
bool lexicographical_compare(InputIterator1 f1, InputIterator1 l1,
                             InputIterator2 f2, InputIterator2 l2,
                             Compare comp)
{
    for (; f1 != l1 && f2 != l2; ++f1, ++f2)
    {
        if (comp(*f1, *f2))
        {
            return true;
        }
        if (comp(*f2, *f1))
        {
            return false;
        }
    }
    return f1 == l1 && f2 != l2;
}

template <class ForwardIterator> ForwardIterator unique(ForwardIterator first, ForwardIterator last)
{
    if (first == last)
    {
        return last;
    }

    ForwardIterator new_end = first++;

    while (first != last)
    {
        if (!(*new_end == *first))
        {
            *++new_end = *first;
        }
        ++first;
    }

    return ++new_end;
}

template <class ForwardIterator, class BinaryPredicate> ForwardIterator unique(ForwardIterator first, ForwardIterator last, BinaryPredicate pred)
{
    if (first == last)
    {
        return last;
    }

    ForwardIterator new_end = first++;

    while (first != last)
    {
        if (!pred(*new_end, *first))
        {
            *++new_end = *first;
        }
        ++first;
    }

    return ++new_end;
}

template <class InputIterator, class OutputIterator> OutputIterator copy(InputIterator first, InputIterator last, OutputIterator output)
{
    // TODO: Specialize for POD types
    while (first != last)
    {
        *output++ = *first++;
    }
    return output;
}

template <class RandomAccessIterator, class OutputIterator> OutputIterator copy_backward(RandomAccessIterator first, RandomAccessIterator last, OutputIterator output)
{
    // TODO: Specialize for POD types
    for (difference_type d = last - first; d > 0; --d)
    {
        *--output = *--last;
    }
    return output;
}

template <class InputIterator, class UnaryPredicate> UnaryPredicate for_each(InputIterator first, InputIterator last, UnaryPredicate pred)
{
    while (first != last)
    {
        pred(*first++);
    }
    return pred;
}

template <class InputIterator> void iter_swap(InputIterator first, InputIterator second)
{
    thor::swap(*first, *second);
}

template <class ForwardIterator1, class ForwardIterator2>
inline ForwardIterator2 swap_ranges(ForwardIterator1 first1, ForwardIterator1 last1, ForwardIterator2 first2)
{
    for ( ; first1 != last1; ++first1, ++first2)
    {
        iter_swap(first1, first2);
    }
    return first2;
}

template <class Distance> inline Distance __random(Distance n)
{
    return rand() % n;
}

template <class RandomAccessIterator> void random_shuffle(RandomAccessIterator first, RandomAccessIterator last)
{
    if (first == last)
    {
        return;
    }

    for (RandomAccessIterator iter = first + 1; iter != last; ++iter)
    {
        thor::iter_swap(iter, first + __random((iter - first) + 1));
    }
}

template <class RandomAccessIterator, class RandomNumberGenerator> void random_shuffle(RandomAccessIterator first, RandomAccessIterator last, RandomNumberGenerator prng)
{
    if (first == last)
    {
        return;
    }

    for (RandomAccessIterator iter = first + 1; iter != last; ++iter)
    {
        thor::iter_swap(iter, first + prng((iter - first) + 1));
    }
}

template <class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator merge(InputIterator1 first1, InputIterator1 last1,
                     InputIterator2 first2, InputIterator2 last2,
                     OutputIterator result)
{
    while (first1 != last1 && first2 != last2)
    {
      if (*first2 < *first1)
      {
          *result = *first2;
          ++first2;
      }
      else
      {
          *result = *first1;
          ++first1;
      }
      ++result;
    }
    return copy(first2, last2, copy(first1, last1, result));
}

template <class InputIterator1, class InputIterator2, class OutputIterator, class Compare>
OutputIterator merge(InputIterator1 first1, InputIterator1 last1,
                     InputIterator2 first2, InputIterator2 last2,
                     OutputIterator result, Compare comp)
{
    while (first1 != last1 && first2 != last2)
    {
        if (comp(*first2, *first1))
        {
            *result = *first2;
            ++first2;
        }
        else
        {
            *result = *first1;
            ++first1;
        }
        ++result;
    }
    return thor::copy(first2, last2, thor::copy(first1, last1, result));
}

template <class ForwardIterator, class T> ForwardIterator lower_bound(ForwardIterator first, ForwardIterator last, const T& t)
{
    difference_type size = distance(first, last);
    THOR_ASSERT(size >= 0);
    difference_type half;
    ForwardIterator middle;

    while (size > 0)
    {
        half = size >> 1;
        middle = first;
        advance(middle, half);
        if (*middle < t)
        {
            first = middle;
            ++first;
            size = size - half - 1;
        }
        else
        {
            size = half;
        }
    }
    return first;
}

template <class ForwardIterator, class T, class LessThanComparable> ForwardIterator lower_bound(ForwardIterator first, ForwardIterator last, const T& t, LessThanComparable comp)
{
    difference_type size = thor::distance(first, last);
    THOR_ASSERT(size >= 0);
    difference_type half;
    ForwardIterator middle;

    while (size > 0)
    {
        half = size >> 1;
        middle = first;
        thor::advance(middle, half);
        if (comp(*middle, t))
        {
            first = middle;
            ++first;
            size = size - half - 1;
        }
        else
        {
            size = half;
        }
    }
    return first;
}

template <class ForwardIterator, class T> ForwardIterator upper_bound(ForwardIterator first, ForwardIterator last, const T& t)
{
    difference_type size = thor::distance(first, last);
    THOR_ASSERT(size >= 0);
    difference_type half;

    while (size > 0)
    {
        half = size >> 1;
        ForwardIterator middle = first;
        thor::advance(middle, half);
        if (t < *middle)
        {
            size = half;
        }
        else
        {
            first = middle;
            ++first;
            size = size - half - 1;
        }
    }
    return first;
}

template <class ForwardIterator, class T, class LessThanComparable> ForwardIterator upper_bound(ForwardIterator first, ForwardIterator last, const T& t, LessThanComparable comp)
{
    difference_type size = thor::distance(first, last);
    THOR_ASSERT(size >= 0);
    difference_type half;

    while (size > 0)
    {
        half = size >> 1;
        ForwardIterator middle = first;
        thor::advance(middle, half);
        if (comp(t, *middle))
        {
            size = half;
        }
        else
        {
            first = middle;
            ++first;
            size = size - half - 1;
        }
    }
    return first;
}

template <class ForwardIterator, class T> thor::pair<ForwardIterator, ForwardIterator> equal_range(ForwardIterator first, ForwardIterator last, const T& t)
{
    difference_type size = thor::distance(first, last);
    THOR_ASSERT(size >= 0);
    difference_type half;

    while (size > 0)
    {
        half = size >> 1;
        ForwardIterator middle = first;
        thor::advance(middle, half);
        if (*middle < t)
        {
            first = middle;
            ++first;
            size -= (half - 1);
        }
        else if (t < *middle)
        {
            size = half;
        }
        else
        {
            thor::pair<ForwardIterator, ForwardIterator> result;
            result.first = thor::lower_bound(first, middle, t);
            thor::advance(result.first, size);
            result.second = thor::upper_bound(++middle, first, t);
            return result;
        }
    }
    return thor::pair<ForwardIterator, ForwardIterator>(first, first);
}

template <class ForwardIterator, class T, class LessThanComparable> thor::pair<ForwardIterator, ForwardIterator> equal_range(ForwardIterator first, ForwardIterator last, const T& t, LessThanComparable comp)
{
    difference_type size = thor::distance(first, last);
    THOR_ASSERT(size >= 0);
    difference_type half;

    while (size > 0)
    {
        half = size >> 1;
        ForwardIterator middle = first;
        thor::advance(middle, half);
        if (comp(*middle, t))
        {
            first = middle;
            ++first;
            size -= (half - 1);
        }
        else if (comp(t, *middle))
        {
            size = half;
        }
        else
        {
            thor::pair<ForwardIterator, ForwardIterator> result;
            result.first = thor::lower_bound(first, middle, t, comp);
            thor::advance(result.first, size);
            result.second = thor::upper_bound(++middle, first, t, comp);
            return result;
        }
    }
    return thor::pair<ForwardIterator, ForwardIterator>(first, first);
}

template <class BidirectionalIterator> void reverse(BidirectionalIterator first, BidirectionalIterator last)
{
    if (first == last)
    {
        return;
    }

    while (first != --last)
    {
        thor::iter_swap(first, last);
        if (++first == last)
        {
            break;
        }
    }
}

template <class ForwardIterator, class OutputIterator, class T> OutputIterator remove_copy(ForwardIterator first, ForwardIterator last, OutputIterator result, const T& value)
{
    for (; first != last; ++first)
    {
        if (!(*first == value))
        {
            *result = *first;
            ++result;
        }
    }
    return result;
}

template <class ForwardIterator, class OutputIterator, class Predicate> OutputIterator remove_copy_if(ForwardIterator first, ForwardIterator last, OutputIterator result, Predicate pred)
{
    for (; first != last; ++first)
    {
        if (!pred(*first))
        {
            *result = *first;
            ++result;
        }
    }
    return result;
}

template <class ForwardIterator, class T> ForwardIterator remove(ForwardIterator first, ForwardIterator last, const T& value)
{
    first = thor::find(first, last, value);
    if (first == last)
    {
        return first;
    }

    ForwardIterator next = first;
    return thor::remove_copy(++next, last, first, value);
}

template <class ForwardIterator, class Predicate> ForwardIterator remove_if(ForwardIterator first, ForwardIterator last, Predicate pred)
{
    first = thor::find_if(first, last, pred);
    if (first == last)
    {
        return first;
    }

    ForwardIterator next = first;
    return thor::remove_copy_if(++next, last, first, pred);
}

template <class RingElement>
inline RingElement __gcd(RingElement m, RingElement n)
{
    while (n != 0)
    {
        RingElement t = m % n;
        m = n;
        n = t;
    }
    return m;
}

template <class ForwardIterator>
ForwardIterator __rotate(ForwardIterator first, ForwardIterator middle, ForwardIterator last, const forward_iterator_tag &)
{
    if (first == middle)
    {
        return last;
    }
    if (last == middle)
    {
        return first;
    }

    ForwardIterator first2 = middle;
    do
    {
        thor::swap(*first++, *first2++);
        if (first == middle)
        {
            middle = first2;
        }
    } while (first2 != last);

    ForwardIterator new_middle = first;

    first2 = middle;

    while (first2 != last)
    {
        thor::swap (*first++, *first2++);
        if (first == middle)
        {
            middle = first2;
        }
        else if (first2 == last)
        {
            first2 = middle;
        }
    }

    return new_middle;
}

template <class BidirectionalIterator>
BidirectionalIterator __rotate(BidirectionalIterator first, BidirectionalIterator middle, BidirectionalIterator last, const bidirectional_iterator_tag &)
{
    if (first == middle)
    {
        return last;
    }
    if (last  == middle)
    {
        return first;
    }

    thor::reverse(first,  middle);
    thor::reverse(middle, last);

    while (first != middle && middle != last)
    {
        thor::swap (*first++, *--last);
    }

    if (first == middle)
    {
        thor::reverse(middle, last);
        return last;
    }
    else
    {
        thor::reverse(first,  middle);
        return first;
    }
}

template <class RandomAccessIterator, class T> RandomAccessIterator __rotate(RandomAccessIterator first, RandomAccessIterator middle, RandomAccessIterator last, T*)
{
    difference_type n = last   - first;
    difference_type k = middle - first;
    difference_type l = n - k;
    RandomAccessIterator result = first + (last - middle);

    if (k == 0)
    {
        return last;
    }

    if (k == l)
    {
        thor::swap_ranges(first, middle, middle);
        return result;
    }

    difference_type d = __gcd(n, k);

    for (difference_type i = 0; i < d; ++i)
    {
        T tmp = *first;
        RandomAccessIterator p = first;

        if (k < l)
        {
            for (difference_type j = 0; j < l/d; ++j)
            {
                if (p > first + l)
                {
                    *p = *(p - l);
                    p -= l;
                }

                *p = *(p + k);
                p += k;
            }
        }

        else
        {
            for (difference_type j = 0; j < k/d - 1; ++j)
            {
                if (p < last - k)
                {
                    *p = *(p + k);
                    p += k;
                }

                *p = * (p - l);
                p -= l;
            }
        }

        *p = tmp;
        ++first;
    }

    return result;
}

template <class RandomAccessIterator> RandomAccessIterator __rotate(RandomAccessIterator first, RandomAccessIterator middle, RandomAccessIterator last, const random_access_iterator_tag&)
{
    return __rotate(first, middle, last, THOR_GET_VALUE_TYPE(first, RandomAccessIterator));
}

template <class ForwardIterator> ForwardIterator rotate(ForwardIterator first, ForwardIterator middle, ForwardIterator last)
{
    return __rotate(first, middle, last, THOR_GET_CATEGORY(first, ForwardIterator));
}

// Windows.h #defines min and max, so only include them if not already defined.
#ifndef max
template <class T> const T& max(const T& a, const T& b)
{
    return a < b ? b : a;
}

template <class T, class BinaryPredicate> const T& max(const T& a, const T& b, BinaryPredicate pred)
{
    return pred(a, b) ? b : a;
}
#endif

template <class T> const T& _max(const T& a, const T& b)
{
    return a < b ? b : a;
}

template <class T, class BinaryPredicate> const T& _max(const T& a, const T& b, BinaryPredicate pred)
{
    return pred(a, b) ? b : a;
}

#ifndef min
template <class T> const T& min(const T& a, const T& b)
{
    return b < a ? b : a;
}

template <class T, class BinaryPredicate> const T& min(const T& a, const T& b, BinaryPredicate pred)
{
    return pred(b, a) ? b : a;
}
#endif

template <class T> const T& _min(const T& a, const T& b)
{
    return b < a ? b : a;
}

template <class T, class BinaryPredicate> const T& _min(const T& a, const T& b, BinaryPredicate pred)
{
    return pred(b, a) ? b : a;
}

template <class ForwardIterator> inline ForwardIterator __internal_max_element(ForwardIterator first, ForwardIterator last)
{	// find largest element, using operator<
    ForwardIterator found = first;
    if (first != last)
    {
        for (; first != last; ++first)
        {
            if (*found < *first)
            {
                found = first;
            }
        }
    }
    return found;
}

template <class ForwardIterator> inline ForwardIterator max_element(ForwardIterator first, ForwardIterator last)
{
    first = __internal_max_element(first, last);
    return first;
}

template <class ForwardIterator> inline ForwardIterator __internal_min_element(ForwardIterator first, ForwardIterator last)
{	// find smallest element, using operator<
    ForwardIterator found = first;
    if (first != last)
    {
        for (; first != last; ++first)
        {
            if (first < *found)
            {
                found = first;
            }
        }
    }
    return found;
}

template <class ForwardIterator> inline ForwardIterator min_element( ForwardIterator first, ForwardIterator last )
{
    first = __internal_min_element( first, last );
    return first;
}

template <class InputIterator, class T> inline T __internal_accumulate( InputIterator first, InputIterator last, T val)
{	// return sum of val and all in [first, last)
    for (; first != last; ++first)
    {
        val += *first;
    }
    return val;
}

template <class InputIterator, class T> inline T accumulate( InputIterator first, InputIterator last, T val)
{
    return __internal_accumulate( first, last, val);
}

} // namespace thor

#endif
