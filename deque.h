/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * deque.h
 *
 * This file defines an STL-compatible deque (double-ended queue) container.
 *
 * Extensions/Changes:
 * - Blocks of 256 items are allocated as necessary.
 * - Debug builds check that all iterators are valid (i.e. no trying to erase using an iterator from a different container).
 * - push_back() has changed but remains compatible with STL usage:
 *   * push_back() returns a reference to the added item
 *   * push_back() with zero parameters will default-construct an element
 *   * push_back() variations exist with 1-4 parameters that will in-place
 *     construct an element without necessarily needing a copy constructor.
 *   * push_back_placement() can be used with placement new to construct 
 *     elements with more than 4 parameters.
 * - push_front() has changed but remains compatible with STL usage:
 *   * push_front() returns a reference to the added item
 *   * push_front() with zero parameters will default-construct an element
 *   * push_front() variations exist with 1-4 parameters that will in-place
 *     construct an element without necessarily needing a copy constructor.
 *   * push_front_placement() can be used with placement new to construct 
 *     elements with more than 4 parameters.
 * - insert() has changed but remains compatible with STL usage:
 *   * insert(pos) will default-construct an element at pos
 *   * insert(pos,...) variations with 1-4 additional parameters will
 *     in-place construct an element at pos without necessarily needing a
 *     copy constructor.
 *   * insert_placement(pos) can be used with placement new to construct
 *     elements with more than 4 parameters.
 * - get_contiguous(pos, count):
 *   * returns a pointer to a contiguous block of memory. count is an output parameter
 *     that receives the size of the contiguous block.
 * - Assistance for raw pointer types:
 *   * delete_all() will call delete on every element and clear() the container.
 *   * erase_and_delete() can be used to delete an element and erase it from
 *     the list.
 *   * pop_front_delete() will delete the first element and pop it from the container.
 *   * pop_back_delete() will delete the last element and pop it from the container.
 * - O(1) size() function (spec says that size() may be O(n))
 */

#ifndef THOR_DEQUE_H
#define THOR_DEQUE_H
#pragma once

#ifndef THOR_VECTOR_H
#include "vector.h"
#endif

namespace thor
{

template <class T>
class deque
{
    struct deque_node;

public:
    typedef T value_type;
    typedef T* pointer;
    typedef T& reference;
    typedef const T* const_pointer;
    typedef const T& const_reference;
    typedef thor_size_type size_type;
    typedef thor_diff_type difference_type;

    // extension: the number of elements allocated at once
    enum { block_count = 256 };

    // iterator definitions
    struct iterator_base : public iterator_type<random_access_iterator_tag, T>
    {
        deque_node* m_node;
        T*          m_value;
#ifdef THOR_DEBUG
        const deque*      m_owner;
        iterator_base(deque_node* n, T* v, const deque* o) : m_node(n), m_value(v), m_owner(o) {}
        void verify_iterator() const     { m_owner->verify_iterator(*this); }
#else
        iterator_base(deque_node* n, T* v, const deque*) : m_node(n), m_value(v) {}
        void verify_iterator() const     {}
#endif
        void verify_not_end() const { THOR_DEBUG_ASSERT(m_node && m_value); }
        
        void decr(size_type i)
        {
            THOR_DEBUG_ASSERT(m_node != 0);
            THOR_DEBUG_ASSERT(m_value == 0 || m_node->is_valid(m_value));
            if (m_node->size() == 0 && i != 0)
            {
                m_node = m_node->prev;
                THOR_DEBUG_ASSERT(m_node->size() != 0); // Will happen if the deque is empty
                m_value = m_node->end();
            }

            while (i != 0)
            {
                size_type cur = min<size_type>(i, (m_value - m_node->start()) + 1);
                m_value -= cur;
                i -= cur;
                if (m_value < m_node->start())
                {
                    m_node = m_node->prev;
                    if (m_node->size() != 0)
                    {
                        m_value = m_node->end() - 1;
                    }
                    else
                    {
                        // Hit the end() node; should be done now.
                        m_value = 0;
                        THOR_DEBUG_ASSERT(i == 0);
                    }
                }
            }
        }

        void decr()
        {
            THOR_DEBUG_ASSERT(m_node != 0);
            THOR_DEBUG_ASSERT(m_value == 0 || m_node->is_valid(m_value));
            if (m_value == 0 || --m_value < m_node->start())
            {
                m_node = m_node->prev;
                if (m_node->size() != 0)
                {
                    m_value = m_node->end() - 1;
                }
                else
                {
                    m_value = 0;
                }
            }
        }
        
        void incr(size_type i)
        {
            THOR_DEBUG_ASSERT(m_node != 0);
            THOR_DEBUG_ASSERT(m_value == 0 || m_node->is_valid(m_value));
            if (m_node->size() == 0)
            {
                m_node = m_node->next;
                THOR_DEBUG_ASSERT(m_node->size() != 0); // Will happen if the deque is empty
                m_value = m_node->start();
            }
            while (i != 0)
            {
                size_type cur = min<size_type>(i, m_node->end() - m_value);
                m_value += cur;
                i -= cur;
                if (m_value == m_node->end())
                {
                    m_node = m_node->next;
                    if (m_node->size() != 0)
                    {
                        m_value = m_node->start();
                    }
                    else
                    {
                        // Hit the end() node; should be done now.
                        m_value = 0;
                        THOR_DEBUG_ASSERT(i == 0);
                    }
                }
            }
        }
        
