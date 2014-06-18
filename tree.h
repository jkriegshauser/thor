/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * tree.h
 *
 * ** THOR INTERNAL FILE - NOT FOR APPLICATION USE **
 *
 * This file defines a red-black tree to be used as a base for tree-type containers (map, set, multimap, multiset).
 * This class is not intended to be used outside of internal THOR implementation.
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

template <class Key, class Value, class KeyFromValue, class Compare>
class red_black_tree
{
    enum node_color
    {
        red = 0,
        black = 1
    };

    struct tree_node_base;
    struct tree_node;
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

public:
    // iterator definitions
    struct iterator_base : public iterator_type<bidirectional_iterator_tag, value_type>
    {
        tree_node* m_node;
#ifdef THOR_DEBUG
        const red_black_tree* m_owner;
        iterator_base(tree_node* n, const red_black_tree* o) : m_node(n), m_owner(o) {}
#else
        iterator_base(tree_node* n, const red_black_tree*) : m_node(n) {}
#endif
        void verify_not_end() const { THOR_DEBUG_ASSERT(m_owner->end().m_node != m_node);   }

        void decr()
        {
            if (m_node->color == red && m_node->parent->parent == m_node)
            {
                m_node = m_node->right;
            }
            else if (m_node->left != 0)
            {
                tree_node* y = m_node->left;
                while (y->right != 0)
                {
                    y = y->right;
                }
                m_node = y;
            }
            else
            {
                tree_node* y = m_node->parent;
                while (m_node == y->left)
                {
                    m_node = y;
                    y = y->parent;
                }
                m_node = y;
            }
        }

        void incr()
        {
            verify_not_end();
            if (m_node->right != 0)
            {
                m_node = m_node->right;
                while (m_node->left != 0)
                {
                    m_node = m_node->left;
                }
            }
            else
            {
                tree_node* y = m_node->parent;
                while (m_node == y->right)
                {
                    m_node = y;
                    y = y->parent;
                }
                if (m_node->right != y)
                {
                    m_node = y;
                }
            }
        }

        bool operator == (const iterator_base& i) const { THOR_DEBUG_ASSERT(m_owner == i.m_owner); return m_node == i.m_node; }
        bool operator != (const iterator_base& i) const { THOR_DEBUG_ASSERT(m_owner == i.m_owner); return m_node != i.m_node; }
    };

    template<class Traits> class fwd_iterator : public iterator_base
    {
    public:
        typedef typename Traits::pointer pointer;
        typedef typename Traits::reference reference;
        typedef fwd_iterator<nonconst_traits<value_type> > nonconst_iterator;
        typedef fwd_iterator<Traits> selftype;

        fwd_iterator(tree_node* n = 0, const red_black_tree* o = 0) : iterator_base(n, o) {}
        fwd_iterator(const nonconst_iterator& i) : iterator_base(i) {}
        selftype&  operator = (const nonconst_iterator& i)  { iterator_base::operator = (i); return *this; }
        reference  operator * ()     const                  { verify_not_end(); return m_node->value; }
        pointer    operator -> ()    const                  { verify_not_end(); return &(operator*()); }
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
        typedef rev_iterator<nonconst_traits<value_type> > nonconst_iterator;
        typedef rev_iterator<Traits> selftype;

        rev_iterator(tree_node* n = 0, const red_black_tree* o = 0) : iterator_base(n, o) {}
        rev_iterator(const nonconst_iterator& i) : iterator_base(i) {}
        selftype&  operator = (const nonconst_iterator& i)  { iterator_base::operator = (i); return *this; }
        reference  operator * ()     const                  { verify_not_end(); return m_node->value; }
        pointer    operator -> ()    const                  { verify_not_end(); return &(operator*()); }
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
    red_black_tree() :
        m_root(terminator())
    {}

    red_black_tree(const key_compare& k) :
        m_root(terminator(), k)
    {}

    red_black_tree(const red_black_tree& tree) :
        m_root(terminator(), tree.key_comp())
    {
        if (!tree.empty())
        {
            m_root.color = red;
            m_root.parent = copy(tree.m_root.parent, terminator());
            m_root.left = minimum(m_root.parent);
            m_root.right = maximum(m_root.parent);
            m_root.m_size = tree.m_root.m_size;
        }
    }

    ~red_black_tree()
    {
        clear();
    }

    red_black_tree& operator = (const red_black_tree& rhs)
    {
        if (this != &rhs)
        {
            clear();
            m_root.m_size = 0;
    
            // Copy the key comparator.
            m_root = rhs.key_comp();

            if (rhs.m_root.parent == 0)
            {
                m_root.parent = 0;
                m_root.left = m_root.right = terminator();
            }
            else
            {
                m_root.parent = copy(rhs.m_root.parent, terminator());
                m_root.left = minimum(m_root.parent);
                m_root.right = maximum(m_root.parent);
                m_root.m_size = rhs.m_root.m_size;
            }
        }
        return *this;
    }

    // Size
    bool empty() const
    {
        return size() == 0;
    }

    size_type size() const
    {
        return m_root.m_size;
    }
    
    size_type max_size() const
    {
        return size_type(-1);
    }

    const key_compare& key_comp() const
    {
        return m_root;
    }

    // Iteration
    iterator begin()
    {
        return iterator(m_root.left, this);
    }

    const_iterator begin() const
    {
        return const_iterator(m_root.left, this);
    }

    iterator end()
    {
        return iterator(terminator(), this);
    }

    const_iterator end() const
    {
        return const_iterator(terminator(), this);
    }

    reverse_iterator rbegin()
    {
        return reverse_iterator(m_root.right, this);
    }
    
    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(m_root.right, this);
    }

