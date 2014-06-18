/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * win/file_win.cpp
 *
 * Utility for working with files -- Windows implementation
 */

#include "../file.h"

#include "../auto_closer.h"

#include "../function.h"

#define WIN32_EXTRA_LEAN 1
#include <Windows.h>

namespace thor
{
namespace internal
{

// Yes, this could use ptr_fun(), but it's pretty verbose and ugly, plus it increases sizeof(file_handle) by the size of a function pointer:
#if 0
typedef pointer_to_unary_function<HANDLE, BOOL, BOOL(__stdcall*)(HANDLE)> close_handle_func;
typedef auto_closer<HANDLE, INVALID_HANDLE_VALUE, close_handle_func> file_handle_base;
class file_handle : public file_handle_base
{
public:
    file_handle(HANDLE h) : file_handle_base(h, close_handle_func(::CloseHandle)) {}
};
#else
struct close_handle_wrapper
{
    BOOL operator () (HANDLE h) const
    {
        BOOL b = ::CloseHandle(h);
        THOR_UNUSED(b); THOR_ASSERT(b);
        return b;
    }
};
typedef auto_closer<HANDLE, INVALID_HANDLE_VALUE, close_handle_wrapper> file_handle;
#endif

bool read_properties(HANDLE h, file::properties& props)
{
    BY_HANDLE_FILE_INFORMATION info;
    if (::GetFileInformationByHandle(h, &info) == TRUE)
    {
        props.directory = (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        props.hidden = (info.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0;
        props.read_only = (info.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0;
        props.size = ((uint64)info.nFileSizeHigh << 32) | (uint64)info.nFileSizeLow;
        
        const uint64 epoch = 116444736000000000ULL; // 1/1/1970 00:00:00 UTC as a FILETIME (100ns units)
        
        uint64 val = ((uint64)info.ftCreationTime.dwHighDateTime) << 32 | (uint64)info.ftCreationTime.dwLowDateTime;
        val -= epoch;
        props.create_time = time::seconds(val / 10000000ULL); // 100ns -> seconds
        
        val = ((uint64)info.ftLastWriteTime.dwHighDateTime) << 32 | (uint64)info.ftLastWriteTime.dwLowDateTime;
        val -= epoch;
        props.mod_time = time::seconds(val / 10000000ULL); // 100ns -> seconds

        return true;
    }
    return false;
}

} // namespace internal

namespace file
{

void normalize_path(const wchar_t* path, wstring& out)
{
    out = path;
    size_type i;
    while ((i = out.rfind(L'/')) != wstring::npos)
    {
        out.at(i) = '\\';
    }

    if (out.length() > 0U)
    {
        // Start at index 1 because two backslashes are allowed for UNC paths
        while ((i = out.find(L"\\\\", 1U)) != wstring::npos)
        {
            out.replace(i, 2, L"\\", 1);
        }
    }
}

bool move(const wchar_t* pathFrom, const wchar_t* pathTo)
{
    basic_string<wchar_t, 256> wpathFrom, wpathTo;
    normalize_path(pathFrom, wpathFrom);
    normalize_path(pathTo, wpathTo);
    return ::MoveFileW(wpathFrom.c_str(), wpathTo.c_str()) == TRUE;
}

bool remove(const wchar_t* path)
{
    basic_string<wchar_t, 256> wpath;
    normalize_path(path, wpath);
    return ::DeleteFileW(wpath.c_str()) == TRUE;
}

bool set_properties(const wchar_t* path, const properties& props)
{
    basic_string<wchar_t, 256> wpath(path);
    normalize_path(path, wpath);

    DWORD attrib = ::GetFileAttributesW(wpath.c_str());
    if (attrib == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }

    bool retval = true;

    if (props.hidden) attrib |= FILE_ATTRIBUTE_HIDDEN; else attrib &= ~FILE_ATTRIBUTE_HIDDEN;
    if (props.read_only) attrib |= FILE_ATTRIBUTE_READONLY; else attrib &= ~FILE_ATTRIBUTE_READONLY;
    
    retval &= (::SetFileAttributesW(wpath.c_str(), attrib ? attrib : FILE_ATTRIBUTE_NORMAL) == TRUE);

    if (props.create_time != time::seconds(0) || props.mod_time != time::seconds(0))
    {
        internal::file_handle h(::CreateFileW(wpath.c_str(), FILE_WRITE_ATTRIBUTES, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_BACKUP_SEMANTICS, 0));
        if (h.valid())
        {
            FILE_BASIC_INFO info;
            memset(&info, 0, sizeof(info));
            if (::GetFileInformationByHandleEx(h, FileBasicInfo, (LPVOID)&info, sizeof(info)) == TRUE)
            {
                const uint64 epoch = 116444736000000000ULL; // 1/1/1970 00:00:00 UTC as a FILETIME (100ns units)
                if (props.create_time != time::seconds(0))
                {
                    info.CreationTime.QuadPart = (int64)props.create_time.cvalue();
                    info.CreationTime.QuadPart *= 10000000LL; // seconds -> 100ns
                    info.CreationTime.QuadPart += epoch;                    
                }
                if (props.mod_time != time::seconds(0))
                {
                    info.LastWriteTime.QuadPart = (int64)props.mod_time.cvalue();
                    info.LastWriteTime.QuadPart *= 10000000LL; // seconds -> 100ns
                    info.LastWriteTime.QuadPart += epoch;
                }
                retval &= (::SetFileInformationByHandle(h, FileBasicInfo, (LPVOID)&info, sizeof(info)) == TRUE);
            }
            else
            {
                retval = false;
            }
        }
        else
        {
            retval = false;
        }
    }

    return retval;
}

bool exists(const wchar_t* path, properties* props)
{
    basic_string<wchar_t, 256> wpath;
    normalize_path(path, wpath);

    // Try to open a file
    internal::file_handle h(::CreateFileW(wpath.c_str(), 0, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_BACKUP_SEMANTICS, 0));
    if (h.valid())
    {
        if (props)
        {
            bool b = internal::read_properties(h, *props);
            THOR_UNUSED(b); THOR_ASSERT(b);
        }
        return true;
    }
    return false;
}

bool relative_to_full_path(const wchar_t* path, wstring& out)
{
    DWORD len = ::GetFullPathNameW(path, 0, 0, 0);
    if (len == 0) return false;

    out.resize(len - 1);

    len = ::GetFullPathNameW(path, (DWORD)out.size() + 1, &out[0], 0);
    THOR_ASSERT(len == out.size());
    return len > 0;
}

} // namespace file
} // namespace thor