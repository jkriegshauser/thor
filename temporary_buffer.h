/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * temporary_buffer.h
 *
 * ** THOR INTERNAL FILE - NOT FOR APPLICATION USE **
 *
 * This file defines an internally-used temporary buffer (a sort of simplified vector).
 */

#ifndef THOR_TEMPORARY_BUFFER_H
#define THOR_TEMPORARY_BUFFER_H
#pragma once

#ifndef THOR_BASETYPES_H
#include "basetypes.h"
#endif

#ifndef THOR_ITERATOR_H
#include "iterator.h"
#endif

#ifndef THOR_MEMORY_H
#include "memory.h"
#endif

#ifndef THOR_TYPETRAITS_H
#include "typetraits.h"
#endif

namespace thor
{

template <class ForwardIterator, class T> class __TemporaryBuffer
{
public:
    __TemporaryBuffer(ForwardIterator first, ForwardIterator last)
    {
        m_size = thor::distance(first, last);
        m_requested_size = m_size;
        m_elements = memory::align_alloc<T>::alloc(m_size);
        typetraits<T>::range_construct(m_elements, m_elements + m_size);
    }
    ~__TemporaryBuffer()
    {
        typetraits<T>::range_destruct(m_elements, m_elements + m_size);
        memory::align_alloc<T>::free(m_elements);
        m_elements = 0;
    }

    T* begin()
    {
        return m_elements;
    }

    T* end()
    {
        return m_elements + m_size;
    }

    difference_type size() const
    {
        return m_size;
    }

    difference_type requested_size() const
    {
        return m_requested_size;
    }

private:
    T* m_elements;
    difference_type m_size;
    difference_type m_requested_size;

    // Prevent copy/assign
    __TemporaryBuffer(const __TemporaryBuffer&);
    __TemporaryBuffer& operator = (const __TemporaryBuffer&);
};

}

#endif
