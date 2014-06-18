/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * vector.h
 *
 * This file defines an STL-compatible vector container
 *
 * Extensions/Changes:
 *  - push_back() has changed but remains compatible with STL usage:
 *    * push_back() returns a reference to the added item
 *    * push_back() with zero parameters will default-construct an element
 *    * push_back() variations exist with 1-4 parameters that will in-place
 *      construct an element without necessarily needing a copy constructor.
 *    * push_back_placement() can be used with placement new to construct 
 *      elements with more than 4 parameters.
 *  - insert() has changed but remains compatible with STL usage:
 *    * insert(pos) will default-construct an element at pos
 *    * insert(pos,...) variations with 1-4 additional parameters will
 *      in-place construct an element at pos without necessarily needing a
 *      copy constructor.
 *    * insert_placement(pos) can be used with placement new to construct
 *      elements with more than 4 parameters.
 *  - reduce() reduces the underlying memory usage
 *  - swap_and_pop() will swap an element with the back element and pop it in O(1)
 *  - Assistance for raw pointer types:
 *    * delete_all() will call delete on every element and clear() the vector.
 *    * erase_and_delete() can be used to delete an element and erase it from
 *      the vector.
 *    * pop_back_delete() will delete the last element and pop it from the vector.
 *  - Exponential growth is at the rate of 1/2 * capacity
 *  - The template allows a preallocated amount of space. This space is part of
 *    the vector instance (i.e. it makes sizeof(vector) larger) and is not
 *    allocated on the heap.
 *    * Example: vector<int, 5> reserves space for 5 ints, but can still be
 *      passed to functions that require vector<int>. The sizeof(vector<int,5>)
 *      is equal to (sizeof(vector<int>) + 5 * sizeof(int)).
 *    * Growth above the preallocated amount will use the heap, but the
 *      preallocated amount is unused and effectively wasted.
 *    * swap() between preallocated containers is no longer O(1). Also, swap()
 *      will allocate from the heap and ignore preallocated space.
 */

#ifndef THOR_VECTOR_H
#define THOR_VECTOR_H
#pragma once

#ifndef THOR_ITERATOR_H
#include "iterator.h"
#endif

#ifndef THOR_ALGORITHM_H
#include "algorithm.h"
#endif

#ifndef THOR_FUNCTION_H
#include "function.h"
#endif

#ifndef THOR_SORT_H
#include "sort.h"
#endif

#ifndef THOR_MEMORY_H
#include "memory.h"
#endif

