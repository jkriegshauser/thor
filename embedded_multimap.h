/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * embedded_multimap.h
 *
 * This file defines a multimap (red-black tree) variant that uses a link node embedded in the
 * stored class (Value)
 */

#ifndef THOR_TREE_H
#define THOR_TREE_H
#pragma once

#ifndef THOR_PAIR_H
#include "pair.h"
#endif

#ifndef THOR_ITERATOR_H
#include "iterator.h"
#endif

#ifndef THOR_ALGORITHM_H
#include "algorithm.h"
#endif

#ifndef THOR_MEMORY_H
#include "memory.h"
#endif

namespace thor
{

//
// Prototypes
//

template<class Key, class Value> class embedded_multimap_link;
template<class Key, class Value, embedded_multimap_link<Key, Value> Value::*LINK, class Compare> class embedded_multimap;

//
// embedded_multimap_link
//
template<class Key, class Value> class embedded_multimap_link
{
    THOR_DECLARE_NOCOPY(embedded_multimap_link);
    enum { alignment = memory::align_selector<Key>::alignment };

public:
    typedef Key     key_type;
    typedef Value   value_type;
    typedef Value*  pointer;

    embedded_multimap_link()
        : color_(uninit)
#ifdef THOR_DEBUG
        , keydebug_(key())
#endif
    {
        THOR_COMPILETIME_ASSERT(THOR_OFFSET_OF(embedded_multimap_link, parent_) == 0, InvalidAssumption);
        clear(false);
    }
    ~embedded_multimap_link() { verify_free(); }

    bool is_contained() const { return color_ != uninit; }

    // Key retrieval
    const key_type& key() const { return *(key_type*)memory::align_forward<alignment>(keybuf_); }

#if 0
protected:
    typedef embedded_multimap_link<key_type, value_type> selftype;
    template <class key_type, class value_type, selftype value_type::*LINK, class Compare> friend class embedded_multimap;
#endif

    enum node_color
    {
        uninit = -1,
        red = 0,
        black = 1
    };

    value_type*  parent_;
    value_type*  left_;
    value_type*  right_;
    node_color  color_;

    void verify_free() const
    {
        verify_owner(0);
    }

    void clear(bool destroyKey = true)
    {
        if (color_ != uninit && destroyKey)
        {
            const_cast<key_type&>(key()).~key_type();
        }
        parent_ = left_ = right_ = 0;
        color_ = uninit;
        set_owner(0);
        THOR_DEBUG_INIT_MEM(keybuf_, sizeof(keybuf_), 0xcc);
    }

    // The key is implemented as a buffer so that it is not constructed until necessary
    byte keybuf_[sizeof(key_type) + alignment];

#ifdef THOR_DEBUG
    const key_type& keydebug_;
    void* owner_;
    void set_owner(void* o) { owner_ = o; }
    void verify_owner(void* o) const
    {
        THOR_DEBUG_ASSERT(owner_ == o);
        THOR_DEBUG_ASSERT(is_contained() == (owner_ != 0));
    }
#else
    void set_owner(void*) {}
    void verify_owner(void*) const {}
#endif
};

template <class Key, class Value, embedded_multimap_link<Key, Value> Value::*LINK, class Compare = less<Key> >
class embedded_multimap
{
    THOR_DECLARE_NOCOPY(embedded_multimap);
public:
    typedef Key key_type;
    typedef Value value_type;
    typedef Compare key_compare;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef thor_size_type size_type;
    typedef thor_diff_type difference_type;

    typedef embedded_multimap_link<key_type, value_type> link_type;

    // iterator definitions
    struct iterator_base : public iterator_type<bidirectional_iterator_tag, value_type>
    {
        pointer node_;
#ifdef THOR_DEBUG
        const embedded_multimap* owner_;
        iterator_base(value_type* n, const embedded_multimap* o) : node_(n), owner_(o) {}
#else
        iterator_base(value_type* n, const embedded_multimap*) : node_(n) {}
#endif
        void verify_not_end() const { THOR_DEBUG_ASSERT(owner_->end().node_ != node_);   }

