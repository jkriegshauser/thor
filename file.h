/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * file.h
 *
 * Utility for working with files
 */

#ifndef THOR_FILE_H
#define THOR_FILE_H
#pragma once

#ifndef THOR_BASETYPES_H
#include "basetypes.h"
#endif

#ifndef THOR_TIME_UTIL_H
#include "time_util.h"
#endif

#ifndef THOR_BASIC_STRING_H
#include "basic_string.h"
#endif

namespace thor
{
namespace file
{

///////////////////////////////////////////////////////////////////////////////
// File structures
///////////////////////////////////////////////////////////////////////////////
struct properties
{
    uint64 size;
    time::seconds create_time;
    time::seconds mod_time;
    bool read_only;
    bool directory;
    bool hidden;

    properties()
        : size(0)
        , read_only(false)
        , directory(false)
        , hidden(false)
    {}
};

///////////////////////////////////////////////////////////////////////////////
// File functions
///////////////////////////////////////////////////////////////////////////////
void normalize_path(const char* path, string& out);
void normalize_path(const wchar_t* path, wstring& out);

bool move(const char* pathFrom, const char* pathTo);
bool move(const wchar_t* pathFrom, const wchar_t* pathTo);

bool remove(const char* path);
bool remove(const wchar_t* path);

bool set_properties(const char* path, const properties& props);
bool set_properties(const wchar_t* path, const properties& props);

bool exists(const char* path, properties* props = 0);
bool exists(const wchar_t* path, properties* props = 0);

bool relative_to_full_path(const char* path, string& out);
bool relative_to_full_path(const wchar_t* path, wstring& out);

} // namespace file

} // namespace thor

#include "file.inl"

#endif