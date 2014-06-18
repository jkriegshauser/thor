/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * embedded_list.h
 *
 * This file defines an STL-like linked list container. This container is
 * an embedded container, which means that the stored object must contain
 * an embedded_list_link member. This embedded_list_link member contains
 * the embedded list node data.
 *
 * This container does not own its members. Therefore, remove will return the
 * member. Also, the stored objects are not created by the containter; they
 * must be created before using push_back(), push_front() or insert().
 *
 * Embedded containers are ideal for storage of items that may be included in
 * several containers. Typically one container will be non-embedded and will
 * "own" the item; other containers will be embedded containers and do not
 * own the item. This prevents several small nodes from being created to
 * represent containers storing pointers to items.
 *
 * Writing to the end() node can be very dangerous since this node doesn't
 * actually exist. The end() node actually overlaps the container object so
 * that end().[link node] aligns with embedded_list::m_head. Therefore, any
 * writes to the end() node will overwrite memory. Debug builds have several
 * asserts to try and prevent this from happening.
 *
 * Since this container is not defined by the STL specification, it generally
 * follows the list container. Differences are called out here:
 * - push_front(), push_back() and insert() take pointers to the contained type
 *   instead of construction parameters. This is because items must already
 *   be constructed before inserting.
 * - pop_front() and pop_back() return the item that is popped. This is because
 *   the embedded_list container does not have ownership of the item. It is
 *   then up to the caller to destroy the item. For convenience,
 *   pop_front_delete() and pop_back_delete() exist to delete the item.
 * - erase() is not a member since the embedded_list does not have ownership of
 *   the contained items. Instead, the remove() function exists to remove the
 *   item from the embedded_list and return to the caller. It is then up to the
 *   caller to delete the item. For convenience, remove_delete() exists which
 *   deletes the item after removing. Also, remove functions exist that take
 *   a range of items and/or a Predicate. It is safe to delete objects inside
 *   the Predicate.
 * - Similarly, clear() no longer exists. Instead, remove_all() and delete_all()
 *   exist to remove all items and delete all items, respectively. Also,
 *   a version of remove_all() exists that executes a Predicate on each item.
 *   It is safe to delete objects inside the Predicate.
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

//
// Prototypes
//
template <class T> class embedded_list_link;
template <class T, embedded_list_link<T> T::*U> class embedded_list;

//
// embedded_list_link definition
//
template <class T> class embedded_list_link
{
    // No copy
    embedded_list_link(const embedded_list_link&);
    embedded_list_link& operator = (const embedded_list_link&);

public:
    embedded_list_link() : next(0), prev(0) { set_owner(0); }
    ~embedded_list_link()
    {
        verify_free();
    }

    bool is_contained() const
    {
        return next != 0;
    }

#if 0
protected:
    template<class T, embedded_list_link<T> T::*LINK> friend class embedded_list;
#endif

#ifdef THOR_DEBUG
    void* owner;
    void set_owner(void* o) { owner = o; }
    void verify_owner(void* o) const
    {
        THOR_DEBUG_ASSERT(owner == o);
        THOR_DEBUG_ASSERT(is_contained());
    }
#else
    void set_owner(void*) {}
    void verify_owner(void*) const {}
#endif

    void verify_free() const
    {
        THOR_ASSERT(!is_contained());
        THOR_DEBUG_ASSERT(owner == 0);
    }

    void clear()
    {
        next = 0;
        prev = 0;
        set_owner(0);
    }

    T* next;
    T* prev;
};