        void decr()
        {
            link_type& link = node_->*LINK;
            if (link.color_ == link_type::red && (link.parent_->*LINK).parent_ == node_)
            {
                node_ = link.right_;
            }
            else if (link.left_ != 0)
            {
                value_type* y = link.left_;
                while ((y->*LINK).right_ != 0)
                {
                    y = (y->*LINK).right_;
                }
                node_ = y;
            }
            else
            {
                value_type* y = link.parent_;
                while (node_ == (y->*LINK).left_)
                {
                    node_ = y;
                    y = (y->*LINK).parent_;
                }
                node_ = y;
            }
        }

        void incr()
        {
            link_type& link = node_->*LINK;
            verify_not_end();
            if (link.right_ != 0)
            {
                node_ = link.right_;
                while ((node_->*LINK).left_ != 0)
                {
                    node_ = (node_->*LINK).left_;
                }
            }
            else
            {
                value_type* y = link.parent_;
                while (node_ == (y->*LINK).right_)
                {
                    node_ = y;
                    y = (y->*LINK).parent_;
                }
                if ((node_->*LINK).right_ != y)
                {
                    node_ = y;
                }
            }
        }

        bool operator == (const iterator_base& i) const { THOR_DEBUG_ASSERT(owner_ == i.owner_); return node_ == i.node_; }
        bool operator != (const iterator_base& i) const { THOR_DEBUG_ASSERT(owner_ == i.owner_); return node_ != i.node_; }
    };

    template<class Traits> class fwd_iterator : public iterator_base
    {
    public:
        typedef typename Traits::pointer pointer;
        typedef typename Traits::reference reference;
        typedef fwd_iterator<nonconst_traits<value_type> > nonconst_iterator;
        typedef fwd_iterator<Traits> selftype;

        fwd_iterator(value_type* n = 0, const embedded_multimap* o = 0) : iterator_base(n, o) {}
        fwd_iterator(const nonconst_iterator& i) : iterator_base(i) {}
        selftype&  operator = (const nonconst_iterator& i)  { iterator_base::operator = (i); return *this; }
        reference  operator * ()     const                  { verify_not_end(); return *node_; }
        pointer    operator -> ()    const                  { verify_not_end(); return &(operator*()); }
        selftype&  operator -- ()     /* --iterator */      {                    decr(); return *this; }
        selftype   operator -- (int)  /* iterator-- */      { selftype n(*this); decr(); return n; }
        selftype&  operator ++ ()     /* ++iterator */      {                    incr(); return *this; }
        selftype   operator ++ (int)  /* iterator++ */      { selftype n(*this); incr(); return n; }

        const key_type& key() const { verify_not_end(); return (node_->*LINK).key(); }
    };

    template<class Traits> class rev_iterator : public iterator_base
    {
    public:
        typedef typename Traits::pointer pointer;
        typedef typename Traits::reference reference;
        typedef rev_iterator<nonconst_traits<value_type> > nonconst_iterator;
        typedef rev_iterator<Traits> selftype;

        rev_iterator(value_type* n = 0, const embedded_multimap* o = 0) : iterator_base(n, o) {}
        rev_iterator(const nonconst_iterator& i) : iterator_base(i) {}
        selftype&  operator = (const nonconst_iterator& i)  { iterator_base::operator = (i); return *this; }
        reference  operator * ()     const                  { verify_not_end(); return *node_; }
        pointer    operator -> ()    const                  { verify_not_end(); return &(operator*()); }
        selftype&  operator -- ()     /* --iterator */      {                    incr(); return *this; }
        selftype   operator -- (int)  /* iterator-- */      { selftype n(*this); incr(); return n; }
        selftype&  operator ++ ()     /* ++iterator */      {                    decr(); return *this; }
        selftype   operator ++ (int)  /* iterator++ */      { selftype n(*this); decr(); return n; }

        const key_type& key() const { verify_not_end(); return (node_->*LINK).key(); }
    };

    typedef fwd_iterator<nonconst_traits<value_type> > iterator;
    typedef fwd_iterator<const_traits<value_type>    > const_iterator;