        void incr()
        {
            THOR_DEBUG_ASSERT(m_node != 0);
            THOR_DEBUG_ASSERT(m_value == 0 || m_node->is_valid(m_value));
            if (m_value == 0 || ++m_value == m_node->end())
            {
                m_node = m_node->next;
                if (m_node->size() != 0)
                {
                    m_value = m_node->start();
                }
                else
                {
                    m_value = 0;
                }
            }
        }
        
        void mod(difference_type i)
        {
            if (i > 0)
            {
                incr((size_type)i);
            }
            else
            {
                decr((size_type)-i);
            }
        }

    private:
        static difference_type internal_diff(const iterator_base& high, const iterator_base& low)
        {
            THOR_DEBUG_ASSERT(high.m_node != low.m_node);

            difference_type val = high.m_node->size() == 0 ? 0 : high.m_value - high.m_node->start();
            val += (low.m_node->size() == 0 ? 0 : low.m_node->end() - low.m_value);
            
            deque_node* node = high.m_node->prev;
            while (node != low.m_node)
            {
                THOR_DEBUG_ASSERT(node->size() != 0); // Terminator should never be encountered.
                if (node->size() == 0)
                {
                    return val;
                }

                val += node->size();
                node = node->prev;
            }

            return val;
        }

    public:
        difference_type diff(const iterator_base& rhs) const
        {
            THOR_DEBUG_ASSERT(m_owner == rhs.m_owner);
            THOR_DEBUG_ASSERT(m_node != 0 && rhs.m_node != 0);
            
            if (m_node == rhs.m_node)
            {
                // Best case: both iterators reference values in the same node
                return m_value - rhs.m_value;
            }

            // Optimization if either iterator is the terminator
            if (m_node->size() == 0)
            {
                return internal_diff(*this, rhs);
            }
            else if (rhs.m_node->size() == 0)
            {
                return -internal_diff(rhs, *this);
            }

            // Need to figure out which iterator is higher than the previous one. Walk forward
            // until either the given node is encountered or the terminator. This should tell
            // us the order.
            deque_node* node = m_node->prev;
            difference_type diffnodes = 0;
            for (;;)
            {
                if (node->size() == 0)
                {
                    // Encountered the terminator, so the given node must be higher (or unrelated)
                    return -internal_diff(rhs, *this);
                }
                if (node == rhs.m_node)
                {
                    // Encountered the rhs node, so now we can calculate the size difference.
                    difference_type val = diffnodes * block_count;
                    val += (m_value - m_node->start());
                    val += (rhs.m_node->end() - rhs.m_value);
                    return val;
                }
                ++diffnodes;
                node = node->prev;
            }
        }

        bool operator == ( const iterator_base& i ) const
        {
            THOR_DEBUG_ASSERT(m_owner == i.m_owner);
            THOR_DEBUG_ASSERT(m_value == 0 || m_node->size() != 0);
            THOR_DEBUG_ASSERT(i.m_value == 0 || i.m_node->size() != 0);
            return m_value == i.m_value && m_node == i.m_node;
        }

        bool operator != ( const iterator_base& i ) const { return ! operator == (i); }
    };

    template<class Traits> class fwd_iterator : public iterator_base
    {
    public:
        typedef typename Traits::pointer pointer;
        typedef typename Traits::reference reference;
        typedef fwd_iterator<nonconst_traits<T> > nonconst_iterator;
        typedef fwd_iterator<Traits> selftype;

        fwd_iterator(deque_node* n = 0, T* v = 0, const deque* o = 0) : iterator_base(n, v, o) {}
        fwd_iterator(const nonconst_iterator& i) : iterator_base(i) {}
        selftype&  operator = (const nonconst_iterator& i)  { iterator_base::operator = (i); return *this; }
        reference  operator * ()  const                     { verify_not_end(); verify_iterator(); return *m_value; }
        pointer    operator -> () const                     { return &(operator*()); }
        selftype   operator - (difference_type i) const     { selftype n(*this); n.mod(-i); return n; }
        selftype&  operator -= (difference_type i)          {                    mod(-i);   return *this; }
        selftype&  operator -- ()     /* --iterator */      {                    decr();    return *this; }
        selftype   operator -- (int)  /* iterator-- */      { selftype n(*this); decr();    return n; }
        selftype   operator + (difference_type i) const     { selftype n(*this); n.mod(i);  return n; }
        selftype&  operator += (difference_type i)          {                    mod(i);    return *this; }
        selftype&  operator ++ ()     /* ++iterator */      {                    incr();    return *this; }
        selftype   operator ++ (int)  /* iterator++ */      { selftype n(*this); incr();    return n; }

        difference_type operator - (const selftype& i) const { return diff(i); }
    };

    template<class Traits> class rev_iterator : public iterator_base
    {
    public:
        typedef typename Traits::pointer pointer;
        typedef typename Traits::reference reference;
        typedef rev_iterator<nonconst_traits<T> > nonconst_iterator;
        typedef rev_iterator<Traits> selftype;

        rev_iterator(deque_node* n = 0, T* v = 0, const deque* o = 0) : iterator_base(n, v, o) {}
        rev_iterator(const nonconst_iterator& i) : iterator_base(i) {}
        selftype&  operator = (const nonconst_iterator& i)  { iterator_base::operator = (i); return *this; }
        reference  operator * ()  const                     { verify_not_end(); verify_iterator(); return *m_value; }
        pointer    operator -> () const                     { return &(operator*()); }
        selftype   operator - (difference_type i) const     { selftype n(*this); n.mod(i);  return n; }
        selftype&  operator -= (difference_type i)          {                    mod(i);    return *this; }
        selftype&  operator -- ()     /* --iterator */      {                    incr();    return *this; }
        selftype   operator -- (int)  /* iterator-- */      { selftype n(*this); incr();    return n; }
        selftype   operator + (difference_type i) const     { selftype n(*this); n.mod(-i); return n; }
        selftype&  operator += (difference_type i)          {                    mod(-i);   return *this; }
        selftype&  operator ++ ()     /* ++iterator */      {                    decr();    return *this; }
        selftype   operator ++ (int)  /* iterator++ */      { selftype n(*this); decr();    return n; }

        difference_type operator - (const selftype& i) const { return i.diff(*this); }
    };

