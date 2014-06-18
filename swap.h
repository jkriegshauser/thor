/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * sort.h
 *
 * ** THOR INTERNAL FILE - NOT FOR APPLICATION USE **
 *
 * This file defines the STL-compatible swap function. It is a separate file to allow for
 * easy specialization.
 */

#ifndef THOR_SWAP_H
#define THOR_SWAP_H
#pragma once

namespace thor
{

template <class T> void swap(T& lhs, T& rhs)
{
    T temp(lhs);
    lhs = rhs;
    rhs = temp;
}

} // namespace thor

#endif

