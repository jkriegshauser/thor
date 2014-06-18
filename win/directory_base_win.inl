/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * win/directory_base_win.inl
 *
 * Utility for working with directories -- Windows base implementation
 */

#include "../directory.h"

namespace thor
{
namespace dir
{
namespace internal
{

void* listing_create(const wchar_t* path, entry_t<wchar_t>& start);
void  listing_destroy(void* listing);
bool  listing_next(void* listing, entry_t<wchar_t>& out);

template<typename T_CHAR> class listing_base
{
    enum { num_counters = 2 };
public:
    listing_base(const T_CHAR* path) THOR_NOTHROW
        : counter_(0)
    {
        handle_ = listing_create(path, start_);
        // THOR_ASSERT(handle_);
    }
    ~listing_base()
    {
        listing_destroy(handle_);
    }
    entry_t<T_CHAR>* first()
    {
        return handle_ ? &start_ : NULL;
    }
    entry_t<T_CHAR>* next(entry_t<T_CHAR>*)
    {
        entry_t<T_CHAR>* e = &entry_[counter_++ % num_counters];
        bool b = listing_next(handle_, *e);
        if (b)
        {
            return e;
        }
        return 0;
    }
    bool valid() const { return handle_ != 0; }
    void set_basepath(const basic_string<T_CHAR>& str)
    {
        start_.base_path = str;
        for (int i = 0; i != num_counters; ++i)
        {
            entry_[i].base_path = str;
        }
    }

    void* handle_;
    size_type counter_;
    entry_t<T_CHAR> start_;
    entry_t<T_CHAR> entry_[num_counters];
};

// Specialization for char
template<> class listing_base<char>
{
    enum { num_counters = 2 };
public:
    listing_base(const char* path) THOR_NOTHROW
        : counter_(0)
    {
        basic_string<wchar_t, 256> wpath;
        bool b = utf8_to_wide(path, wpath);
        THOR_ASSERT(b); THOR_UNUSED(b);
        entry_t<wchar_t> start;
        handle_ = listing_create(wpath.c_str(), start);
        wide_to_utf8(start, start_);
        // THOR_ASSERT(handle_);
    }
    ~listing_base()
    {
        listing_destroy(handle_);
    }
    entry_t<char>* first()
    {
        return handle_ ? &start_ : 0;
    }
    entry_t<char>* next(entry_t<char>*)
    {
        entry_t<wchar_t> temp;
        bool b = listing_next(handle_, temp);
        if (b)
        {
            entry_t<char>* e = &entry_[counter_++ % num_counters];
            // save off the basepath since it's already set
            string basepath = e->base_path;
            wide_to_utf8(temp, *e);
            e->base_path = basepath;
            return e;
        }
        return 0;
    }
    bool valid() const { return handle_ != 0; }
    void set_basepath(const string& str)
    {
        start_.base_path = str;
        for (int i = 0; i != num_counters; ++i)
        {
            entry_[i].base_path = str;
        }
    }

    void* handle_;
    size_type counter_;
    entry_t<char> start_;
    entry_t<char> entry_[num_counters];
};

} // namespace internal
} // namespace dir
} // namespace thor