    typedef rev_iterator<nonconst_traits<value_type> > reverse_iterator;
    typedef rev_iterator<const_traits<value_type>    > const_reverse_iterator;

    // constructors
    embedded_multimap() :
        m_root(terminator())
    {}

    embedded_multimap(const key_compare& k) :
        m_root(terminator(), k)
    {}

    ~embedded_multimap()
    {
        // Should be empty at destruction time since we don't own the elements
        THOR_DEBUG_ASSERT(empty());
        remove_all();
    }

    // Size
    bool empty() const { return size() == 0; }
    size_type size() const { return m_root.size_; }
    size_type max_size() const { return size_type(-1); }
    const key_compare& key_comp() const { return m_root; }

    // Iteration
    iterator begin()                        { return iterator(m_root.left_, this); }
    iterator end()                          { return iterator(terminator(), this); }
    const_iterator begin() const            { return const_iterator(m_root.left_, this); }
    const_iterator end() const              { return const_iterator(terminator(), this); }
    const_iterator cbegin() const           { return const_iterator(m_root.left_, this); }
    const_iterator cend() const             { return const_iterator(terminator(), this); }
    reverse_iterator rbegin()               { return reverse_iterator(m_root.right_, this); }
    reverse_iterator rend()                 { return reverse_iterator(terminator(), this); }
    const_reverse_iterator rbegin() const   { return const_reverse_iterator(m_root.right_, this); }
    const_reverse_iterator rend() const     { return const_reverse_iterator(terminator(),  this); }
    const_reverse_iterator crbegin() const  { return const_reverse_iterator(m_root.right_, this); }
    const_reverse_iterator crend() const    { return const_reverse_iterator(terminator(),  this); }

    // Searching
    iterator lower_bound(const key_type& k)
    {
        return iterator(lower_bound_internal(k), this);
    }

    const_iterator lower_bound(const key_type& k) const
    {
        return const_iterator(lower_bound_internal(k), this);
    }

    iterator upper_bound(const key_type& k)
    {
        return iterator(upper_bound_internal(k), this);
    }

    const_iterator upper_bound(const key_type& k) const
    {
        return const_iterator(upper_bound_internal(k), this);
    }

    pair<iterator,iterator> equal_range(const key_type& k)
    {
        return pair<iterator,iterator>(lower_bound(k),upper_bound(k));
    }

    pair<const_iterator,const_iterator> equal_range(const key_type& k) const
    {
        return pair<const_iterator,const_iterator>(lower_bound(k),upper_bound(k));
    }

    void swap(embedded_multimap& rhs)
    {
        thor::swap(m_root, rhs.m_root);
        // Fixup terminators if in use
        if (rhs.m_root.left_ == terminator())
        {
            THOR_ASSERT(rhs.m_root.right_ == terminator());
            rhs.m_root.left_ = rhs.m_root.right_ = rhs.terminator();
        }
        else
        {
            THOR_ASSERT(rhs.m_root.parent_ != 0 && link(rhs.m_root.parent_).parent_ == terminator());
            link(rhs.m_root.parent_).parent_ = rhs.terminator();
        }
        if (m_root.left_ == rhs.terminator())
        {
            THOR_ASSERT(m_root.right_ == rhs.terminator());
            m_root.left_ = m_root.right_ = terminator();
        }
        else
        {
            THOR_ASSERT(m_root.parent_ != 0 && link(m_root.parent_).parent_ == rhs.terminator());
            link(m_root.parent_).parent_ = terminator();
        }

        internal_set_owner(m_root.parent_);
        rhs.internal_set_owner(rhs.m_root.parent_);
    }

    iterator insert(const key_type& k, pointer p)
    {
        THOR_DEBUG_ASSERT(p != 0);
        THOR_DEBUG_ASSERT(!link(p).is_contained());
        internal_insert(k, p);
        return iterator(p, this);
    }

    pointer remove(iterator pos)
    {
        verify_iterator(pos);
        pos.verify_not_end();
        return internal_remove(pos.node_);
    }