    typedef fwd_iterator<nonconst_traits<T> > iterator;
    typedef fwd_iterator<const_traits<T>    > const_iterator;

    typedef rev_iterator<nonconst_traits<T> > reverse_iterator;
    typedef rev_iterator<const_traits<T>    > const_reverse_iterator;

    // constructors
    deque() :
        m_head(terminator(), terminator(), 0, 0),
        m_size(0)
    {}

    deque(size_type n) :
        m_head(terminator(), terminator(), 0, 0),
        m_size(n)
    {
        if (n != 0)
        {
            // Round up
            const size_type nodecount = ((n + (block_count-1)) / block_count);
            m_nodes.reserve(nodecount);
            for (size_type i = 0; i != nodecount; ++i )
            {
                const size_type elems = min<size_type>(block_count, n);
                n -= elems;

                deque_node* node = insert_node(terminator(), 0, elems);
                typetraits<T>::range_construct(node->start(), node->end());

                m_nodes.push_back(node);
            }
        }
    }

    deque(size_type n, const T& t) :
        m_head(terminator(), terminator(), 0, 0),
        m_size(n)
    {
        if (n != 0)
        {
            // Round up
            const size_type nodecount = ((n + (block_count-1)) / block_count);
            m_nodes.reserve(nodecount);
            for (size_type i = 0; i != nodecount; ++i )
            {
                const size_type elems = min<size_type>(block_count, n);
                n -= elems;

                deque_node* node = insert_node(terminator(), 0, elems);
                typetraits<T>::range_construct(node->start(), node->end(), t);

                m_nodes.push_back(node);
            }
        }
    }

    deque(const deque& D) :
        m_head(terminator(), terminator(), 0, 0),
        m_size(0)
    {
        operator = (D);
    }

    
    template <class InputIterator> deque(InputIterator first, InputIterator last) :
        m_head(terminator(), terminator(), 0, 0),
        m_size(0)
    {
        insert(end(), first, last);
    }
    
    ~deque()
    {
        clear();
    }

    template <class InputIterator> void assign(InputIterator first, InputIterator last)
    {
        // TODO: This could probably be done better (re-use existing nodes, etc)
        clear();
        insert(end(), first, last);
    }

    deque& operator = (const deque& D)
    {
        clear();
        m_size = D.m_size;
        m_nodes.reserve(D.m_nodes.size());

        for (size_type i = 0; i != D.m_nodes.size(); ++i)
        {
            const deque_node* rhsnode = D.m_nodes[i];
            deque_node* lhsnode = insert_node(terminator(), rhsnode->startindex, rhsnode->endindex);
            THOR_DEBUG_ASSERT(rhsnode->size() == lhsnode->size());
            typetraits<T>::range_construct(lhsnode->start(), lhsnode->end(), rhsnode->start());

            m_nodes.push_back(lhsnode);
        }
        return *this;
    }

    size_type size() const
    {
        return m_size;
    }

    size_type max_size() const
    {
        return size_type(-1);
    }

    bool empty() const
    {
        return m_size == 0;
    }

    // element access
    T& operator [] (size_type n)
    {
        return *element(n);
    }

    const T& operator [] (size_type n) const
    {
        return *element(n);
    }

    T& at(size_type n)
    {
        return *element(n);
    }

    const T& at(size_type n) const
    {
        return *element(n);
    }

    T& front()
    {
        THOR_ASSERT(!empty());
        return *element(0);
    }
    
    const T& front() const
    {
        THOR_ASSERT(!empty());
        return *element(0);
    }
    
    T& back()
    {
        THOR_ASSERT(!empty());
        return *element(m_size - 1);
    }
    
    const T& back() const
    {
        THOR_ASSERT(!empty());
        return *element(m_size - 1);
    }

    // extension that returns a pointer to the contiguous space at pos.
    // the count parameter returns the number of items available.
    // get_contiguous(end()) returns a null pointer with a count of zero.
    T* get_contiguous(iterator pos, size_type& count)
    {
        verify_iterator(pos);
        if (pos.m_node != terminator())
        {
            count = pos.m_node->end() - pos.m_value;
            return pos.m_value;
        }

        count = 0;
        return 0;
    }
    const T* get_contiguous(const_iterator pos, size_type& count) const
    {
        verify_iterator(pos);
        if (pos.m_node != terminator())
        {
            count = pos.m_node->end() - pos.m_value;
            return pos.m_value;
        }

        count = 0;
        return 0;
    }

    // iteration
    iterator begin()
    {
        return iterator(m_head.next, empty() ? 0 : m_head.next->start(), this);
    }

    const_iterator begin() const
    {
        return const_iterator(m_head.next, empty() ? 0 : m_head.next->start(), this);
    }

    iterator end()
    {
        return iterator(terminator(), 0, this);
    }

    const_iterator end() const
    {
        return const_iterator(terminator(), 0, this);
    }

    reverse_iterator rbegin()
    {
        return reverse_iterator(m_head.prev, empty() ? 0 : m_head.prev->end() - 1, this);
    }

    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(m_head.prev, empty() ? 0 : m_head.prev->end() - 1, this);
    }

    reverse_iterator rend()
    {
        return reverse_iterator(terminator(), 0, this);
    }

