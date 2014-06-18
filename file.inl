/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * file.inl
 *
 * Utility for working with directories -- inline implementation
 */

#include "file.h"

namespace thor
{
namespace file
{

inline void normalize_path(const char* path, string& out)
{
    basic_string<wchar_t, 256> wpath, wout;
    bool b = utf8_to_wide(path, wpath);
    THOR_UNUSED(b); THOR_ASSERT(b);
    normalize_path(wpath.c_str(), wout);
    b = wide_to_utf8(wout, out);
    THOR_ASSERT(b);
}

inline bool move(const char* pathFrom, const char* pathTo)
{
    basic_string<wchar_t, 256> wpathFrom, wpathTo;
    bool b = utf8_to_wide(pathFrom, wpathFrom);
    b &= utf8_to_wide(pathTo, wpathTo);
    THOR_UNUSED(b); THOR_ASSERT(b);
    return move(wpathFrom.c_str(), wpathTo.c_str());
}

inline bool remove(const char* path)
{
    basic_string<wchar_t, 256> wpath;
    bool b = utf8_to_wide(path, wpath);
    THOR_UNUSED(b); THOR_ASSERT(b);
    return remove(wpath.c_str());
}

inline bool set_properties(const char* path, const properties& props)
{
    basic_string<wchar_t, 256> wpath;
    bool b = utf8_to_wide(path, wpath);
    THOR_UNUSED(b); THOR_ASSERT(b);
    return set_properties(wpath.c_str(), props);
}

inline bool exists(const char* path, properties* props)
{
    basic_string<wchar_t, 256> wpath;
    bool b = utf8_to_wide(path, wpath);
    THOR_UNUSED(b); THOR_ASSERT(b);
    return exists(wpath.c_str(), props);
}

inline bool relative_to_full_path(const char* path, string& out)
{
    basic_string<wchar_t, 256> wpath, wout;
    bool b = utf8_to_wide(path, wpath);
    THOR_UNUSED(b); THOR_ASSERT(b);
    bool ret = relative_to_full_path(wpath.c_str(), wout);
    if (ret)
    {
        b = wide_to_utf8(wout.c_str(), out);
        THOR_ASSERT(b);
        return true;
    }
    return false;
}

} // namespace file
} // namespace thor