    pointer remove(pointer pos)
    {
        THOR_DEBUG_ASSERT(pos != 0);
        THOR_ASSERT(pos != terminator());
        THOR_ASSERT(link(pos).is_contained());
        link(pos).verify_owner(this);
        return internal_remove(pos);
    }

    template<class Pred> void remove(pointer pos, Pred pred)
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

    size_type remove(const key_type& x)
    {
        pair<iterator,iterator> p = equal_range(x);
        return remove(p.first, p.second);
    }
  
    size_type remove(iterator first, iterator last)
    {
        // verify that the iterators belong to this container
        verify_iterator(first);
        verify_iterator(last);
        if (first == begin() && last == end())
        {
            size_type n = size();
            remove_all();
            return n;
        }
        else
        {
            size_type n = 0;
            while (first != last)
            {
                remove(first++); // must postincrement
                ++n;
            }
            return n;
        }
    }

    size_type remove(const key_type* first, const key_type* last)
    {
        size_type n = 0;
        while (first != last)
        {
            n += remove(*first++);
        }
        return n;
    }

    size_type remove_delete(const key_type& x)
    {
        pair<iterator,iterator> p = equal_range(x);
        return remove_delete(p.first, p.second);
    }

    size_type remove_delete(iterator first, iterator last)
    {
        // verify that the iterators belong to this container
        verify_iterator(first);
        verify_iterator(last);
        if (first == begin() && last == end())
        {
            size_type n = size();
            delete_all();
            return n;
        }
        else
        {
            size_type n = 0;
            while (first != last)
            {
                remove_delete(first++); // must postincrement
                ++n;
            }
            return n;
        }
    }

    size_type remove_delete(const key_type* first, const key_type* last)
    {
        size_type n = 0;
        while (first != last)
        {
            n += remove_delete(*first++);
        }
        return n;
    }

    void remove_all()
    {
        internal_remove_all(m_root.parent_);
        m_root.left_ = m_root.right_ = terminator();
        m_root.parent_ = 0;
        m_root.size_ = 0;
    }

    void delete_all()
    {
        internal_delete_all(m_root.parent_);
        m_root.left_ = m_root.right_ = terminator();
        m_root.parent_ = 0;
        m_root.size_ = 0;
    }

    size_type count(const key_type& k) const
    {
        pair<const_iterator,const_iterator> p(equal_range(k));
        return (size_type)distance(p.first, p.second);
    }

    template<class K> iterator find(const K& k) { return iterator(internal_find(k), this); }
    template<class K> const_iterator find(const K& k) const { return const_iterator(internal_find(k), this); }

private:
    // Use empty member optimization since key_compare is likely going to be an
    // empty class.
    struct root_node : public key_compare
    {
        // The first three members of this structure must match embedded_multimap_link
        pointer         parent_;
        pointer         left_;
        pointer         right_;
        size_type       size_;

        root_node(pointer term) :
            key_compare(),
            parent_(0),
            left_(term),
            right_(term),
            size_(0)
        {}
        
        root_node(pointer term, const key_compare& k) :
            key_compare(k),
            parent_(0)
            left_(term),
            right_(term),
            size_(0)
        {}

        root_node& operator = (const key_compare& kc)
        {
            key_compare::operator = (kc);
            return *this;
        }
    };

    root_node m_root;

    pointer terminator() const { return (pointer)((byte*)&m_root.parent_ - THOR_OFFSET_OF(value_type, *LINK)); }
    void verify_iterator(const iterator_base& i) const { THOR_UNUSED(i); THOR_ASSERT(i.owner_ == this); }

    key_compare& key_comp()
    {
        return static_cast<key_compare&>(m_root);
    }

    static link_type& link(pointer p)
    {
        return p->*LINK;
    }
    
    static const link_type& link(const_pointer p)
    {
        return p->*LINK;
    }

    // utility functions
    static pointer minimum(pointer x)
    {
        while (link(x).left_ != 0)
        {
            x = link(x).left_;
        }
        return x;
    }