    const_reverse_iterator rend() const
    {
        return const_reverse_iterator(terminator(), 0, this);
    }

    void swap(deque& D)
    {
        // Fixup terminators
        deque_node *&lhsn =   m_head.prev->next, *&lhsp =   m_head.next->prev;
        deque_node *&rhsn = D.m_head.prev->next, *&rhsp = D.m_head.next->prev;
        lhsn = lhsp = D.terminator();
        rhsn = rhsp = terminator();

        m_nodes.swap(D.m_nodes);
        thor::swap(m_head, D.m_head);
        thor::swap(m_size, D.m_size);
    }

    // element insertion
    T& push_front()
    {
        T* p = internal_push_front();
        typetraits<T>::construct(p);
        return *p;
    }
    template <class T1> T& push_front(const T1& t1)
    {
        T* p = internal_push_front();
        typetraits<T>::construct(p, t1);
        return *p;
    }
    template <class T1, class T2> T& push_front(const T1& t1, const T2& t2)
    {
        T* p = internal_push_front();
        typetraits<T>::construct(p, t1, t2);
        return *p;
    }
    template <class T1, class T2, class T3> T& push_front(const T1& t1, const T2& t2, const T3& t3)
    {
        T* p = internal_push_front();
        typetraits<T>::construct(p, t1, t2, t3);
        return *p;
    }
    template <class T1, class T2, class T3, class T4> T& push_front(const T1& t1, const T2& t2, const T3& t3, const T4& t4)
    {
        T* p = internal_push_front();
        typetraits<T>::construct(p, t1, t2, t3, t4);
        return *p;
    }
    // Requires use of placement new to construct the item
    // Example: new (d.push_front_placement()) Value;
    void* push_front_placement()
    {
        return internal_push_front();
    }

    T& push_back()
    {
        T* p = internal_push_back();
        typetraits<T>::construct(p);
        return *p;
    }
    template <class T1> T& push_back(const T1& t1)
    {
        T* p = internal_push_back();
        typetraits<T>::construct(p, t1);
        return *p;
    }
    template <class T1, class T2> T& push_back(const T1& t1, const T2& t2)
    {
        T* p = internal_push_back();
        typetraits<T>::construct(p, t1, t2);
        return *p;
    }
    template <class T1, class T2, class T3> T& push_back(const T1& t1, const T2& t2, const T3& t3)
    {
        T* p = internal_push_back();
        typetraits<T>::construct(p, t1, t2, t3);
        return *p;
    }
    template <class T1, class T2, class T3, class T4> T& push_back(const T1& t1, const T2& t2, const T3& t3, const T4& t4)
    {
        T* p = internal_push_back();
        typetraits<T>::construct(p, t1, t2, t3, t4);
        return *p;
    }
    // Requires use of placement new to construct the item
    // Example: new (d.push_back_placement()) Value;
    void* push_back_placement()
    {
        return internal_push_back();
    }

    iterator insert(iterator pos)
    {
        verify_iterator(pos);
        internal_insert(pos.m_node, pos.m_value);
        typetraits<T>::construct(pos.m_value);
        return pos;
    }
    template <class T1> iterator insert(iterator pos, const T1& t1)
    {
        verify_iterator(pos);
        internal_insert(pos.m_node, pos.m_value);
        typetraits<T>::construct(pos.m_value, t1);
        return pos;
    }
    template <class T1, class T2> iterator insert(iterator pos, const T1& t1, const T2& t2)
    {
        verify_iterator(pos);
        internal_insert(pos.m_node, pos.m_value);
        typetraits<T>::construct(pos.m_value, t1, t2);
        return pos;
    }
    template <class T1, class T2, class T3> iterator insert(iterator pos, const T1& t1, const T2& t2, const T3& t3)
    {
        verify_iterator(pos);
        internal_insert(pos.m_node, pos.m_value);
        typetraits<T>::construct(pos.m_value, t1, t2, t3);
        return pos;
    }
    template <class T1, class T2, class T3, class T4> iterator insert(iterator pos, const T1& t1, const T2& t2, const T3& t3, const T4& t4)
    {
        verify_iterator(pos);
        internal_insert(pos.m_node, pos.m_value);
        typetraits<T>::construct(pos.m_value, t1, t2, t3, t4);
        return pos;
    }
    // Requires use of placement new to construct the item
    // Example: new (d.insert_placement(pos)) Value;
    void* insert_placement(iterator pos)
    {
        verify_iterator(pos);
        internal_insert(pos.m_node, pos.m_value);
        return pos.m_value;
    }

    void insert(iterator pos, size_type n, const T& t)
    {
        verify_iterator(pos);

        if (n == 0)
        {
            return;
        }

        // Just grow the middle; could optimize for front/back
        pos.m_value = grow(pos.m_node, pos.m_value, n);

        // Loop and construct
        while (n != 0)
        {
            size_type remain = min<size_type>(n, pos.m_node->end() - pos.m_value);
            typetraits<T>::range_construct(pos.m_value, pos.m_value + remain, t);
            n -= remain;
            pos.m_node = pos.m_node->next;
            pos.m_value = pos.m_node->start();
        }

        validate();
    }

    template <class InputIterator> void insert(iterator pos, InputIterator first, InputIterator last)
    {
        verify_iterator(pos);

        const size_type n = distance(first, last);
        if (n == 0)
        {
            return;
        }

        // Just grow the middle; could optimize for front/back
        pos.m_value = grow(pos.m_node, pos.m_value, n);

        // Loop and construct
        while (first != last)
        {
            THOR_DEBUG_ASSERT(pos.m_node != terminator());
            typetraits<T>::construct(pos.m_value, *first);
            ++first, ++pos.m_value;

            if (pos.m_value == pos.m_node->end())
            {
                pos.m_node = pos.m_node->next;
                pos.m_value = pos.m_node->start();
            }
        }

        validate();
    }

