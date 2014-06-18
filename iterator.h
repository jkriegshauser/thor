/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * iterator.h
 *
 * This file defines functionality required by iterators
 */

#ifndef THOR_ITERATOR_H
#define THOR_ITERATOR_H
#pragma once

#ifndef THOR_BASETYPES_H
#include "basetypes.h"
#endif

namespace thor
{

template <class T> struct nonconst_traits
{
    typedef T  value_type;
    typedef T* pointer;
    typedef T& reference;
    typedef nonconst_traits<T> non_const_traits;
};

template <class T> struct const_traits
{
    typedef T  value_type;
    typedef const T* pointer;
    typedef const T& reference;
    typedef nonconst_traits<T> non_const_traits;
};

struct false_type {};
struct true_type {};

#ifdef __GNUC__
// GCC 3.2.3 really doesn't like the crazy template discrimination that happens below

#define THOR_GET_CATEGORY(f,t) typename iterator_traits<t>::iterator_category()
#define THOR_GET_VALUE_TYPE(f,t) ((typename iterator_traits<t>::value_type*)0)

template <class T> struct is_pod_type
{
    typedef false_type Type;
};

#else
// Is Pointer Type discrimination
template <int Is> struct bool2type
{
    typedef false_type Ret;
};

template <> struct bool2type<1>
{
    typedef true_type Ret;
};

template <> struct bool2type<0>
{
    typedef false_type Ret;
};

// Note that these functions are only used at compile time for type/size discrimination and therefore need no implementation
struct PointerShim { PointerShim(const volatile void*); };

char IsP(bool, PointerShim);
char* IsP(bool, ...);

template <class T> struct IsPtr
{
    static T& nullrep();
    enum { Ret = (sizeof(IsP(false,nullrep())) == sizeof(char)) };
};

template <class T>
struct IsPtrType
{
    enum { Is = IsPtr<T>::Ret };
    typedef bool2type<Is> BoolType;
    typedef typename BoolType::Ret Type;
    static Type Ret() { return Type(); }
};

template <class T> struct is_pod_type
{
    typedef typename IsPtrType<T>::Type Type;
};

#define THOR_GET_CATEGORY(f,t) get_category(f, IsPtrType<t>().Ret())
#define THOR_GET_VALUE_TYPE(f,t) get_value_type(f, IsPtrType<t>().Ret())

#endif

// Specializations for plain-old-data types
#define DEFINE_POD_TYPE(T) template <> struct is_pod_type<T> { typedef true_type Type; }
DEFINE_POD_TYPE(bool);
DEFINE_POD_TYPE(char);
DEFINE_POD_TYPE(unsigned char);
DEFINE_POD_TYPE(short);
DEFINE_POD_TYPE(unsigned short);
DEFINE_POD_TYPE(int);
DEFINE_POD_TYPE(unsigned int);
DEFINE_POD_TYPE(long);
DEFINE_POD_TYPE(unsigned long);
DEFINE_POD_TYPE(long long);
DEFINE_POD_TYPE(unsigned long long);
DEFINE_POD_TYPE(float);
DEFINE_POD_TYPE(double);
DEFINE_POD_TYPE(long double);
#ifdef _NATIVE_WCHAR_T_DEFINED
DEFINE_POD_TYPE(wchar_t);
#endif

#undef DEFINE_POD_TYPE

// Iterator types
struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag : public input_iterator_tag {};
struct bidirectional_iterator_tag : public forward_iterator_tag {};
struct random_access_iterator_tag : public bidirectional_iterator_tag {};

template <class Category, class T, class Pointer = T*, class Reference = T&>
struct iterator_type
{
    typedef Category        iterator_category;
    typedef T               value_type;
    typedef Pointer         pointer;
    typedef Reference       reference;
    typedef thor_size_type size_type;
    typedef thor_diff_type difference_type;
};

template <class Iterator>
struct iterator_traits
{
    typedef typename Iterator::iterator_category    iterator_category;
    typedef typename Iterator::value_type           value_type;
    typedef typename Iterator::pointer              pointer;
    typedef typename Iterator::reference            reference;
};

template <class T>
struct iterator_traits<const T*>
{
    typedef random_access_iterator_tag iterator_category;
    typedef T                          value_type;
    typedef const T*                   pointer;
    typedef const T&                   reference;
};

template <class T>
struct iterator_traits<T*>
{
    typedef random_access_iterator_tag iterator_category;
    typedef T                          value_type;
    typedef const T*                   pointer;
    typedef const T&                   reference;
};

template <> struct iterator_type<output_iterator_tag, void, void, void>
{
    typedef output_iterator_tag iterator_category;
    typedef void                value_type;
    typedef void                pointer;
    typedef void                reference;
};

#ifndef __GNUC__
template <class InputIterator> random_access_iterator_tag get_category(const InputIterator&, const true_type&)
{
    return random_access_iterator_tag();
}

template <class InputIterator> typename InputIterator::iterator_category get_category(const InputIterator&, const false_type&)
{
    return InputIterator::iterator_category();
}

template <class T> T* get_value_type(const T*, const true_type&)
{
    return static_cast<T*>(0);
}

template <class InputIterator> typename InputIterator::value_type* get_value_type(const InputIterator&, const false_type&)
{
    return static_cast<typename InputIterator::value_type*>(0);
}
#endif

// Distance implementation for pointers and random access iterators: Constant time
template <class InputIterator> difference_type __distance(const InputIterator& first, const InputIterator& last, const random_access_iterator_tag&)
{
    return (last - first);
}

// Distance implementation for all other input iterators: Linear time
template <class InputIterator, class T> difference_type __distance(const InputIterator& first, const InputIterator& last, const T&)
{
    difference_type d = 0;
    InputIterator iter(first);
    while (iter != last)
    {
        ++iter, ++d;
    }
    return d;
}

// Discriminating distance() implementation: Will be constant time for random access iterators and pointers and linear time for all other iterators
template <class InputIterator> difference_type distance(const InputIterator& first, const InputIterator& last)
{
    return __distance(first, last, THOR_GET_CATEGORY(first,InputIterator));
}

template <class RandomAccessIterator, class Distance> void __advance(RandomAccessIterator& iter, Distance n, const random_access_iterator_tag&)
{
    iter += n;
}

template <class InputIterator, class Distance, class T> void __advance(InputIterator& iter, Distance n, const T&)
{
    if (n > 0)
    {
        while (n-- > 0)
        {
            ++iter;
        }
    }
    else
    {
        while (n++ < 0)
        {
            --iter;
        }
    }
}

template <class InputIterator, class Distance> void advance(InputIterator& iter, Distance n)
{
    __advance(iter, n, THOR_GET_CATEGORY(iter,InputIterator));
}

template <class InputIterator, class T> InputIterator __find(InputIterator first, InputIterator last, const T& val, const random_access_iterator_tag&)
{
    difference_type check_count = (last - first) / 4;
    for (; check_count > 0; --check_count)
    {
        if (*first == val) return first;
        ++first;

        if (*first == val) return first;
        ++first;

        if (*first == val) return first;
        ++first;

        if (*first == val) return first;
        ++first;
    }

    switch (last - first)
    {
    case 3:
        if (*first == val) return first;
        ++first;
    case 2:
        if (*first == val) return first;
        ++first;
    case 1:
        if (*first == val) return first;
        ++first;
    case 0:
    default:
        return first;
    }
}

template <class InputIterator, class T> InputIterator __find(InputIterator first, InputIterator last, const T& val, const input_iterator_tag&)
{
    while (first != last && !(*first == val))
    {
        ++first;
    }
    return first;
}

template <class InputIterator, class T> InputIterator find(InputIterator first, InputIterator last, const T& val)
{
    return __find(first, last, val, THOR_GET_CATEGORY(first,InputIterator));
}

template <class InputIterator, class Predicate> InputIterator __find_if(InputIterator first, InputIterator last, Predicate pred, const random_access_iterator_tag&)
{
    difference_type check_count = (last - first) / 4;
    for (; check_count > 0; --check_count)
    {
        if (pred(*first)) return first;
        ++first;

        if (pred(*first)) return first;
        ++first;

        if (pred(*first)) return first;
        ++first;

        if (pred(*first)) return first;
        ++first;
    }

    switch (last - first)
    {
    case 3:
        if (pred(*first)) return first;
        ++first;
    case 2:
        if (pred(*first)) return first;
        ++first;
    case 1:
        if (pred(*first)) return first;
        ++first;
    case 0:
    default:
        return first;
    }
}

template <class InputIterator, class Predicate> InputIterator __find_if(InputIterator first, InputIterator last, Predicate pred, const input_iterator_tag&)
{
    while (first != last && !pred(*first))
    {
        ++first;
    }
    return first;
}

template <class InputIterator, class Predicate> InputIterator find_if(InputIterator first, InputIterator last, Predicate pred)
{
    return __find_if(first, last, pred, THOR_GET_CATEGORY(first, InputIterator));
}

}; // namespace thor

#ifndef THOR_TYPETRAITS_H
#include "typetraits.h"
#endif

#endif