    static pointer maximum(pointer x)
    {
        while (link(x).right_ != 0)
        {
            x = link(x).right_;
        }
        return x;
    }

    // Does not construct the value_type
    void internal_insert(const key_type& key, pointer which)
    {
        // Construct a copy of the key
        link_type& l = link(which);
        l.verify_free();
        
        l.color_ = link_type::red;
        l.left_ = l.right_ = 0;
        new (&const_cast<key_type&>(l.key())) key_type(key);

        pointer y = terminator();
        pointer x = m_root.parent_;
        while (x != 0)
        {
            y = x;
            x = key_comp()(key, link(x).key()) ? link(x).left_ : link(x).right_;
        }

        if (y == terminator() ||
            x != 0 ||     // If x != 0, the remainder succeeds to true
            key_comp()(key, link(y).key()))
        {
            link(y).left_ = which;
            if (y == terminator())
            {
                m_root.parent_ = which;
                m_root.right_ = which;
            }
            else if (y == m_root.left_)
            {
                m_root.left_ = which;   // maintain leftmost pointing to min node
            }
        }
        else
        {
            link(y).right_ = which;
            if (y == m_root.right_)
            {
                m_root.right_ = which;  // maintain rightmost pointing to max node
            }
        }
        l.parent_ = y;
        l.set_owner(this);
        rebalance(which, m_root.parent_);
        ++m_root.size_;
    }

    void rebalance(pointer x, pointer& root)
    {
        link(x).color_ = link_type::red;
        while (x != root && link(link(x).parent_).color_ == link_type::red)
        {
            if (link(x).parent_ == link(link(link(x).parent_).parent_).left_)
            {
                pointer y = link(link(link(x).parent_).parent_).right_;
                if (y && link(y).color_ == link_type::red)
                {
                    link(link(x).parent_).color_ = link_type::black;
                    link(y).color_ = link_type::black;
                    link(link(link(x).parent_).parent_).color_ = link_type::red;
                    x = link(link(x).parent_).parent_;
                }
                else
                {
                    if (x == link(link(x).parent_).right_)
                    {
                        x = link(x).parent_;
                        rotate_left(x, root);
                    }
                    link(link(x).parent_).color_ = link_type::black;
                    link(link(link(x).parent_).parent_).color_ = link_type::red;
                    rotate_right(link(link(x).parent_).parent_, root);
                }
            }
            else
            {
                pointer y = link(link(link(x).parent_).parent_).left_;
                if (y && link(y).color_ == link_type::red)
                {
                    link(link(x).parent_).color_ = link_type::black;
                    link(y).color_ = link_type::black;
                    link(link(link(x).parent_).parent_).color_ = link_type::red;
                    x = link(link(x).parent_).parent_;
                }
                else
                {
                    if (x == link(link(x).parent_).left_)
                    {
                        x = link(x).parent_;
                        rotate_right(x, root);
                    }
                    link(link(x).parent_).color_ = link_type::black;
                    link(link(link(x).parent_).parent_).color_ = link_type::red;
                    rotate_left(link(link(x).parent_).parent_, root);
                }
            }
        }
        link(root).color_ = link_type::black;
    }

    pointer internal_remove(pointer pos)
    {
        THOR_DEBUG_ASSERT(pos != terminator());
        pos = rebalance_for_remove(pos, m_root.parent_, m_root.left_, m_root.right_);
        THOR_DEBUG_ASSERT(m_root.size_ != 0);
        --m_root.size_;
        link(pos).clear(true);
        return pos;
    }
    
