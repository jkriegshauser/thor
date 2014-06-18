/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * embedded_hash_multimap.h
 *
 * This file defines a hash_multimap variant that uses a link node embedded in the
 * stored class (T).
 */

#ifndef THOR_EMBEDDED_HASH_MULTIMAP_H
#define THOR_EMBEDDED_HASH_MULTIMAP_H

#ifndef THOR_VECTOR_H
#include "vector.h"
#endif

#ifndef THOR_MEMORY_H
#include "memory.h"
#endif

#ifndef THOR_ITERATOR_H
#include "iterator.h"
#endif

#ifndef THOR_PAIR_H
#include "pair.h"
#endif

#ifndef THOR_HASH_FUNCS_H
#include "hash_funcs.h"
#endif

#ifndef THOR_POLICY_H
#include "policy.h"
#endif

namespace thor
{

//
// Prototypes
//
template <class Key, class T> class embedded_hash_multimap_link;
template <class Key, class T, embedded_hash_multimap_link<Key, T> T::*LINK, class HashFunc, class PartitionPolicy> class embedded_hash_multimap;


//
// embedded_hash_multimap_link
//
template <typename Key, typename T> class embedded_hash_multimap_link
{
    THOR_DECLARE_NOCOPY(embedded_hash_multimap_link);

    enum { alignment = memory::align_selector<Key>::alignment };

public:
    typedef Key         key_type;
    typedef T           value_type;
    typedef value_type* pointer;

    embedded_hash_multimap_link()
#ifdef THOR_DEBUG
    : keydebug(key())
#endif
    {
        THOR_COMPILETIME_ASSERT(THOR_OFFSET_OF(embedded_hash_multimap_link, listnext) == 0, InvalidAssumption);
        clear(false);
    }
    ~embedded_hash_multimap_link() { verify_free(); }

    bool is_contained() const { return listnext != 0; }

    // Functions to retrieve the key
    const key_type& key() const { return *(const key_type*)memory::align_forward<alignment>(keybuf); }

#if 0
protected:
    typedef embedded_hash_multimap_link<key_type, value_type> selftype;
    template <typename key_type, typename value_type, selftype value_type::*LINK, typename HashFunc> friend class embedded_hash_multimap;
#endif

    pointer listnext;
    pointer listprev;
    pointer hashnext;
    pointer hashprev;

    void verify_free() const
    {
        verify_owner(0);
    }

    void clear(bool destroy = true)
    {
        if (listnext != 0 && destroy)
        {
            // Destruct the key
            const_cast<key_type&>(key()).~key_type();
        }
        hashval = 0;
        listnext = listprev = hashnext = hashprev = 0;
        set_owner(0);
        THOR_DEBUG_INIT_MEM(keybuf, sizeof(keybuf), 0);
    }

    // The key is implemented as a buffer so that it is not constructed until necessary
    thor_byte keybuf[sizeof(key_type) + alignment];
    thor_size_type hashval;

#ifdef THOR_DEBUG
    const key_type& keydebug;
    void* owner;
    void set_owner(void* o) { owner = o; }
    void verify_owner(void* o) const
    {
        THOR_DEBUG_ASSERT(owner == o);
        THOR_DEBUG_ASSERT(is_contained() == (owner != 0));
    }
#else
    void set_owner(void*) {}
    void verify_owner(void*) const {}
#endif
};

template
<
    typename Key,
    typename T,
    embedded_hash_multimap_link<Key, T> T::*LINK,
    typename HashFunc = hash<Key>,
    class PartitionPolicy = policy::base2_partition
> class embedded_hash_multimap
{
    THOR_DECLARE_NOCOPY(embedded_hash_multimap);
    typedef PartitionPolicy partition_type;
public:
    typedef Key                 key_type;
    typedef T                   value_type;
    typedef HashFunc            hasher;
    typedef value_type*         pointer;
    typedef const value_type*   const_pointer;
    typedef value_type&         reference;
    typedef const value_type&   const_reference;
    typedef thor_size_type      size_type;
    typedef thor_diff_type      difference_type;

    typedef embedded_hash_multimap_link<Key, T> link_type;

    struct iterator_base : public iterator_type<bidirectional_iterator_tag, value_type>
    {
        pointer m_node;
        enum mode
        {
            mode_list,
            mode_hash,
        } m_mode;
#ifdef THOR_DEBUG
        const embedded_hash_multimap* m_owner;
        iterator_base(pointer n, mode m, const embedded_hash_multimap* o) : m_node(n), m_mode(m), m_owner(o) {}
#else
        iterator_base(pointer n, mode m, const embedded_hash_multimap*) : m_node(n), m_mode(m) {}
#endif

        void verify_not_end() const { THOR_DEBUG_ASSERT(m_owner->terminator() != m_node); }

        void decr()
        {
            THOR_DEBUG_ASSERT(m_owner->terminator() != (m_mode == mode_hash ? (m_node->*LINK).hashprev : (m_node->*LINK).listprev)); // Verify that we're not the front node
            m_node = m_mode == mode_hash ? (m_node->*LINK).hashprev : (m_node->*LINK).listprev;
        }
        void incr()
        {
            verify_not_end();
            m_node = m_mode == mode_hash ? (m_node->*LINK).hashnext : (m_node->*LINK).listnext;
        }

        bool operator == ( const iterator_base& i ) const { THOR_DEBUG_ASSERT(m_owner == i.m_owner); return m_node == i.m_node; }
        bool operator != ( const iterator_base& i ) const { THOR_DEBUG_ASSERT(m_owner == i.m_owner); return m_node != i.m_node; }
    };

    template<typename Traits> class fwd_iterator : public iterator_base
    {
    public:
        typedef typename Traits::pointer pointer;
        typedef typename Traits::reference reference;
        typedef fwd_iterator<nonconst_traits<value_type> > nonconst_iterator;
        typedef fwd_iterator<Traits> selftype;
        typedef typename iterator_base::mode mode;

        fwd_iterator() : iterator_base(0, mode_list, 0) {}
        fwd_iterator(pointer n, mode m, const embedded_hash_multimap* o) : iterator_base(n, m, o) {}
        fwd_iterator(const nonconst_iterator& i) : iterator_base(i) {}
        selftype&  operator = (const nonconst_iterator& i)  { iterator_base::operator = (i); return *this; }
        reference  operator * ()  const                     { verify_not_end(); return *m_node; }
        pointer    operator -> () const                     { verify_not_end(); return  m_node; }
        selftype&  operator -- ()     /* --iterator */      {                    decr(); return *this; }
        selftype   operator -- (int)  /* iterator-- */      { selftype n(*this); decr(); return n; }
        selftype&  operator ++ ()     /* ++iterator */      {                    incr(); return *this; }
        selftype   operator ++ (int)  /* iterator++ */      { selftype n(*this); incr(); return n; }

        const key_type& key() const { verify_not_end(); return (m_node->*LINK).key(); }
    };

    template<typename Traits> class rev_iterator : public iterator_base
    {
    public:
        typedef typename Traits::pointer pointer;
        typedef typename Traits::reference reference;
        typedef rev_iterator<nonconst_traits<value_type> > nonconst_iterator;
        typedef rev_iterator<Traits> selftype;
        typedef typename iterator_base::mode mode;

        rev_iterator() : iterator_base(0, mode_list, 0) {}
        rev_iterator(pointer n, mode m, const embedded_hash_multimap* o) : iterator_base(n, m, o) {}
        rev_iterator(const nonconst_iterator& i) : iterator_base(i) {}
        selftype&  operator = (const nonconst_iterator& i)  { iterator_base::operator = (i); return *this; }
        reference  operator * ()  const                     { verify_not_end(); return *m_node; }
        pointer    operator -> () const                     { verify_not_end(); return  m_node; }
        selftype&  operator -- ()     /* --iterator */      {                    incr(); return *this; }
        selftype   operator -- (int)  /* iterator-- */      { selftype n(*this); incr(); return n; }
        selftype&  operator ++ ()     /* ++iterator */      {                    decr(); return *this; }
        selftype   operator ++ (int)  /* iterator++ */      { selftype n(*this); decr(); return n; }

        const key_type& key() const { verify_not_end(); return (m_node->*LINK).key(); }
    };

    typedef fwd_iterator<nonconst_traits<value_type> > iterator;
    typedef fwd_iterator<const_traits<value_type>    > const_iterator;
    typedef rev_iterator<nonconst_traits<value_type> > reverse_iterator;
    typedef rev_iterator<const_traits<value_type>    > const_reverse_iterator;

    // constructors
    embedded_hash_multimap() :
        m_root(terminator())
    {}

    embedded_hash_multimap(size_type n) :
        m_root(terminator())
    {
        resize(n);
    }

    embedded_hash_multimap(size_type n, const hasher& h) :
        m_root(terminator(), h)
    {
        resize(n);
    }

    ~embedded_hash_multimap()
    {
        // Should be empty at destruction time since we don't own the elements
        THOR_DEBUG_ASSERT(empty());
        remove_all();
        memory::align_alloc<value_type*>::free(m_root.m_buckets);
    }
    
    // iteration
    iterator begin(bool mode_hash = false)
    {
        return iterator(m_root.m_listhead, mode_hash ? iterator::mode_hash : iterator::mode_list, this);
    }

    iterator end()
    {
        return iterator(terminator(), iterator::mode_list, this);
    }

    const_iterator begin(bool mode_hash = false) const
    {
        return const_iterator(m_root.m_listhead, mode_hash ? iterator::mode_hash : iterator::mode_list, this);
    }

    const_iterator end() const
    {
        return const_iterator(terminator(), iterator::mode_list, this);
    }

    reverse_iterator rbegin(bool mode_hash = false)
    {
        return reverse_iterator(m_root.m_listtail, mode_hash ? iterator::mode_hash : iterator::mode_list, this);
    }

    reverse_iterator rend()
    {
        return reverse_iterator(terminator(), iterator::mode_list, this);
    }

    const_reverse_iterator rbegin(bool mode_hash = false) const
    {
        return const_reverse_iterator(m_root.m_listtail, mode_hash ? iterator::mode_hash : iterator::mode_list, this);
    }

    const_reverse_iterator rend() const
    {
        return const_reverse_iterator(terminator(), iterator::mode_list, this);
    }

    size_type size() const
    {
        return m_root.m_size;
    }

    size_type max_size() const
    {
        return size_type(-1);
    }

    bool empty() const
    {
        return m_root.m_size == 0;
    }

    size_type bucket_count() const
    {
        return m_root.m_bucket_count;
    }
    
    const hasher& hash_funct() const
    {
        return static_cast<const hasher&>(m_root);
    }

    void resize(size_type n)
    {
        size_type bc = bucket_count();
        if (n > bc)
        {
            internal_resize(n);
        }
    }

    void swap(embedded_hash_multimap& rhs)
    {
        // must fix up terminators first
        // also note that pointers must be assigned simultaneously (i.e. node.prev->next = node.next->prev = terminator() doesn't work)
        {
            T *&Rhead = link(m_root.m_listhead).listprev, *&Rtail = link(m_root.m_listtail).listnext;
            T *&Lhead = link(rhs.m_root.m_listhead).listprev, *&Ltail = link(rhs.m_root.m_listtail).listnext;
            Rhead = Rtail = rhs.terminator();
            Lhead = Ltail = terminator();
        }
        {
            T *&Rhead = link(m_root.m_hashhead).hashprev, *&Rtail = link(m_root.m_hashtail).hashnext;
            T *&Lhead = link(rhs.m_root.m_hashhead).hashprev, *&Ltail = link(rhs.m_root.m_hashtail).hashnext;
            Rhead = Rtail = rhs.terminator();
            Lhead = Ltail = terminator();
        }
        thor::swap(m_root, rhs.m_root);

        set_owner(this);
        rhs.set_owner(&rhs);
    }

    iterator insert(const key_type& k, pointer p)
    {
        THOR_DEBUG_ASSERT(p != 0);
        internal_insert(terminator(), k, p);
        return iterator(p, iterator::mode_hash, this);
    }

    void move(iterator which, iterator pos)
    {
        verify_iterator(which);
        verify_iterator(pos);
        which.verify_not_end();
        if (which.m_node != pos.m_node && link(which.m_node).listnext != pos.m_node && which.m_node != terminator())
        {
            // Remove from current
            link(link(which.m_node).listnext).listprev = link(which.m_node).listprev;
            link(link(which.m_node).listprev).listnext = link(which.m_node).listnext;

            // Insert before pos
            link(which.m_node).listnext = pos.m_node;
            link(which.m_node).listprev = link(pos.m_node).listprev;
            link(link(which.m_node).listnext).listprev = which.m_node;
            link(link(which.m_node).listprev).listnext = which.m_node;
        }
    }

    void move(pointer which, iterator pos)
    {
        THOR_DEBUG_ASSERT(which != terminator());
        link(which).verify_owner(this);
        verify_iterator(pos);

        link_type& l = link(which);
        if (which != pos.m_node && l.listnext != pos.m_node && which != terminator())
        {
            // Remove from current
            link(l.listnext).listprev = l.listprev;
            link(l.listprev).listnext = l.listnext;

            // Insert before pos
            l.listnext = pos.m_node;
            l.listprev = link(pos.m_node).listprev;
            link(l.listnext).listprev = which;
            link(l.listprev).listnext = which;
        }
    }

    pointer remove(iterator pos)
    {
        verify_iterator(pos);
        pos.verify_not_end();
        return internal_remove(pos.m_node);
    }

    pointer remove(pointer pos)
    {
        THOR_DEBUG_ASSERT(pos != 0);
        THOR_ASSET(pos != terminator());
        link(pos).verify_owner(this);
        return internal_remove(pos);
    }

    template <class Pred> void remove(pointer pos, Pred pred)
    {
        pred(remove(pos));
    }

    void remove_delete(iterator pos)
    {
        delete remove(pos);
    }

    void remove_delete(pointer pos)
    {
        delete remove(pos);
    }

    size_type remove(const key_type& k)
    {
        pointer node = internal_find(k);
        if (node == terminator())
        {
            return 0;
        }

        size_type erasecount = 0;
        do 
        {
            pointer next = link(node).hashnext;
            internal_remove(node);
            node = next;
            ++erasecount;
        } while(node != terminator() && k == link(node).key());

        THOR_DEBUG_ASSERT(count(k) == 0);
        
        return erasecount;
    }

    template <class Pred> void remove(const key_type& k, Pred pred)
    {
        pointer node = internal_find(k);
        if (node == terminator())
        {
            return 0;
        }

        do
        {
            pointer next = link(node).hashnext;
            pred(internal_remove(node));
            node = next;
        } while (node != terminator() && k == link(node).key());

        THOR_DEBUG_ASSERT(count(k) == 0);
    }

    size_type remove_delete(const key_type& k)
    {
        pointer node = internal_find(k);
        if (node == terminator())
        {
            return 0;
        }

        size_type erasecount = 0;
        do
        {
            pointer next = link(node).hashnext;
            delete internal_remove(node);
            node = next;
            ++erasecount;
        } while(node != terminator() && k == link(node).key());

        THOR_DEBUG_ASSERT(count(k) == 0);

        return erasecount;
    }

    void remove(iterator first, iterator last)
    {
        verify_iterator(first);
        verify_iterator(last);
        while (first != last)
        {
            remove(first++); // Must postincrement
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
        pointer p = m_root.m_listhead;
        m_root.m_listhead = m_root.m_listtail = terminator();
        m_root.m_hashhead = m_root.m_hashtail = terminator();
        while (p != m_root.m_listtail)
        {
            pointer next = link(p).listnext;
            link(p).clear();
            p = next;
        }

        // clean up the buckets
        memory::align_alloc<value_type*>::free(m_root.m_buckets);
        m_root.m_buckets = 0;
        m_root.m_bucket_count = 0;
        m_root.m_size = 0;
    }

    void delete_all()
    {
        pointer p = m_root.m_listhead;
        m_root.m_listhead = m_root.m_listtail =
        m_root.m_hashhead = m_root.m_hashtail = terminator();
        while (p != m_root.m_listtail)
        {
            pointer next = link(p).listnext;
            link(p).clear();
            delete p;
            p = next;
        }

        // clean up all the buckets
        memory::align_alloc<value_type*>::free(m_root.m_buckets);
        m_root.m_buckets = 0;
        m_root.m_bucket_count = 0;
        m_root.m_size = 0;
    }

    const_iterator find(const key_type& k) const
    {
        if (bucket_count() != 0)
        {
            return const_iterator(internal_find(k), const_iterator::mode_hash, this);
        }
        return end();
    }

    iterator find(const key_type& k)
    {
        if (bucket_count() != 0)
        {
            return iterator(internal_find(k), iterator::mode_hash, this);
        }
        return end();
    }
    
    size_type count(const key_type& k) const
    {
        pointer node = internal_find(k);
        if (node == terminator())
        {
            return 0;
        }

        size_type count = 0;
        do
        {
            ++count;
            node = link(node).hashnext;
        } while (node != terminator() && (k == link(node).key()));

        return count;
    }

    pair<const_iterator, const_iterator> equal_range(const key_type& k, size_type* count = 0) const
    {
        pointer first = internal_find(k);
        if (first == terminator())
        {
            if (count != 0)
            {
                *count = 0;
            }
            return pair<const_iterator, const_iterator>(end(), end());
        }

        size_type localcount = 1;
        pointer last = link(first).hashnext;
        while (last != terminator() && k == link(last).key())
        {
            last = link(last).hashnext;
            ++localcount;
        }

        if (count != 0)
        {
            *count = localcount;
        }
        
        return pair<const_iterator, const_iterator>(const_iterator(first, const_iterator::mode_hash, this), const_iterator(last, const_iterator::mode_hash, this));
    }

    pair<iterator, iterator> equal_range(const key_type& k, size_type* count = 0)
    {
        pointer first = internal_find(k);
        if (first == terminator())
        {
            if (count != 0)
            {
                *count = 0;
            }
            return pair<iterator, iterator>(end(), end());
        }

        size_type localcount = 1;
        pointer last = link(first).hashnext;
        while (last != terminator() && k == link(last).key())
        {
            last = link(last).hashnext;
            ++localcount;
        }

        if (count != 0)
        {
            *count = localcount;
        }

        return pair<iterator, iterator>(iterator(first, iterator::mode_hash, this), iterator(last, iterator::mode_hash, this));
    }

private:
    pointer internal_insert(pointer listwhere, const key_type& k, pointer p)
    {
        link(p).verify_free();

        link_type& l = link(p);

        // Construct the copy of the key
        new (&const_cast<key_type&>(l.key())) key_type(k);

        resize(size() + 1);
        const size_type hashval = hash_funct()(k);
        const size_type bucket = partition_type::bucket_index(hashval, bucket_count());
        pointer& b = m_root.m_buckets[bucket];
        pointer iter = terminator();
        if (b != 0)
        {
            iter = b;
            do
            {
                if (link(iter).hashval == hashval)
                {
                    // Inner loop until we end this run of matching hashes
                    do 
                    {
                        bool keymatch = (k == link(iter).key());
                        iter = link(iter).hashnext;
                        if (keymatch)
                        {
                            // Found matching key, just insert after so that we don't have to fix up the bucket
                            break;
                        }
                    } while (iter != terminator() && link(iter).hashval == hashval);

                    // This ends a run of matching hashes, so insert here.
                    break;
                }
                iter = link(iter).hashnext;
            } while (iter != terminator() && partition_type::bucket_index(link(iter).hashval, bucket_count()) == bucket);
        }

        // Insert new node before iter
        THOR_DEBUG_ASSERT(b != iter);
        ++m_root.m_size;
        l.set_owner(this);
        l.hashval = hashval;
        l.listnext = listwhere;
        l.listprev = link(listwhere).listprev;
        link(l.listnext).listprev = p;
        link(l.listprev).listnext = p;
        l.hashnext = iter;
        l.hashprev = link(iter).hashprev;
        link(l.hashnext).hashprev = p;
        link(l.hashprev).hashnext = p;
        if (b == 0)
        {
            b = p;
        }
        return p;
    }

    void internal_resize(size_type n)
    {
        size_type bc = bucket_count();
        THOR_DEBUG_ASSERT(n > bc);
        if (0 == bc)
        {
            // Initial size
            bc = partition_type::initial_size;
        }
        bc = partition_type::resize(bc, n);
        if (bc != bucket_count())
        {
            // Build the larger bucket array
            memory::align_alloc<value_type*>::free(m_root.m_buckets);
            m_root.m_buckets = memory::align_alloc<value_type*>::alloc(bc);
            m_root.m_bucket_count = bc;
            typetraits<value_type*>::range_construct(m_root.m_buckets, m_root.m_buckets + bc);

            // Walk backwards. This will cause items within buckets to stay in the same order
            pointer node = m_root.m_hashtail;

            // Reset the root node so that we can rebuild the list
            m_root.m_hashhead = m_root.m_hashtail = terminator();
            while (node != terminator())
            {
                pointer prev = link(node).hashprev;

                // Current item may be at a new bucket index
                pointer& b = m_root.m_buckets[ partition_type::bucket_index(link(node).hashval, bucket_count()) ];
                pointer  insertnode = b != 0 ? b : m_root.m_hashhead /*newroot.m_head*/;

                // Always add to the front of the bucket or, if bucket is empty, the entire list
                link(node).hashprev = link(insertnode).hashprev;
                link(node).hashnext = insertnode;
                link(link(node).hashprev).hashnext = node;
                link(link(node).hashnext).hashprev = node;
                b = node;

                node = prev;
            }
        }
    }

    pointer internal_find(const key_type& k) const
    {
        if (bucket_count() != 0)
        {
            const size_type hashval = hash_funct()(k);
            const size_type bucket = partition_type::bucket_index(hashval, bucket_count());
            pointer node = m_root.m_buckets[bucket];
            if (node)
            {
                do
                {
                    if (link(node).hashval == hashval)
                    {
                        // Inner loop until we end this run of matching hashes
                        do 
                        {
                            if (k == link(node).key())
                            {
                                // Found it
                                return node;
                            }
                            node = link(node).hashnext;
                        } while (node != terminator() && link(node).hashval == hashval);

                        // This ends a run of matching hashes without finding a key match
                        return terminator();
                    }
                    node = link(node).hashnext;
                } while (node != terminator() && partition_type::bucket_index(link(node).hashval, bucket_count()) == bucket);
            }
        }
        return terminator();
    }

    pointer internal_remove(pointer n)
    {
        link_type& l = link(n);
        link(l.hashprev).hashnext = l.hashnext;
        link(l.hashnext).hashprev = l.hashprev;
        link(l.listprev).listnext = l.listnext;
        link(l.listnext).listprev = l.listprev;
        l.clear();

        const size_type bucket = partition_type::bucket_index(l.hashval, bucket_count());
        pointer& b = m_root.m_buckets[bucket];
        if (b == n)
        {
            if (l.hashnext == terminator() || partition_type::bucket_index(link(l.hashnext).hashval, bucket_count()) != bucket)
            {
                // bucket is now empty
                b = 0;
            }
            else
            {
                b = l.hashnext;
            }
        }
        --m_root.m_size;
        return n;
    }

    pointer terminator() const
    {
        return (pointer)((thor_byte*)&m_root.m_listhead - THOR_OFFSET_OF(T, *LINK));
    }

    static link_type& link(pointer p)
    {
        return p->*LINK;
    }

    static const link_type& link(const_pointer p)
    {
        return p->*LINK;
    }

    void verify_iterator(const iterator_base& i) const
    {
        THOR_DEBUG_ASSERT(i.m_owner == this);
    }

    hasher& hash_funct()
    {
        return static_cast<hasher&>(m_root);
    }

    // Inherit from hasher since hasher generally has no members
    struct empty_member_opt : public hasher
    {
        // The first four members of this structure must match embedded_hash_multimap_link
        pointer m_listhead;
        pointer m_listtail;
        pointer m_hashhead;
        pointer m_hashtail;

        value_type** m_buckets;
        size_type m_bucket_count;
        size_type m_size;

        empty_member_opt(pointer term) :
            hasher(),
            m_listhead(term),
            m_listtail(term),
            m_hashhead(term),
            m_hashtail(term),
            m_buckets(0),
            m_bucket_count(0),
            m_size(0)
        {
            THOR_COMPILETIME_ASSERT(THOR_OFFSET_OF(link_type, listnext) == 0, IncorrectOffset1);
            THOR_COMPILETIME_ASSERT(THOR_OFFSET_OF(link_type, listprev) == (THOR_OFFSET_OF(empty_member_opt, m_listtail) - THOR_OFFSET_OF(empty_member_opt, m_listhead)), IncorrectOffset2);
            THOR_COMPILETIME_ASSERT(THOR_OFFSET_OF(link_type, hashnext) == (THOR_OFFSET_OF(empty_member_opt, m_hashhead) - THOR_OFFSET_OF(empty_member_opt, m_listhead)), IncorrectOffset3);
            THOR_COMPILETIME_ASSERT(THOR_OFFSET_OF(link_type, hashprev) == (THOR_OFFSET_OF(empty_member_opt, m_hashtail) - THOR_OFFSET_OF(empty_member_opt, m_listhead)), IncorrectOffset4);
        }

        empty_member_opt(pointer term, const hasher& h) :
            hasher(h),
            m_listhead(term),
            m_listtail(term),
            m_hashhead(term),
            m_hashtail(term),
            m_buckets(0),
            m_bucket_count(0),
            m_size(0)
        {}

        empty_member_opt& operator = (const hasher& h)
        {
            hasher::operator = (h);
            return *this;
        }
    };

    empty_member_opt m_root;
};

} // namespace thor

#endif

