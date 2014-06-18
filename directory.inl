/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * directory.inl
 *
 * Utility for working with directories -- inline implementations
 */

#ifndef THOR_BASIC_STRING_H
#include "basic_string.h"
#endif

#ifndef THOR_DIRECTORY_H
#include "directory.h"
#endif

namespace thor
{
namespace dir
{

inline bool create(const char* path, bool recursive)
{
    basic_string<wchar_t, 256> wpath;
    bool b = utf8_to_wide(path, wpath);
    THOR_UNUSED(b); THOR_ASSERT(b);
    return create(wpath.c_str(), recursive);
}

inline bool remove(const char* path, bool empty_only)
{
    basic_string<wchar_t, 256> wpath;
    bool b = utf8_to_wide(path, wpath);
    THOR_UNUSED(b); THOR_ASSERT(b);
    return remove(wpath.c_str(), empty_only);
}

inline bool exists(const char* path)
{
    basic_string<wchar_t, 256> wpath;
    bool b = utf8_to_wide(path, wpath);
    THOR_UNUSED(b); THOR_ASSERT(b);
    return exists(wpath.c_str());
}

inline bool rename(const char* path, const char* newname)
{
    basic_string<wchar_t, 256> wpath, wnewname;
    bool b = utf8_to_wide(path, wpath) & utf8_to_wide(newname, wnewname);
    THOR_UNUSED(b); THOR_ASSERT(b);
    return rename(wpath.c_str(), wnewname.c_str());
}

inline uint64 freespace(const char* path)
{
    basic_string<wchar_t, 256> wpath;
    bool b = utf8_to_wide(path, wpath);
    THOR_UNUSED(b); THOR_ASSERT(b);
    return freespace(wpath.c_str());
}

} // namespace dir
} // namespace thor