    pointer rebalance_for_remove(pointer z,
					            pointer& root,
					            pointer& leftmost,
					            pointer& rightmost)
    {
        pointer y = z;
        pointer x = 0;
        pointer x_parent = 0;
        if (link(y).left_ == 0)     // z has at most one non-null child. y == z.
        {
            x = link(y).right_;     // x might be null.
        }
        else if (link(y).right_ == 0)  // z has exactly one non-null child. y == z.
        {
            x = link(y).left_;    // x is not null.
        }
        else                     // z has two non-null children.  Set y to
        {
            y = link(y).right_;   //   z's successor.  x might be null.
            while (link(y).left_ != 0)
            {
                y = link(y).left_;
            }
            x = link(y).right_;
        }
        if (y != z)          // relink y in place of z.  y is z's successor
        {
            link(link(z).left_).parent_ = y; 
            link(y).left_ = link(z).left_;
            if (y != link(z).right_)
            {
                x_parent = link(y).parent_;
                if (x != 0)
                {
                    link(x).parent_ = link(y).parent_;
                }
                link(link(y).parent_).left_ = x;      // y must be a child of left
                link(y).right_ = link(z).right_;
                link(link(z).right_).parent_ = y;
            }
            else
            {
                x_parent = y;
            }
            if (root == z)
            {
                root = y;
            }
            else if (link(link(z).parent_).left_ == z)
            {
                link(link(z).parent_).left_ = y;
            }
            else 
            {
                link(link(z).parent_).right_ = y;
            }
            link(y).parent_ = link(z).parent_;
            thor::swap(link(y).color_, link(z).color_);
            y = z;
            // y now points to node to be actually deleted
        }
        else                        // y == z
        {
            x_parent = link(y).parent_;
            if (x)
            {
                link(x).parent_ = link(y).parent_;
            }
            if (root == z)
            {
                root = x;
            }
            else if (link(link(z).parent_).left_ == z)
            {
                link(link(z).parent_).left_ = x;
            }
            else
            {
                link(link(z).parent_).right_ = x;
            }
            if (leftmost == z) 
            {
                if (link(z).right_ == 0)        // z->left must be null also
                {
                    leftmost = link(z).parent_;
                }
                // makes leftmost == header if z == root
                else
                {
                    leftmost = minimum(x);
                }
            }
            if (rightmost == z)  
            {
                if (link(z).left_ == 0)         // z->right must be null also
                {
                    rightmost = link(z).parent_;
                }
                // makes rightmost == header if z == root
                else                      // x == z->left
                {
                    rightmost = maximum(x);
                }
            }
        }
        if (link(y).color_ != link_type::red)
        { 
            while (x != root && (x == 0 || link(x).color_ == link_type::black))
            {
                if (x == link(x_parent).left_)
                {
                    pointer w = link(x_parent).right_;
                    if (link(w).color_ == link_type::red)
                    {
                        link(w).color_ = link_type::black;
                        link(x_parent).color_ = link_type::red;
                        rotate_left(x_parent, root);
                        w = link(x_parent).right_;
                    }
                    if ((link(w).left_ == 0 || 
                         link(link(w).left_).color_ == link_type::black) && (link(w).right_ == 0 || 
                         link(link(w).right_).color_ == link_type::black))
                    {
                        link(w).color_ = link_type::red;
                        x = x_parent;
                        x_parent = link(x_parent).parent_;
                    }
                    else
                    {
                        if (link(w).right_ == 0 || 
                            link(link(w).right_).color_ == link_type::black)
                        {
                            if (link(w).left_)
                            {
                                link(link(w).left_).color_ = link_type::black;
                            }
                            link(w).color_ = link_type::red;
                            rotate_right(w, root);
                            w = link(x_parent).right_;
                        }
                        link(w).color_ = link(x_parent).color_;
                        link(x_parent).color_ = link_type::black;
                        if (link(w).right_)
                        {
                            link(link(w).right_).color_ = link_type::black;
                        }
                        rotate_left(x_parent, root);
                        break;
                    }
                }
                else                  // same as above, with right <-> left.
                {
                    pointer w = link(x_parent).left_;
                    if (link(w).color_ == link_type::red)
                    {
                        link(w).color_ = link_type::black;
                        link(x_parent).color_ = link_type::red;
                        rotate_right(x_parent, root);
                        w = link(x_parent).left_;
                    }
                    if ((link(w).right_ == 0 || 
                         link(link(w).right_).color_ == link_type::black) && (link(w).left_ == 0 || 
                         link(link(w).left_).color_ == link_type::black))
                    {
                        link(w).color_ = link_type::red;
                        x = x_parent;
                        x_parent = link(x_parent).parent_;
                    }
                    else
                    {
                        if (link(w).left_ == 0 || 
                            link(link(w).left_).color_ == link_type::black)
                        {
                            if (link(w).right_)
                            {
                                link(link(w).right_).color_ = link_type::black;
                            }
                            link(w).color_ = link_type::red;
                            rotate_left(w, root);
                            w = link(x_parent).left_;
                        }
                        link(w).color_ = link(x_parent).color_;
                        link(x_parent).color_ = link_type::black;
                        if (link(w).left_)
                        {
                            link(link(w).left_).color_ = link_type::black;
                        }
                        rotate_right(x_parent, root);
                        break;
                    }
                }
            }
            if (x)
            {
                link(x).color_ = link_type::black;
            }
        }
        return y;
    }