namespace thor
{

template <typename T, unsigned T_PREALLOC = 0> class vector;

// Specialization for the base vector that does no preallocation.
template <typename T> class vector<T, 0>
{
public:
    // STL-compatible typedefs
    typedef T value_type;
    typedef T* pointer;
    typedef T& reference;
    typedef const T* const_pointer;
    typedef const T& const_reference;
    typedef thor_size_type size_type;
    typedef thor_diff_type difference_type;

    // Iterator base class
    struct iterator_base : public iterator_type<random_access_iterator_tag, T>
    {
        typedef THOR_TYPENAME vector<T>::pointer pointer;
        pointer m_element;
#ifdef THOR_DEBUG
        const vector* m_vector;
        iterator_base(pointer p, const vector* v) : m_element(p), m_vector(v) {}
#else
        iterator_base(pointer p, const vector*) : m_element(p) {}
#endif
        void verify_range(bool allowEnd = false) const
        {
            THOR_UNUSED(allowEnd);
            THOR_DEBUG_ASSERT(m_element >= m_vector->m_elements && m_element < (m_vector->m_elements + m_vector->m_size + allowEnd));
        }
        void decr() {                 --m_element; }
        void incr() { verify_range(); ++m_element; }
        bool operator == (const iterator_base& i) const { THOR_DEBUG_ASSERT(m_vector == i.m_vector); return m_element == i.m_element; }
        bool operator != (const iterator_base& i) const { THOR_DEBUG_ASSERT(m_vector == i.m_vector); return m_element != i.m_element; }
        bool operator <  (const iterator_base& i) const { THOR_DEBUG_ASSERT(m_vector == i.m_vector); return m_element <  i.m_element; }
        bool operator >  (const iterator_base& i) const { THOR_DEBUG_ASSERT(m_vector == i.m_vector); return i.m_element <  m_element; }
        bool operator <= (const iterator_base& i) const { THOR_DEBUG_ASSERT(m_vector == i.m_vector); return !(i.m_element <  m_element); }
        bool operator >= (const iterator_base& i) const { THOR_DEBUG_ASSERT(m_vector == i.m_vector); return !(m_element <  i.m_element); }
    };

    // Forward iterator template
    template<typename Traits> class fwd_iterator : public iterator_base
    {
    public:
        typedef typename Traits::pointer pointer;
        typedef typename Traits::reference reference;
        typedef fwd_iterator<nonconst_traits<T> > nonconst_iterator;
        typedef fwd_iterator<Traits> selftype;

        fwd_iterator(pointer p = 0, const vector* o = 0) : iterator_base((typename iterator_base::pointer)p, o) {}
        fwd_iterator(const nonconst_iterator& i) : iterator_base(i) {}
        selftype&  operator = (const nonconst_iterator& i)  { iterator_base::operator = (i); return *this; }
        reference  operator * () const                      { verify_range(); return *m_element; }
        pointer    operator -> () const                     { verify_range(); return  m_element; }
        selftype   operator - (difference_type i) const     { selftype n(*this); n.m_element -= i; return n; }
        selftype&  operator -= (difference_type i)          {                    m_element -= i;   return *this; }
        selftype&  operator -- ()     /* --iterator */      {                    decr();           return *this; }
        selftype   operator -- (int)  /* iterator-- */      { selftype n(*this); decr();           return n; }
        selftype   operator + (difference_type i) const     { selftype n(*this); n.m_element += i; return n; }
        selftype&  operator += (difference_type i)          {                    m_element += i;   return *this; }
        selftype&  operator ++ ()     /* ++iterator */      {                    incr();           return *this; }
        selftype   operator ++ (int)  /* iterator++ */      { selftype n(*this); incr();           return n; }

        difference_type operator - (const selftype& t) const { THOR_DEBUG_ASSERT(m_vector == t.m_vector); return m_element - t.m_element; }
    };

    // Reverse iterator template
    template<typename Traits> class rev_iterator : public iterator_base
    {
    public:
        typedef typename Traits::pointer pointer;
        typedef typename Traits::reference reference;
        typedef rev_iterator<nonconst_traits<T> > nonconst_iterator;
        typedef rev_iterator<Traits> selftype;

        rev_iterator(pointer p = 0, const vector* o = 0) : iterator_base((typename iterator_base::pointer)p, o) {}
        rev_iterator(const nonconst_iterator& i) : iterator_base(i) {}
        selftype&  operator = (const nonconst_iterator& i)  { iterator_base::operator = (i); return *this; }
        reference  operator * () const                      { verify_range(); return *m_element; }
        pointer    operator -> () const                     { verify_range(); return  m_element; }
        selftype   operator - (difference_type i) const     { selftype n(*this); n.m_element += i; return n; }
        selftype&  operator -= (difference_type i)          {                    m_element += i;   return *this; }
        selftype&  operator -- ()     /* --iterator */      {                    incr();           return *this; }
        selftype   operator -- (int)  /* iterator-- */      { selftype n(*this); incr();           return n; }
        selftype   operator + (difference_type i) const     { selftype n(*this); n.m_element -= i; return n; }
        selftype&  operator += (difference_type i)          {                    m_element -= i;   return *this; }
        selftype&  operator ++ ()     /* ++iterator */      {                    decr();           return *this; }
        selftype   operator ++ (int)  /* iterator++ */      { selftype n(*this); decr();           return n; }

        difference_type operator - (const selftype& t) const { THOR_DEBUG_ASSERT(m_vector == t.m_vector); return t.m_element - m_element; }
    };

    typedef fwd_iterator<nonconst_traits<T> > iterator;
    typedef fwd_iterator<const_traits<T>    > const_iterator;

    typedef rev_iterator<nonconst_traits<T> > reverse_iterator;
    typedef rev_iterator<const_traits<T>    > const_reverse_iterator;

    // constructors
    vector() :
        m_elements(0),
        m_size(0),
        m_capacity(0)
    {}

    vector(size_type n)
    {
        if (n != 0)
        {
            m_elements = alloc(n, m_capacity);
            THOR_ASSERT(m_capacity >= n);
            m_size = n;
            typetraits<T>::range_construct(m_elements, m_elements + m_size);
        }
        else
        {
            m_elements = 0;
            m_size = m_capacity = 0;
        }
    }

    vector(size_type n, const T& t)
    {
        if (n != 0)
        {
            m_elements = alloc(n, m_capacity);
            THOR_ASSERT(m_capacity >= n);
            m_size = n;
            typetraits<T>::range_construct(m_elements, m_elements + m_size, t);
        }
        else
        {
            m_elements = 0;
            m_size = m_capacity = 0;
        }
    }
    
    vector(const vector& V) :
        m_size(V.size()),
        m_capacity(V.size())
    {
        if (m_size)
        {
            m_elements = alloc(m_size, m_capacity);
            typetraits<T>::range_construct(m_elements, m_elements + m_size, V.m_elements);
        }
        else
        {
            m_elements = 0;
        }
    }

    template <typename InputIterator> vector(InputIterator first, InputIterator last) : m_elements(0), m_size(0), m_capacity(0)
    {
        insert(end(), first, last);
    }

    virtual ~vector()
    {
        clear();
        dealloc(m_elements);
        m_elements = 0;
    }

    vector& operator = (const vector& V)
    {
        if (this != &V)
        {
            clear();
            reserve(V.size());
            m_size = V.size();
            typetraits<T>::range_construct(m_elements, end_ptr(), V.m_elements);
        }
        return *this;
    }

    // Forward iteration
    iterator begin()
    {
        return iterator(m_elements, this);
    }
    
    const_iterator begin() const
    {
        return const_iterator(m_elements, this);
    }

    iterator end()
    {
        return iterator(end_ptr(), this);
    }
    
    const_iterator end() const
    {
        return const_iterator(end_ptr(), this);
    }

    // Reverse iteration
    reverse_iterator rbegin()
    {
        return reverse_iterator(m_elements ? end_ptr() - 1 : 0, this);
    }

    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(m_elements ? end_ptr() - 1 : 0, this);
    }

