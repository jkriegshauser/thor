/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * system.h
 *
 * Defines standard system and platform abstractions
 */

#include "../system.h"

#include <WinSock2.h>
#include <WS2tcpip.h>

#define WIN32_EXTRA_LEAN 1
#include <Windows.h>

namespace thor
{

namespace system
{

size_type get_process_id()
{
    return (size_type)::GetCurrentProcessId();
}

bool get_process_path(string& out)
{
    basic_string<wchar_t, 512> temp;
    if (!get_process_path(temp)) return false;
    wide_to_utf8(temp, out);
    return true;
}

bool get_process_path(wstring& out)
{
    basic_string<wchar_t, 512> temp;
    temp.resize(temp.embedded_size);
    for (;;)
    {
        DWORD size = ::GetModuleFileNameW(0, &temp[0], (DWORD)temp.size());
        if (size == 0)
        {
            // An error occurred.
            return false;
        }
        if (size == temp.size())
        {
            // Needs to be bigger
            temp.reserve(temp.size() * 2);
            temp.resize(temp.size() * 2);
            continue;
        }
        // Have plenty of space.
        temp.resize(size);
        out = temp;
        return true;
    }
}

bool get_command_line(string& out)
{
    basic_string<wchar_t, 512> temp;
    if (!get_command_line(temp)) return false;
    wide_to_utf8(temp, out);
    return true;
}

bool get_command_line(wstring& out)
{
    const wchar_t* str = ::GetCommandLineW();
    if (str == 0) return false;
    out = str;
    return true;
}

void sleep(size_type ms)
{
    ::Sleep((DWORD)ms);
}

void yield()
{
    ::YieldProcessor();
}

size_type get_cpu_count()
{
    SYSTEM_INFO info;
    ::GetSystemInfo(&info);
    return size_type(info.dwNumberOfProcessors);
}

void get_os_version(string& str)
{
    basic_string<wchar_t, 512> temp;
    get_os_version(temp);
    wide_to_utf8(temp, str);
}

void get_os_version(wstring& str)
{
    OSVERSIONINFOW info;
    ::memset(&info, 0, sizeof(info));
    info.dwOSVersionInfoSize = sizeof(info);
    ::GetVersionExW(&info);
    str.format(L"%u.%u.%u", info.dwMajorVersion, info.dwMinorVersion, info.dwBuildNumber);
}

void get_os_name(string& str)
{
    str = "Windows";
}

void get_os_name(wstring& str)
{
    str = L"Windows";
}

bool env_get(const char* var, string& out)
{
    out.clear();
    DWORD size = ::GetEnvironmentVariableA(var, 0, 0);
    if (size == 0) return false;
    out.resize(size);
    ::GetEnvironmentVariableA(var, &out[0], (DWORD)out.size());
    out.pop_back(); // NUL char
    return true;
}

bool env_get(const wchar_t* var, wstring& out)
{
    out.clear();
    DWORD size = ::GetEnvironmentVariableW(var, 0, 0);
    if (size == 0) return false;
    out.resize(size);
    ::GetEnvironmentVariableW(var, &out[0], (DWORD)out.size());
    out.pop_back(); // NUL char
    return true;
}

bool env_set(const char* var, const char* val)
{
    return ::SetEnvironmentVariableA(var, val) == TRUE;
}

bool env_set(const wchar_t* var, const wchar_t* val)
{
    return ::SetEnvironmentVariableW(var, val) == TRUE;
}

bool get_machine_name(string& out)
{
    basic_string<wchar_t, 512> temp;
    if (!get_machine_name(temp)) return false;
    wide_to_utf8(temp, out);
    return true;
}

bool get_machine_name(wstring& out)
{
    wchar_t temp[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(temp)/sizeof(wchar_t);
    if (::GetComputerNameW(temp, &size) != TRUE) return false;
    out = temp;
    return true;
}

bool get_local_ip(string& out)
{
    basic_string<wchar_t, 32> temp;
    bool success = get_local_ip(temp);
    wide_to_utf8(temp, out);
    return success;
}

#pragma comment (lib, "ws2_32.lib")
bool get_local_ip(wstring& out)
{
    WSADATA data;
    ::WSAStartup(MAKEWORD(2,2), &data);

    bool success = false;

    basic_string<wchar_t, 32> machine_name;
    if (get_machine_name(machine_name))
    {
        addrinfoW* p = 0;
        addrinfoW  hints;
        ::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        int val = ::GetAddrInfoW(machine_name.c_str(), 0, &hints, &p);
        if (val == 0 && p != 0)
        {
            struct sockaddr_in* psin = (struct sockaddr_in*)(p->ai_addr);
            out.resize(32);
            DWORD size = (DWORD)out.size();
            if (::WSAAddressToStringW((struct sockaddr*)psin, sizeof(*psin), NULL, &out[0], &size) == 0 && size > 0)
            {
                out.resize((size_type)(size - 1)); // remove appended NUL
                success = true;
            }
        }
    }

    if (!success)
    {
        out.assign(wstring::lit_allow_share, L"127.0.0.1");
    }
    ::WSACleanup();
    return success;
}

void get_console_title(string& out)
{
    basic_string<wchar_t, 512> temp;
    get_console_title(temp);
    wide_to_utf8(temp, out);
}

void get_console_title(wstring& out)
{
    wchar_t temp[512 + 1];
    ::GetConsoleTitleW(temp, sizeof(temp)/sizeof(wchar_t));
    temp[512] = wchar_t(0);
    out = temp;
}

void set_console_title(const char* val)
{
    ::SetConsoleTitleA(val);
}

void set_console_title(const wchar_t* val)
{
    ::SetConsoleTitleW(val);
}

void get_working_dir(string& out)
{
    basic_string<wchar_t, 512> temp;
    get_working_dir(temp);
    wide_to_utf8(temp, out);
}

void get_working_dir(wstring& out)
{
    DWORD len = ::GetCurrentDirectoryW(0, NULL);
    THOR_DEBUG_ASSERT(len > 0);
    out.resize(len);
    ::GetCurrentDirectoryW((DWORD)out.size(), &out[0]);
    THOR_DEBUG_ASSERT(out.back() == wstring::value_type(0));
    out.pop_back(); // Remove NUL
}

bool set_working_dir(const char* dir)
{
    basic_string<wchar_t, 512> temp;
    utf8_to_wide(dir, temp);
    return set_working_dir(temp.c_str());
}

bool set_working_dir(const wchar_t* dir)
{
    return ::SetCurrentDirectoryW(dir) == TRUE;
}

void get_module_path(string& out)
{
    basic_string<wchar_t, 512> temp;
    get_module_path(temp);
    wide_to_utf8(temp, out);
}

void get_module_path(wstring& out)
{
    DWORD len = ::GetDllDirectoryW(0, 0);
    THOR_DEBUG_ASSERT(len > 0);
    out.resize(len);
    len = ::GetDllDirectoryW((DWORD)out.size(), &out[0]);
    THOR_DEBUG_ASSERT(out.back() == wstring::value_type(0));
    out.pop_back(); // Remove NUL
}

bool set_module_path(const char* path)
{
    basic_string<wchar_t, 512> temp;
    utf8_to_wide(path, temp);
    return set_module_path(temp.c_str());
}

bool set_module_path(const wchar_t* path)
{
    return ::SetDllDirectoryW(path) == TRUE;
}

void get_absolute_path(const char* relative, string& absolute)
{
    basic_string<wchar_t, 512> temp, temp_abs;
    utf8_to_wide(relative, temp);
    get_absolute_path(temp.c_str(), temp_abs);
    wide_to_utf8(temp_abs, absolute);
}

void get_absolute_path(const wchar_t* relative, wstring& absolute)
{
    DWORD len = ::GetFullPathNameW(relative, 0, 0, 0);
    THOR_DEBUG_ASSERT(len > 0);
    absolute.resize(len);
    ::GetFullPathNameW(relative, (DWORD)absolute.size(), &absolute[0], 0);
    THOR_DEBUG_ASSERT(absolute.back() == wstring::value_type(0));
    
    // Apparently GetFullPathNameW() lies to us about the length. Measure the string here.
    absolute.resize(string_length(absolute.c_str()));
}

}

}