    // element removal
    void pop_front()
    {
        THOR_ASSERT(!empty());
        if (!empty())
        {
            deque_node* node = m_head.next;
            THOR_DEBUG_ASSERT(node == m_nodes.front());
            THOR_DEBUG_ASSERT(node != terminator());
            node->destroy_start();
            if (node->size() == 0)
            {
                // Destroy the front node.
                node->prev->next = node->next;
                node->next->prev = node->prev;
                destroy_node(node);
                m_nodes.erase(m_nodes.begin());
            }
            --m_size;

            validate();
        }
    }

    // extension that deletes the front item and pops it from the deque
    // valid for containers of pointer types only
    void pop_front_delete()
    {
        delete front();
        pop_front();
    }

    void pop_back()
    {
        THOR_ASSERT(!empty());
        if (!empty())
        {
            deque_node* node = m_head.prev;
            THOR_DEBUG_ASSERT(node == m_nodes.back());
            THOR_DEBUG_ASSERT(node != terminator());
            node->destroy_end();
            if (node->size() == 0)
            {
                // Destroy the back node
                node->prev->next = node->next;
                node->next->prev = node->prev;
                destroy_node(node);
                m_nodes.pop_back();
            }
            --m_size;

            validate();
        }
    }

    // extension that deletes the back item and pops it from the deque
    // valid for containers of pointer types only
    void pop_back_delete()
    {
        delete back();
        pop_back();
    }

    void clear()
    {
        for (size_type i = 0; i != m_nodes.size(); ++i)
        {
            deque_node* node = m_nodes[i];
            THOR_DEBUG_ASSERT(node->size() != 0);
            destroy_node(node);
        }
        m_size = 0;
        m_head.next = m_head.prev = terminator();
        m_nodes.clear();
        validate();
    }

    // Extension that deletes all elements, followed by a clear().
    // Valid for deques of pointer types only
    void delete_all()
    {
        for (size_type i = 0; i != m_nodes.size(); ++i)
        {
            deque_node* node = m_nodes[i];
            for (T* p = node->start(); p != node->end(); ++p)
            {
                delete *p;
            }
            THOR_DEBUG_ASSERT(node->size() != 0);
            destroy_node(node);
        }
        m_size = 0;
        m_head.next = m_head.prev = terminator();
        m_nodes.clear();
        validate();
    }

    iterator erase(iterator pos)
    {
        THOR_ASSERT(!empty());
        verify_iterator(pos);
        pos.verify_not_end();

        --m_size;

        size_type nodeindex = find(m_nodes.begin(), m_nodes.end(), pos.m_node) - m_nodes.begin();
        if (nodeindex < (m_nodes.size() / 2))
        {
            // Closer to beginning
            deque_node* fromnode = pos.m_node;
            deque_node* tonode = fromnode;
            T* to = pos.m_value;
            const T* from = to - 1;
            for (;;)
            {
                if (from < fromnode->start())
                {
                    fromnode = fromnode->prev;
                    from = fromnode->end() - 1;
                    if (fromnode == terminator())
                    {
                        break;
                    }
                }
                if (to < tonode->start())
                {
                    tonode = tonode->prev;
                    to = tonode->end() - 1;
                    THOR_DEBUG_ASSERT(tonode != terminator());
                }
                *to = *from;
                --to, --from;
            }
            THOR_DEBUG_ASSERT(m_nodes.front() == tonode);
            tonode->destroy_start();
            if (tonode->size() == 0)
            {
                tonode->next->prev = tonode->prev;
                tonode->prev->next = tonode->next;
                m_nodes.erase(m_nodes.begin());
                destroy_node(tonode);
            }
        }
        else
        {
            // Closer to end
            deque_node* tonode = pos.m_node;
            deque_node* fromnode = tonode;
            T* to = pos.m_value;
            const T* from = to + 1;
            for (;;)
            {
                if (from == fromnode->end())
                {
                    fromnode = fromnode->next;
                    from = fromnode->start();
                    if (fromnode == terminator())
                    {
                        break;
                    }
                }
                if (to == tonode->end())
                {
                    tonode = tonode->next;
                    to = tonode->start();
                    THOR_DEBUG_ASSERT(tonode != terminator());
                }
                *to = *from;
                ++to, ++from;
            }
            THOR_DEBUG_ASSERT(m_nodes.back() == tonode);
            tonode->destroy_end();
            if (tonode->size() == 0)
            {
                tonode->next->prev = tonode->prev;
                tonode->prev->next = tonode->next;
                m_nodes.pop_back();
                destroy_node(tonode);
            }
        }

        validate();
        return m_nodes.empty() ? end() : pos;
    }

    // extension that deletes the element at pos followed by erasing it
    // valid for deques of pointer types only
    iterator erase_and_delete(iterator pos)
    {
        delete *pos;
        return erase(pos);
    }