    reverse_iterator rend()
    {
        return reverse_iterator(m_elements ? m_elements - 1 : 0, this);
    }

    const_reverse_iterator rend() const
    {
        return const_reverse_iterator(m_elements ? m_elements - 1 : 0, this);
    }

    // Size/capacity
    size_type size() const
    {
        return m_size;
    }
    
    size_type max_size() const
    {
        return size_type(-1);
    }
    
    size_type capacity() const
    {
        return m_capacity;
    }
    
    bool empty() const
    {
        return m_size == 0;
    }

    // Accessing elements (always O(1)). NOTE! This will not cause growth of the vector.
    T& operator [] (size_type n)
    {
        THOR_ASSERT(m_elements == 0 || n < size());
        return m_elements[n];
    }
    
    const T& operator [] (size_type n) const
    {
        THOR_ASSERT(m_elements == 0 || n < size());
        return m_elements[n];
    }

    T& at(size_type n)
    {
        THOR_ASSERT(m_elements == 0 || n < size());
        return m_elements[n];
    }
    
    const T& at(size_type n) const
    {
        THOR_ASSERT(m_elements == 0 || n < size());
        return m_elements[n];
    }

    T& front()
    {
        THOR_ASSERT(m_elements == 0 || !empty());
        return m_elements[0];
    }
    
    const T& front() const
    {
        THOR_ASSERT(m_elements == 0 || !empty());
        return m_elements[0];
    }
    
    T& back()
    {
        THOR_ASSERT(!empty());
        return *(end_ptr() - 1);
    }
    
    const T& back() const
    {
        THOR_ASSERT(!empty());
        return *(end_ptr() - 1);
    }