    reverse_iterator rend()
    {
        return reverse_iterator(terminator(), this);
    }

    const_reverse_iterator rend() const
    {
        return const_reverse_iterator(terminator(), this);
    }

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

    void swap(red_black_tree& rhs)
    {
        thor::swap(m_root, rhs.m_root);
        // Fixup terminators if in use
        if (rhs.m_root.left == terminator())
        {
            THOR_ASSERT(rhs.m_root.right == terminator());
            rhs.m_root.left = rhs.m_root.right = rhs.terminator();
        }
        else
        {
            THOR_ASSERT(rhs.m_root.parent != 0 && rhs.m_root.parent->parent == terminator());
            rhs.m_root.parent->parent = rhs.terminator();
        }
        if (m_root.left == rhs.terminator())
        {
            THOR_ASSERT(m_root.right == rhs.terminator());
            m_root.left = m_root.right = terminator();
        }
        else
        {
            THOR_ASSERT(m_root.parent != 0 && m_root.parent->parent == rhs.terminator());
            m_root.parent->parent = terminator();
        }
    }

    void clear()
    {
        internal_erase(m_root.parent);        
        m_root.left = m_root.right = terminator();
        m_root.parent = 0;
        m_root.color = red;
        m_root.m_size = 0;
    }

    pair<iterator, bool> insert_unique(const value_type& v)
    {
        const Key& key = KeyFromValue()(v);
        bool bnew;
        tree_node* node = internal_insert_unique(key, bnew);
        if (bnew)
        {
            typetraits<value_type>::construct(&node->value, v);
        }
        return pair<iterator, bool>(iterator(node, this), bnew);
    }

    // The value_type returned always needs to be constructed
    value_type* key_insert_unique(const key_type& key)
    {
        bool bnew;
        value_type* p = &internal_insert_unique(key, bnew)->value;
        if (!bnew)
        {
            // Need to destruct the current value so that the caller can always construct it.
            typetraits<value_type>::destruct(p);
        }
        return p;
    }

    template <class InputIterator> void insert_unique(InputIterator first, InputIterator last)
    {
        for (; first != last; ++first)
        {
            insert_unique(*first);
        }
    }

    iterator insert_equal(const value_type& v)
    {
        tree_node* node = internal_insert_equal(KeyFromValue()(v));
        typetraits<value_type>::construct(&node->value, v);
        return iterator(node, this);
    }

    value_type* key_insert_equal(const key_type& key)
    {
        return &internal_insert_equal(key)->value;
    }

    template <class InputIterator> void insert_equal(InputIterator first, InputIterator last)
    {
        for (; first != last; ++first)
        {
            insert_equal(*first);
        }
    }

