/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * list.h
 *
 * This file defines an STL-compatible linked list container.
 *
 * Extensions/Changes:
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
 * - Assistance for raw pointer types:
 *   * delete_all() will call delete on every element and clear() the list.
 *   * erase_and_delete() can be used to delete an element and erase it from
 *     the list.
 *   * pop_front_delete() will delete the first element and pop it from the list.
 *   * pop_back_delete() will delete the last element and pop it from the list.
 * - O(1) size() function (spec says that size() may be O(n))
 * - No initial heap allocation since &node is the terminator node (most std::list implementations construct a dead listnode
 *   from the heap as soon as they're constructed).
 * - move() function can be used to move an item in the list in O(1) without
 *   copying or constructing.
 * - The template allows a preallocated number of entries. These entires are included as part
 *   of the class and are not allocated from the heap.
 *   * Example: list<int, 5> reserves space for 5 int entries, but can still be
 *     passed to functions that require list<int>.
 *   * Growth above the preallocated amount will continue using the preallocated
 *     amount augmented with heap memory.
 *   * swap() between preallocated containers is O(n) for each preallocated list.
 *     Also, swap() will allocate from the heap and ignore preallocated space.
 *   * swap(), sort(), splice() and merge() will convert preallocated storage to heap storage.
 *   * splice(pos, list) is O(1) for standard lists but O(n) for lists with preallocated storage.
 */

#ifndef THOR_LIST_H
#define THOR_LIST_H
#pragma once

#ifndef THOR_BASETYPES_H
#include "basetypes.h"
#endif

#ifndef THOR_TYPETRAITS_H
#include "typetraits.h"
#endif

#ifndef THOR_ITERATOR_H
#include "iterator.h"
#endif

#ifndef THOR_FUNCTION_H
#include "function.h"
#endif

#ifndef THOR_ALGORITHM_H
#include "algorithm.h"
#endif

#ifndef THOR_FREELIST_H
#include "freelist.h"
#endif

#ifndef THOR_MEMORY_H
#include "memory.h"
#endif

namespace thor
{

template<class T, thor_size_type T_PREALLOC = 0> class list;

template <class T, class StrictWeakOrdering>
void __listsort(list<T>& L, StrictWeakOrdering order)
{
    if (L.size() < 2)
    {
        return;
    }

    list<T> carry;
    list<T> counter[64];

    thor_size_type fill = 0, i;
    while (!L.empty())
    {
        carry.splice(carry.begin(), L, L.begin());
        i = 0;
        while (i < fill && !counter[i].empty())
        {
            counter[i].merge(carry, order);
            carry.swap(counter[i++]);
        }
        carry.swap(counter[i]);
        if (i == fill)
        {
            ++fill;
        }
    }

    for (i = 1; i < fill; ++i)
    {
        counter[i].merge(counter[i - 1], order);
    }
    L.swap(counter[fill - 1]);
}

template <class T> class list<T, 0>
{
protected:
    struct list_node;
public:
    typedef T               value_type;
    typedef T*              pointer;
    typedef T&              reference;
    typedef const T*        const_pointer;
    typedef const T&        const_reference;
    typedef thor_size_type size_type;
    typedef thor_diff_type difference_type;

    // iterator definitions
    struct iterator_base : public iterator_type<bidirectional_iterator_tag, T>
    {
        list_node* m_element;
#ifdef THOR_DEBUG
        const list* m_list;
        iterator_base(list_node* n, const list* o) : m_element(n), m_list(o) {}
#else
        iterator_base(list_node* n, const list*) : m_element(n) {}
#endif
        void verify_not_end() const { THOR_DEBUG_ASSERT(m_list->end().m_element != m_element);   }
        void decr() {                     m_element = m_element->prev; }
        void incr() { verify_not_end();   m_element = m_element->next; }
        bool operator == (const iterator_base& i) const { THOR_DEBUG_ASSERT(m_list == i.m_list); return m_element == i.m_element; }
        bool operator != (const iterator_base& i) const { THOR_DEBUG_ASSERT(m_list == i.m_list); return m_element != i.m_element; }
    };

    template<class Traits> class fwd_iterator : public iterator_base
    {
    public:
        typedef typename Traits::pointer pointer;
        typedef typename Traits::reference reference;
        typedef fwd_iterator<nonconst_traits<T> > nonconst_iterator;
        typedef fwd_iterator<Traits> selftype;

        fwd_iterator(list_node* n = 0, const list* l = 0) : iterator_base(n, l) {}
        fwd_iterator(const nonconst_iterator& i) : iterator_base(i) {}
        selftype&  operator = (const nonconst_iterator& i)  { iterator_base::operator = (i); return *this; }
        reference  operator * ()  const                     { verify_not_end(); return m_element->m_value; }
        pointer    operator -> () const                     { verify_not_end(); return &(operator*()); }
        selftype&  operator -- ()     /* --iterator */      {                    decr(); return *this; }
        selftype   operator -- (int)  /* iterator-- */      { selftype n(*this); decr(); return n; }
        selftype&  operator ++ ()     /* ++iterator */      {                    incr(); return *this; }
        selftype   operator ++ (int)  /* iterator++ */      { selftype n(*this); incr(); return n; }
    };

    template<class Traits> class rev_iterator : public iterator_base
    {
    public:
        typedef typename Traits::pointer pointer;
        typedef typename Traits::reference reference;
        typedef rev_iterator<nonconst_traits<T> > nonconst_iterator;
        typedef rev_iterator<Traits> selftype;

        rev_iterator(list_node* n = 0, const list* l = 0) : iterator_base(n, l) {}
        rev_iterator(const nonconst_iterator& i) : iterator_base(i) {}
        selftype&  operator = (const nonconst_iterator& i)  { iterator_base::operator = (i); return *this; }
        reference  operator * ()  const                     { verify_not_end(); return m_element->m_value; }
        pointer    operator -> () const                     { verify_not_end(); return &(operator*()); }
        selftype&  operator -- ()     /* --iterator */      {                    incr(); return *this; }
        selftype   operator -- (int)  /* iterator-- */      { selftype n(*this); incr(); return n; }
        selftype&  operator ++ ()     /* ++iterator */      {                    decr(); return *this; }
        selftype   operator ++ (int)  /* iterator++ */      { selftype n(*this); decr(); return n; }
    };

    typedef fwd_iterator<nonconst_traits<T> > iterator;
    typedef fwd_iterator<const_traits<T>    > const_iterator;

    typedef rev_iterator<nonconst_traits<T> > reverse_iterator;
    typedef rev_iterator<const_traits<T>    > const_reverse_iterator;

    // Constructors
    list() :
        m_head(terminator(), terminator()),
        m_size(0)
    {}

    list(size_type n) :
        m_head(terminator(), terminator()),
        m_size(0)
    {
        while(n-- != 0)
        {
            typetraits<T>::construct(alloc_back());
        }
    }

    list(size_type n, const T& t) :
        m_head(terminator(), terminator()),
        m_size(0)
    {
        while (n-- != 0)
        {
            typetraits<T>::construct(alloc_back(), t);
        }
    }

    list(const list& L) :
        m_head(terminator(), terminator()),
        m_size(0)
    {
        insert(end(), L.begin(), L.end());
    }

    template <class InputIterator> list(InputIterator first, InputIterator last) :
        m_head(terminator(), terminator()),
        m_size(0)
    {
        insert(end(), first, last);
    }
    
    virtual ~list()
    {
        clear();
    }

    // Forward iteration
    iterator begin()
    {
        return iterator(m_head.next, this);
    }
    
    const_iterator begin() const
    {
        return const_iterator(m_head.next, this);
    }

    iterator end()
    {
        return iterator(terminator(), this);
    }

    const_iterator end() const
    {
        return const_iterator(terminator(), this);
    }

    // Reverse iteration
    reverse_iterator rbegin()
    {
        return reverse_iterator(m_head.prev, this);
    }

    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(m_head.prev, this);
    }

    reverse_iterator rend()
    {
        return reverse_iterator(terminator(), this);
    }

    const_reverse_iterator rend() const
    {
        return const_reverse_iterator(terminator(), this);
    }

    // Size
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

    list& operator = (const list& L)
    {
        if (this != &L)
        {
            if (empty())
            {
                insert(end(), L.begin(), L.end());
            }
            else
            {
                iterator write = begin();
                iterator wend  = end();
                const_iterator read = L.begin();
                const_iterator rend = L.end();
                while (write != wend && read != rend)
                {
                    *write = *read;
                    ++write, ++read;
                }

                if (read != rend)
                {
                    insert(wend, read, rend);
                }
                else
                {
                    erase(write, wend);
                }
            }
        }
        return *this;
    }

    // Accessing elements
    T& front()
    {
        THOR_ASSERT(!empty());
        return m_head.next->m_value;
    }
    
    const T& front() const
    {
        THOR_ASSERT(!empty());
        return m_head.next->m_value;
    }
    
    T& back()
    {
        THOR_ASSERT(!empty());
        return m_head.prev->m_value;
    }

    const T& back() const
    {
        THOR_ASSERT(!empty());
        return m_head.prev->m_value;
    }

    // Adding elements to the front of the list
    T& push_front()
    {
        typetraits<T>::construct(alloc_front());
        return front();
    }
    template <class T1> T& push_front(const T1& t1)
    {
        typetraits<T>::construct(alloc_front(), t1);
        return front();
    }
    template <class T1, class T2> T& push_front(const T1& t1, const T2& t2)
    {
        typetraits<T>::construct(alloc_front(), t1, t2);
        return front();
    }
    template <class T1, class T2, class T3> T& push_front(const T1& t1, const T2& t2, const T3& t3)
    {
        typetraits<T>::construct(alloc_front(), t1, t2, t3);
        return front();
    }
    template <class T1, class T2, class T3, class T4> T& push_front(const T1& t1, const T2& t2, const T3& t3, const T4& t4)
    {
        typetraits<T>::construct(alloc_front(), t1, t2, t3, t4);
        return front();
    }
    // Requires the use of placement new to construct the element.
    // Example: new (l.push_front_placement()) Element(arg1, arg2);
    void* push_front_placement()
    {
        return alloc_front();
    }

    // Adding elements to the back of the list
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
    // Requires the use of placement new to construct the element.
    // Example: new (l.push_back_placement()) Element(arg1, arg2);
    void* push_back_placement()
    {
        return alloc_back();
    }

    void pop_front()
    {
        THOR_DEBUG_ASSERT(!empty());
        if (!empty())
        {
            erase_node(m_head.next);
        }
    }

    // Like pop_front(), only deletes the front value as well. Only valid
    // for pointer types.
    void pop_front_delete()
    {
        if (!empty())
        {
            delete m_head.next->m_value;
            erase_node(m_head.next);
        }
    }

    void pop_back()
    {
        THOR_DEBUG_ASSERT(!empty());
        if (!empty())
        {
            erase_node(m_head.prev);
        }
    }

    // Like pop_back(), only deletes the back value as well. Only valid
    // for pointer types.
    void pop_back_delete()
    {
        if (!empty())
        {
            delete m_head.prev->m_value;
            erase_node(m_head.prev);
        }
    }

    void swap(list& L)
    {
        THOR_ASSERT(this != &L);
        
        if (!is_always_shareable())
        {
            make_shareable();
        }
        if (!L.is_always_shareable())
        {
            L.make_shareable();
        }

        // must fix up terminators first
        // also note that pointers must be assigned simultaneously (i.e. m_head.prev->next = m_head.next->prev = terminator() doesn't work)
        list_node *&Rhead = m_head.next->prev,   *&Rtail = m_head.prev->next;
        list_node *&Lhead = L.m_head.next->prev, *&Ltail = L.m_head.prev->next;
        Rhead = Rtail = L.terminator();
        Lhead = Ltail = terminator();

        thor::swap(m_head, L.m_head);
        thor::swap(m_size, L.m_size);
    }

    template <class InputIterator> void assign(InputIterator first, InputIterator last)
    {
        clear();
        insert(begin(), first, last);
    }

    void assign(size_type n, const T& u)
    {
        clear();
        while (n-- > 0)
        {
            push_back(u);
        }
    }

    iterator insert(iterator pos)
    {
        iterator ret(insert_node(pos), this);
        typetraits<T>::construct(&ret.m_element->m_value);
        return ret;
    }
    template <class T1> iterator insert(iterator pos, const T1& t1)
    {
        iterator ret(insert_node(pos), this);
        typetraits<T>::construct(&ret.m_element->m_value, t1);
        return ret;
    }
    template <class T1, class T2> iterator insert(iterator pos, const T1& t1, const T2& t2)
    {
        iterator ret(insert_node(pos), this);
        typetraits<T>::construct(&ret.m_element->m_value, t1, t2);
        return ret;
    }
    template <class T1, class T2, class T3> iterator insert(iterator pos, const T1& t1, const T2& t2, const T3& t3)
    {
        iterator ret(insert_node(pos), this);
        typetraits<T>::construct(&ret.m_element->m_value, t1, t2, t3);
        return ret;
    }
    template <class T1, class T2, class T3, class T4> iterator insert(iterator pos, const T1& t1, const T2& t2, const T3& t3, const T4& t4)
    {
        iterator ret(insert_node(pos), this);
        typetraits<T>::construct(&ret.m_element->m_value, t1, t2, t3, t4);
        return ret;
    }
    // Requires the use of placement new to construct the element.
    // Example: new (l.insert_placement(pos)) Element(arg1, arg2);
    void* insert_placement(iterator pos)
    {
        return &insert_node(pos)->m_value;
    }

    template <class InputIterator> void insert(iterator pos, InputIterator first, InputIterator last)
    {
        while (first != last)
        {
            list_node* node = insert_node(pos);
            typetraits<T>::construct(&node->m_value, *first);
            ++first;
        }
    }
    
    void insert(iterator pos, size_type n, const T& t)
    {
        verify_iterator(pos);
        while (n-- != 0)
        {
            list_node* node = insert_node(pos);
            typetraits<T>::construct(&node->m_value, t);
        }
    }

    iterator erase(iterator pos)
    {
        verify_iterator(pos);
        pos.verify_not_end();

        iterator next(pos.m_element->next, this);
        erase_node(pos.m_element);
        return next;
    }

    iterator erase(iterator first, iterator last)
    {
        verify_iterator(first);
        verify_iterator(last);

        list_node* n1 = first.m_element;
        list_node* n2 = last.m_element;

        while (n1 != n2)
        {
            list_node* next = n1->next;
            erase_node(n1);
            n1 = next;
        }

        return last;
    }

    // Similar to erase(pos), but also deletes the element. Only valid for pointer types.
    iterator erase_and_delete(iterator pos)
    {
        verify_iterator(pos);
        pos.verify_not_end();

        iterator next(pos.m_element->next, this);
        delete pos.m_element->m_value;
        erase_node(pos.m_element);
        return next;
    }

    void clear()
    {
        list_node* node = m_head.next;
        m_size = 0;
        list_node* last = m_head.prev = m_head.next = terminator();
        while (node != last)
        {
            list_node* next = node->next;
            dealloc_node(node);
            node = next;
        }
    }

    void resize(size_type n, const T& t = T())
    {
        if (size() <= n)
        {
            // Grow
            insert(end(), n - m_size, t);
        }
        else
        {
            // Shrink
            do
            {
                pop_back();
            }
            while (n < size());
        }
    }

    void splice(iterator pos, list& L)
    {
        THOR_ASSERT(this != &L);
        verify_iterator(pos);
        if(!L.empty() && this != &L)
        {
            // Steal list_nodes from L
            if (L.is_always_shareable())
            {
                pos.m_element->prev->next = L.m_head.next;
                L.m_head.next->prev = pos.m_element->prev;
                pos.m_element->prev = L.m_head.prev;
                L.m_head.prev->next = pos.m_element;
                m_size += L.m_size;

                // Reset L
                L.m_head.prev = L.m_head.next = L.terminator();
                L.m_size = 0;
            }
            else
            {
                // Not shareable, so splice individual nodes
                iterator first(L.begin()), last(L.end());
                while (first != last)
                {
                    splice(pos, L, first++);
                }
            }
        }
    }

    void splice(iterator pos, list& L, iterator i)
    {
        THOR_ASSERT(this != &L);
        verify_iterator(pos);
        L.verify_iterator(i);
        i.verify_not_end();

        if (L.end() != i)
        {
            // Remove i from L
            i.m_element->prev->next = i.m_element->next;
            i.m_element->next->prev = i.m_element->prev;
            --L.m_size;

            if (L.is_node_shareable(i.m_element))
            {
                // Add i to this at pos
                pos.m_element->prev->next = i.m_element;
                i.m_element->prev = pos.m_element->prev;
                pos.m_element->prev = i.m_element;
                i.m_element->next = pos.m_element;
                ++m_size;
            }
            else
            {
                // Node is not shareable, so must copy and erase
                list_node* node = insert_node(pos);
                typetraits<T>::construct(&node->m_value, i.m_element->m_value);
                L.dealloc_node(i.m_element);
            }
        }
    }
    
    void splice(iterator pos, list& L, iterator first, iterator last)
    {
        while (first != last)
        {
            splice(pos, L, first++);
        }
    }

    void remove(const T& value)
    {
        list_node* n1 = m_head.next;
        while (n1 != terminator())
        {
            list_node* next = n1->next;
            if (n1->m_value == value)
            {
                erase_node(n1);
            }
            n1 = next;
        }
    }

    // list must be sorted in order to use this
    void unique()
    {
        unique_internal(equal_to<T>());
    }

    // list must be sorted in order to use this
    template <class BinaryPredicate> void unique(BinaryPredicate pred)
    {
        unique_internal(pred);
    }

    void merge(list& L)
    {
        merge_internal(L, less<T>());
    }

    template <class StrictWeakOrdering> void merge(list& L, StrictWeakOrdering comp)
    {
        merge_internal(L, comp);
    }
    
    void sort()
    {
        sort_internal(less<T>());
    }

    template <class Compare> void sort(Compare comp)
    {
        sort_internal(comp);
    }

    // extensions
    bool validate() const
    {
#define THOR_ASSERT_RETURN(expr) THOR_ASSERT(expr); if (!(expr)) return false
        THOR_ASSERT_RETURN(m_head.next->prev == terminator());
        THOR_ASSERT_RETURN(m_head.prev->next == terminator());
        size_type localcount = 0;
        list_node* n = m_head.next;
        while (n != terminator())
        {
            THOR_ASSERT_RETURN(n->next->prev == n);
            THOR_ASSERT_RETURN(n->prev->next == n);
            ++localcount;
            n = n->next;
        }
        THOR_ASSERT_RETURN(localcount == m_size);
        return true;
#undef THOR_ASSERT_RETURN
    }

    void delete_all()
    {
        list_node* node = m_head.next;
        m_size = 0;
        list_node* last = m_head.prev = m_head.next = terminator();
        while (node != last)
        {
            list_node* next = node->next;
            delete node->m_value;
            dealloc_node(node);
            node = next;
        }
    }

    // Moves the item at 'which' to before 'pos'
    void move(iterator which, iterator pos)
    {
        verify_iterator(which);
        verify_iterator(pos);
        which.verify_not_end();
        if (which != pos && which.m_element != terminator() && which.m_element->next != pos.m_element)
        {
            // Remove from current
            which.m_element->prev->next = which.m_element->next;
            which.m_element->next->prev = which.m_element->prev;

            // Insert at pos
            which.m_element->prev = pos.m_element->prev;
            which.m_element->next = pos.m_element;
            pos.m_element->prev->next = which.m_element;
            pos.m_element->prev = which.m_element;
        }
    }

protected:
    struct list_node_base
    {
        list_node* next;
        list_node* prev;
        list_node_base(list_node* n = 0, list_node* p = 0) : next(n), prev(p) {}
    };

    struct list_node : public list_node_base
    {
        T m_value;
    };

    typedef memory::align_alloc<list_node> align_alloc;

    // (address of)m_head also happens to be the end() node (see terminator()).  m_head.next is the head pointer and m_head.prev is the tail pointer
    list_node_base  m_head;
    size_type       m_size;

    // The node allocated does not have m_value constructed
    T* alloc_front()
    {
        THOR_ASSERT(m_head.next->prev == terminator());
        m_head.next->prev = alloc_node(m_head.next, terminator());
        m_head.next = m_head.next->prev;
        ++m_size;
        return &m_head.next->m_value;
    }

    // The node allocated does not have m_value constructed
    T* alloc_back()
    {
        THOR_ASSERT(m_head.prev->next == terminator());
        m_head.prev->next = alloc_node(terminator(), m_head.prev);
        m_head.prev = m_head.prev->next;
        ++m_size;
        return &m_head.prev->m_value;
    }

    list_node* insert_node(const iterator& pos)
    {
        verify_iterator(pos);
        list_node* node = alloc_node(pos.m_element, pos.m_element->prev);
        node->next->prev = node;
        node->prev->next = node;
        ++m_size;
        return node;
    }

    void erase_node(list_node* node)
    {
        THOR_ASSERT(node != terminator());
        node->next->prev = node->prev;
        node->prev->next = node->next;
        dealloc_node(node);
        --m_size;
    }

    // The node does not have m_value constructed
    list_node* alloc_node(list_node* next, list_node* prev, bool require_shareable = false)
    {
        list_node* node = alloc_node(require_shareable);
        THOR_DEBUG_ASSERT(align_alloc::is_aligned(node));
        return (list_node*)new (node) list_node_base(next, prev);
    }

    // The node must already be removed from the list
    void dealloc_node(list_node* node)
    {
        THOR_ASSERT(node != terminator());
        node->~list_node();
        free_node(node);
    }

    // Only allocates the memory; list_node is not constructed
    virtual list_node* alloc_node(bool /*require_shareable*/)
    {
        return align_alloc::alloc();
    }

    // Does not destruct list_node; just frees the underlying memory
    virtual void free_node(list_node* node)
    {
        align_alloc::free(node);
    }

    virtual bool is_always_shareable() const
    {
        return true;
    }

    virtual bool is_node_shareable(list_node*)
    {
        return true;
    }

    void make_shareable()
    {
        list_node* node = m_head.next;
        while (node != terminator())
        {
            list_node* next = node->next;
            if (!is_node_shareable(node))
            {
                list_node* newnode = alloc_node(node->next, node->prev, true);
                typetraits<T>::construct(&newnode->m_value, node->m_value);
                node->next->prev = newnode;
                node->prev->next = newnode;
                dealloc_node(node);
            }
            node = next;
        }
    }

    list_node* terminator() const { return (list_node*)&m_head; }

    void verify_iterator(const iterator_base& i) const { THOR_UNUSED(i); THOR_ASSERT(i.m_list == this); }

    template <class BinaryPredicate> void unique_internal(BinaryPredicate pred)
    {
        list_node* last = terminator();
        if (size() > 1)
        {
            list_node* first = m_head.next;
            for (;;)
            {
                list_node* prev = first;
                first = first->next;
                if (first == last)
                {
                    break;
                }
                if (pred(first->m_value, prev->m_value))
                {
                    erase_node(prev);
                }
            }
        }
    }

    template <class StrictWeakOrdering> void merge_internal(list& L, StrictWeakOrdering comp)
    {
        THOR_ASSERT(this != &L);
        if (this != &L && !L.empty())
        {
            if (!empty())
            {
                list_node* write = m_head.next;
                list_node* read  = L.m_head.next;
                for (;;)
                {
                    if (comp(read->m_value, write->m_value))
                    {
                        list_node* readpos = read;
                        read = read->next;
                        splice(iterator(write, this), L, iterator(readpos, &L));
                        if (read == L.terminator())
                        {
                            break;
                        }
                    }
                    else
                    {
                        write = write->next;
                        if (write == terminator())
                        {
                            break;
                        }
                    }
                }
            }

            // Add any remaining elements
            splice(end(), L);
        }
    }

    template <class StrictWeakOrdering> void sort_internal(StrictWeakOrdering order)
    {
        if (size() < 2)
        {
            return;
        }

        list carry;
        list counter[64];

        thor_size_type fill = 0, i;
        while (!empty())
        {
            carry.splice(carry.begin(), *this, begin());
            i = 0;
            while (i < fill && !counter[i].empty())
            {
                counter[i].merge(carry, order);
                carry.swap(counter[i++]);
            }
            carry.swap(counter[i]);
            if (i == fill)
            {
                ++fill;
            }
        }

        for (i = 1; i < fill; ++i)
        {
            counter[i].merge(counter[i - 1], order);
        }
        swap(counter[fill - 1]);
    }
};

// List with preallocated nodes
template <class T, thor_size_type T_PREALLOC> class list : public list<T, 0>
{
    typedef list<T, 0> baseclass;
public:
    typedef typename baseclass::size_type size_type;
    
    // All constructors must default construct baseclass so that v-table is set up
    // before allocation.
    list() : baseclass()
    {}

    list(size_type n) : baseclass()
    {
        while(n-- != 0)
        {
            typetraits<T>::construct(alloc_back());
        }
    }

    list(size_type n, const T& t) : baseclass()
    {
        while (n-- != 0)
        {
            typetraits<T>::construct(alloc_back(), t);
        }
    }

    list(const list& L) : baseclass()
    {
        insert(end(), L.begin(), L.end());
    }

    template <class InputIterator> list(InputIterator first, InputIterator last) : baseclass()
    {
        insert(end(), first, last);
    }

    virtual ~list()
    {
        clear();
    }

    list& operator = (const baseclass& L)
    {
        baseclass::operator = (L);
        return *this;
    }
    list& operator = (const list& L)
    {
        baseclass::operator = (L);
        return *this;
    }

protected:
    typedef typename baseclass::list_node list_node;
    typedef typename baseclass::list_node_base list_node_base;
    freelist<list_node, T_PREALLOC> m_freelist;

    virtual list_node* alloc_node(bool require_shareable)
    {
        list_node* node = require_shareable ? 0 : m_freelist.alloc_node();
        if (node == 0)
        {
            node = baseclass::alloc_node(require_shareable);
        }
        return node;
    }

    virtual void free_node(list_node* node)
    {
        if (!m_freelist.free_node(node))
        {
            baseclass::free_node(node);
        }
    }

    virtual bool is_always_shareable() const
    {
        return false;
    }

    virtual bool is_node_shareable(list_node* node)
    {
        return !m_freelist.is_owned_node(node);
    }
};

// Swap specialization
template <class T, size_type U> void swap(thor::list<T,U>& lhs, thor::list<T,U>& rhs)
{
    lhs.swap(rhs);
}

template <class T, size_type U, size_type V> void swap(thor::list<T,U>& lhs, thor::list<T,V>& rhs)
{
    lhs.swap(rhs);
}

} // namespace thor

// Global operators
template <class T> bool operator == (const thor::list<T>& l1, const thor::list<T>& l2)
{
    return l1.size() == l2.size() && thor::equal(l1.begin(), l1.end(), l2.begin());
}

template <class T> bool operator != (const thor::list<T>& l1, const thor::list<T>& l2)
{
    return !(l1 == l2);
}

template <class T> bool operator < (const thor::list<T>& l1, const thor::list<T>& l2)
{
    return thor::lexicographical_compare(l1.begin(), l1.end(), l2.begin(), l2.end());
}

template <class T> bool operator > (const thor::list<T>& l1, const thor::list<T>& l2)
{
    return thor::lexicographical_compare(l1.begin(), l1.end(), l2.begin(), l2.end(), thor::greater<T>());
}

template <class T> bool operator <= (const thor::list<T>& l1, const thor::list<T>& l2)
{
    return !(l1 > l2);
}

template <class T> bool operator >= (const thor::list<T>& l1, const thor::list<T>& l2)
{
    return !(l1 < l2);
}

#endif