    // Reserve: causes exact growth if necessary.
    void reserve(size_type n)
    {
        if (n > capacity())
        {
            pointer new_elements = alloc(n, m_capacity);
            THOR_ASSERT(m_capacity >= n);
            typetraits<T>::range_move(new_elements, new_elements + m_size, m_elements);
            dealloc(m_elements);
            m_elements = new_elements;
        }
    }

    // Adding and removing elements to the end of the vector.
    T& push_back()
    {
        typetraits<T>::construct(alloc_back());
        return back();
    }
    template <class T1> T& push_back(const T1& t1)
    {
        typetraits<T>::construct(alloc_back(), t1);
        return back();
    }
    template <class T1, class T2> T& push_back(const T1& t1, const T2& t2)
    {
        typetraits<T>::construct(alloc_back(), t1, t2);
        return back();
    }
    template <class T1, class T2, class T3> T& push_back(const T1& t1, const T2& t2, const T3& t3)
    {
        typetraits<T>::construct(alloc_back(), t1, t2, t3);
        return back();
    }
    template <class T1, class T2, class T3, class T4> T& push_back(const T1& t1, const T2& t2, const T3& t3, const T4& t4)
    {
        typetraits<T>::construct(alloc_back(), t1, t2, t3, t4);
        return back();
    }
    
    // Extension: push_back_placement(). 
    // Requires using placement new to construct an element.
    // Example: new(v.push_back_placement()) Element(param1, param2);
    void* push_back_placement()
    {
        return alloc_back();
    }

    void pop_back()
    {
        THOR_ASSERT(!empty());
        if (!empty())
        {
            typetraits<T>::destruct(&m_elements[--m_size]);
        }
    }

    void pop_back_delete()
    {
        THOR_ASSERT(!empty());
        if (!empty())
        {
            --m_size;
            delete m_elements[m_size];
            typetraits<T>::destruct(&m_elements[m_size]);
        }
    }

    // O(1) swap with another vector of the same type, unless preallocated
    // vectors are used, then swap() requires O(n)
    void swap(vector& V)
    {
        if (!can_swap())
        {
            make_swappable();
        }
        if (!V.can_swap())
        {
            V.make_swappable();
        }
        internal_swap(V);
    }

    // Clears and populates the vector with n copies of t.
    // Postcondition: size() == n.
    void assign(size_type n, const T& t)
    {
        clear();
        if (n > m_capacity)
        {
            reserve(n);
        }
        m_size = n;
        typetraits<T>::range_construct(m_elements, m_elements + m_size, t);
    }

    // Clears and populates the vector with the range [first, last). Does exact growth
    // if current capacity() is insufficient.
    template <typename InputIterator> void assign(InputIterator first, InputIterator last)
    {
        clear();
        size_type size = distance(first, last);
        if (size > m_capacity)
        {
            reserve(size);
        }
        pointer pos = m_elements;
        while (first != last)
        {
            typetraits<T>::construct(pos, *first);
            ++first, ++pos;
        }
        m_size = size;
    }

    // Insert elements anywhere in the vector.
    iterator insert(iterator pos)
    {
        typetraits<T>::construct(internal_insert(pos));
        return pos;
    }
    template <class T1> iterator insert(iterator pos, const T1& t1)
    {
        typetraits<T>::construct(internal_insert(pos), t1);
        return pos;
    }
    template <class T1, class T2> iterator insert(iterator pos, const T1& t1, const T2& t2)
    {
        typetraits<T>::construct(internal_insert(pos), t1, t2);
        return pos;
    }
    template <class T1, class T2, class T3> iterator insert(iterator pos, const T1& t1, const T2& t2, const T3& t3)
    {
        typetraits<T>::construct(internal_insert(pos), t1, t2, t3);
        return pos;
    }
    template <class T1, class T2, class T3, class T4> iterator insert(iterator pos, const T1& t1, const T2& t2, const T3& t3, const T4& t4)
    {
        typetraits<T>::construct(internal_insert(pos), t1, t2, t3, t4);
        return pos;
    }

