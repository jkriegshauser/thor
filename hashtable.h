/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * hashtable.h
 *
 * ** THOR INTERNAL FILE - NOT FOR APPLICATION USE **
 *
 * This file defines a dynamic hashtable to be used as a base for hashtable-type containers (hash_map, hash_set, hash_multimap, hash_multiset)
 *
 * NOTE: Do not use hashtable directly.  Instead use one of the following implementations: hash_map, hash_multimap, hash_set, hash_multiset
 */

#ifndef THOR_HASHTABLE_H
#define THOR_HASHTABLE_H
#pragma once

#ifndef THOR_VECTOR_H
#include "vector.h"
#endif

#ifndef THOR_ITERATOR_H
#include "iterator.h"
#endif

#ifndef THOR_PAIR_H
#include "pair.h"
#endif

namespace thor
{

template
<
    typename Key,
    typename Value,
    typename HashFunc,
    typename KeyFromValue,
    typename PartitionPolicy
> class hashtable
{
    typedef PartitionPolicy partition_type;
    struct hash_node_base;
    struct hash_node;
public:
    typedef Key key_type;
    typedef Value value_type;
    typedef HashFunc hasher;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef thor_size_type size_type;
    typedef thor_diff_type difference_type;

    struct iterator_base : public iterator_type<bidirectional_iterator_tag, value_type>
    {
        hash_node* m_node;
        enum iter_mode
        {
            mode_list,
            mode_hash,
        } m_mode;
#ifdef THOR_DEBUG
        const hashtable* m_owner;
        iterator_base(hash_node* n, iter_mode m, const hashtable* o) : m_node(n), m_mode(m), m_owner(o) {}
#else
        iterator_base(hash_node* n, iter_mode m, const hashtable*) : m_node(n), m_mode(m) {}
#endif

        void verify_not_end() { THOR_DEBUG_ASSERT(m_owner->terminator() != m_node); }

        void decr()
        {
            THOR_DEBUG_ASSERT(m_owner->terminator() != (m_mode == mode_hash ? m_node->hashprev : m_node->listprev)); // Verify that we're not the front node
            m_node = m_mode == mode_hash ? m_node->hashprev : m_node->listprev;
        }
        void incr()
        {
            verify_not_end();
            m_node = m_mode == mode_hash ? m_node->hashnext : m_node->listnext;
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
        typedef typename iterator_base::iter_mode iter_mode;

        fwd_iterator(hash_node* n = 0, iter_mode m = mode_list, const hashtable* o = 0) : iterator_base(n, m, o) {}
        fwd_iterator(const nonconst_iterator& i) : iterator_base(i) {}
        selftype&  operator = (const nonconst_iterator& i)  { iterator_base::operator = (i); return *this; }
        reference  operator * ()                            { verify_not_end(); return m_node->value; }
        pointer    operator -> ()                           { verify_not_end(); return &(operator*()); }
        selftype&  operator -- ()     /* --iterator */      {                    decr(); return *this; }
        selftype   operator -- (int)  /* iterator-- */      { selftype n(*this); decr(); return n; }
        selftype&  operator ++ ()     /* ++iterator */      {                    incr(); return *this; }
        selftype   operator ++ (int)  /* iterator++ */      { selftype n(*this); incr(); return n; }
    };

    template<typename Traits> class rev_iterator : public iterator_base
    {
    public:
        typedef typename Traits::pointer pointer;
        typedef typename Traits::reference reference;
        typedef rev_iterator<nonconst_traits<value_type> > nonconst_iterator;
        typedef rev_iterator<Traits> selftype;
        typedef typename iterator_base::iter_mode iter_mode;

        rev_iterator(hash_node* n = 0, iter_mode m = mode_list, const hashtable* o = 0) : iterator_base(n, m, o) {}
        rev_iterator(const nonconst_iterator& i) : iterator_base(i) {}
        selftype&  operator = (const nonconst_iterator& i)  { iterator_base::operator = (i); return *this; }
        reference  operator * ()                            { verify_not_end(); return m_node->value; }
        pointer    operator -> ()                           { verify_not_end(); return &(operator*()); }
        selftype&  operator -- ()     /* --iterator */      {                    incr(); return *this; }
        selftype   operator -- (int)  /* iterator-- */      { selftype n(*this); incr(); return n; }
        selftype&  operator ++ ()     /* ++iterator */      {                    decr(); return *this; }
        selftype   operator ++ (int)  /* iterator++ */      { selftype n(*this); decr(); return n; }
    };

    typedef fwd_iterator<nonconst_traits<value_type> > iterator;
    typedef fwd_iterator<const_traits<value_type>    > const_iterator;
    typedef rev_iterator<nonconst_traits<value_type> > reverse_iterator;
    typedef rev_iterator<const_traits<value_type>    > const_reverse_iterator;

    // constructors
    hashtable() :
        m_root(terminator())
    {}

    hashtable(size_type n) :
        m_root(terminator())
    {
        resize(n);
    }

    hashtable(size_type n, const hasher& h) :
        m_root(terminator(), h)
    {
        resize(n);
    }

    hashtable(const hashtable& h) :
        m_root(terminator(), h.hash_funct())
    {
        resize(h.size());
        insert_equal(h.begin(false), h.end());
    }

    ~hashtable()
    {
        clear();
    }
    
    // iteration
    iterator begin(bool mode_hash)
    {
        return iterator(m_root.m_listhead, mode_hash ? iterator::mode_hash : iterator::mode_list, this);
    }

    iterator end()
    {
        return iterator(terminator(), iterator::mode_list, this);
    }

    const_iterator begin(bool mode_hash) const
    {
        return const_iterator(m_root.m_listhead, mode_hash ? iterator::mode_hash : iterator::mode_list, this);
    }

    const_iterator end() const
    {
        return const_iterator(terminator(), iterator::mode_list, this);
    }

    reverse_iterator rbegin(bool mode_hash)
    {
        return reverse_iterator(m_root.m_listtail, mode_hash ? iterator::mode_hash : iterator::mode_list, this);
    }

    reverse_iterator rend()
    {
        return reverse_iterator(terminator(), iterator::mode_list, this);
    }

    const_reverse_iterator rbegin(bool mode_hash) const
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

    hashtable& operator=(const hashtable& rhs)
    {
        clear();
        m_root = static_cast<const hasher&>(rhs.m_root);

        resize(rhs.size());
        insert_equal(rhs.begin(false), rhs.end());

        return *this;
    }

    void swap(hashtable& rhs)
    {
        // must fix up terminators first
        // also note that pointers must be assigned simultaneously (i.e. node.prev->next = node.next->prev = terminator() doesn't work)
        {
            hash_node *&Rhead = m_root.m_listhead->listprev, *&Rtail = m_root.m_listtail->listnext;
            hash_node *&Lhead = rhs.m_root.m_listhead->listprev, *&Ltail = rhs.m_root.m_listtail->listnext;
            Rhead = Rtail = rhs.terminator();
            Lhead = Ltail = terminator();
        }
        {
            hash_node *&Rhead = m_root.m_hashhead->hashprev, *&Rtail = m_root.m_hashtail->hashnext;
            hash_node *&Lhead = rhs.m_root.m_hashhead->hashprev, *&Ltail = rhs.m_root.m_hashtail->hashnext;
            Rhead = Rtail = rhs.terminator();
            Lhead = Ltail = terminator();
        }
        thor::swap(m_root, rhs.m_root);
    }

    void clear()
    {
        hash_node* p = m_root.m_listhead;
        m_root.m_listhead = m_root.m_listtail = terminator();
        m_root.m_hashhead = m_root.m_hashtail = terminator();
        while (p != m_root.m_listtail)
        {
            hash_node* next = p->listnext;
            destroy_node(p);
            p = next;
        }

        // clean up the buckets
        memory::align_alloc<hash_node*>::free(m_root.m_buckets);
        m_root.m_buckets = 0;
        m_root.m_bucket_count = 0;
        m_root.m_size = 0;
    }

    pair<iterator, bool> insert_unique(const value_type& v)
    {
        bool bnew;
        hash_node* node = internal_insert_unique(terminator(), KeyFromValue()(v), bnew);
        if (bnew)
        {
            typetraits<value_type>::construct(&node->value, v);
        }
        return pair<iterator, bool>(iterator(node, iterator::mode_hash, this), bnew);
    }

    template <typename InputIterator> void insert_unique(InputIterator first, InputIterator last)
    {
        while (first != last)
        {
            insert_unique(*first);
            ++first;
        }
    }

    value_type* key_insert_unique(const Key& k)
    {
        bool bnew;
        hash_node* node = internal_insert_unique(terminator(), k, bnew);
        if (!bnew)
        {
            // Must destruct so that caller can always construct
            typetraits<value_type>::destruct(&node->value);
        }
        return &node->value;
    }

    iterator insert_equal(const value_type& v)
    {
        hash_node* node = internal_insert_equal(terminator(), KeyFromValue()(v));
        typetraits<value_type>::construct(&node->value, v);
        return iterator(node, iterator::mode_hash, this);
    }

    template <typename InputIterator> void insert_equal(InputIterator first, InputIterator last)
    {
        while (first != last)
        {
            insert_equal(*first);
            ++first;
        }
    }

    value_type* key_insert_equal(const Key& k)
    {
        return &internal_insert_equal(terminator(), k)->value;
    }

    iterator iterator_from_value_type(const value_type& v)
    {
        THOR_DEBUG_ASSERT(&v != 0);
        hash_node* node = (hash_node*)(((thor_byte*)&v) - THOR_OFFSET_OF(hash_node, value));
        THOR_DEBUG_ASSERT(&node->value == &v);
        iterator iter(node, iterator::mode_hash, this);
#ifdef _DEBUG
        // Verify that the node is actually in the list
        const Key& key = KeyFromValue()(v);
        pair<iterator, iterator> range = equal_range(key);
        for ( ; range.first != range.second && range.first != iter; ++range.first ) {}
        THOR_DEBUG_ASSERT(range.first != range.second); // The node was not actually found in the tree.
#endif
        return iter;
    }

    void move(iterator which, iterator pos)
    {
        verify_iterator(which);
        verify_iterator(pos);
        which.verify_not_end();
        if (which.m_node != pos.m_node && which.m_node->listnext != pos.m_node && which.m_node != terminator())
        {
            // Remove from current
            which.m_node->listnext->listprev = which.m_node->listprev;
            which.m_node->listprev->listnext = which.m_node->listnext;

            // Insert before pos
            which.m_node->listnext = pos.m_node;
            which.m_node->listprev = pos.m_node->listprev;
            which.m_node->listnext->listprev = which.m_node;
            which.m_node->listprev->listnext = which.m_node;
        }
    }

    void erase(iterator pos)
    {
        verify_iterator(pos);
        pos.verify_not_end();
        internal_erase(pos.m_node);
    }

    size_type erase(const key_type& k)
    {
        hash_node* node = internal_find(k);
        if (node == terminator())
        {
            return 0;
        }

        size_type erasecount = 0;
        do 
        {
            hash_node* next = node->hashnext;
            internal_erase(node);
            node = next;
            ++erasecount;
        } while(node != terminator() && k == KeyFromValue()(node->value));

        THOR_DEBUG_ASSERT(count(k) == 0);
        
        return erasecount;
    }

    void erase(iterator first, iterator last)
    {
        while (first != last)
        {
            erase(first++); // Must postincrement
        }
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
        hash_node* node = internal_find(k);
        if (node == terminator())
        {
            return 0;
        }

        size_type count = 0;
        do
        {
            ++count;
            node = node->hashnext;
        } while (node != terminator() && (k == KeyFromValue()(node->value)));

        return count;
    }

    pair<const_iterator, const_iterator> equal_range(const key_type& k, size_type* count = 0) const
    {
        hash_node* first = internal_find(k);
        if (first == terminator())
        {
            if (count != 0)
            {
                *count = 0;
            }
            return pair<const_iterator, const_iterator>(end(), end());
        }

        size_type localcount = 1;
        hash_node* last = first->hashnext;
        while (last != terminator() && k == KeyFromValue()(last->value))
        {
            last = last->hashnext;
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
        hash_node* first = internal_find(k);
        if (first == terminator())
        {
            if (count != 0)
            {
                *count = 0;
            }
            return pair<iterator, iterator>(end(), end());
        }

        size_type localcount = 1;
        hash_node* last = first->hashnext;
        while (last != terminator() && k == KeyFromValue()(last->value))
        {
            last = last->hashnext;
            ++localcount;
        }

        if (count != 0)
        {
            *count = localcount;
        }

        return pair<iterator, iterator>(iterator(first, iterator::mode_hash, this), iterator(last, iterator::mode_hash, this));
    }

private:
    struct hash_node_base
    {
        hash_node*  listnext; // linked list of all items
        hash_node*  listprev;
        hash_node*  hashnext; // linked list of items in a particular hash bucket
        hash_node*  hashprev;
        hash_node_base() {}
        hash_node_base(hash_node* hn, hash_node* ln) : hashnext(hn), hashprev(hn->hashprev), listnext(ln), listprev(ln->listprev) {}
    };

    struct hash_node : public hash_node_base
    {
        size_type       hashval;
        value_type      value;
    };

    // Allocates and deallocates memory only. Return value is not constructed.
    hash_node* alloc_node()
    {
        return memory::align_alloc<hash_node>::alloc();
    }
    void dealloc_node(hash_node* node)
    {
        memory::align_alloc<hash_node>::free(node);
    }

    // Everything but hash_node::value has been constructed/set when this function returns
    hash_node* alloc_node(hash_node* hashnext, hash_node* listnext, size_type hashval)
    {
        hash_node* node = alloc_node();
        new (node) hash_node_base(hashnext, listnext);
        node->listnext->listprev = node;
        node->listprev->listnext = node;
        node->hashnext->hashprev = node;
        node->hashprev->hashnext = node;
        node->hashval = hashval;
        return node;
    }

    // Destroys the entire node.
    void destroy_node(hash_node* node)
    {
        typetraits<value_type>::destruct(&node->value);
        ((hash_node_base*)node)->~hash_node_base();
        dealloc_node(node);
    }

    hash_node* internal_insert_unique(hash_node* listwhere, const Key& k, bool& bnew)
    {
        resize(size() + 1);
        const size_type hashval = hash_funct()(k);
        const size_type bucket = partition_type::bucket_index(hashval, bucket_count());
        hash_node*& b = m_root.m_buckets[bucket];
        hash_node* iter = terminator();
        if (b != 0)
        {
            iter = b;
            do
            {
                if (iter->hashval == hashval)
                {
                    // Inner loop until we end this run of matching hashes
                    do 
                    {
                        if (k == KeyFromValue()(iter->value))
                        {
                            // Found matching entry
                            bnew = false;
                            return iter;
                        }
                        iter = iter->hashnext;
                    } while (iter != terminator() && iter->hashval == hashval);

                    // This ends a run of matching hashes, so insert here.
                    break;
                }
                iter = iter->hashnext;
            } while (iter != terminator() && partition_type::bucket_index(iter->hashval, bucket_count()) == bucket);
        }

        // Insert new node before iter
        THOR_DEBUG_ASSERT(b != iter);
        ++m_root.m_size;
        iter = alloc_node(iter, listwhere, hashval);
        if (b == 0)
        {
            b = iter;
        }
        bnew = true;
        return iter;
    }

    hash_node* internal_insert_equal(hash_node* listwhere, const Key& k)
    {
        resize(size() + 1);
        const size_type hashval = hash_funct()(k);
        const size_type bucket = partition_type::bucket_index(hashval, bucket_count());
        hash_node*& b = m_root.m_buckets[bucket];
        hash_node* iter = terminator();
        if (b != 0)
        {
            iter = b;
            do
            {
                if (iter->hashval == hashval)
                {
                    // Inner loop until we end this run of matching hashes
                    do 
                    {
                        bool keymatch = (k == KeyFromValue()(iter->value));
                        iter = iter->hashnext;
                        if (keymatch)
                        {
                            // Found matching key, just insert after so that we don't have to fix up the bucket
                            break;
                        }
                    } while (iter != terminator() && iter->hashval == hashval);

                    // This ends a run of matching hashes, so insert here.
                    break;
                }
                iter = iter->hashnext;
            } while (iter != terminator() && partition_type::bucket_index(iter->hashval, bucket_count()) == bucket);
        }

        // Insert new node before iter
        THOR_DEBUG_ASSERT(b != iter);
        ++m_root.m_size;
        iter = alloc_node(iter, listwhere, hashval);
        if (b == 0)
        {
            b = iter;
        }
        return iter;
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
            memory::align_alloc<hash_node*>::free(m_root.m_buckets);
            m_root.m_buckets = memory::align_alloc<hash_node*>::alloc(bc);
            m_root.m_bucket_count = bc;
            typetraits<hash_node*>::range_construct(m_root.m_buckets, m_root.m_buckets + bc);

            // Walk backwards. This will cause items within buckets to stay in the same order
            hash_node* node = m_root.m_hashtail;

            // Reset the root node so that we can rebuild the list
            m_root.m_hashhead = m_root.m_hashtail = terminator();
            while (node != terminator())
            {
                hash_node* prev = node->hashprev;

                // Current item may be at a new bucket index
                hash_node*& b = m_root.m_buckets[ partition_type::bucket_index(node->hashval, bucket_count()) ];
                hash_node* insertnode = b != 0 ? b : m_root.m_hashhead /*newroot.m_head*/;

                // Always add to the front of the bucket or, if bucket is empty, the entire list
                node->hashprev = insertnode->hashprev;
                node->hashnext = insertnode;
                node->hashprev->hashnext = node;
                node->hashnext->hashprev = node;
                b = node;

                node = prev;
            }
        }
    }

    hash_node* internal_find(const Key& k) const
    {
        if (bucket_count() != 0)
        {
            const size_type hashval = hash_funct()(k);
            const size_type bucket = partition_type::bucket_index(hashval, bucket_count());
            hash_node* node = m_root.m_buckets[bucket];
            if (node)
            {
                do
                {
                    if (node->hashval == hashval)
                    {
                        // Inner loop until we end this run of matching hashes
                        do 
                        {
                            if (k == KeyFromValue()(node->value))
                            {
                                // Found it
                                return node;
                            }
                            node = node->hashnext;
                        } while (node != terminator() && node->hashval == hashval);

                        // This ends a run of matching hashes without finding a key match
                        return terminator();
                    }
                    node = node->hashnext;
                } while (node != terminator() && partition_type::bucket_index(node->hashval, bucket_count()) == bucket);
            }
        }
        return terminator();
    }

    void internal_erase(hash_node* n)
    {
        n->hashprev->hashnext = n->hashnext;
        n->hashnext->hashprev = n->hashprev;
        n->listprev->listnext = n->listnext;
        n->listnext->listprev = n->listprev;
        const size_type bucket = partition_type::bucket_index(n->hashval, bucket_count());
        hash_node*& b = m_root.m_buckets[bucket];
        if (b == n)
        {
            if (n->hashnext == terminator() || partition_type::bucket_index(n->hashnext->hashval, bucket_count()) != bucket)
            {
                // bucket is now empty
                b = 0;
            }
            else
            {
                b = n->hashnext;
            }
        }
        destroy_node(n);
        --m_root.m_size;
    }

    hash_node* terminator() const { return (hash_node*)&m_root.m_listhead; }
    void verify_iterator(const iterator_base& i) const
    {
        THOR_UNUSED(i);
        THOR_ASSERT(i.m_owner == this);
    }

    hasher& hash_funct()
    {
        return static_cast<hasher&>(m_root);
    }

    // Inherit from hasher since hasher generally has no members
    struct empty_member_opt : public hasher
    {
        // The first four members of this structure must match hash_node_base
        hash_node*  m_listhead; // i.e. hash_node_base::listnext
        hash_node*  m_listtail; // i.e. hash_node_base::listprev
        hash_node*  m_hashhead; // i.e. hash_node_base::hashnext
        hash_node*  m_hashtail; // i.e. hash_node_base::hashprev
        hash_node** m_buckets;
        size_type m_bucket_count;
        size_type m_size;

        empty_member_opt(hash_node* term) :
            hasher(),
            m_listhead(term),
            m_listtail(term),
            m_hashhead(term),
            m_hashtail(term),
            m_buckets(0),
            m_bucket_count(0),
            m_size(0)
        {}

        empty_member_opt(hash_node* term, const hasher& h) :
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

