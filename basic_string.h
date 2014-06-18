/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * basic_string.h
 *
 * This file defines an STL-compatible string container
 *
 * Extensions/Changes:
 * - reduce() reduces the underlying memory usage
 * - shrink_to_fit() from C++11 is implemented
 * - var-arg constructors to do printf-style construction
 * - format, append_format, insert_format, replace_format variations exist for printf-style operations
 * - a pre-allocated memory block can be specified as a template parameter
 * - works as a holder for literal/external strings
 */

#ifndef THOR_BASIC_STRING_H
#define THOR_BASIC_STRING_H
#pragma once

#ifndef THOR_ITERATOR_H
#include "iterator.h"
#endif

#ifndef THOR_MEMORY_H
#include "memory.h"
#endif

#ifndef THOR_ATOMIC_INTEGER_H
#include "atomic_integer.h"
#endif

#ifndef THOR_STRING_UTIL_H
#include "string_util.h"
#endif

#ifndef THOR_SWAP_H
#include "swap.h"
#endif

#ifndef THOR_ALGORITHM_H
#include "algorithm.h"
#endif

#ifndef THOR_HASH_FUNCS_H
#include "hash_funcs.h"
#endif

namespace thor
{

template <typename T, thor_size_type T_SIZE = 0> class basic_string;

// Specialization for the base vector that does no preallocation.
template <typename T> class basic_string<T, 0>
{
public:
    // STL-compatible typedefs
    typedef T               value_type;
    typedef T*              pointer;
    typedef T&              reference;
    typedef const T*        const_pointer;
    typedef const T&        const_reference;
    typedef thor_size_type size_type;
    typedef thor_diff_type difference_type;

    // npos
    static const size_type npos = size_type(-1);
    
    // embedded_size
    static const size_type embedded_size = 0;

	// Iterator base class
    struct iterator_base : public iterator_type<random_access_iterator_tag, T>
    {
        typedef THOR_TYPENAME basic_string<T>::pointer pointer;
        pointer element;
#ifdef THOR_DEBUG
        const basic_string* owner;
        iterator_base(pointer element_, const basic_string* owner_)
        : element(element_), owner(owner_)
        {}
#else
        iterator_base(pointer element_, const basic_string*)
        : element(element_)
        {}
#endif
        void verify_owner(const basic_string* owner_)
        {
            THOR_UNUSED(owner_);
            THOR_DEBUG_ASSERT(owner == owner_);
        }
        void verify_range(bool allowEnd = false) const
        {
	        THOR_UNUSED(allowEnd);
	        THOR_DEBUG_ASSERT(element >= owner->elements_ && element < (owner->elements_ + owner->size_ + allowEnd));
        }
        void decr() {                 --element; }  
        void incr() { verify_range(); ++element; }
        bool operator == (const iterator_base& i) const { THOR_DEBUG_ASSERT(owner == i.owner); return element == i.element; }
        bool operator != (const iterator_base& i) const { THOR_DEBUG_ASSERT(owner == i.owner); return element != i.element; }
        bool operator <  (const iterator_base& i) const { THOR_DEBUG_ASSERT(owner == i.owner); return element <  i.element; }
        bool operator >  (const iterator_base& i) const { THOR_DEBUG_ASSERT(owner == i.owner); return i.element <  element; }
        bool operator <= (const iterator_base& i) const { THOR_DEBUG_ASSERT(owner == i.owner); return !(i.element <  element); }
        bool operator >= (const iterator_base& i) const { THOR_DEBUG_ASSERT(owner == i.owner); return !(element <  i.element); }
    };

    // Forward iterator template
    template<typename Traits> class fwd_iterator : public iterator_base
    {
    public:
        typedef typename Traits::pointer pointer;
        typedef typename Traits::reference reference;
        typedef fwd_iterator<nonconst_traits<T> > nonconst_iterator;
        typedef fwd_iterator<Traits> selftype;

        fwd_iterator(pointer p = 0, const basic_string* o = 0) : iterator_base((typename iterator_base::pointer)p, o) {}
        fwd_iterator(const nonconst_iterator& i) : iterator_base(i) {}
        selftype&  operator = (const nonconst_iterator& i)  { iterator_base::operator = (i); return *this; }
        reference  operator * () const                      { verify_range(); return *element; }
        selftype   operator - (difference_type i) const     { selftype n(*this); n.element -= i; return n; }
        selftype&  operator -= (difference_type i)          {                    element -= i;   return *this; }
        selftype&  operator -- ()     /* --iterator */      {                    decr();         return *this; }
        selftype   operator -- (int)  /* iterator-- */      { selftype n(*this); decr();         return n; }
        selftype   operator + (difference_type i) const     { selftype n(*this); n.element += i; return n; }
        selftype&  operator += (difference_type i)          {                    element += i;   return *this; }
        selftype&  operator ++ ()     /* ++iterator */      {                    incr();         return *this; }
        selftype   operator ++ (int)  /* iterator++ */      { selftype n(*this); incr();         return n; }

        difference_type operator - (const selftype& t) const { THOR_DEBUG_ASSERT(owner == t.owner); return element - t.element; }
    };

    // Reverse iterator template
    template<typename Traits> class rev_iterator : public iterator_base
    {
    public:
        typedef typename Traits::pointer pointer;
        typedef typename Traits::reference reference;
        typedef rev_iterator<nonconst_traits<T> > nonconst_iterator;
        typedef rev_iterator<Traits> selftype;

        rev_iterator(pointer p = 0, const basic_string* o = 0) : iterator_base((typename iterator_base::pointer)p, o) {}
        rev_iterator(const nonconst_iterator& i) : iterator_base(i) {}
        selftype&  operator = (const nonconst_iterator& i)  { iterator_base::operator = (i); return *this; }
        reference  operator * () const                      { verify_range(); return *element; }
        selftype   operator - (difference_type i) const     { selftype n(*this); n.element += i; return n; }
        selftype&  operator -= (difference_type i)          {                    element += i;   return *this; }
        selftype&  operator -- ()     /* --iterator */      {                    incr();         return *this; }
        selftype   operator -- (int)  /* iterator-- */      { selftype n(*this); incr();         return n; }
        selftype   operator + (difference_type i) const     { selftype n(*this); n.element -= i; return n; }
        selftype&  operator += (difference_type i)          {                    element -= i;   return *this; }
        selftype&  operator ++ ()     /* ++iterator */      {                    decr();         return *this; }
        selftype   operator ++ (int)  /* iterator++ */      { selftype n(*this); decr();         return n; }

        difference_type operator - (const selftype& t) const { THOR_DEBUG_ASSERT(owner == t.owner); return t.element - element; }
    };

    // Iterator definitions
    typedef fwd_iterator<nonconst_traits<T> > iterator;
    typedef fwd_iterator<const_traits<T>    > const_iterator;

    typedef rev_iterator<nonconst_traits<T> > reverse_iterator;
    typedef rev_iterator<const_traits<T>    > const_reverse_iterator;

    enum Format { fmt };
    enum Literal
    {
        lit_allow_share,    // holds a literal string and will freely share with other thor::strings
        lit_copy_on_share   // holds a string owned externally; any attempts to share will result in a copy
    };

    // Constructors
    explicit basic_string();
    basic_string(const basic_string& rhs);
    basic_string(const basic_string& rhs, size_type pos, size_type len = npos);
    basic_string(const_pointer s);
    basic_string(const_pointer s, size_type len);
    basic_string(size_type fill, value_type c);
    template<class InputIterator> basic_string(InputIterator first, InputIterator last);

    // Formatting constructor extensions
    basic_string(Format, const_pointer s, ...);
    basic_string(const_pointer s, va_list va);

    // Literal string construtor extensions
    basic_string(Literal, const_pointer s);
    basic_string(Literal, const_pointer s, size_type len);

    // Destructor
    virtual ~basic_string();

    // Forward Iteration
    iterator        begin()        { return       iterator(elements_, this); }
    iterator        end()          { return       iterator(end_ptr(), this); }
    const_iterator  begin()  const { return const_iterator(elements_, this); }
    const_iterator  end()    const { return const_iterator(end_ptr(), this); }
    const_iterator  cbegin() const { return const_iterator(elements_, this); }  // C++11
    const_iterator  cend()   const { return const_iterator(end_ptr(), this); }  // C++11

    // Reverse iteration
    reverse_iterator       rbegin()        { return       reverse_iterator(end_ptr() - 1, this); }
    reverse_iterator       rend()          { return       reverse_iterator(elements_ - 1, this); }
    const_reverse_iterator rbegin()  const { return const_reverse_iterator(end_ptr() - 1, this); }
    const_reverse_iterator rend()    const { return const_reverse_iterator(elements_ - 1, this); }
    const_reverse_iterator crbegin() const { return const_reverse_iterator(end_ptr() - 1, this); }  // C++11
    const_reverse_iterator crend()   const { return const_reverse_iterator(elements_ - 1, this); }  // C++11

    // Capacity
    size_type size()         const { return size_; }
    size_type length()       const { return size_; }
    size_type max_capacity() const { return size_type(-1); }
    size_type capacity()     const { return capacity_; }
    bool      empty()        const { return size_ == 0; }

    // Memory management
    void resize(size_type n);
    void resize(size_type n, value_type c);
    void reserve(size_type n = 0);
    void clear();
    void reduce(size_type n = 0);
    void shrink_to_fit() { reduce(0); } // C++11

    // Element access
    reference       operator [] (size_type index);
    const_reference operator [] (size_type index) const;
    reference       at(size_type index);
    const_reference at(size_type index) const;
    reference       front();            // C++11
    const_reference front() const;      // C++11
    reference       back();             // C++11
    const_reference back() const;       // C++11

    // Operators
    basic_string& operator =  (const basic_string& str);
    basic_string& operator =  (const_pointer s);
    basic_string& operator =  (value_type c);
    basic_string& operator += (const basic_string& str);
    basic_string& operator += (const_pointer s);
    basic_string& operator += (value_type c);
    basic_string  operator +  (const basic_string& str) const;
    basic_string  operator +  (const_pointer s) const;
    basic_string  operator +  (value_type c) const;

    // Appending
    basic_string& append(const basic_string& str);
    basic_string& append(const basic_string& str, size_type pos, size_type len = npos);
    basic_string& append(const_pointer s);
    basic_string& append(const_pointer s, size_type len);
    basic_string& append(size_type len, value_type fill);
    template<class InputIterator> basic_string& append(InputIterator first, InputIterator last);
    value_type&   push_back(value_type c);
    value_type&   push_back();
    size_type     append_format(const_pointer s, ...);
    size_type     append_format_v(const_pointer s, va_list va);

    // Assigning
    basic_string& assign(const basic_string& str);
    basic_string& assign(const basic_string& str, size_type pos, size_type len = npos);
    basic_string& assign(const_pointer s);
    basic_string& assign(const_pointer s, size_type len);
    basic_string& assign(size_type len, value_type fill);
    template<class InputIterator> basic_string& assign(InputIterator first, InputIterator last);
    basic_string& assign(Literal lit, const_pointer s);
    basic_string& assign(Literal lit, const_pointer s, size_type len);
    size_type     format(const_pointer s, ...);
    size_type     format_v(const_pointer s, va_list va);

    // Insertion
    basic_string& insert(size_type pos, const basic_string& str);
    basic_string& insert(size_type pos, const basic_string& str, size_type subpos, size_type len = npos);
    basic_string& insert(size_type pos, const_pointer s);
    basic_string& insert(size_type pos, const_pointer s, size_type len);
    basic_string& insert(size_type pos, size_type len, value_type fill);
    void          insert(iterator  pos, size_type len, value_type fill);
    iterator      insert(iterator  pos, value_type c);
    template<class InputIterator> void insert(iterator pos, InputIterator first, InputIterator last);
    size_type     insert_format(size_type pos, const_pointer s, ...);
    size_type     insert_format(iterator  pos, const_pointer s, ...);
    size_type     insert_format_v(size_type pos, const_pointer s, va_list va);
    size_type     insert_format_v(iterator  pos, const_pointer s, va_list va);

    // Erasure
    basic_string& erase(size_type pos = 0, size_type len = npos);
    iterator      erase(iterator pos);
    iterator      erase(iterator first, iterator last);
    value_type    pop_back();

    // Replacement
    basic_string& replace(size_type pos, size_type len, const basic_string& str);
    basic_string& replace(iterator pos1, iterator pos2, const basic_string& str);
    basic_string& replace(size_type pos, size_type len, const basic_string& str, size_type subpos, size_type sublen = npos);
    basic_string& replace(size_type pos, size_type len, const_pointer s);
    basic_string& replace(iterator pos1, iterator pos2, const_pointer s);
    basic_string& replace(size_type pos, size_type len, const_pointer s, size_type n);
    basic_string& replace(iterator pos1, iterator pos2, const_pointer s, size_type n);
    basic_string& replace(size_type pos, size_type len, size_type fill_len, value_type fill);
    basic_string& replace(iterator pos1, iterator pos2, size_type fill_len, value_type fill);
    template<class InputIterator> basic_string& replace(iterator pos1, iterator pos2, InputIterator first, InputIterator last);
    size_type     replace_format(size_type pos, size_type len, const_pointer s, ...);
    size_type     replace_format(iterator pos1, iterator pos2, const_pointer s, ...);
    size_type     replace_format_v(size_type pos, size_type len, const_pointer s, va_list va);
    size_type     replace_format_v(iterator pos1, iterator pos2, const_pointer s, va_list va);

    // Swap
    void swap(basic_string& rhs);

    // String operations
    const_pointer c_str() const { return elements_; }
    const_pointer data()  const { return elements_; }
    size_type copy(pointer out, size_type n, size_type pos = 0) const;
    basic_string substr(size_type pos = 0, size_type len = npos) const;

    // Find
    size_type  find(const basic_string& str, size_type pos = 0) const;
    size_type  find(const_pointer s, size_type pos = 0) const;
    size_type  find(const_pointer s, size_type pos, size_type len) const;
    size_type  find(value_type c, size_type pos = 0) const;
    size_type rfind(const basic_string& str, size_type pos = npos) const;
    size_type rfind(const_pointer s, size_type pos = npos) const;
    size_type rfind(const_pointer s, size_type pos, size_type len) const;
    size_type rfind(value_type c, size_type pos = npos) const;
    size_type  find_i(const basic_string& str, size_type pos = 0) const;
    size_type  find_i(const_pointer s, size_type pos = 0) const;
    size_type  find_i(const_pointer s, size_type pos, size_type len) const;
    size_type  find_i(value_type c, size_type pos = 0) const;
    size_type rfind_i(const basic_string& str, size_type pos = npos) const;
    size_type rfind_i(const_pointer s, size_type pos = npos) const;
    size_type rfind_i(const_pointer s, size_type pos, size_type len) const;
    size_type rfind_i(value_type c, size_type pos = npos) const;
    size_type find_first_of(const basic_string& str, size_type pos = 0) const;
    size_type find_first_of(const_pointer s, size_type pos = 0) const;
    size_type find_first_of(const_pointer s, size_type pos, size_type len) const;
    size_type find_first_of(value_type c, size_type pos = 0) const;
    size_type find_last_of(const basic_string& str, size_type pos = npos) const;
    size_type find_last_of(const_pointer s, size_type pos = npos) const;
    size_type find_last_of(const_pointer s, size_type pos, size_type len) const;
    size_type find_last_of(value_type c, size_type pos = npos) const;
    size_type find_first_not_of(const basic_string& str, size_type pos = 0) const;
    size_type find_first_not_of(const_pointer s, size_type pos = 0) const;
    size_type find_first_not_of(const_pointer s, size_type pos, size_type len) const;
    size_type find_first_not_of(value_type c, size_type pos = 0) const;
    size_type find_last_not_of(const basic_string& str, size_type pos = npos) const;
    size_type find_last_not_of(const_pointer s, size_type pos = npos) const;
    size_type find_last_not_of(const_pointer s, size_type pos, size_type len) const;
    size_type find_last_not_of(value_type c, size_type pos = npos) const;

    // Comparison
    int compare(const basic_string& str) const;
    int compare(size_type pos, size_type len, const basic_string& str) const;
    int compare(size_type pos, size_type len, const basic_string& str, size_type subpos, size_type sublen) const;
    int compare(const_pointer s) const;
    int compare(size_type pos, size_type len, const_pointer s) const;
    int compare(size_type pos, size_type len, const_pointer s, size_type n) const;
    int compare_i(const basic_string& str) const;
    int compare_i(size_type pos, size_type len, const basic_string& str) const;
    int compare_i(size_type pos, size_type len, const basic_string& str, size_type subpos, size_type sublen) const;
    int compare_i(const_pointer s) const;
    int compare_i(size_type pos, size_type len, const_pointer s) const;
    int compare_i(size_type pos, size_type len, const_pointer s, size_type n) const;

protected:
    // Memory allocation must be aligned according to ref_counter's requirements
    typedef atomic_integer<int> ref_counter;
    enum { alignment = memory::align_selector<ref_counter>::alignment };
    typedef memory::align_alloc<thor_byte, alignment> align_alloc;

    static pointer empty_string()
    {
        static value_type NUL(0);
        return &NUL;
    }

    virtual thor_byte* alloc(size_type raw_needed, size_type& raw_avail, bool& shareable)
    {
        shareable = true;
        raw_avail = raw_needed;
        return align_alloc::alloc(raw_needed);
    }

    virtual void free(thor_byte* data)
    {
        align_alloc::free(data);
    }

    pointer end_ptr() const
    {
        return elements_ + size_;
    }

    thor_byte* ref_get_mem() const
    {
        THOR_DEBUG_ASSERT((capacity_ + 1) > 1); // assert if capacity_ is zero or npos
        return ((thor_byte*)elements_) - sizeof(ref_counter);
    }

    ref_counter& ref_get_counter() const
    {
        return *(ref_counter*)ref_get_mem();
    }

    int ref_get() const
    {
        if ((capacity_ + 1) <= 1) // zero ref count if capacity_ is zero or npos
        {
            return 0;
        }
        return ref_get_counter().get();
    }

    void ref_add() const
    {
        ++ref_get_counter();
    }

    void ref_release()
    {
        // Can't release if capacity_ is zero or npos
        if ((capacity_ + 1) > 1 && --ref_get_counter() == 0)
        {
            // Actually deleting the string
            ref_get_counter().~atomic_integer();
            THOR_DEBUG_INIT_MEM(ref_get_mem(), sizeof(ref_counter) + ((capacity_ + 1) * sizeof(value_type)), 0xdd);
            free(ref_get_mem());
        }
    }

    bool is_shareable() const
    {
        return ref_get() != 0;
    }

    enum make_writeable_options
    {
        exact = 1,
        copy_existing  = 2,
        allow_shrink = 4,

        exact_copy = exact|copy_existing,
        shrink_copy = allow_shrink|copy_existing,
        exact_shrink_copy = exact|copy_existing|allow_shrink
    };

    template<make_writeable_options options> void make_writeable(size_type needed)
    {
        // +1 to needed and capacity to ignore npos
        if ((needed + 1) > (capacity_ + 1) || ref_get() > 1)
        {
            make_writeable_internal<options>(needed);
        }
    }

    template<make_writeable_options options> void make_writeable_internal(size_type needed)
    {
        if (THOR_SUPPRESS_WARNING((options & shrink_copy) == copy_existing) && needed < size_)
        {
            needed = size_;
        }

        if (!THOR_SUPPRESS_WARNING(options & exact) && (capacity_ + 1) > 1)
        {
            // exponential growth
            needed = thor::_max(needed, capacity_ + (capacity_ >> 1));
            // round to multiple of 16 bytes
            needed = (needed + 15) & ~0xF;
        }

        // Allocate new space
        bool shareable;
        size_type raw_avail;
        size_type raw_needed = ((needed + 1) * sizeof(value_type)) + sizeof(ref_counter);
        thor_byte* data = alloc(raw_needed, raw_avail, shareable);
        THOR_DEBUG_INIT_MEM(data, raw_avail, 0xbb);
        new (data) ref_counter(shareable ? 1 : 0);
        pointer new_string = (pointer)(data + sizeof(ref_counter));
        size_type new_capacity = ((raw_avail - sizeof(ref_counter)) / sizeof(value_type)) - 1;
        size_type new_size = size_;

        // Copy old string to new
        if (THOR_SUPPRESS_WARNING(options & copy_existing))
        {
            if (THOR_SUPPRESS_WARNING(options & allow_shrink))
            {
                const size_type copy_len = thor::_min(needed, size_);
                typetraits<T>::copy(new_string, elements_, copy_len);
                new_string[copy_len] = T(0);
            }
            else
            {
                typetraits<T>::copy(new_string, elements_, size_ + 1);
            }
        }

        ref_release();

        elements_ = new_string;
        size_ = new_size;
        capacity_ = new_capacity;
    }

private:
    pointer   elements_;
    size_type size_;
    size_type capacity_;
};

typedef basic_string<char> string;
typedef basic_string<wchar_t> wstring;

// UTF-8 to/from wchar_t support:
string  wide_to_utf8(const wchar_t* s);
string  wide_to_utf8(const wstring& str);
bool    wide_to_utf8(const wchar_t* s, string& out);
bool    wide_to_utf8(const wstring& str, string& out);
wstring utf8_to_wide(const char* s);
wstring utf8_to_wide(const string& str);
bool    utf8_to_wide(const char* s, wstring& out);
bool    utf8_to_wide(const string& str, wstring& out);

bool    utf8_is_valid(const char* s);
bool    utf8_is_valid(const string& str);
size_type wide_length(const char* s);
size_type wide_length(const string& str);
size_type utf8_length(const wchar_t* s);
size_type utf8_length(const wstring& str);

///////////////////////////////////////////////////////////////////////////////
// Inline implementations
///////////////////////////////////////////////////////////////////////////////

// Constructors
template<typename T> basic_string<T>::basic_string()
    : elements_(empty_string())
    , size_(0)
    , capacity_(0)
{}

template<typename T> basic_string<T>::basic_string(const basic_string<T>& str)
    : elements_(empty_string())
    , size_(0)
    , capacity_(0)
{
    assign(str);
}

template<typename T> basic_string<T>::basic_string(const basic_string& str, size_type pos, size_type len = npos)
    : elements_(empty_string())
    , size_(0)
    , capacity_(0)
{
    assign(str, pos, len);
}

template<typename T> basic_string<T>::basic_string(const_pointer s)
    : elements_(empty_string())
    , size_(0)
    , capacity_(0)
{
    assign(s);
}

template<typename T> basic_string<T>::basic_string(const_pointer s, size_type len)
    : elements_(empty_string())
    , size_(0)
    , capacity_(0)
{
    assign(s, len);
}

template<typename T> basic_string<T>::basic_string(size_type len, value_type fill)
    : elements_(empty_string())
    , size_(0)
    , capacity_(0)
{
    assign(len, fill);
}

template<typename T> template<class InputIterator> basic_string<T>::basic_string(InputIterator first, InputIterator last)
    : elements_(empty_string())
    , size_(0)
    , capacity_(0)
{
    assign(first, last);
}

template<typename T> basic_string<T>::basic_string(Format, const_pointer s, ...)
    : elements_(empty_string())
    , size_(0)
    , capacity_(0)
{
    va_list va;
    va_start(va, s);
    format_v(s, va);
    va_end(va);
}

template<typename T> basic_string<T>::basic_string(const_pointer s, va_list va)
    : elements_(empty_string())
    , size_(0)
    , capacity_(0)
{
    format_v(s, va);
}

template<typename T> basic_string<T>::basic_string(Literal lit, const_pointer s)
    : elements_(const_cast<pointer>(s))
    , size_(string_length(s))
    , capacity_(lit == lit_allow_share ? npos : 0)
{
}

template<typename T> basic_string<T>::basic_string(Literal lit, const_pointer s, size_type len)
    : elements_(const_cast<pointer>(s))
    , size_(len)
    , capacity_(lit == lit_allow_share ? npos : 0)
{
    // This is a literal string; we can't insert a NUL character at len, so the length
    // must match.
    THOR_DEBUG_ASSERT(len == string_length(s));
}


template<typename T> basic_string<T>::~basic_string()
{
    ref_release();
}

template<typename T> void basic_string<T>::resize(size_type n)
{
    if (n != size_)
    {
        make_writeable<shrink_copy>(n);
        if (n < size_)
        {
            size_ = n;
            *end_ptr() = T(0);
        }
        else
        {
            typetraits<T>::range_copy(elements_ + size_, elements_ + n + 1, T(0));
            size_ = n;
        }
    }
}

template<typename T> void basic_string<T>::resize(size_type n, value_type c)
{
    if (n != size_)
    {
        make_writeable<shrink_copy>(n);
        if (n < size_)
        {
            size_ = n;
            *end_ptr() = T(0);
        }
        else
        {
            typetraits<T>::range_copy(elements_ + size_, elements_ + n, c);
            size_ = n;
        }
    }
}

template<typename T> void basic_string<T>::reserve(size_type n)
{
    // +1 to n and capacity_ to ignore npos
    if ((n + 1) > (capacity_ + 1))
    {
        make_writeable<exact_copy>(n);
    }
}

template<typename T> void basic_string<T>::clear()
{
    if (ref_get() == 1)
    {
        size_ = 0;
        *end_ptr() = T(0);
    }
    else
    {
        ref_release();
        elements_ = empty_string();
        size_ = 0;
        capacity_ = 0;
    }
}

template<typename T> void basic_string<T>::reduce(size_type n)
{
    // No point in doing this for a string that we don't own
    if (ref_get() != 1) return;

    if (n < size_) n = size_;

    if (n == 0)
    {
        ref_release();
        elements_ = empty_string();
        size_ = 0;
        capacity_ = 0;
    }
    else if (capacity_ > n)
    {
        make_writeable_internal<exact_shrink_copy>(n);
    }
}

template<typename T> typename basic_string<T>::reference basic_string<T>::operator [] (size_type index)
{
    THOR_DEBUG_ASSERT(index < size_); // Don't allow NUL
    make_writeable<copy_existing>(size_);
    return elements_[index];
}

template<typename T> typename basic_string<T>::const_reference basic_string<T>::operator [] (size_type index) const
{
    THOR_DEBUG_ASSERT(index <= size_);
    return elements_[index];
}

template<typename T> typename basic_string<T>::reference basic_string<T>::at(size_type index)
{
    THOR_DEBUG_ASSERT(index < size_); // Don't allow NUL
    make_writeable<copy_existing>(size_);
    return elements_[index];
}

template<typename T> typename basic_string<T>::const_reference basic_string<T>::at(size_type index) const
{
    THOR_DEBUG_ASSERT(index <= size_);
    return elements_[index];
}

template<typename T> typename basic_string<T>::reference basic_string<T>::front()
{
    THOR_DEBUG_ASSERT(!empty());
    make_writeable<copy_existing>(size_);
    return elements_[0];
}

template<typename T> typename basic_string<T>::const_reference basic_string<T>::front() const
{
    THOR_DEBUG_ASSERT(!empty());
    return elements_[0];
}

template<typename T> typename basic_string<T>::reference basic_string<T>::back()
{
    THOR_DEBUG_ASSERT(!empty());
    make_writeable<copy_existing>(size_);
    return elements_[size_ - 1];
}

template<typename T> typename basic_string<T>::const_reference basic_string<T>::back() const
{
    THOR_DEBUG_ASSERT(!empty());
    return elements_[size_ - 1];
}

template<typename T> basic_string<T>& basic_string<T>::operator =  (const basic_string& str)
{
    return assign(str);
}

template<typename T> basic_string<T>& basic_string<T>::operator =  (const_pointer s)
{
    return assign(s);
}

template<typename T> basic_string<T>& basic_string<T>::operator =  (value_type c)
{
    return assign(1, c);
}

template<typename T> basic_string<T>& basic_string<T>::operator += (const basic_string& str)
{
    return append(str);
}

template<typename T> basic_string<T>& basic_string<T>::operator += (const_pointer s)
{
    return append(s);
}

template<typename T> basic_string<T>& basic_string<T>::operator += (value_type c)
{
    return append(1, c);
}

template<typename T> basic_string<T>  basic_string<T>::operator +  (const basic_string& str) const
{
    basic_string temp(*this);
    temp += str;
    return temp;
}

template<typename T> basic_string<T>  basic_string<T>::operator +  (const_pointer s) const
{
    basic_string temp(*this);
    temp += s;
    return temp;
}

template<typename T> basic_string<T>  basic_string<T>::operator +  (value_type c) const
{
    basic_string temp(*this);
    temp += c;
    return temp;
}

template<typename T> basic_string<T>& basic_string<T>::append(const basic_string& str)
{
    if (empty())
    {
        assign(str);
    }
    else if (!str.empty())
    {
        make_writeable<copy_existing>(size_ + str.size_);
        typetraits<T>::copy(end_ptr(), str.elements_, str.size_ + 1);
        size_ += str.size_;
    }
    return *this;
}

template<typename T> basic_string<T>& basic_string<T>::append(const basic_string& str, size_type pos, size_type len = npos)
{
    if (empty())
    {
        assign(str, pos, len);
    }
    else
    {
        THOR_DEBUG_ASSERT(str.size_ >= pos);
        const size_type max_len = str.size_ - pos;
        if (len > max_len) len = max_len;

        if (len > 0)
        {
            make_writeable<copy_existing>(size_ + len);
            typetraits<T>::copy(end_ptr(), str.elements_ + pos, len);
            size_ += len;
            *end_ptr() = T(0);
        }
    }
    return *this;
}

template<typename T> basic_string<T>& basic_string<T>::append(const_pointer s)
{
    return append(s, string_length(s));
}

template<typename T> basic_string<T>& basic_string<T>::append(const_pointer s, size_type len)
{
    if (empty())
    {
        assign(s, len);
    }
    else if (len != 0)
    {
        make_writeable<copy_existing>(size_ + len);
        typetraits<T>::copy(end_ptr(), s, len);
        size_ += len;
        *end_ptr() = T(0);
    }
    return *this;
}

template<typename T> basic_string<T>& basic_string<T>::append(size_type len, value_type fill)
{
    if (empty())
    {
        assign(len, fill);
    }
    else if (len != 0)
    {
        make_writeable<copy_existing>(size_ + len);
        typetraits<T>::range_copy(end_ptr(), end_ptr() + len, fill);
        size_ += len;
        *end_ptr() = T(0);
    }
    return *this;
}

template<typename T> template<class InputIterator> basic_string<T>& basic_string<T>::append(InputIterator first, InputIterator last)
{
    if (empty())
    {
        assign(first, last);
    }
    else
    {
        difference_type len = distance(first, last);
        THOR_DEBUG_ASSERT(len >= 0);
        if (len > 0)
        {
            make_writeable<copy_existing>(size_ + len);
            pointer p = end_ptr();
            do
            {
                THOR_DEBUG_ASSERT(p < &elements_[size_ + len]);
                *p++ = *first++;
            } while (first != last);
            size_ += len;
            *end_ptr() = T(0);
        }
    }
    return *this;
}

template<typename T> typename basic_string<T>::value_type& basic_string<T>::push_back(value_type c)
{
    make_writeable<copy_existing>(size_ + 1);
    elements_[size_++] = c;
    *end_ptr() = T(0);
    return back();
}

template<typename T> typename basic_string<T>::value_type& basic_string<T>::push_back()
{
    make_writeable<copy_existing>(size_ + 1);
    THOR_DEBUG_ASSERT(*end_ptr() == T(0));
    elements_[++size_] = T(0);
    return back();
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::append_format(const_pointer s, ...)
{
    va_list va;
    va_start(va, s);
    size_type len = append_format_v(s, va);
    va_end(va);
    return len;
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::append_format_v(const_pointer s, va_list va)
{
    const size_type len = string_format_count_v(s, va);
    if (len != 0 && len != npos)
    {
        make_writeable<copy_existing>(size_ + len);
        const size_type written = string_format_v(elements_ + size_, len + 1, s, va);
        THOR_DEBUG_ASSERT(written == len); THOR_UNUSED(written);
        size_ += len;
        THOR_DEBUG_ASSERT(*end_ptr() == T(0));
    }
    return len;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T> basic_string<T>& basic_string<T>::assign(const basic_string& str)
{
    if (str.elements_ != elements_)
    {
        if (str.is_shareable())
        {
            // Shareable string
            str.ref_add();
            ref_release();
            elements_   = str.elements_;
            size_       = str.size_;
            capacity_   = str.capacity_;
        }
        else if (str.capacity_ == str.max_capacity())
        {
            // Literal string
            ref_release();
            elements_   = str.elements_;
            size_       = str.size_;
            capacity_   = str.capacity_;
        }
        else if (str.empty())
        {
            clear();
        }
        else
        {
            // Not shareable; make a copy
            make_writeable<exact>(str.size_);
            size_ = str.size_;
            typetraits<T>::copy(elements_, str.elements_, size_ + 1);
            THOR_DEBUG_ASSERT(*end_ptr() == T(0));
        }
    }
    return *this;
}

template<typename T> basic_string<T>& basic_string<T>::assign(const basic_string& str, size_type pos, size_type len)
{
    if (pos == 0 && len >= str.size_)
    {
        assign(str);
    }
    else
    {
        THOR_DEBUG_ASSERT(pos <= str.size_);
        const size_type max_len = str.size_ - pos;
        if (len > max_len) len = max_len;

        assign(str.elements_ + pos, len);
    }
    return *this;
}

template<typename T> basic_string<T>& basic_string<T>::assign(const_pointer s)
{
    return assign(s, string_length(s));
}

template<typename T> basic_string<T>& basic_string<T>::assign(const_pointer s, size_type len)
{
    if (len == 0)
    {
        clear();
    }
    else
    {
        make_writeable<exact>(len);
        size_ = len;
        typetraits<T>::copy(elements_, s, size_);
        *end_ptr() = T(0);
    }
    return *this;
}

template<typename T> basic_string<T>& basic_string<T>::assign(size_type len, value_type fill)
{
    if (len == 0)
    {
        clear();
    }
    else
    {
        make_writeable<exact>(len);
        size_ = len;
        typetraits<T>::range_copy(elements_, end_ptr(), fill);
        *end_ptr() = T(0);
    }
    return *this;
}

template<typename T> template<class InputIterator> basic_string<T>& basic_string<T>::assign(InputIterator first, InputIterator last)
{
    difference_type len = distance(first, last);
    THOR_DEBUG_ASSERT(len >= 0);
    if (len <= 0)
    {
        clear();
    }
    else
    {
        make_writeable<exact>((size_type)len);
        size_ = (size_type)len;
        pointer p = elements_;
        while (first != last)
        {
            *p++ = *first++;
            THOR_DEBUG_ASSERT(p <= end_ptr());
        }
        *end_ptr() = T(0);
    }
    return *this;
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::format(const_pointer s, ...)
{
    va_list va;
    va_start(va, s);
    size_type count = format_v(s, va);
    va_end(va);
    return count;
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::format_v(const_pointer s, va_list va)
{
    size_type len = string_format_count_v(s, va);
    if (len == 0 || len == npos)
    {
        clear();
        return 0;
    }
    else
    {
        make_writeable<exact>(len);
        size_type actual = string_format_v(elements_, capacity_ + 1, s, va);
        size_ = actual;
        THOR_DEBUG_ASSERT(*end_ptr() == T(0));
        return size_;
    }
}

template<typename T> basic_string<T>& basic_string<T>::assign(Literal lit, const_pointer s)
{
    ref_release();
    elements_ = const_cast<pointer>(s);
    size_ = string_length(s);
    capacity_ = lit == lit_allow_share ? npos : 0;
    return *this;
}

template<typename T> basic_string<T>& basic_string<T>::assign(Literal lit, const_pointer s, size_type len)
{
    ref_release();
    elements_ = const_cast<pointer>(s);
    size_ = len;
    capacity_ = lit == lit_allow_share ? npos : 0;

    // This is a literal string; we can't insert a NUL character so size must match given
    THOR_DEBUG_ASSERT(size_ == string_length(s));
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T> basic_string<T>& basic_string<T>::insert(size_type pos, const basic_string& str)
{
    THOR_DEBUG_ASSERT(pos <= size_);
    if (!str.empty())
    {
        make_writeable<copy_existing>(size_ + str.size_);
        typetraits<T>::copy_overlap(elements_ + pos + str.size_, elements_ + pos, (size_ - pos) + 1);
        typetraits<T>::copy(elements_ + pos, str.elements_, str.size_);
        size_ += str.size_;
    }
    return *this;
}

template<typename T> basic_string<T>& basic_string<T>::insert(size_type pos, const basic_string& str, size_type subpos, size_type len = npos)
{
    THOR_DEBUG_ASSERT(pos <= size_);
    THOR_DEBUG_ASSERT(subpos <= str.size_);
    
    const size_type max_sublen = str.size_ - subpos;
    if (len > max_sublen) len = max_sublen;
    
    if (len != 0)
    {
        make_writeable<copy_existing>(size_ + len);
        typetraits<T>::copy_overlap(elements_ + pos + len, elements_ + pos, (size_ - pos) + 1);
        typetraits<T>::copy(elements_ + pos, str.elements_ + subpos, len);
        size_ += len;
    }
    return *this;
}

template<typename T> basic_string<T>& basic_string<T>::insert(size_type pos, const_pointer s)
{
    return insert(pos, s, string_length(s));
}

template<typename T> basic_string<T>& basic_string<T>::insert(size_type pos, const_pointer s, size_type len)
{
    THOR_DEBUG_ASSERT(pos <= size_);

    if (len != 0)
    {
        make_writeable<copy_existing>(size_ + len);
        typetraits<T>::copy_overlap(elements_ + pos + len, elements_ + pos, (size_ - pos) + 1);
        typetraits<T>::copy(elements_ + pos, s, len);
        size_ += len;
    }
    return *this;
}

template<typename T> basic_string<T>& basic_string<T>::insert(size_type pos, size_type len, value_type fill)
{
    THOR_DEBUG_ASSERT(pos <= size_);

    if (len != 0)
    {
        make_writeable<copy_existing>(size_ + len);
        typetraits<T>::copy_overlap(elements_ + pos + len, elements_ + pos, (size_ - pos) + 1);
        typetraits<T>::range_copy(elements_ + pos, elements_ + pos + len, fill);
        size_ += len;
    }
    return *this;
}

template<typename T> void basic_string<T>::insert(iterator  pos, size_type len, value_type fill)
{
    pos.verify_owner(this);
    pos.verify_range(true);
    insert(pos.element - elements_, len, fill);
}

template<typename T> typename basic_string<T>::iterator basic_string<T>::insert(iterator  pos, value_type c)
{
    pos.verify_owner(this);
    pos.verify_range(true);
    const size_type index = pos.element - elements_;
    insert(index, 1, c);
    return iterator(elements_ + index, this);
}

template<typename T> template<class InputIterator> void basic_string<T>::insert(iterator pos, InputIterator first, InputIterator last)
{
    pos.verify_owner(this);
    pos.verify_range(true);
    difference_type len = distance(first, last);
    THOR_DEBUG_ASSERT(len >= 0);
    if (len > 0)
    {
        const size_type index = pos.element - elements_;
        make_writeable<copy_existing>(size_ + len);
        typetraits<T>::copy_overlap(elements_ + index + len, elements_ + index, (size_ - index) + 1);
        size_ += len;
        pointer p = elements_ + index;
        do
        {
            THOR_DEBUG_ASSERT(p >= (elements_ + index) && p < (elements_ + index + len));
            *p++ = *first++;
        } while (first != last);
        *end_ptr() = T(0);
    }
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::insert_format(size_type pos, const_pointer s, ...)
{
    va_list va;
    va_start(va, s);
    size_type count = insert_format_v(pos, s, va);
    va_end(va);
    return count;
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::insert_format(iterator  pos, const_pointer s, ...)
{
    pos.verify_owner(this);
    pos.verify_range(true);
    va_list va;
    va_start(va, s);
    size_type count = insert_format_v(pos.element - elements_, s, va);
    va_end(va);
    return count;
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::insert_format_v(size_type pos, const_pointer s, va_list va)
{
    const size_type len = string_format_count_v(s, va);
    if (len != 0 && len != npos)
    {
        make_writeable<copy_existing>(size_ + len);
        size_ += len;
        typetraits<T>::copy_overlap(elements_ + pos + len, elements_ + pos, (size_ - pos) + 1);
        value_type c = elements_[pos + len]; // Have to save and restore this character because string_format_v will NUL-terminate
        const size_type actual = string_format_v(elements_ + pos, len + 1, s, va);
        elements_[pos + len] = c;
        THOR_DEBUG_ASSERT(actual == len);
        THOR_DEBUG_ASSERT(*end_ptr() == T(0));
        return actual;
    }
    return 0;
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::insert_format_v(iterator  pos, const_pointer s, va_list va)
{
    pos.verify_owner(this);
    pos.verify_range(true);
    return insert_format_v(pos.element - elements_, s, va);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T> basic_string<T>& basic_string<T>::erase(size_type pos = 0, size_type len = npos)
{
    THOR_DEBUG_ASSERT(pos < size_);
    const size_type max_len = size_ - pos;
    if (len > max_len) len = max_len;

    make_writeable<copy_existing>(size_); // must do the full amount first
    typetraits<T>::copy_overlap(elements_ + pos, elements_ + pos + len, (size_ - (pos + len)) + 1);
    size_ -= len;

    return *this;
}

template<typename T> typename basic_string<T>::iterator basic_string<T>::erase(iterator pos)
{
    pos.verify_owner(this);
    pos.verify_range();

    size_type index = pos.element - elements_;
    make_writeable<copy_existing>(size_); // must do the full amount first
    typetraits<T>::copy_overlap(elements_ + index, elements_ + index + 1, (size_ - (index + 1)) + 1);
    --size_;

    return iterator(elements_ + index, this);
}

template<typename T> typename basic_string<T>::iterator basic_string<T>::erase(iterator first, iterator last)
{
    first.verify_owner(this);
    first.verify_range(true);
    last.verify_owner(this);
    last.verify_range(true);
    
    const size_type pos = first.element - elements_;
    const difference_type len = distance(first, last);
    THOR_DEBUG_ASSERT(len >= 0);
    erase(pos, (size_type)len);

    return iterator(elements_ + pos, this);
}

template<typename T> typename basic_string<T>::value_type basic_string<T>::pop_back()
{
    THOR_DEBUG_ASSERT(!empty());
    value_type c = back();
    make_writeable<shrink_copy>(size_ - 1);
    elements_[--size_] = T(0);
    return c;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T> basic_string<T>& basic_string<T>::replace(size_type pos, size_type len, const basic_string& str)
{
    return replace(pos, len, str.elements_, str.size_);
}

template<typename T> basic_string<T>& basic_string<T>::replace(iterator pos1, iterator pos2, const basic_string& str)
{
    pos1.verify_range(true);
    pos1.verify_owner(this);
    pos2.verify_range(true);
    pos2.verify_owner(this);

    return replace(pos1.element - elements_, pos2 - pos1, str.elements_, str.size_);
}

template<typename T> basic_string<T>& basic_string<T>::replace(size_type pos, size_type len, const basic_string& str, size_type subpos, size_type sublen = npos)
{
    THOR_DEBUG_ASSERT(pos + len <= size_);
    THOR_DEBUG_ASSERT(subpos <= str.size_);
    const size_type max_sublen = str.size_ - subpos;
    if (sublen > max_sublen) sublen = max_sublen;

    return replace(pos, len, str.elements_ + subpos, sublen);
}

template<typename T> basic_string<T>& basic_string<T>::replace(size_type pos, size_type len, const_pointer s)
{
    THOR_DEBUG_ASSERT(pos + len <= size_);
    return replace(pos, len, s, string_length(s));
}

template<typename T> basic_string<T>& basic_string<T>::replace(iterator pos1, iterator pos2, const_pointer s)
{
    pos1.verify_range(true);
    pos1.verify_owner(this);
    pos2.verify_range(true);
    pos2.verify_owner(this);

    return replace(pos1.element - elements_, pos2 - pos1, s, string_length(s));
}

template<typename T> basic_string<T>& basic_string<T>::replace(size_type pos, size_type len, const_pointer s, size_type n)
{
    THOR_DEBUG_ASSERT(pos + len <= size_);
    if (len < n)
    {
        // Growing
        const size_type growth = n - len;
        make_writeable<copy_existing>(size_ + growth);
        typetraits<T>::copy_overlap(elements_ + pos + n, elements_ + pos + len, (size_ - (pos + len)) + 1);
        typetraits<T>::copy(elements_ + pos, s, n);
        size_ += growth;
    }
    else if (len > n)
    {
        const size_type shrink = len - n;
        make_writeable<exact_copy>(size_); // Must start with the same size
        typetraits<T>::copy_overlap(elements_ + pos + n, elements_ + pos + len, (size_ - (pos + len)) + 1);
        typetraits<T>::copy(elements_ + pos, s, n);
        size_ -= shrink;
    }
    else if (len != 0)
    {
        // Even replacement
        make_writeable<exact_copy>(size_);
        typetraits<T>::copy(elements_ + pos, s, n);
    }
    return *this;
}

template<typename T> basic_string<T>& basic_string<T>::replace(iterator pos1, iterator pos2, const_pointer s, size_type n)
{
    pos1.verify_range(true);
    pos1.verify_owner(this);
    pos2.verify_range(true);
    pos2.verify_owner(this);

    return replace(pos1.element - elements_, pos2 - pos1, s, n);
}

template<typename T> basic_string<T>& basic_string<T>::replace(size_type pos, size_type len, size_type fill_len, value_type fill)
{
    THOR_DEBUG_ASSERT(pos + len <= size_);
    if (len < fill_len)
    {
        // Growing
        const size_type growth = fill_len - len;
        make_writeable<copy_existing>(size_ + growth);
        typetraits<T>::copy_overlap(elements_ + pos + fill_len, elements_ + pos + len, (size_ - (pos + len)) + 1);
        typetraits<T>::range_copy(elements_ + pos, elements_ + pos + fill_len, fill);
        size_ += growth;
    }
    else if (len > fill_len)
    {
        const size_type shrink = len - fill_len;
        make_writeable<exact_copy>(size_); // Must start with the same size
        typetraits<T>::copy_overlap(elements_ + pos + fill_len, elements_ + pos + len, (size_ - (pos + len)) + 1);
        typetraits<T>::range_copy(elements_ + pos, elements_ + pos + fill_len, fill);
        size_ -= shrink;
    }
    else if (len != 0)
    {
        // Even replacement
        make_writeable<exact_copy>(size_);
        typetraits<T>::range_copy(elements_ + pos, elements_ + pos + len, fill);
    }
    return *this;
}

template<typename T> basic_string<T>& basic_string<T>::replace(iterator pos1, iterator pos2, size_type fill_len, value_type fill)
{
    pos1.verify_range(true);
    pos1.verify_owner(this);
    pos2.verify_range(true);
    pos2.verify_owner(this);

    return replace(pos1.element - elements_, pos2 - pos1, fill_len, fill);
}

template<typename T> template<class InputIterator> basic_string<T>& basic_string<T>::replace(iterator pos1, iterator pos2, InputIterator first, InputIterator last)
{
    pos1.verify_range(true);
    pos1.verify_owner(this);
    pos2.verify_range(true);
    pos2.verify_owner(this);

    const size_type pos = pos1.element - elements_;
    const size_type len = pos2 - pos1;
    const size_type count = distance(first, last);
    if (len < count)
    {
        // Growing
        const size_type growth = count - len;
        make_writeable<copy_existing>(size_ + growth);
        typetraits<T>::copy_overlap(elements_ + pos + count, elements_ + pos + len, (size_ - (pos + len)) + 1);
        size_ += growth;
        pointer p = elements_ + pos;
        while (first != last)
        {
            THOR_DEBUG_ASSERT(p >= elements_ && p < end_ptr());
            *p++ = *first++;
        }
    }
    else if (len > count)
    {
        // Shrinking
        const size_type shrink = len - count;
        make_writeable<exact_copy>(size_);
        typetraits<T>::copy_overlap(elements_ + pos + count, elements_ + pos + len, (size_ - (pos + len)) + 1);
        size_ -= shrink;
        pointer p = elements_ + pos;
        while (first != last)
        {
            THOR_DEBUG_ASSERT(p >= elements_ && p < end_ptr());
            *p++ = *first++;
        }
    }
    else
    {
        // Equal size
        make_writeable<exact_copy>(size_);
        pointer p = elements_ + pos;
        while (first != last)
        {
            THOR_DEBUG_ASSERT(p >= elements_ && p < end_ptr());
            *p++ = *first++;
        }
    }
    return *this;
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::replace_format(size_type pos, size_type len, const_pointer s, ...)
{
    va_list va;
    va_start(va, s);
    size_type count = replace_format_v(pos, len, s, va);
    va_end(va);
    return count;
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::replace_format(iterator pos1, iterator pos2, const_pointer s, ...)
{
    pos1.verify_owner(this);
    pos2.verify_owner(this);
    pos1.verify_range(true);
    pos2.verify_range(true);

    va_list va;
    va_start(va, s);
    size_type count = replace_format_v(pos1.element - elements_, pos2 - pos1, s, va);
    va_end(va);
    return count;
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::replace_format_v(size_type pos, size_type len, const_pointer s, va_list va)
{
    THOR_DEBUG_ASSERT(pos + len <= size_);
    const size_type count = string_format_count_v(s, va);
    THOR_DEBUG_ASSERT(count != npos);
    if (len < count)
    {
        // Growing
        const size_type growth = count - len;
        make_writeable<copy_existing>(size_ + growth);
        typetraits<T>::copy_overlap(elements_ + pos + count, elements_ + pos + len, (size_ - (pos + len)) + 1);
        value_type c = elements_[pos + count]; // string_format_v overwrites with NUL
        const size_type actual = string_format_v(elements_ + pos, count + 1, s, va);
        THOR_UNUSED(actual); THOR_DEBUG_ASSERT(actual == count);
        elements_[pos + count] = c;
        size_ += growth;
    }
    else if (len > count)
    {
        const size_type shrink = len - count;
        make_writeable<exact_copy>(size_); // Must start with the same size
        typetraits<T>::copy_overlap(elements_ + pos + count, elements_ + pos + len, (size_ - (pos + len)) + 1);
        value_type c = elements_[pos + count]; // string_format_v overwrites with NUL
        const size_type actual = string_format_v(elements_ + pos, count + 1, s, va);
        THOR_UNUSED(actual); THOR_DEBUG_ASSERT(actual == count);
        elements_[pos + count] = c;
        size_ -= shrink;
    }
    else if (len != 0)
    {
        // Even replacement
        make_writeable<exact_copy>(size_);
        value_type c = elements_[pos + count]; // string_format_v overwrites with NUL
        const size_type actual = string_format_v(elements_ + pos, count + 1, s, va);
        THOR_UNUSED(actual); THOR_DEBUG_ASSERT(actual == count);
        elements_[pos + count] = c;
    }
    return count;
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::replace_format_v(iterator pos1, iterator pos2, const_pointer s, va_list va)
{
    pos1.verify_owner(this);
    pos2.verify_owner(this);
    pos1.verify_range(true);
    pos2.verify_range(true);

    return replace_format_v(pos1.element - elements_, pos2 - pos1, s, va);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T> void basic_string<T>::swap(basic_string<T>& rhs)
{
    if ((is_shareable() || capacity_ == npos) && (rhs.is_shareable() || capacity_ == npos))
    {
        // Easy swap.
        thor::swap(elements_, rhs.elements_);
        thor::swap(size_,     rhs.size_);
        thor::swap(capacity_, rhs.capacity_);
    }
    else
    {
        basic_string temp(*this);
        *this = rhs;
        rhs = temp;
    }
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::copy(pointer out, size_type n, size_type pos = 0) const
{
    THOR_DEBUG_ASSERT(pos <= size_);
    const size_type max_len = size_ - pos;
    if (n > max_len) n = max_len;
    typetraits<T>::copy(out, elements_ + pos, n);
    return n;
}

template<typename T> basic_string<T> basic_string<T>::substr(size_type pos = 0, size_type len = npos) const
{
    THOR_DEBUG_ASSERT(pos <= size_);
    const size_type max_len = size_ - pos;
    return basic_string(elements_ + pos, len > max_len ? max_len : len);
}


///////////////////////////////////////////////////////////////////////////////

template<typename T> typename basic_string<T>::size_type basic_string<T>::find(const basic_string& str, size_type pos) const
{
    return find(str.elements_, pos, str.size_);
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::find(const_pointer s, size_type pos) const
{
    return find(s, pos, string_length(s));
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::find(const_pointer s, size_type pos, size_type len) const
{
    THOR_DEBUG_ASSERT(pos <= size_);
    if (pos > size_) pos = size_;
    const_pointer last = elements_ + (size_ - len);
    const_pointer p = elements_ + pos;
    while (p <= last)
    {
        if (memory_compare(p, s, len) == 0)
        {
            return p - elements_;
        }
        ++p;
    }
    return npos;
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::find(value_type c, size_type pos) const
{
    THOR_DEBUG_ASSERT(pos <= size_);
    if (pos > size_) pos = size_;
    const_pointer p = elements_ + pos;
    while (p < end_ptr())
    {
        if (*p == c)
        {
            return p - elements_;
        }
        ++p;
    }
    return npos;
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::rfind(const basic_string& str, size_type pos) const
{
    return rfind(str.elements_, pos, str.size_);
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::rfind(const_pointer s, size_type pos) const
{
    return rfind(s, pos, string_length(s));
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::rfind(const_pointer s, size_type pos, size_type len) const
{
    if (pos > size_) pos = size_;
    const_pointer p = elements_ + (pos - len);
    while (p >= elements_)
    {
        if (memory_compare(p, s, len) == 0)
        {
            return p - elements_;
        }
        --p;
    }
    return npos;
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::rfind(value_type c, size_type pos) const
{
    if (pos > size_) pos = size_;
    const_pointer p = elements_ + pos - 1;
    while (p >= elements_)
    {
        if (*p == c)
        {
            return p - elements_;
        }
        --p;
    }
    return npos;
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::find_i(const basic_string& str, size_type pos) const
{
    return find_i(str.elements_, pos, str.size_);
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::find_i(const_pointer s, size_type pos) const
{
    return find_i(s, pos, string_length(s));
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::find_i(const_pointer s, size_type pos, size_type len) const
{
    THOR_DEBUG_ASSERT(pos <= size_);
    if (pos > size_) pos = size_;
    const_pointer last = elements_ + (size_ - len);
    const_pointer p = elements_ + pos;
    while (p <= last)
    {
        if (memory_compare_i(p, s, len) == 0)
        {
            return p - elements_;
        }
        ++p;
    }
    return npos;
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::find_i(value_type c, size_type pos) const
{
    THOR_DEBUG_ASSERT(pos <= size_);
    const_pointer p = elements_ + pos;
    c = (value_type)towlower(c);
    while (p != end_ptr())
    {
        if (towlower(*p) == c)
        {
            return p - elements_;
        }
        ++p;
    }
    return npos;
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::rfind_i(const basic_string& str, size_type pos) const
{
    return rfind_i(str.elements_, pos, str.size_);
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::rfind_i(const_pointer s, size_type pos) const
{
    return rfind_i(s, pos, string_length(s));
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::rfind_i(const_pointer s, size_type pos, size_type len) const
{
    if (pos > size_) pos = size_;
    const_pointer p = elements_ + (pos - len);
    while (p >= elements_)
    {
        if (memory_compare_i(p, s, len) == 0)
        {
            return p - elements_;
        }
        --p;
    }
    return npos;
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::rfind_i(value_type c, size_type pos) const
{
    if (pos > size_) pos = size_;
    const_pointer p = elements_ + pos - 1;
    c = (value_type)towlower(c);
    while (p >= elements_)
    {
        if (towlower(*p) == c)
        {
            return p - elements_;
        }
        --p;
    }
    return npos;
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::find_first_of(const basic_string& str, size_type pos) const
{
    return find_first_of(str.elements_, pos, str.size_);
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::find_first_of(const_pointer s, size_type pos) const
{
    return find_first_of(s, pos, string_length(s));
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::find_first_of(const_pointer s, size_type pos, size_type len) const
{
    THOR_DEBUG_ASSERT(pos <= size_);
    const_pointer s_end = s + len;
    const_pointer p = elements_ + pos;
    while (p != end_ptr())
    {
        for (const_pointer p2 = s; p2 != s_end; ++p2)
        {
            if (*p2 == *p)
            {
                return p - elements_;
            }
        }
        ++p;
    }
    return npos;
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::find_first_of(value_type c, size_type pos) const
{
    return find(c, pos);
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::find_last_of(const basic_string& str, size_type pos) const
{
    return find_last_of(str.elements_, pos, str.size_);
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::find_last_of(const_pointer s, size_type pos) const
{
    return find_last_of(s, pos, string_length(s));
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::find_last_of(const_pointer s, size_type pos, size_type len) const
{
    if (pos > size_) pos = size_;
    const_pointer s_end = s + len;
    const_pointer p = elements_ + pos - 1;
    while (p >= elements_)
    {
        for (const_pointer p2 = s; p2 != s_end; ++p2)
        {
            if (*p == *p2)
            {
                return p - elements_;
            }
        }
        --p;
    }
    return npos;
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::find_last_of(value_type c, size_type pos) const
{
    return rfind(c, pos);
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::find_first_not_of(const basic_string& str, size_type pos) const
{
    return find_first_not_of(str.elements_, pos, str.size_);
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::find_first_not_of(const_pointer s, size_type pos) const
{
    return find_first_not_of(s, pos, string_length(s));
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::find_first_not_of(const_pointer s, size_type pos, size_type len) const
{
    THOR_DEBUG_ASSERT(pos <= size_);
    const_pointer s_end = s + len;
    const_pointer p = elements_ + pos;
    while (p < end_ptr())
    {
        const_pointer p2 = s;
        for (; p2 != s_end; ++p2)
        {
            if (*p2 == *p) break;
        }
        if (p2 == s_end) return p - elements_;
        ++p;
    }
    return npos;
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::find_first_not_of(value_type c, size_type pos) const
{
    THOR_DEBUG_ASSERT(pos <= size);
    const_pointer p = elements_ + pos;
    while (p < end_ptr())
    {
        if (*p != c) return p - elements_;
        ++p;
    }
    return npos;
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::find_last_not_of(const basic_string& str, size_type pos) const
{
    return find_last_not_of(str.elements_, pos, str.size_);
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::find_last_not_of(const_pointer s, size_type pos) const
{
    return find_last_not_of(s, pos, string_length(s));
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::find_last_not_of(const_pointer s, size_type pos, size_type len) const
{
    if (pos > size_) pos = size_;
    const_pointer s_end = s + len;
    const_pointer p = elements_ + pos - 1;
    while (p >= elements_)
    {
        const_pointer p2 = s;
        for (; p2 != s_end; ++p2)
        {
            if (*p2 == *p) break;
        }
        if (p2 == s_end) return p - elements_;
        --p;
    }
    return npos;
}

template<typename T> typename basic_string<T>::size_type basic_string<T>::find_last_not_of(value_type c, size_type pos) const
{
    if (pos > size_) pos = size_;
    const_pointer p = elements_ + pos - 1;
    while (p >= elements_)
    {
        if (*p != c) return p - elements_;
        --p;
    }
    return npos;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T> int basic_string<T>::compare(const basic_string& str) const
{
    return compare(0, size_, str.elements_, str.size_);
}

template<typename T> int basic_string<T>::compare(size_type pos, size_type len, const basic_string& str) const
{
    return compare(pos, len, str.elements_, str.size_);
}

template<typename T> int basic_string<T>::compare(size_type pos, size_type len, const basic_string& str, size_type subpos, size_type sublen) const
{
    THOR_DEBUG_ASSERT(subpos <= str.size_);
    const size_type max_sublen = str.size_ - subpos;
    return compare(pos, len, str.elements_ + subpos, sublen > max_sublen ? max_sublen : sublen);
}

template<typename T> int basic_string<T>::compare(const_pointer s) const
{
    return compare(0, size_, s, string_length(s));
}

template<typename T> int basic_string<T>::compare(size_type pos, size_type len, const_pointer s) const
{
    return compare(pos, len, s, string_length(s));
}

template<typename T> int basic_string<T>::compare(size_type pos, size_type len, const_pointer s, size_type n) const
{
    THOR_DEBUG_ASSERT(pos <= size_);
    const size_type max_len = size_ - pos;
    if (len > max_len) len = max_len;

    const size_type min_len = len < n ? len : n;
    int v = memory_compare(elements_ + pos, s, min_len);
    if (v == 0 && len != n)
    {
        return len < n ? -1 : 1;
    }
    return v;
}

template<typename T> int basic_string<T>::compare_i(const basic_string& str) const
{
    return compare_i(0, size_, str.elements_, str.size_);
}

template<typename T> int basic_string<T>::compare_i(size_type pos, size_type len, const basic_string& str) const
{
    return compare_i(pos, len, str.elements_, str.size_);
}

template<typename T> int basic_string<T>::compare_i(size_type pos, size_type len, const basic_string& str, size_type subpos, size_type sublen) const
{
    THOR_DEBUG_ASSERT(subpos <= str.size_);
    const size_type max_sublen = str.size_ - subpos;
    return compare_i(pos, len, str.elements_ + subpos, sublen > max_sublen ? max_sublen : sublen);
}

template<typename T> int basic_string<T>::compare_i(const_pointer s) const
{
    return compare_i(0, size_, s, string_length(s));
}

template<typename T> int basic_string<T>::compare_i(size_type pos, size_type len, const_pointer s) const
{
    return compare_i(pos, len, s, string_length(s));
}

template<typename T> int basic_string<T>::compare_i(size_type pos, size_type len, const_pointer s, size_type n) const
{
    THOR_DEBUG_ASSERT(pos <= size_);
    const size_type max_len = size_ - pos;
    if (len > max_len) len = max_len;

    const size_type min_len = len < n ? len : n;
    int v = memory_compare_i(elements_ + pos, s, min_len);
    if (v == 0 && len != n)
    {
        return len < n ? -1 : 1;
    }
    return v;
}

///////////////////////////////////////////////////////////////////////////////

inline string wide_to_utf8(const wchar_t* s)
{
    string outstr;
    bool b = wide_to_utf8(s, outstr);
    THOR_DEBUG_ASSERT(b); THOR_UNUSED(b);
    return outstr;
}

inline string wide_to_utf8(const wstring& str)
{
    string outstr;
    bool b = wide_to_utf8(str.c_str(), outstr);
    THOR_DEBUG_ASSERT(b); THOR_UNUSED(b);
    return outstr;
}

inline bool wide_to_utf8(const wstring& str, string& out)
{
    return wide_to_utf8(str.c_str(), out);
}

inline wstring utf8_to_wide(const char* s)
{
    wstring outstr;
    bool b = utf8_to_wide(s, outstr);
    THOR_DEBUG_ASSERT(b); THOR_UNUSED(b);
    return outstr;
}

inline wstring utf8_to_wide(const string& str)
{
    wstring outstr;
    bool b = utf8_to_wide(str.c_str(), outstr);
    THOR_DEBUG_ASSERT(b); THOR_UNUSED(b);
    return outstr;
}

inline bool utf8_to_wide(const string& str, wstring& out)
{
    return utf8_to_wide(str.c_str(), out);
}

inline bool utf8_is_valid(const string& str)
{
    return utf8_is_valid(str.c_str());
}

inline size_type wide_length(const string& str)
{
    return wide_length(str.c_str());
}

inline size_type utf8_length(const wstring& str)
{
    return utf8_length(str.c_str());
}

///////////////////////////////////////////////////////////////////////////////

template<typename T, thor_size_type T_SIZE> class basic_string : public basic_string<T, 0>
{
    typedef basic_string<T, 0> baseclass;
public:
    typedef typename baseclass::value_type value_type;
    typedef typename baseclass::pointer pointer;
    typedef typename baseclass::const_pointer const_pointer;
    typedef typename baseclass::size_type size_type;

    static const size_type embedded_size = T_SIZE;

    // Constructors must default-construct and then perform operations once v-table is initialized
    explicit basic_string();
    basic_string(const baseclass& rhs);
    basic_string(const baseclass& rhs, size_type pos, size_type len = npos);
    basic_string(const_pointer s);
    basic_string(const_pointer s, size_type len);
    basic_string(size_type fill, value_type c);
    template<class InputIterator> basic_string(InputIterator first, InputIterator last);
    basic_string(baseclass::Format, const_pointer s, ...);
    basic_string(const_pointer s, va_list va);
    basic_string(baseclass::Literal, const_pointer s);
    basic_string(baseclass::Literal, const_pointer s, size_type len);
    ~basic_string();

    basic_string& operator =  (const baseclass& str);
    basic_string& operator =  (const_pointer s);
    basic_string& operator =  (value_type c);
    basic_string& operator += (const baseclass& str);
    basic_string& operator += (const_pointer s);
    basic_string& operator += (value_type c);
    baseclass     operator +  (const baseclass& str) const;
    baseclass     operator +  (const_pointer s) const;
    baseclass     operator +  (value_type c) const;

protected:
    virtual thor_byte* alloc(size_type raw_needed, size_type& raw_avail, bool& shareable);
    virtual void free(thor_byte* data);

private:
    using baseclass::ref_counter;
    thor_byte fixed_data_[((T_SIZE + 1) * sizeof(T)) + sizeof(ref_counter)];
};

///////////////////////////////////////////////////////////////////////////////

template<typename T, thor_size_type T_SIZE> basic_string<T, T_SIZE>::basic_string() : baseclass()
{
}

template<typename T, thor_size_type T_SIZE> basic_string<T, T_SIZE>::basic_string(const baseclass& rhs) : baseclass()
{
    assign(rhs);
}

template<typename T, thor_size_type T_SIZE> basic_string<T, T_SIZE>::basic_string(const baseclass& rhs, size_type pos, size_type len) : baseclass()
{
    assign(rhs, pos, len);
}

template<typename T, thor_size_type T_SIZE> basic_string<T, T_SIZE>::basic_string(const_pointer s) : baseclass()
{
    assign(s);
}

template<typename T, thor_size_type T_SIZE> basic_string<T, T_SIZE>::basic_string(const_pointer s, size_type len) : baseclass()
{
    assign(s, len);
}

template<typename T, thor_size_type T_SIZE> basic_string<T, T_SIZE>::basic_string(size_type fill_len, value_type fill) : baseclass()
{
    assign(fill_len, fill);
}

template<typename T, thor_size_type T_SIZE> template<class InputIterator> basic_string<T, T_SIZE>::basic_string(InputIterator first, InputIterator last) : baseclass()
{
    assign(first, last);
}

template<typename T, thor_size_type T_SIZE> basic_string<T, T_SIZE>::basic_string(typename baseclass::Format, const_pointer s, ...) : baseclass()
{
    va_list va;
    va_start(va, s);
    format_v(s, va);
    va_end(va);
}

template<typename T, thor_size_type T_SIZE> basic_string<T, T_SIZE>::basic_string(const_pointer s, va_list va) : baseclass()
{
    format_v(s, va);
}

template<typename T, thor_size_type T_SIZE> basic_string<T, T_SIZE>::basic_string(typename baseclass::Literal lit, const_pointer s) : baseclass()
{
    assign(lit, s);
}

template<typename T, thor_size_type T_SIZE> basic_string<T, T_SIZE>::basic_string(typename baseclass::Literal lit, const_pointer s, size_type len) : baseclass()
{
    assign(lit, s, len);
}

template<typename T, thor_size_type T_SIZE> basic_string<T, T_SIZE>::~basic_string()
{
}

template<typename T, thor_size_type T_SIZE> basic_string<T, T_SIZE>& basic_string<T, T_SIZE>::operator = (const baseclass& str)
{
    baseclass::operator = (str);
    return *this;
}

template<typename T, thor_size_type T_SIZE> basic_string<T, T_SIZE>& basic_string<T, T_SIZE>::operator = (const_pointer s)
{
    baseclass::operator = (s);
    return *this;
}

template<typename T, thor_size_type T_SIZE> basic_string<T, T_SIZE>& basic_string<T, T_SIZE>::operator = (value_type c)
{
    baseclass::operator = (c);
    return *this;
}

template<typename T, thor_size_type T_SIZE> basic_string<T, T_SIZE>& basic_string<T, T_SIZE>::operator += (const baseclass& str)
{
    baseclass::operator += (str);
    return *this;
}

template<typename T, thor_size_type T_SIZE> basic_string<T, T_SIZE>& basic_string<T, T_SIZE>::operator += (const_pointer s)
{
    baseclass::operator += (s);
    return *this;
}

template<typename T, thor_size_type T_SIZE> basic_string<T, T_SIZE>& basic_string<T, T_SIZE>::operator += (value_type c)
{
    baseclass::operator += (c);
    return *this;
}

template<typename T, thor_size_type T_SIZE> basic_string<T, 0> basic_string<T, T_SIZE>::operator + (const baseclass& str) const
{
    return baseclass::operator + (str);
}

template<typename T, thor_size_type T_SIZE> basic_string<T, 0> basic_string<T, T_SIZE>::operator + (const_pointer s) const
{
    return baseclass::operator + (s);
}

template<typename T, thor_size_type T_SIZE> basic_string<T, 0> basic_string<T, T_SIZE>::operator + (value_type c) const
{
    return baseclass::operator + (c);
}

template<typename T, thor_size_type T_SIZE> thor_byte* basic_string<T, T_SIZE>::alloc(size_type raw_needed, size_type& raw_avail, bool& shareable)
{
    if (raw_needed <= sizeof(fixed_data_))
    {
        shareable = false;
        raw_avail = sizeof(fixed_data_);
        return fixed_data_;
    }
    return baseclass::alloc(raw_needed, raw_avail, shareable);
}

template<typename T, thor_size_type T_SIZE> void basic_string<T, T_SIZE>::free(thor_byte* data)
{
    if (data != fixed_data_)
    {
        baseclass::free(data);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Hash functions
///////////////////////////////////////////////////////////////////////////////
template<typename T_CHAR, size_type T_SIZE> struct hash<basic_string<T_CHAR, T_SIZE> >
{
    size_type operator () (const basic_string<T_CHAR, T_SIZE>& str) const
    {
        return __hashstring(str.c_str(), str.length());
    }
};

} // namespace thor

template<typename T> bool operator == (const thor::basic_string<T>& lhs, const thor::basic_string<T>& rhs) { return lhs.compare(rhs) == 0; }
template<typename T> bool operator == (const T*               lhs, const thor::basic_string<T>& rhs) { return rhs.compare(lhs) == 0; }
template<typename T> bool operator == (const thor::basic_string<T>& lhs, const T*               rhs) { return lhs.compare(rhs) == 0; }
template<typename T> bool operator != (const thor::basic_string<T>& lhs, const thor::basic_string<T>& rhs) { return lhs.compare(rhs) != 0; }
template<typename T> bool operator != (const T*               lhs, const thor::basic_string<T>& rhs) { return rhs.compare(lhs) != 0; }
template<typename T> bool operator != (const thor::basic_string<T>& lhs, const T*               rhs) { return lhs.compare(rhs) != 0; }
template<typename T> bool operator <  (const thor::basic_string<T>& lhs, const thor::basic_string<T>& rhs) { return lhs.compare(rhs) < 0; }
template<typename T> bool operator <  (const T*               lhs, const thor::basic_string<T>& rhs) { return rhs.compare(lhs) > 0; }
template<typename T> bool operator <  (const thor::basic_string<T>& lhs, const T*               rhs) { return lhs.compare(rhs) < 0; }
template<typename T> bool operator <= (const thor::basic_string<T>& lhs, const thor::basic_string<T>& rhs) { return lhs.compare(rhs) <= 0; }
template<typename T> bool operator <= (const T*               lhs, const thor::basic_string<T>& rhs) { return rhs.compare(lhs) >= 0; }
template<typename T> bool operator <= (const thor::basic_string<T>& lhs, const T*               rhs) { return lhs.compare(rhs) <= 0; }
template<typename T> bool operator >  (const thor::basic_string<T>& lhs, const thor::basic_string<T>& rhs) { return lhs.compare(rhs) > 0; }
template<typename T> bool operator >  (const T*               lhs, const thor::basic_string<T>& rhs) { return rhs.compare(lhs) < 0; }
template<typename T> bool operator >  (const thor::basic_string<T>& lhs, const T*               rhs) { return lhs.compare(rhs) > 0; }
template<typename T> bool operator >= (const thor::basic_string<T>& lhs, const thor::basic_string<T>& rhs) { return lhs.compare(rhs) >= 0; }
template<typename T> bool operator >= (const T*               lhs, const thor::basic_string<T>& rhs) { return rhs.compare(lhs) <= 0; }
template<typename T> bool operator >= (const thor::basic_string<T>& lhs, const T*               rhs) { return lhs.compare(rhs) >= 0; }

#endif