    // Extension: insert_placement(). 
    // Requires using placement new to construct an element.
    // Example: new(v.insert_placement(pos)) Element(param1, param2);
    void* insert_placement(iterator pos)
    {
        return internal_insert(pos);
    }
    
    // Inserts the range [first, last) at the location indicated by pos
    template <typename InputIterator> void insert(iterator pos, InputIterator first, InputIterator last)
    {
        verify_iterator(pos);
        pos.verify_range(true);
        difference_type new_elements = thor::distance(first, last);
        THOR_ASSERT(new_elements >= 0);
        if (m_size + new_elements > m_capacity)
        {
            ptrdiff_t index = pos.m_element - m_elements;
            growby(new_elements);
            pos.m_element = m_elements + index;
        }
        
        typetraits<T>::range_construct(m_elements + m_size, m_elements + m_size + new_elements, T());
        typetraits<T>::copy_backwards(pos.m_element + new_elements, pos.m_element, m_elements + m_size - pos.m_element);
        m_size += new_elements;
        while (first != last)
        {
            *pos = *first;
            ++pos, ++first;
        }
    }

    // Inserts n copies of t at the location indicated by pos
    void insert(iterator pos, size_type n, const T& t)
    {
        verify_iterator(pos);
        pos.verify_range(true);
        if (m_size + n > m_capacity)
        {
            ptrdiff_t index = pos.m_element - m_elements;
            growby(n);
            pos.m_element = m_elements + index;
        }

        typetraits<T>::range_construct(m_elements + m_size, m_elements + m_size + n, T());
        typetraits<T>::copy_backwards(pos.m_element + n, pos.m_element, m_elements + m_size - pos.m_element);
        typetraits<T>::range_copy(pos.m_element, pos.m_element + n, t);

        m_size += n;
    }

    // Erases the element at the position indicated by pos.
    // Requires O(n) time. Consider using swap_and_pop() if possible.
    iterator erase(iterator pos)
    {
        verify_iterator(pos);
        pos.verify_range();
        typetraits< T >::copy_overlap(pos.m_element, pos.m_element + 1, end_ptr() - (pos.m_element + 1));
        typetraits< T >::destruct(m_elements + --m_size);
        return pos;
    }

    // Erases the element(s) in the range [first, last)
    // Requires O(n) time.
    iterator erase(iterator first, iterator last)
    {
        verify_iterator(first);
        first.verify_range(true);
        verify_iterator(last);
        last.verify_range(true);
        THOR_ASSERT(first.m_element <= last.m_element);

        if(first.m_element < last.m_element)
        {
            const size_type num_after = end_ptr() - last.m_element;
            typetraits< T >::copy_overlap(first.m_element, last.m_element, num_after);
            T * const new_end = first.m_element + num_after;
            typetraits< T >::range_destruct(new_end, m_elements + m_size);
            m_size = new_end - m_elements;
        }

        return first;
    }

    // Erases all items. Note that this does not free the memory. Use reduce() to free the memory.
    void clear()
    {
        typetraits<T>::range_destruct(m_elements, m_elements + m_size);
        m_size = 0;
    }
    
    // Changes the size of the vector to be the requested size. Will perform exponential growth.
    // If exact growth is prefered, call reserve() before calling resize().
    // Postcondition: size() == new_len
    void resize(size_type new_len)
    {
        if(new_len > m_size)
        {
            if (new_len > m_capacity)
            {
                growby(new_len - m_capacity);
            }

            T *new_end = m_elements + new_len;
            typetraits<T>::range_construct(m_elements + m_size, new_end);
            m_size = new_len;
        }
        else
        {
            T *new_end = m_elements + new_len;
            typetraits<T>::range_destruct(new_end, end_ptr());
            m_size = new_len;
        }
    }

    // Changes the size of the vector to be the requested size by appending copies of t if necessary.
    // Will perform exponential growth. If exact growth is prefered, call reserve() before
    // calling resize().
    // Postcondition: size() == new_len.
    void resize(size_type new_len, const T& t)
    {
        if(new_len > m_size)
        {
            if(new_len > m_capacity)
            {
                growby(new_len - m_capacity);
            }

            T *new_end = m_elements + new_len;
            typetraits<T>::range_construct(m_elements + m_size, new_end, t);
            m_size = new_len;
        }
        else
        {
            T *new_end = m_elements + new_len;
            typetraits<T>::range_destruct(new_end, end_ptr());
            m_size = new_len;
        }
    }