    iterator iterator_from_value_type(const value_type& v)
    {
        THOR_DEBUG_ASSERT(&v != 0);
        tree_node* node = (tree_node*)(((thor_byte*)&v) - THOR_OFFSET_OF(tree_node, value));
        THOR_DEBUG_ASSERT(&node->value == &v);
        iterator iter(node, this);
#ifdef _DEBUG
        // Verify that the node is actually in the tree
        const Key& key = KeyFromValue()(v);
        pair<iterator, iterator> range = equal_range(key);
        for (; range.first != range.second && range.first != iter; ++range.first) {}
        THOR_DEBUG_ASSERT(range.first != range.second); // The node was not actually found in the tree.
#endif
        return iter;
    }

    void erase(iterator pos)
    {
        // verify that the iterator belongs to this container
        verify_iterator(pos);
        pos.verify_not_end();
        tree_node* y = rebalance_for_erase(pos.m_node, m_root.parent, m_root.left, m_root.right);
        dealloc_node(y);
        THOR_DEBUG_ASSERT(m_root.m_size != 0);
        --m_root.m_size;
    }
  
    size_type erase(const key_type& x)
    {
        pair<iterator,iterator> p = equal_range(x);
        return erase(p.first, p.second);
    }
  
    size_type erase(iterator first, iterator last)
    {
        // verify that the iterators belong to this container
        verify_iterator(first);
        verify_iterator(last);
        if (first == begin() && last == end())
        {
            size_type n = size();
            clear();
            return n;
        }
        else
        {
            size_type n = 0;
            while (first != last)
            {
                erase(first++); // must postincrement
                ++n;
            }
            return n;
        }
    }

    size_type erase(const key_type* first, const key_type* last)
    {
        size_type n = 0;
        while (first != last)
        {
            n += erase(*first++);
        }
        return n;
    }

    size_type count(const key_type& k) const
    {
        pair<const_iterator,const_iterator> p(equal_range(k));
        return (size_type)distance(p.first, p.second);
    }

    template<class K> iterator find(const K& k) { return iterator(internal_find(k), this); }
    template<class K> const_iterator find(const K& k) const { return const_iterator(internal_find(k), this); }

private:
    struct tree_node;

    struct tree_node_base
    {
        node_color  color;
        tree_node*  parent;
        tree_node*  left;
        tree_node*  right;
        tree_node_base(node_color _color = red, tree_node* _parent = 0, tree_node* _left = 0, tree_node* _right = 0) :
            color(_color), parent(_parent), left(_left), right(_right)
        {}
    };

    struct tree_node : public tree_node_base
    {
        value_type value;
    };

    // Use empty member optimization since key_compare is likely going to be an
    // empty class.
    struct empty_member_opt : public tree_node_base, public key_compare
    {
        size_type       m_size;

        empty_member_opt(tree_node* term) :
            tree_node_base(red, 0, term, term),
            key_compare(),
            m_size(0)
        {}
        
        empty_member_opt(tree_node* term, const key_compare& k) :
            tree_node_base(red, 0, term, term),
            key_compare(k),
            m_size(0)
        {}

        empty_member_opt& operator = (const key_compare& kc)
        {
            key_compare::operator = (kc);
            return *this;
        }
    };

    empty_member_opt m_root;

    tree_node* terminator() const { return (tree_node*)&static_cast<const tree_node_base&>(m_root); }
    void verify_iterator(const iterator_base& i) const { THOR_UNUSED(i); THOR_ASSERT(i.m_owner == this); }

    key_compare& key_comp()
    {
        return static_cast<key_compare&>(m_root);
    }

    // Only allocates memory for the node; does not construct anything
    tree_node* alloc_node()
    {
        return memory::align_alloc<tree_node>::alloc();
    }
    // Only frees memory for the node; does not destroy anything
    void free_node(tree_node* node)
    {
        memory::align_alloc<tree_node>::free(node);
    }

    tree_node* create_node()
    {
        tree_node* tmp = alloc_node();
        new (tmp) tree_node_base;
        return tmp;
    }
    void dealloc_node(tree_node* node)
    {
        typetraits<value_type>::destruct(&node->value);
        ((tree_node_base*)node)->~tree_node_base();
        free_node(node);
    }

    // utility functions
    static tree_node* minimum(tree_node* x)
    {
        while (x->left != 0)
        {
            x = x->left;
        }
        return x;
    }