    iterator erase(iterator first, iterator last)
    {
        verify_iterator(first);
        verify_iterator(last);

        if (first == last)
        {
            return first;
        }

        first.verify_not_end();

        if (last.m_node == terminator())
        {
            // ends at end(), no need to copy anything
            size_type removenodes = 0;
            deque_node* node = first.m_node;
            T* start = first.m_value;
            do 
            {
                const size_type num = node->end() - start;
                node->destroy_end(num);
                m_size -= num;
                deque_node* next = node->next;
                if (node->size() == 0)
                {
                    ++removenodes;
                    node->next->prev = node->prev;
                    node->prev->next = node->next;
                    destroy_node(node);
                }
                node = next;
                start = node->start();
            } while (node != terminator());

            m_nodes.erase(m_nodes.end() - removenodes, m_nodes.end());

            validate();
            return last;
        }

        // Guaranteed to not end at end().

        // Search the node array to approximate if we're closer to the front or back
        size_type nodeindex = find(m_nodes.begin(), m_nodes.end(), first.m_node) - m_nodes.begin();
        if (nodeindex < (m_nodes.size() / 2))
        {
            // Closer to front, does not end at end()
            deque_node* tonode = last.m_node;
            T* to = last.m_value;

            deque_node* fromnode = first.m_node;
            const T* from = first.m_value;
            
            // Copy backwards
            for (;;)
            {
                --to, --from;
                if (from < fromnode->start())
                {
                    fromnode = fromnode->prev;
                    if (fromnode == terminator())
                    {
                        break;
                    }
                    from = fromnode->end() - 1;
                }
                if (to < tonode->start())
                {
                    tonode = tonode->prev;
                    to = tonode->end() - 1;
                    THOR_DEBUG_ASSERT(tonode != terminator());
                }
                *to = *from;
            }
            
            size_type removenodes = 0;
            ++to;
            do
            {
                const size_type num = to - tonode->start();
                tonode->destroy_start(num);
                m_size -= num;
                deque_node* prev = tonode->prev;
                if (tonode->size() == 0)
                {
                    ++removenodes;
                    tonode->next->prev = tonode->prev;
                    tonode->prev->next = tonode->next;
                    destroy_node(tonode);
                }
                tonode = prev;
                to = tonode->end();
            } while (tonode != terminator());

            m_nodes.erase(m_nodes.begin(), m_nodes.begin() + removenodes);

            validate();
            return last;
        }
        else
        {
            // Closer to end, does not end at end()
            deque_node* tonode = first.m_node;
            T* to = first.m_value;

            deque_node* fromnode = last.m_node;
            const T* from = last.m_value;

            for (;;)
            {
                if (from == fromnode->end())
                {
                    fromnode = fromnode->next;
                    if (fromnode == terminator())
                    {
                        break;
                    }
                    from = fromnode->start();
                }
                if (to == tonode->end())
                {
                    tonode = tonode->next;
                    to = tonode->start();
                    THOR_DEBUG_ASSERT(tonode != terminator());
                }
                *to = *from;
                ++to, ++from;
            }

            size_type removenodes = 0;
            do
            {
                const size_type num = tonode->end() - to;
                tonode->destroy_end(num);
                m_size -= num;
                deque_node* next = tonode->next;
                if (tonode->size() == 0)
                {
                    ++removenodes;
                    tonode->next->prev = tonode->prev;
                    tonode->prev->next = tonode->next;
                    destroy_node(tonode);
                }
                tonode = next;
                to = tonode->start();
            } while (tonode != terminator());

            m_nodes.erase(m_nodes.end() - removenodes, m_nodes.end());

            validate();
            return first;
        }
    }

    void resize(size_type n)
    {
        if (size() < n)
        {
            deque_node* node;
            T* p = growback(node, n - size());
            do 
            {
                typetraits<T>::range_construct(p, node->end());
                node = node->next;
                p = node->start();
            } while (node != terminator());
        }
        else
        {
            erase_last(size() - n);
        }
        validate();
    }

    void resize(size_type n, const T& t)
    {
        if (size() < n)
        {
            deque_node* node;
            T* p = growback(node, n - size());
            do 
            {
                typetraits<T>::range_construct(p, node->end(), t);
                node = node->next;
                p = node->start();
            } while (node != terminator());
        }
        else
        {
            erase_last(size() - n);
        }
        validate();
    }

private:
    void verify_iterator(const iterator_base& b) const
    {
        THOR_UNUSED(b);
        THOR_DEBUG_ASSERT(b.m_node != 0); // Must have a valid node
        THOR_DEBUG_ASSERT(b.m_owner == this);  // Owner must match
        THOR_DEBUG_ASSERT(b.m_node == terminator() || b.m_value != 0); // Null values are only allowed for the terminator
        THOR_DEBUG_ASSERT(b.m_node == terminator() || b.m_node->size() != 0); // Zero size are only allowed for the terminator
        
        // Node must be found in our node array
        THOR_DEBUG_ASSERT(b.m_node == terminator() || find(m_nodes.begin(), m_nodes.end(), b.m_node) != m_nodes.end());

        // b.m_value must be a valid member of b.m_node
        THOR_DEBUG_ASSERT(b.m_node == terminator() || b.m_node->is_valid(b.m_value));
    }

    // Debugging
    void validate() const
    {
#if 0
        THOR_DEBUG_ASSERT(m_head.prev->next == terminator());
        THOR_DEBUG_ASSERT(m_head.next->prev == terminator());

        if (m_head.prev != terminator())
        {
            if (m_head.prev != m_head.next)
            {
                THOR_DEBUG_ASSERT(m_head.prev->startindex == 0);
                THOR_DEBUG_ASSERT(m_head.next->endindex == block_count);
            }
        }

        size_type s = 0;
        deque_node* node = m_head.next;
        while (node != terminator())
        {
            THOR_DEBUG_ASSERT((node->startindex == 0 && node->endindex == block_count) || node == m_head.next || node == m_head.prev);
            s += node->size();
            node = node->next;
        }
        THOR_DEBUG_ASSERT(s == m_size);
#endif
    }