    // Extensions:

    // Reduces capacity() to max(n, size()).  This is done by reallocating the underlying
    // memory.  If empty() and n is zero, the memory is freed.
    void reduce(size_type n = 0)
    {
        if (n < size())
        {
            n = size();
        }
        if (n == 0)
        {
            typetraits<T>::range_destruct(m_elements, end_ptr());
            dealloc(m_elements);
            m_elements = 0;
            m_capacity = 0;
        }
        else if (n != capacity())
        {
            pointer new_elements = alloc(n, m_capacity);
            if (new_elements != m_elements)
            {
                typetraits<T>::range_move(new_elements, new_elements + m_size, m_elements);
                dealloc(m_elements);
                m_elements = new_elements;
            }
        }
    }

    // Calls delete on every element. Only valid if T is a pointer type.
    // Postcondition: size() == 0
    void delete_all()
    {
        iterator first = begin(), last = end();
        while (first != last)
        {
            delete *first;
            ++first;
        }
        clear();
    }

    // Calls erase and delete on the element indicated by pos. Only valid if T is a pointer type.
    void erase_and_delete(iterator pos)
    {
        verify_iterator(pos);
        pos.verify_range();
        delete *pos;
        erase(pos);
    }

    // Removes an object by swapping with the last element and popping the last element.
    void swap_and_pop(size_type n)
    {
        THOR_DEBUG_ASSERT(n < size());
        thor::swap(back(), m_elements[n]);
        pop_back();
    }

    virtual bool can_swap() const
    {
        return true;
    }

protected:
    enum { alignment = memory::align_selector<T>::alignment };
    typedef memory::align_alloc<T> align_alloc;

    pointer   m_elements;
    size_type m_size;
    size_type m_capacity;

    virtual pointer alloc(size_type requested, size_type& actual)
    {
        actual = requested;
        return align_alloc::alloc(requested);
    }

    virtual void    dealloc(pointer p)
    {
        align_alloc::free(p);
    }

    // Grows exponentially by max(capacity + n, capacity + 1/2 capacity)
    void growby(size_type n)
    {
        size_type new_capacity = thor::_max(m_capacity + n, m_capacity + (m_capacity >> 1));
        pointer new_elements = alloc(new_capacity, m_capacity);
        typetraits<T>::range_move(new_elements, new_elements + m_size, m_elements);
        dealloc(m_elements);
        m_elements = new_elements;
    }

    // Return pointers to the end element
    T* end_ptr()
    {
        return &m_elements[m_size];
    }

    const T* end_ptr() const
    {
        return &m_elements[m_size];
    }

    // Reserves space for an element at the back, but does not construct it.
    T* alloc_back()
    {
        if (m_size == m_capacity)
        {
            growby(1);
        }
        return &m_elements[m_size++];
    }

    // Inserts space for an element at the position indicated by pos, but does
    // not construct it. If an element was already present at that location it
    // is destructed.
    T* internal_insert(iterator& pos)
    {
        verify_iterator(pos);
        pos.verify_range(true);
        if (m_size == m_capacity)
        {
            difference_type index = pos.m_element - m_elements;
            growby(1);
            pos.m_element = m_elements + index;
        }

        if (pos.m_element != end_ptr())
        {
            typetraits<T>::construct(end_ptr());
            typetraits<T>::copy_backwards(pos.m_element + 1, pos.m_element, end_ptr() - pos.m_element);
            typetraits<T>::destruct(pos.m_element);
        }

        ++m_size;

        return pos.m_element;
    }

    void internal_swap(vector& V)
    {
        THOR_DEBUG_ASSERT(can_swap() && V.can_swap());
        thor::swap(m_elements, V.m_elements);
        thor::swap(m_size,     V.m_size    );
        thor::swap(m_capacity, V.m_capacity);
    }

