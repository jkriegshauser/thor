/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * freelist.h
 *
 * ** THOR INTERNAL FILE - NOT FOR APPLICATION USE **
 *
 * This file defines a preallocated free list of nodes for preallocated
 * node-based containers. It is not intended for use outside of THOR.
 */

#ifndef THOR_FREELIST_H
#define THOR_FREELIST_H
#pragma once

#ifndef THOR_BASETYPES_H
#include "basetypes.h"
#endif

#ifndef THOR_MEMORY_H
#include "memory.h"
#endif

namespace thor
{

template <class T, size_type T_COUNT> class freelist
{
    const static size_type alignment = memory::align_selector<T>::alignment;
public:
    freelist() : m_head(0)
    {
        // Build the free list
        THOR_COMPILETIME_ASSERT(sizeof(freenode) <= sizeof(T), SizeTooSmall);
        size_type count = T_COUNT;
        thor_byte* space = memory::align_forward<alignment>(m_reserved);
        while (count-- != 0)
        {
            freenode* node = new (space) freenode(m_head);

            // Ensure that is_owned_node is working correctly and alignment is correct.
            THOR_DEBUG_ASSERT(is_owned_node((T*)node));
            THOR_DEBUG_ASSERT(memory::align_alloc<T>::is_aligned((T*)node));
            
            m_head = node;
            space += sizeof(T);
        }
    }
    
    ~freelist()
    {}

    T* alloc_node()
    {
        if (m_head != 0)
        {
            freenode* node = m_head;
            m_head = node->m_next;
            return (T*)node;
        }
        return 0;
    }

    bool free_node(T* node)
    {
        if (!is_owned_node(node))
        {
            return false;
        }

#ifdef THOR_DEBUG
        // Verify that it's not already in the list
        freenode* fn = m_head;
        while (fn)
        {
            THOR_DEBUG_ASSERT(fn != (freenode*)node);
            fn = fn->m_next;
        }
#endif

        freenode* head = new (node) freenode(m_head);
        m_head = head;
        return true;
    }

    bool is_owned_node(T* node) const
    {
        return (thor_byte*)node >= m_reserved && (thor_byte*)node <= (m_reserved + alignment + (T_COUNT - 1) * sizeof(T));
    }

private:
    struct freenode
    {
        freenode* m_next;

        freenode(freenode* next) : m_next(next) {}
    };
    
    thor_byte m_reserved[sizeof(T) * T_COUNT + alignment];
    freenode* m_head;
};

} // namespace thor

#endif