//
// embedded_list definition
//
template <class T, embedded_list_link<T> T::*LINK> class embedded_list
{
    // No copy
    embedded_list(const embedded_list&);
    embedded_list& operator = (const embedded_list&);

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
        T* m_element;
#ifdef THOR_DEBUG
        const embedded_list* m_list;
        iterator_base(T* n, const embedded_list* o) : m_element(n), m_list(o) {}
#else
        iterator_base(T* n, const embedded_list*)   : m_element(n) {}
#endif
        void verify_not_end() const { THOR_DEBUG_ASSERT(m_list->end().m_element != m_element);   }
        void decr() { m_element = (m_element->*LINK).prev; }
        void incr() { m_element = (m_element->*LINK).next; }
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

        fwd_iterator() : iterator_base(0, 0) {}
        fwd_iterator(T* n, const embedded_list* l) : iterator_base(n, l) {}
        fwd_iterator(const nonconst_iterator& i) : iterator_base(i) {}
        selftype&  operator = (const nonconst_iterator& i)  { iterator_base::operator = (i); return *this; }
        reference  operator * ()  const                     { verify_not_end(); THOR_DEBUG_ASSERT(m_element != 0); return *m_element; }
        pointer    operator -> () const                     { verify_not_end(); return &(operator*()); }
        selftype&  operator -- ()     /* --iterator */      {                    decr(); return *this; }
        selftype   operator -- (int)  /* iterator-- */      { selftype n(*this); decr(); return n; }
        selftype&  operator ++ ()     /* ++iterator */      { verify_not_end();                    incr(); return *this; }
        selftype   operator ++ (int)  /* iterator++ */      { verify_not_end(); selftype n(*this); incr(); return n; }
    };

    template<class Traits> class rev_iterator : public iterator_base
    {
    public:
        typedef typename Traits::pointer pointer;
        typedef typename Traits::reference reference;
        typedef rev_iterator<nonconst_traits<T> > nonconst_iterator;
        typedef rev_iterator<Traits> selftype;

        rev_iterator() : iterator_base(0, 0) {}
        rev_iterator(T* n, const embedded_list* l) : iterator_base(n, l) {}
        rev_iterator(const nonconst_iterator& i) : iterator_base(i) {}
        selftype&  operator = (const nonconst_iterator& i)  { iterator_base::operator = (i); return *this; }
        reference  operator * ()  const                     { verify_not_end(); THOR_DEBUG_ASSERT(m_element != 0); return *m_element; }
        pointer    operator -> () const                     { verify_not_end(); return &(operator*()); }
        selftype&  operator -- ()     /* --iterator */      {                    incr(); return *this; }
        selftype   operator -- (int)  /* iterator-- */      { selftype n(*this); incr(); return n; }
        selftype&  operator ++ ()     /* ++iterator */      { verify_not_end();                    decr(); return *this; }
        selftype   operator ++ (int)  /* iterator++ */      { verify_not_end(); selftype n(*this); decr(); return n; }
    };

    typedef fwd_iterator<nonconst_traits<T> > iterator;
    typedef fwd_iterator<const_traits<T>    > const_iterator;

    typedef rev_iterator<nonconst_traits<T> > reverse_iterator;
    typedef rev_iterator<const_traits<T>    > const_reverse_iterator;

    // Constructors
    embedded_list()
    : m_size(0)
    {
        m_head.next = terminator();
        m_head.prev = terminator();
    }

    template <class InputIterator> embedded_list(InputIterator first, InputIterator last)
    : m_size(0)
    {
        m_head.next = terminator();
        m_head.prev = terminator();
        insert(end(), first, last);
    }
    
    ~embedded_list()
    {
        // List should be empty at destruction time
        THOR_DEBUG_ASSERT(empty());
        remove_all();
        m_head.clear();
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

    // Accessing elements
    T& front()
    {
        THOR_ASSERT(!empty());
        T* obj = !empty() ? m_head.next : 0;
        return *obj;
    }
    
    const T& front() const
    {
        THOR_ASSERT(!empty());
        const T* obj = !empty() ? m_head.next : 0;
        return *obj;
    }
    
    T& back()
    {
        THOR_ASSERT(!empty());
        T* obj = !empty() ? m_head.prev : 0;
        return *obj;
    }

    const T& back() const
    {
        THOR_ASSERT(!empty());
        const T* obj = !empty() ? m_head.prev : 0;
        return *obj;
    }

    // Adding elements to the front of the list
    T& push_front(T* p)
    {
        THOR_DEBUG_ASSERT(p != 0);
        link(p).verify_free();
        link(p).set_owner(this);
        link(p).next = m_head.next;
        link(p).prev = terminator();
        link(m_head.next).prev = p;
        m_head.next = p;
        ++m_size;
        return *p;
    }

    // Adding elements to the back of the list
    T& push_back(T* p)
    {
        THOR_DEBUG_ASSERT(p != 0);
        link(p).verify_free();
        link(p).set_owner(this);
        link(p).next = terminator();
        link(p).prev = m_head.prev;
        link(m_head.prev).next = p;
        m_head.prev = p;
        ++m_size;
        return *p;
    }

    /*gift*/ T* pop_front()
    {
        THOR_DEBUG_ASSERT(!empty());
        if (!empty())
        {
            pointer p = m_head.next;
            m_head.next = link(p).next;
            link(m_head.next).prev = terminator();
            link(p).clear();
            --m_size;
            return p;
        }
        return 0;
    }

    // Like pop_front(), only deletes the front value as well. Only valid
    // for pointer types.
    void pop_front_delete()
    {
        delete pop_front();
    }

    /*gift*/ T* pop_back()
    {
        THOR_DEBUG_ASSERT(!empty());
        if (!empty())
        {
            pointer p = m_head.prev;
            m_head.prev = link(p).prev;
            link(m_head.prev).next = terminator();
            link(p).clear();
            --m_size;
            return p;
        }
        return 0;
    }

    // Like pop_back(), only deletes the back value as well. Only valid
    // for pointer types.
    void pop_back_delete()
    {
        delete pop_back();
    }

    void swap(embedded_list& L)
    {
        THOR_ASSERT(this != &L);
        
        // must fix up terminators first
        // also note that pointers must be assigned simultaneously (i.e. m_head.prev->next = m_head.next->prev = terminator() doesn't work)
        T *&Rhead = link(m_head.next).prev,   *&Rtail = link(m_head.prev).next;
        T *&Lhead = link(L.m_head.next).prev, *&Ltail = link(L.m_head.prev).next;
        Rhead = Rtail = L.terminator();
        Lhead = Ltail = terminator();

        T* temp;
        { temp = m_head.prev; m_head.prev = L.m_head.prev; L.m_head.prev = temp; }
        { temp = m_head.next; m_head.next = L.m_head.next; L.m_head.next = temp; }
        thor::swap(m_size, L.m_size);

        // Update ownership
        set_owner(this);
        L.set_owner(&L);
    }

    iterator insert(iterator pos, T* p)
    {
        THOR_DEBUG_ASSERT(p != 0);
        verify_iterator(pos);
        link(p).verify_free();
        
        link(p).prev = link(pos.m_element).prev;
        link(link(p).prev).next = p;
        link(p).next = pos.m_element;
        link(pos.m_element).prev = p;
        link(p).set_owner(this);
        ++m_size;
        return iterator(p, this);
    }

    template <class InputIterator> void insert(iterator pos, InputIterator first, InputIterator last)
    {
        while (first != last)
        {
            insert(pos, &(*(first++)));
        }
    }
    
    T* remove(iterator pos)
    {
        verify_iterator(pos);
        pos.verify_not_end();

        remove(pos.m_element);

        return pos.m_element;
    }

    T* remove(T* pos)
    {
        THOR_DEBUG_ASSERT(pos != 0);
        THOR_DEBUG_ASSERT(pos != terminator());
        link(pos).verify_owner(this);

        link(link(pos).next).prev = link(pos).prev;
        link(link(pos).prev).next = link(pos).next;
        link(pos).next = link(pos).prev = 0;
        link(pos).clear();
        --m_size;

        return pos;
    }

    void remove_delete(iterator pos)
    {
        delete remove(pos);
    }

    void remove_delete(T* pos)
    {
        delete remove(pos);
    }

    template <class Pred> void remove(iterator pos, Pred pred)
    {
        pred(remove(pos));
    }

    void remove(iterator first, iterator last)
    {
        verify_iterator(first);
        verify_iterator(last);
        while (first != last)
        {
            remove(first++);
        }
    }

    template <class Pred> void remove(iterator first, iterator last, Pred pred)
    {
        verify_iterator(first);
        verify_iterator(last);
        while (first != last)
        {
            pred(remove(first++));
        }
    }

    void remove_all()
    {
        T* node = m_head.next;
        T* last = m_head.next = m_head.prev = terminator();
        m_size = 0;

        while (node != last)
        {
            T* next = link(node).next;
            link(node).clear();
            node = next;
        }
    }

    template <class Pred> void remove_all(Pred pred)
    {
        T* node = m_head.next;
        T* last = m_head.next = m_head.prev = terminator();
        m_size = 0;

        while (node != last)
        {
            T* next = link(node).next;
            link(node).clear();
            pred(node);
            node = next;
        }
    }

    void delete_all()
    {
        T* node = m_head.next;
        T* last = m_head.next = m_head.prev = terminator();
        m_size = 0;

        while (node != last)
        {
            T* next = link(node).next;
            link(node).clear();
            delete node;
            node = next;
        }
    }

    void splice(iterator pos, embedded_list& L)
    {
        THOR_ASSERT(this != &L);
        verify_iterator(pos);
        
        if(!L.empty() && this != &L)
        {
            // Steal list_nodes from L
            L.set_owner(this);
            link(link(pos.m_element).prev).next = L.m_head.next;
            link(L.m_head.next).prev = link(pos.m_element).prev;
            link(pos.m_element).prev = L.m_head.prev;
            link(L.m_head.prev).next = pos.m_element;
            m_size += L.m_size;

            // Reset L
            L.m_head.prev = L.m_head.next = L.terminator();
            L.m_size = 0;
        }
    }

    void splice(iterator pos, embedded_list& L, iterator i)
    {
        THOR_ASSERT(this != &L);
        verify_iterator(pos);
        L.verify_iterator(i);
        i.verify_not_end();

        if (this != &L && L.end() != i)
        {
            // Remove i from L
            link(link(i.m_element).prev).next = link(i.m_element).next;
            link(link(i.m_element).next).prev = link(i.m_element).prev;
            --L.m_size;

            // Add i to this at pos
            link(link(pos.m_element).prev).next = i.m_element;
            link(i.m_element).prev = link(pos.m_element).prev;
            link(pos.m_element).prev = i.m_element;
            link(i.m_element).next = pos.m_element;
            link(i.m_element).set_owner(this);
            ++m_size;
        }
    }
    
    void splice(iterator pos, embedded_list& L, iterator first, iterator last)
    {
        while (first != last)
        {
            splice(pos, L, first++);
        }
    }

    // list must be sorted in order to use this.
    // The iterator returned is the new end, similar to how thor::unique works.
    iterator unique()
    {
        return iterator(unique_internal(equal_to<T>()), this);
    }

    // list must be sorted in order to use this.
    // The iterator returned is the new end, similar to how thor::unique works.
    template <class BinaryPredicate> iterator unique(BinaryPredicate pred)
    {
        return iterator(unique_internal(pred), this);
    }

    void merge(embedded_list& L)
    {
        merge_internal(L, less<T>());
    }

    template <class StrictWeakOrdering> void merge(embedded_list& L, StrictWeakOrdering comp)
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
        THOR_ASSERT_RETURN(link(m_head.next).prev == terminator());
        THOR_ASSERT_RETURN(link(m_head.prev).next == terminator());
        size_type localcount = 0;
        T* n = m_head.next;
        while (n != terminator())
        {
            link(n).verify_owner();
            THOR_ASSERT_RETURN(link(link(n).next).prev == n);
            THOR_ASSERT_RETURN(link(link(n).prev).next == n);
            ++localcount;
            n = link(n).next;
        }
        THOR_ASSERT_RETURN(localcount == m_size);
        return true;
#undef THOR_ASSERT_RETURN
    }

    // Moves the item at 'which' to before 'pos'
    void move(iterator which, iterator pos)
    {
        verify_iterator(which);
        verify_iterator(pos);
        which.verify_not_end();
        if (which != pos && which.m_element != terminator() && link(which.m_element).next != pos.m_element)
        {
            // Remove from current
            link(link(which.m_element).prev).next = link(which.m_element).next;
            link(link(which.m_element).next).prev = link(which.m_element).prev;

            // Insert at pos
            link(which.m_element).prev = link(pos.m_element).prev;
            link(which.m_element).next = pos.m_element;
            link(link(pos.m_element).prev).next = which.m_element;
            link(pos.m_element).prev = which.m_element;
        }
    }

    void move(T* which, iterator pos)
    {
        THOR_DEBUG_ASSERT(which != 0);
        THOR_DEBUG_ASSERT(which != terminator());
        link(which).verify_owner(this);
        verify_iterator(pos);

        if (which != pos.m_element && which != terminator() && link(which).next != pos.m_element)
        {
            // Remove from current
            link(link(which).prev).next = link(which).next;
            link(link(which).next).prev = link(which).prev;

            // Insert at pos
            link(which).prev = link(pos.m_element).prev;
            link(which).next = pos.m_element;
            link(link(pos.m_element).prev).next = which;
            link(pos.m_element).prev = which;
        }
    }

