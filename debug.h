/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * debug.h
 *
 * Provides platform-abstracted debug facilities
 */

#ifndef THOR_DEBUG_H
#define THOR_DEBUG_H
#pragma once

#ifndef THOR_BASETYPES_H
#include "basetypes.h"
#endif

#ifndef THOR_THREAD_H
#include "thread.h"
#endif

namespace thor
{

namespace debug
{

void crash();
void debugbreak();

void set_thread_name(const char* name, thread_id tid = thread_id::get_current_thread());

void debug_output(const char* fmt, ...);
void debug_output_v(const char* fmt, va_list va);
void debug_output(const wchar_t* fmt, ...);
void debug_output_v(const wchar_t* fmt, va_list va);

}

}

#endif