    static tree_node* maximum(tree_node* x)
    {
        while (x->right != 0)
        {
            x = x->right;
        }
        return x;
    }

    tree_node* clone_node(tree_node* x)
    {
        tree_node* tmp = create_node();
        typetraits<value_type>::construct(&tmp->value, x->value);
        tmp->color = x->color;
        return tmp;
    }

    tree_node* copy(tree_node* x, tree_node* p)
    {
        tree_node* top = clone_node(x);
        top->parent = p;
        if (x->right)
        {
            top->right = copy(x->right, top);
        }
        p = top;
        x = x->left;

        while (x != 0)
        {
            tree_node* y = clone_node(x);
            p->left = y;
            y->parent = p;
            if (x->right)
            {
                y->right = copy(x->right, y);
            }
            p = y;
            x = x->left;
        }

        return top;
    }

    void internal_erase(tree_node* x)
    {
        // erase without re-balancing
        while (x != 0)
        {
            internal_erase(x->right);
            tree_node* y = x->left;
            dealloc_node(x);
            x = y;
        }
    }

    // The node that is returned has not been constructed if bnew is true
    tree_node* internal_insert_unique(const key_type& key, bool& bnew)
    {
        tree_node* y = terminator();
        tree_node* x = m_root.parent;
        bool comp = true;
        while (x != 0)
        {
            y = x;
            comp = key_comp()(key, KeyFromValue()(x->value));
            x = comp ? x->left : x->right;
        }
        iterator j(y, this);
        if (comp)
        {
            if (y == m_root.left)
            {
                bnew = true;
                return internal_insert(y, y, key);
            }
            else
            {
                --j;
            }
        }
        if (key_comp()(KeyFromValue()(j.m_node->value), key))
        {
            bnew = true;
            return internal_insert(x, y, key);
        }
        bnew = false;
        return j.m_node;
    }

    tree_node* internal_insert_equal(const Key& key)
    {
        tree_node* y = terminator();
        tree_node* x = m_root.parent;
        while(x != 0)
        {
            y = x;
            x = key_comp()(key, KeyFromValue()(x->value)) ? x->left : x->right;
        }
        return internal_insert(x, y, key);
    }

    // Does not construct the value_type
    tree_node* internal_insert(tree_node* x, tree_node* y, const key_type& key)
    {
        tree_node* z;

        if (y == terminator() ||
            x != 0 ||     // If x != 0, the remainder succeeds to true
            key_comp()(key, KeyFromValue()(y->value)))
        {
            z = create_node();
            y->left = z;
            if (y == terminator())
            {
                m_root.parent = z;
                m_root.right = z;
            }
            else if (y == m_root.left)
            {
                m_root.left = z;   // maintain leftmost pointing to min node
            }
        }
        else
        {
            z = create_node();
            y->right = z;
            if (y == m_root.right)
            {
                m_root.right = z;  // maintain rightmost pointing to max node
            }
        }
        z->parent = y;
        z->left = z->right = 0;
        rebalance(z, m_root.parent);
        ++m_root.m_size;
        return z;
    }

    void rebalance(tree_node* x, tree_node*& root)
    {
        x->color = red;
        while (x != root && x->parent->color == red)
        {
            if (x->parent == x->parent->parent->left)
            {
                tree_node* y = x->parent->parent->right;
                if (y && y->color == red)
                {
                    x->parent->color = black;
                    y->color = black;
                    x->parent->parent->color = red;
                    x = x->parent->parent;
                }
                else
                {
                    if (x == x->parent->right)
                    {
                        x = x->parent;
                        rotate_left(x, root);
                    }
                    x->parent->color = black;
                    x->parent->parent->color = red;
                    rotate_right(x->parent->parent, root);
                }
            }
            else
            {
                tree_node* y = x->parent->parent->left;
                if (y && y->color == red)
                {
                    x->parent->color = black;
                    y->color = black;
                    x->parent->parent->color = red;
                    x = x->parent->parent;
                }
                else
                {
                    if (x == x->parent->left)
                    {
                        x = x->parent;
                        rotate_right(x, root);
                    }
                    x->parent->color = black;
                    x->parent->parent->color = red;
                    rotate_left(x->parent->parent, root);
                }
            }
        }
        root->color = black;
    }
    