protected:
    embedded_list_link<T>   m_head;
    size_type               m_size;

    T* terminator() const
    {
        return (T*)((thor_byte*)&m_head - THOR_OFFSET_OF(T, *LINK));
    }

    static embedded_list_link<T>& link(T* p)
    {
        return p->*LINK;
    }

    static const embedded_list_link<T>& link(const T* p)
    {
        return p->*LINK;
    }

    void set_owner(void* owner)
    {
        THOR_UNUSED(owner);
#ifdef THOR_DEBUG
        T* p = m_head.next;
        while (p != terminator())
        {
            link(p).set_owner(owner);
            p = link(p).next;
        }
#endif
    }

    void verify_iterator(const iterator_base& i) const { THOR_UNUSED(i); THOR_ASSERT(i.m_list == this); }

    // Helper functions
    template <class BinaryPredicate> T* unique_internal(BinaryPredicate pred)
    {
        T* last = terminator();
        if (size() > 1)
        {
            T* first = m_head.next;
            for (;;)
            {
                T* prev = first;
                first = link(first).next;
                if (first == last)
                {
                    break;
                }
                if (pred(*first, *prev))
                {
                    move(prev, iterator(last, this));
                    last = prev;
                }
            }
        }
        return last;
    }

    template <class StrictWeakOrdering> void merge_internal(embedded_list& rhs, StrictWeakOrdering comp)
    {
        THOR_ASSERT(this != &rhs);
        if (this != &rhs && !rhs.empty())
        {
            if (!empty())
            {
                T* write = m_head.next;
                T* read  = rhs.m_head.next;
                for (;;)
                {
                    if (comp(*read, *write))
                    {
                        T* readpos = read;
                        read = link(read).next;
                        splice(iterator(write, this), rhs, iterator(readpos, &rhs));
                        if (read == rhs.terminator())
                        {
                            break;
                        }
                    }
                    else
                    {
                        write = link(write).next;
                        if (write == terminator())
                        {
                            break;
                        }
                    }
                }
            }

            // Add any remaining elements
            splice(end(), rhs);
        }
    }

    template <class StrictWeakOrdering> void sort_internal(StrictWeakOrdering order)
    {
        if (size() < 2)
        {
            return;
        }

        embedded_list carry;
        embedded_list counter[64];

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


// Swap specialization
template <class T, embedded_list_link<T> T::*U> void swap(thor::embedded_list<T,U>& lhs, thor::embedded_list<T,U>& rhs)
{
    lhs.swap(rhs);
}

} // namespace thor