    void erase_last(size_type count)
    {
        THOR_DEBUG_ASSERT(count <= size());
        size_type removecount = 0;
        while (count != 0)
        {
            deque_node* node = m_head.prev;
            THOR_DEBUG_ASSERT(node != terminator());
            const size_type num = min(count, node->size());
            node->destroy_end(num);
            m_size -= num;
            count -= num;
            deque_node* prev = node->prev;
            if (node->size() == 0)
            {
                node->next->prev = node->prev;
                node->prev->next = node->next;
                destroy_node(node);
                ++removecount;
            }
            node = prev;
        }
        m_nodes.erase(m_nodes.end() - removecount, m_nodes.end());
    }

    T* internal_push_front()
    {
        growfront(1);
        THOR_DEBUG_ASSERT(m_head.next != terminator() && m_head.next->size() != 0);
        validate();
        return m_head.next->start();
    }

    T* internal_push_back()
    {
        deque_node* node;
        growback(node, 1);
        THOR_DEBUG_ASSERT(m_head.prev != terminator() && m_head.prev->size() != 0);
        validate();
        return node->end() - 1;
    }

    void internal_insert(deque_node*& node, T*& e)
    {
        if (node == terminator())
        {
            // insert at back
            THOR_DEBUG_ASSERT(e == 0);
            e = internal_push_back();
            node = m_head.prev;
        }
        else if (node == m_head.next && e == node->start())
        {
            // insert at front
            e = internal_push_front();
            node = m_head.next;
        }
        else
        {
            e = grow(node, e, 1);
        }

        validate();
    }

    // Generic grow function that allows inserting 'n' elements anywhere in the deque.
    // When this function returns, node and return value point to the start of 'n'
    // elements that have not been constructed. m_size has been adjusted.
    T* grow(deque_node*& node, T* p, size_type n)
    {
        // Insert into the middle.
        THOR_DEBUG_ASSERT(n != 0);

        // Switch to using growback() if inserting at the end.
        if (node == terminator())
        {
            THOR_DEBUG_ASSERT(p == 0);
            return growback(node, n);
        }

        // At this point, we're guaranteed to have nodes and that we
        // are inserting before the terminator.
        THOR_DEBUG_ASSERT(!m_nodes.empty());
        THOR_DEBUG_ASSERT(node != terminator());

        // Must actually search the array of nodes for our node to find out
        // whether we're closer to the front or back. Note that this is an
        // approximation.
        size_type index = find(m_nodes.begin(), m_nodes.end(), node) - m_nodes.begin();
        if (index < (m_nodes.size() / 2))
        {
            // Closer to front
            deque_node* fromnode = m_nodes.front();
            T* from = fromnode->start();

            node = growfront(n);
            T* to = node->start();

            while (from != p)
            {
                typetraits<T>::construct(to, *from);
                typetraits<T>::destruct(from);
                ++to, ++from;
                if (from == fromnode->end())
                {
                    fromnode = fromnode->next;
                    THOR_DEBUG_ASSERT(fromnode != terminator());
                    from = fromnode->start();
                }
                if (to == node->end())
                {
                    node = node->next;
                    THOR_DEBUG_ASSERT(fromnode != terminator());
                    to = node->start();
                }
            }

            return to;
        }
        else
        {
            // Closer to back
            deque_node* fromnode = m_head.prev;
            T* from = fromnode->end();

            deque_node* dummy;
            growback(dummy, n);

            deque_node* tonode = m_head.prev;
            T* to = tonode->end();

            // Must copy backwards
            while (from != p)
            {
                --from, --to, --n;
                if (from < fromnode->start())
                {
                    fromnode = fromnode->prev;
                    THOR_DEBUG_ASSERT(fromnode != terminator());
                    from = fromnode->end() - 1;
                }
                if (to < tonode->start())
                {
                    tonode = tonode->prev;
                    THOR_DEBUG_ASSERT(tonode != terminator());
                    to = tonode->end() - 1;
                }
                typetraits<T>::construct(to, *from);
                typetraits<T>::destruct(from);
            }

            return p;
        }
    }

    // Adds n elements to the front (which will change indices for everything else)
    // New elements have not been constructed. The starting node is always accessible through 
    // m_root.next or m_nodes.front() (or the return value). m_size is adjusted.
    deque_node* growfront(size_type n)
    {
        THOR_ASSERT(n != 0);

        m_size += n;

        // Consume elements from the first node if possible
        if (!m_nodes.empty() && m_nodes.front()->startindex != 0)
        {
            deque_node* first = m_nodes.front();
            size_type num = min(n, first->startindex);
            first->startindex -= num;
            n -= num;

            if (n == 0)
            {
                return first;
            }
        }

        size_type needed = (n + block_count - 1) / block_count;
        THOR_DEBUG_ASSERT(needed != 0);
        m_nodes.insert(m_nodes.begin(), needed, (deque_node*)0);

        // (needed-1) full blocks
        while (needed-- > 1)
        {
            m_nodes[needed] = insert_node(m_head.next, 0, block_count);
            THOR_DEBUG_ASSERT(n > block_count);
            n -= block_count;
        }

        // The first block is a partial block.
        m_nodes[0] = insert_node(m_head.next, block_count - n, block_count);
        return m_nodes[0];
    }

