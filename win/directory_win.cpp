/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * win/directory_win.cpp
 *
 * Utility for working with directories -- Windows implementation
 */

#include "../directory.h"

#define WIN32_EXTRA_LEAN 1
#include <Windows.h>

namespace thor
{

namespace dir
{

bool create(const wchar_t* path, bool recursive)
{
    basic_string<wchar_t, 256> wpath, temp;
    file::normalize_path(path, wpath);

    if (recursive)
    {
        size_type i = wstring::npos;
        while ((i = wpath.find(L'\\', i + 1)) != wstring::npos)
        {
            temp.assign(wpath.c_str(), i);
            ::CreateDirectoryW(temp.c_str(), NULL);
        }
    }

    ::CreateDirectoryW(wpath.c_str(), NULL);
    return exists(path);
}

#pragma comment(lib, "shell32.lib")
bool remove(const wchar_t* path, bool empty_only)
{
    basic_string<wchar_t, 256> wpath;
    file::normalize_path(path, wpath);

    if (empty_only)
    {
        return ::RemoveDirectoryW(wpath.c_str()) == TRUE;
    }
    else
    {
        // Get the full path
        DWORD len = ::GetFullPathNameW(wpath.c_str(), 0, NULL, NULL);
        THOR_ASSERT(len != 0);
        wchar_t* buffer = (wchar_t*)::alloca((len + 2) * sizeof(wchar_t));
        len = ::GetFullPathNameW(wpath.c_str(), len + 2, buffer, NULL);
        THOR_ASSERT(len != 0);
        buffer[len] = buffer[len + 1] = L'\0';

        // Do a shell operation since it can delete a non-empty directory
        SHFILEOPSTRUCTW str;
        str.fAnyOperationsAborted = FALSE;
        str.fFlags = FOF_SILENT | FOF_NOERRORUI | FOF_NOCONFIRMATION;
        str.hNameMappings = NULL;
        str.hwnd = NULL;
        str.lpszProgressTitle = L"";
        str.pFrom = buffer;
        str.pTo = NULL;
        str.wFunc = FO_DELETE;
        if (FAILED(::SHFileOperationW(&str)) || str.fAnyOperationsAborted != FALSE)
        {
            return false;
        }
        return true;
    }
}

bool exists(const wchar_t* path)
{
    return file::exists(path);
}

bool rename(const wchar_t* path, const wchar_t* newpath)
{
    basic_string<wchar_t, 256> wpath, wnewpath;
    file::normalize_path(path, wpath);
    file::normalize_path(newpath, wnewpath);
    return ::MoveFileW(wpath.c_str(), wnewpath.c_str()) == TRUE;
}

uint64 freespace(const wchar_t* path)
{
    basic_string<wchar_t, 256> wpath;
    file::normalize_path(path, wpath);

    ULARGE_INTEGER freespace, totalspace;
    return ::GetDiskFreeSpaceExW(wpath.c_str(), &freespace, &totalspace, 0) == TRUE ? freespace.QuadPart : uint64(-1);
}

namespace internal
{

void w32fd_to_entry(const WIN32_FIND_DATAW& fd, entry_t<wchar_t>& out)
{
    out.directory = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    out.hidden = (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0;
    out.read_only = (fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0;
    out.size = ((uint64)fd.nFileSizeHigh << 32) | (uint64)fd.nFileSizeLow;

    const uint64 epoch = 116444736000000000ULL; // 1/1/1970 00:00:00 UTC as a FILETIME (100ns units)
    uint64 val = ((uint64)fd.ftCreationTime.dwHighDateTime) << 32 | (uint64)fd.ftCreationTime.dwLowDateTime;
    val -= epoch;
    out.create_time = time::seconds(val / 10000000ULL); // 100ns -> seconds

    val = ((uint64)fd.ftLastWriteTime.dwHighDateTime) << 32 | (uint64)fd.ftLastWriteTime.dwLowDateTime;
    val -= epoch;
    out.mod_time = time::seconds(val / 10000000ULL); // 100ns -> seconds

    out.name = fd.cFileName;
}

void* listing_create(const wchar_t* path, entry_t<wchar_t>& start)
{
    basic_string<wchar_t, 256> wpath;
    file::normalize_path(path, wpath);

    THOR_ASSERT(!wpath.empty());
    if (!wpath.empty()) wpath.append(L"\\*");

    // Normalize again since we possibly added file separators
    file::normalize_path(wpath.c_str(), wpath);

    WIN32_FIND_DATAW data;
    HANDLE h = ::FindFirstFileW(wpath.c_str(), &data);
    if (h == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    // Skip . and .. files
    BOOL b = TRUE;
    while (b == TRUE && (string_compare(data.cFileName, L".") == 0 || string_compare(data.cFileName, L"..") == 0))
    {
        b = ::FindNextFileW(h, &data);
    }
    if (b == TRUE)
    {
        w32fd_to_entry(data, start);
        return (void*)h;
    }
    ::FindClose(h);
    return 0;
}

void listing_destroy(void* listing)
{
    HANDLE h = (HANDLE)listing;
    if (h == INVALID_HANDLE_VALUE) return;

    BOOL b = ::FindClose((HANDLE)listing);
    THOR_UNUSED(b); THOR_ASSERT(b == TRUE);
}

bool listing_next(void* listing, entry_t<wchar_t>& out)
{
    HANDLE h = (HANDLE)listing;
    if (h == INVALID_HANDLE_VALUE) return false;

    WIN32_FIND_DATAW data;
    BOOL b;
    do
    {
        b = ::FindNextFileW(h, &data);
        // Skip the . and .. entries
    } while (b == TRUE && (string_compare(data.cFileName, L".") == 0 || string_compare(data.cFileName, L"..") == 0));
    
    if (b == TRUE)
    {
        w32fd_to_entry(data, out);
        return true;
    }
    return false;
}

} // namespace internal

} // namespace dir
} // namespace thor