    void make_swappable()
    {
        THOR_DEBUG_ASSERT(!can_swap());
        vector<T,0> v(*this);
        typetraits<T>::range_destruct(m_elements, end_ptr());
        dealloc(m_elements);
        m_elements = 0;
        m_size = m_capacity = 0;
        THOR_DEBUG_ASSERT(can_swap());
        internal_swap(v);
        THOR_DEBUG_ASSERT(can_swap());
    }

    void verify_iterator(const iterator_base& i) const { THOR_UNUSED(i); THOR_ASSERT(i.m_vector == this); }
};

// The vector class that allows preallocation. Inherits from the base vector class
// so that it can be used in calls that require vector<T>.
template <typename T, unsigned T_PREALLOC> class vector : public vector<T, 0>
{
    typedef vector<T, 0> baseclass;
public:
    typedef typename baseclass::pointer pointer;
    typedef typename baseclass::size_type size_type;

    // Constructors must default-construct and then allocate once v-table is set up.
    vector() : baseclass()
    {}
    
    vector(size_type n) : baseclass()
    {
        resize(n);
    }

    vector(size_type n, const T& t) : baseclass()
    {
        assign(n, t);
    }

    vector(const baseclass& V) : baseclass()
    {
        baseclass::operator = (V);
    }

    template <typename InputIterator> vector(InputIterator first, InputIterator last) : baseclass()
    {
        assign(first, last);
    }

    virtual ~vector()
    {
        clear();
        dealloc(m_elements);
        m_elements = 0;
    }

    vector& operator = (const baseclass& V)
    {
        baseclass::operator = (V);
        return *this;
    }

    vector& operator = (const vector& V)
    {
        baseclass::operator = (V);
        return *this;
    }

    virtual bool can_swap() const
    {
        return !is_using_prealloc(m_elements);
    }

protected:
    virtual pointer alloc(size_type requested, size_type& actual)
    {
        if (requested <= T_PREALLOC)
        {
            actual = T_PREALLOC;
            return (pointer)memory::align_forward<baseclass::alignment>(m_prealloc);
        }
        return baseclass::alloc(requested, actual);
    }

    virtual void    dealloc(pointer p)
    {
        if (is_using_prealloc(p))
        {
            // Do nothing, using our preallocated space
        }
        else
        {
            baseclass::dealloc(p);
        }
    }

private:
    thor_byte m_prealloc[T_PREALLOC * sizeof(T) + baseclass::alignment];

    bool is_using_prealloc(pointer p) const
    {
        return p >= (pointer)m_prealloc && p <= (pointer)(m_prealloc + baseclass::alignment);
    }
};

// Swap specializations
template <class T, size_type U> void swap(vector<T, U>& lhs, vector<T, U>& rhs)
{
    lhs.swap(rhs);
}

template <class T, size_type U, size_type V> void swap(vector<T, U>& lhs, vector<T, V>& rhs)
{
    lhs.swap(rhs);
}

} // namespace thor

// Global comparator functions
template <class T, class U> bool operator == (const thor::vector<T,0>& v1, const thor::vector<U,0>& v2)
{
    return v1.size() == v2.size() && thor::equal(v1.begin(), v1.end(), v2.begin());
}

template <class T, class U> bool operator != (const thor::vector<T,0>& v1, const thor::vector<U,0>& v2)
{
    return !(v1 == v2);
}

template <class T, class U> bool operator < (const thor::vector<T,0>& v1, const thor::vector<U,0>& v2)
{
    return thor::lexicographical_compare(v1.begin(), v1.end(), v2.begin(), v2.end());
}

template <class T, class U> bool operator > (const thor::vector<T,0>& v1, const thor::vector<U,0>& v2)
{
    return thor::lexicographical_compare(v1.begin(), v1.end(), v2.begin(), v2.end(), thor::greater<T>());
}

template <class T, class U> bool operator <= (const thor::vector<T,0>& v1, const thor::vector<U,0>& v2)
{
    return !(v1 > v2);
}

template <class T, class U> bool operator >= (const thor::vector<T,0>& v1, const thor::vector<U,0>& v2)
{
    return !(v1 < v2);
}

#endif

