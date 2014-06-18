/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * directory.h
 *
 * Utility for working with directories
 */

#ifndef THOR_DIRECTORY_H
#define THOR_DIRECTORY_H
#pragma once

#ifndef THOR_BASIC_STRING_H
#include "basic_string.h"
#endif

#ifndef THOR_FILE_H
#include "file.h"
#endif

namespace thor
{
namespace dir
{

///////////////////////////////////////////////////////////////////////////////
// Directory functions
///////////////////////////////////////////////////////////////////////////////

// Create a directory if it doesn't exist.
bool create(const char* path, bool recursive = true);
bool create(const wchar_t* path, bool recursive = true);

// Remove (delete) a directory. If the directory has file contents, then remove
// will fail unless empty_only is false.
bool remove(const char* path, bool empty_only = true);
bool remove(const wchar_t* path, bool empty_only = true);

// Returns true if a path exists. False otherwise.
bool exists(const char* path);
bool exists(const wchar_t* path);

// Returns true if the rename succeeded. False otherwise.
bool rename(const char* path, const char* newname);
bool rename(const wchar_t* path, const wchar_t* newname);

// Returns the number of bytes available in a directory. Returns uint64(-1) in case of failure.
uint64 freespace(const char* path);
uint64 freespace(const wchar_t* path);

///////////////////////////////////////////////////////////////////////////////
// Directory listing container
//   Simulates a conatiner for the purposes of listing a directory
///////////////////////////////////////////////////////////////////////////////
// Properties of a directory entry gathered by the listing function. The base_path
// is the full path of the directory containing the entry.
template <typename T_CHAR> struct entry_t : public file::properties
{
    basic_string<T_CHAR>     base_path;
    basic_string<T_CHAR, 32> name;
};

typedef entry_t<char>       entry;      // C-string version (UTF-8 encoding)
typedef entry_t<wchar_t>    wentry;     // Wide-char version

} // namespace dir

inline bool utf8_to_wide(const dir::entry& e, dir::wentry& w)
{
    static_cast<file::properties&>(w) = static_cast<const file::properties&>(e);
    bool b = utf8_to_wide(e.base_path, w.base_path);
    b     &= utf8_to_wide(e.name, w.name);
    THOR_ASSERT(b);
    return b;
}

inline bool wide_to_utf8(const dir::wentry& w, dir::entry& e)
{
    static_cast<file::properties&>(e) = static_cast<const file::properties&>(w);
    bool b = wide_to_utf8(w.base_path, e.base_path);
    b     &= wide_to_utf8(w.name, e.name);
    THOR_ASSERT(b);
    return b;
}

} // namespace thor

// Platform-specific base
#if defined(WIN32)
#include "win/directory_base_win.inl"
#else
#error Unsupported platform!
#endif

namespace thor
{
namespace dir
{

template<typename T_CHAR> class listing_t : protected internal::listing_base<T_CHAR>
{
    THOR_DECLARE_NOCOPY(listing_t);
public:
    typedef entry_t<T_CHAR> value_type;
    typedef value_type*     pointer;
    typedef value_type&     reference;
    typedef const pointer   const_pointer;
    typedef const reference const_reference;
    typedef thor_size_type  size_type;
    typedef thor_diff_type  difference_type;

    class const_iterator : public iterator_type<forward_iterator_tag, value_type>
    {
        typename entry_t<T_CHAR>* element_;
        listing_t* owner_;
        void incr() { THOR_ASSERT(owner_); THOR_ASSERT(element_); element_ = owner_->next(element_); }
        void verify_not_end() const { THOR_ASSERT(owner_); THOR_ASSERT(element_); }
    public:
        typedef typename entry_t<T_CHAR> value_type;
        typedef typename const_traits<value_type> Traits;
        typedef typename Traits::pointer pointer;
        typedef typename Traits::reference reference;
        typedef const_iterator selftype;

        const_iterator(value_type* v = 0, listing_t* o = 0) : element_(v), owner_(o) {}
        reference operator * ()  const                  { verify_not_end(); return *element_; }
        pointer   operator -> () const                  { verify_not_end(); return element_; }        
        selftype& operator ++ ()    /* ++iterator */    {                    incr(); return *this; }
        selftype  operator ++ (int) /* iterator++ */    { selftype n(*this); incr(); return n; }
        bool operator == (const const_iterator& rhs) const { return rhs.owner_ == owner_ && rhs.element_ == element_; }
        bool operator != (const const_iterator& rhs) const { return ! operator == (rhs); }
    };

    listing_t(const T_CHAR* path) : internal::listing_base<T_CHAR>(path)
    {
        THOR_ASSERT(path && *path);
        bool b = file::relative_to_full_path(path, basepath_);
        THOR_ASSERT(b); THOR_UNUSED(b);

        set_basepath(basepath_);
    }
    ~listing_t() {}

    const_iterator begin() { return const_iterator(first(), this); }
    const_iterator end()   { return const_iterator(NULL,    this); }

    bool valid() const { return internal::listing_base<T_CHAR>::valid(); }

private:
    basic_string<T_CHAR> basepath_;
};

typedef listing_t<char> listing;
typedef listing_t<wchar_t> wlisting;

} // namespace dir
} // namespace thor

#include "directory.inl"

#endif