/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * math_util.h
 *
 * Math functions
 */

#ifndef THOR_MATH_UTIL_H
#define THOR_MATH_UTIL_H
#pragma once

#ifndef THOR_BASETYPES_H
#include "basetypes.h"
#endif

namespace thor
{

namespace math
{

template<typename T> T clamp(const T& initial, const T& min, const T& max)
{
    if (initial < min) return min;
    if (max < initial) return max;
    return initial;
}

}

}

#endif