    tree_node* rebalance_for_erase(tree_node* z,
					               tree_node*& root,
					               tree_node*& leftmost,
					               tree_node*& rightmost)
    {
        tree_node* y = z;
        tree_node* x = 0;
        tree_node* x_parent = 0;
        if (y->left == 0)     // z has at most one non-null child. y == z.
        {
            x = y->right;     // x might be null.
        }
        else if (y->right == 0)  // z has exactly one non-null child. y == z.
        {
            x = y->left;    // x is not null.
        }
        else                     // z has two non-null children.  Set y to
        {
            y = y->right;   //   z's successor.  x might be null.
            while (y->left != 0)
            {
                y = y->left;
            }
            x = y->right;
        }
        if (y != z)          // relink y in place of z.  y is z's successor
        {
            z->left->parent = y; 
            y->left = z->left;
            if (y != z->right)
            {
                x_parent = y->parent;
                if (x != 0)
                {
                    x->parent = y->parent;
                }
                y->parent->left = x;      // y must be a child of left
                y->right = z->right;
                z->right->parent = y;
            }
            else
            {
                x_parent = y;
            }
            if (root == z)
            {
                root = y;
            }
            else if (z->parent->left == z)
            {
                z->parent->left = y;
            }
            else 
            {
                z->parent->right = y;
            }
            y->parent = z->parent;
            thor::swap(y->color, z->color);
            y = z;
            // y now points to node to be actually deleted
        }
        else                        // y == z
        {
            x_parent = y->parent;
            if (x)
            {
                x->parent = y->parent;
            }
            if (root == z)
            {
                root = x;
            }
            else if (z->parent->left == z)
            {
                z->parent->left = x;
            }
            else
            {
                z->parent->right = x;
            }
            if (leftmost == z) 
            {
                if (z->right == 0)        // z->left must be null also
                {
                    leftmost = z->parent;
                }
                // makes leftmost == header if z == root
                else
                {
                    leftmost = minimum(x);
                }
            }
            if (rightmost == z)  
            {
                if (z->left == 0)         // z->right must be null also
                {
                    rightmost = z->parent;
                }
                // makes rightmost == header if z == root
                else                      // x == z->left
                {
                    rightmost = maximum(x);
                }
            }
        }
        if (y->color != red)
        { 
            while (x != root && (x == 0 || x->color == black))
            {
                if (x == x_parent->left)
                {
                    tree_node* w = x_parent->right;
                    if (w->color == red)
                    {
                        w->color = black;
                        x_parent->color = red;
                        rotate_left(x_parent, root);
                        w = x_parent->right;
                    }
                    if ((w->left == 0 || 
                         w->left->color == black) && (w->right == 0 || 
                         w->right->color == black))
                    {
                        w->color = red;
                        x = x_parent;
                        x_parent = x_parent->parent;
                    }
                    else
                    {
                        if (w->right == 0 || 
                            w->right->color == black)
                        {
                            if (w->left)
                            {
                                w->left->color = black;
                            }
                            w->color = red;
                            rotate_right(w, root);
                            w = x_parent->right;
                        }
                        w->color = x_parent->color;
                        x_parent->color = black;
                        if (w->right)
                        {
                            w->right->color = black;
                        }
                        rotate_left(x_parent, root);
                        break;
                    }
                }
                else                  // same as above, with right <-> left.
                {
                    tree_node* w = x_parent->left;
                    if (w->color == red)
                    {
                        w->color = black;
                        x_parent->color = red;
                        rotate_right(x_parent, root);
                        w = x_parent->left;
                    }
                    if ((w->right == 0 || 
                         w->right->color == black) && (w->left == 0 || 
                         w->left->color == black))
                    {
                        w->color = red;
                        x = x_parent;
                        x_parent = x_parent->parent;
                    }
                    else
                    {
                        if (w->left == 0 || 
                            w->left->color == black)
                        {
                            if (w->right)
                            {
                                w->right->color = black;
                            }
                            w->color = red;
                            rotate_left(w, root);
                            w = x_parent->left;
                        }
                        w->color = x_parent->color;
                        x_parent->color = black;
                        if (w->left)
                        {
                            w->left->color = black;
                        }
                        rotate_right(x_parent, root);
                        break;
                    }
                }
            }
            if (x)
            {
                x->color = black;
            }
        }
        return y;
    }