    // The combination outnode/return value points to the first new element added.
    // The m_size member is adjusted.
    T* growback(deque_node*& outnode, size_type n)
    {
        THOR_DEBUG_ASSERT(n != 0);

        m_size += n;
        outnode = 0;
        T* start = 0;

        if (!m_nodes.empty() && m_head.prev->endindex != block_count)
        {
            // Consume nodes from the back node if possible
            outnode = m_head.prev;
            start = outnode->end();
            size_type num = min<size_type>(n, block_count - outnode->endindex);
            outnode->endindex += num;
            n -= num;

            if (n == 0)
            {
                return start;
            }
        }

        size_type needed = (n + block_count - 1) / block_count;
        THOR_DEBUG_ASSERT(needed != 0);
        m_nodes.reserve(m_nodes.size() + needed);

        // (needed-1) full blocks
        for (size_type i = 1; i < needed; ++i)
        {
            m_nodes.push_back(insert_node(terminator(), 0, block_count));
            THOR_DEBUG_ASSERT(n > block_count);
            n -= block_count;
        }

        // The last block is a partial block
        m_nodes.push_back(insert_node(terminator(), 0, n));
        if (outnode == 0)
        {
            outnode = m_nodes[m_nodes.size() - needed];
            return outnode->start();
        }
        return start;
    }

    T* element(size_type n) const
    {
        // The deque will assert if the calculated index is invalid
        THOR_ASSERT(n < m_size);
        n += m_nodes.front()->startindex;
        deque_node* node = m_nodes[n / block_count];
        T* e = &node->values[n % block_count];
        THOR_DEBUG_ASSERT(node->is_valid(e));
        return e;
    }

    // These functions merely alloc/free memory for the node. No construction takes place.
    deque_node* alloc_node()
    {
        return memory::align_alloc<deque_node>::alloc();
    }
    void free_node(deque_node* node)
    {
        memory::align_alloc<deque_node>::free(node);
    }

    // Value elements are not constructed
    deque_node* create_node(deque_node* next, deque_node* prev, size_type startindex, size_type endindex)
    {
        deque_node* node = alloc_node();
        new (node) deque_node_base(next, prev, startindex, endindex);
        return node;
    }

    // Value elements between start() and end() are destructed
    void destroy_node(deque_node* node)
    {
        typetraits<T>::range_destruct(node->start(), node->end());
        ((deque_node_base*)node)->~deque_node_base();
        free_node(node);
    }

    deque_node* insert_node(deque_node* before, size_type startindex, size_type endindex)
    {
        deque_node* node = create_node(before, before->prev, startindex, endindex);
        node->prev->next = node;
        node->next->prev = node;
        return node;
    }

    struct deque_node_base
    {
        deque_node*     next;
        deque_node*     prev;
        size_type       startindex;
        size_type       endindex;
        
        deque_node_base(deque_node* n=0, deque_node* p=0, size_type s=0, size_type e=0) : next(n), prev(p), startindex(s), endindex(e) {}
    };

    // The deque_node class is never actually constructed. After memory is allocated,
    // the deque_node_base is constructed and values are constructed as they are needed.
    // This saves time by not default constructing all of the value elements.
    struct deque_node : public deque_node_base
    {
        // Note: since this class is never actually constructed, these elements
        // are not default constructed.
        T values[block_count];

        T* start() const            { return (T*)&values[startindex]; }
        T* end()   const            { return (T*)&values[endindex];   }
        size_type size() const      { return endindex - startindex; }
        bool is_valid(T* p) const   { return p >= start() && p < end(); }

        // Destroy first element
        void destroy_start()
        {
            THOR_DEBUG_ASSERT((startindex + 1) <= endindex);
            typetraits<T>::destruct(&values[startindex++]);
        }
        // Destroy n elements at the start
        void destroy_start(size_type n)
        {
            THOR_DEBUG_ASSERT((startindex + n) <= endindex);
            typetraits<T>::range_destruct(&values[startindex], &values[startindex + n]);
            startindex += n;
        }
        // Destroy last element
        void destroy_end()
        {
            THOR_DEBUG_ASSERT(endindex >= 1);
            THOR_DEBUG_ASSERT((endindex - 1) >= startindex);
            typetraits<T>::destruct(&values[--endindex]);
        }
        // Destroy n elements at the end
        void destroy_end(size_type n)
        {
            THOR_DEBUG_ASSERT(endindex >= n);
            THOR_DEBUG_ASSERT((endindex - n) >= startindex);
            typetraits<T>::range_destruct(&values[endindex - n], &values[endindex]);
            endindex -= n;
        }
    };

    deque_node* terminator() const { return (deque_node*)&m_head; }

    vector<deque_node*> m_nodes;
    deque_node_base     m_head;
    size_type           m_size;
};

// Swap specialization
template <class T> void swap(deque<T>& lhs, deque<T>& rhs)
{
    lhs.swap(rhs);
}

} // namespace thor

// Global operators
template <class T> bool operator == (const thor::deque<T>& lhs, const thor::deque<T>& rhs)
{
    if (lhs.size() != rhs.size())
    {
        return false;
    }

    thor::deque<T>::const_iterator liter(lhs.begin()), riter(rhs.begin());
    thor::deque<T>::const_iterator eiter(lhs.end());
    while (liter != eiter)
    {
        if (!(*liter++ == *riter++))
        {
            return false;
        }
    }
    return true;
}

template <class T> bool operator != (const thor::deque<T>& lhs, const thor::deque<T>& rhs)
{
    return !(lhs == rhs);
}

template <class T> bool operator < (const thor::deque<T>& lhs, const thor::deque<T>& rhs)
{
    return thor::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <class T> bool operator > (const thor::deque<T>& lhs, const thor::deque<T>& rhs)
{
    return thor::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), thor::greater<T>());
}

template <class T> bool operator >= (const thor::deque<T>& lhs, const thor::deque<T>& rhs)
{
    return !(lhs < rhs);
}

template <class T> bool operator <= (const thor::deque<T>& lhs, const thor::deque<T>& rhs)
{
    return !(lhs > rhs);
}

#endif

