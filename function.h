/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * function.h
 *
 * This file defines several STL-compatible functors
 */

#ifndef THOR_FUNCTION_H
#define THOR_FUNCTION_H
#pragma once

namespace thor
{

template <class Arg, class Result> class unary_function
{
public:
    typedef Arg     argument_type;
    typedef Result  result_type;
};

template <class Arg1, class Arg2, class Result> class binary_function
{
public:
    typedef Arg1    first_argument_type;
    typedef Arg2    second_argument_type;
    typedef Result  result_type;
};

// Modified pointer_to_unary_function allows the Function parameter to be explicitly specified. This allows for
// different calling conventions (__stdcall, etc)
template <class Arg, class Result, class Function = Result(*)(Arg)> class pointer_to_unary_function : public unary_function<Arg, Result>
{
public:
    explicit pointer_to_unary_function(Function func) : func_(func) {}

    Result operator () (Arg arg1) const
    {
        return func_(arg1);
    }
protected:
    Function func_;
};

// Modified pointer_to_unary_function allows the Function parameter to be explicitly specified. This allows for
// different calling conventions (__stdcall, etc)
template <class Arg1, class Arg2, class Result, class Function = Result(*)(Arg1,Arg2)> class pointer_to_binary_function : public binary_function<Arg1, Arg2, Result>
{
public:
    explicit pointer_to_binary_function(Function func) : func_(func) {}

    Result operator () (Arg1 arg1, Arg2 arg2) const
    {
        return func_(arg1, arg2);
    }
protected:
    Function func_;
};

template <class Arg, class Result> pointer_to_unary_function<Arg, Result> ptr_fun(Result (*f)(Arg))
{
    return pointer_to_unary_function<Arg, Result>(f);
}

template <class Arg1, class Arg2, class Result> pointer_to_binary_function<Arg1, Arg2, Result> ptr_fun(Result (*f)(Arg1, Arg2))
{
    return pointer_to_binary_function<Arg1, Arg2, Result>(f);
}

template <class Pair> class select1st : public unary_function<Pair, typename Pair::first_type>
{
public:
    typedef unary_function<Pair, typename Pair::first_type> base_type;
    typedef typename base_type::argument_type argument_type;
    typedef typename base_type::result_type   result_type;

    const result_type& operator () (const argument_type& p) const
    {
        return p.first;
    }
};

template <class T> class identity : public unary_function<T, T>
{
public:
    const T& operator () (const T& t) const
    {
        return t;
    }
};

template <class T> struct less
{
    bool operator () (const T& lhs, const T& rhs) const
    {
        return lhs < rhs;
    }
};

template <class T> struct greater
{
    bool operator () (const T& lhs, const T& rhs) const
    {
        return lhs > rhs;
    }
};

template <class T> struct equal_to
{
    bool operator () (const T& lhs, const T& rhs) const
    {
        return lhs == rhs;
    }
};

}; // namespace thor

#endif