    void rotate_left(tree_node* x, tree_node*& root)
    {
        tree_node* y = x->right;
        x->right = y->left;
        if (y->left !=0)
        {
            y->left->parent = x;
        }
        y->parent = x->parent;

        if (x == root)
        {
            root = y;
        }
        else if (x == x->parent->left)
        {
            x->parent->left = y;
        }
        else
        {
            x->parent->right = y;
        }
        y->left = x;
        x->parent = y;
    }

    void rotate_right(tree_node* x, tree_node*& root)
    {
        tree_node* y = x->left;
        x->left = y->right;
        if (y->right != 0)
        {
            y->right->parent = x;
        }
        y->parent = x->parent;

        if (x == root)
        {
            root = y;
        }
        else if (x == x->parent->right)
        {
            x->parent->right = y;
        }
        else
        {
            x->parent->left = y;
        }
        y->right = x;
        x->parent = y;
    }

    template <class K> tree_node* internal_find(const K& k) const
    {
        tree_node* y = terminator();            // Last node which is not less than k. 
        tree_node* x = m_root.parent;    // Current node. 

        while (x != 0)
        {
            if (!key_comp()(KeyFromValue()(x->value), k))
            {
                y = x, x = x->left;
            }
            else
            {
                x = x->right;
            }
        }
        if (y == terminator() || key_comp()(k, KeyFromValue()(y->value)))
        {
            y = terminator();
        }
        return y;
    }

    tree_node* lower_bound_internal(const key_type& k) const
    {
        tree_node* y = terminator();            // Last node which is not less than k.
        tree_node* x = m_root.parent;    // Current node.
    
        while (x != 0)
        {
            if (!key_comp()(KeyFromValue()(x->value), k))
            {
                y = x, x = x->left;
            }
            else
            {
                x = x->right;
            }
        }
    
        return y;
    }

    tree_node* upper_bound_internal(const key_type& k) const
    {
        tree_node* y = terminator(); // Last node which is greater than k.
        tree_node* x = m_root.parent; // Current node.
    
        while (x != 0) 
        {
            if (key_comp()(k, KeyFromValue()(x->value)))
            {
                y = x, x = x->left;
            }
            else
            {
                x = x->right;
            }
        }
        return y;
    }
};

}; // namespace thor

// Global operators
template <class Key, class Value, class KeyFromValue, class Compare>
bool operator == (const thor::red_black_tree<Key,Value,KeyFromValue,Compare>& lhs, const thor::red_black_tree<Key,Value,KeyFromValue,Compare>& rhs)
{
    return lhs.size() == rhs.size() && thor::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <class Key, class Value, class KeyFromValue, class Compare>
bool operator < (const thor::red_black_tree<Key,Value,KeyFromValue,Compare>& lhs, const thor::red_black_tree<Key,Value,KeyFromValue,Compare>& rhs)
{
    return thor::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <class Key, class Value, class KeyFromValue, class Compare>
bool operator != (const thor::red_black_tree<Key,Value,KeyFromValue,Compare>& lhs, const thor::red_black_tree<Key,Value,KeyFromValue,Compare>& rhs)
{
    return !(lhs == rhs);
}

template <class Key, class Value, class KeyFromValue, class Compare>
bool operator > (const thor::red_black_tree<Key,Value,KeyFromValue,Compare>& lhs, const thor::red_black_tree<Key,Value,KeyFromValue,Compare>& rhs)
{
    return thor::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), thor::greater<Value>());
}

template <class Key, class Value, class KeyFromValue, class Compare>
bool operator <= (const thor::red_black_tree<Key,Value,KeyFromValue,Compare>& lhs, const thor::red_black_tree<Key,Value,KeyFromValue,Compare>& rhs)
{
    return !(lhs > rhs);
}

template <class Key, class Value, class KeyFromValue, class Compare>
bool operator >= (const thor::red_black_tree<Key,Value,KeyFromValue,Compare>& lhs, const thor::red_black_tree<Key,Value,KeyFromValue,Compare>& rhs)
{
    return !(lhs < rhs);
}

#endif