    void rotate_left(pointer x, pointer& root)
    {
        pointer y = link(x).right_;
        link(x).right_ = link(y).left_;
        if (link(y).left_ != 0)
        {
            link(link(y).left_).parent_ = x;
        }
        link(y).parent_ = link(x).parent_;

        if (x == root)
        {
            root = y;
        }
        else if (x == link(link(x).parent_).left_)
        {
            link(link(x).parent_).left_ = y;
        }
        else
        {
            link(link(x).parent_).right_ = y;
        }
        link(y).left_ = x;
        link(x).parent_ = y;
    }

    void rotate_right(pointer x, pointer& root)
    {
        pointer y = link(x).left_;
        link(x).left_ = link(y).right_;
        if (link(y).right_ != 0)
        {
            link(link(y).right_).parent_ = x;
        }
        link(y).parent_ = link(x).parent_;

        if (x == root)
        {
            root = y;
        }
        else if (x == link(link(x).parent_).right_)
        {
            link(link(x).parent_).right_ = y;
        }
        else
        {
            link(link(x).parent_).left_ = y;
        }
        link(y).right_ = x;
        link(x).parent_ = y;
    }

    template <class K> pointer internal_find(const K& k) const
    {
        pointer y = terminator();           // Last node which is not less than k. 
        pointer x = m_root.parent_;         // Current node. 

        while (x != 0)
        {
            if (!key_comp()(link(x).key(), k))
            {
                y = x, x = link(x).left_;
            }
            else
            {
                x = link(x).right_;
            }
        }
        if (y == terminator() || key_comp()(k, link(y).key()))
        {
            y = terminator();
        }
        return y;
    }

    pointer lower_bound_internal(const key_type& k) const
    {
        pointer y = terminator();           // Last node which is not less than k.
        pointer x = m_root.parent_;         // Current node.
    
        while (x != 0)
        {
            if (!key_comp()(link(x).key(), k))
            {
                y = x, x = link(x).left_;
            }
            else
            {
                x = link(x).right_;
            }
        }
    
        return y;
    }

    pointer upper_bound_internal(const key_type& k) const
    {
        pointer y = terminator(); // Last node which is greater than k.
        pointer x = m_root.parent_; // Current node.
    
        while (x != 0) 
        {
            if (key_comp()(k, link(x).key()))
            {
                y = x, x = link(x).left_;
            }
            else
            {
                x = link(x).right_;
            }
        }
        return y;
    }

    void internal_remove_all(pointer p)
    {
        while (p)
        {
            internal_remove_all(link(p).right_);
            pointer next = link(p).left_;
            link(p).clear(true);
            p = next;
        }
    }

    void internal_delete_all(pointer p)
    {
        while (p)
        {
            internal_delete_all(link(p).right_);
            pointer next = link(p).left_;
            link(p).clear(true);
            delete p;
            p = next;
        }
    }

    void internal_set_owner(pointer p)
    {
        THOR_UNUSED(p);
#ifdef THOR_DEBUG
        while (p)
        {
            internal_set_owner(link(p).right_);
            link(p).set_owner(this);
            p = link(p).left_;
        }
#endif
    }
};

}; // namespace thor

#endif