// Global operators
template <class T, thor::embedded_list_link<T> T::*U> bool operator == (const thor::embedded_list<T,U>& l1, const thor::embedded_list<T,U>& l2)
{
    return l1.size() == l2.size() && thor::equal(l1.begin(), l1.end(), l2.begin());
}

template <class T, thor::embedded_list_link<T> T::*U> bool operator != (const thor::embedded_list<T,U>& l1, const thor::embedded_list<T,U>& l2)
{
    return !(l1 == l2);
}

template <class T, thor::embedded_list_link<T> T::*U> bool operator < (const thor::embedded_list<T,U>& l1, const thor::embedded_list<T,U>& l2)
{
    return thor::lexicographical_compare(l1.begin(), l1.end(), l2.begin(), l2.end());
}

template <class T, thor::embedded_list_link<T> T::*U> bool operator > (const thor::embedded_list<T,U>& l1, const thor::embedded_list<T,U>& l2)
{
    return thor::lexicographical_compare(l1.begin(), l1.end(), l2.begin(), l2.end(), thor::greater<T>());
}

template <class T, thor::embedded_list_link<T> T::*U> bool operator <= (const thor::embedded_list<T,U>& l1, const thor::embedded_list<T,U>& l2)
{
    return !(l1 > l2);
}

template <class T, thor::embedded_list_link<T> T::*U> bool operator >= (const thor::embedded_list<T,U>& l1, const thor::embedded_list<T,U>& l2)
{
    return !(l1 < l2);
}

#endif

