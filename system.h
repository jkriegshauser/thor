/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * system.h
 *
 * Defines standard system and platform abstractions
 */
#ifndef THOR_SYSTEM_H
#define THOR_SYSTEM_H
#pragma once

#ifndef THOR_BASETYPES_H
#include "basetypes.h"
#endif

#ifndef THOR_BASIC_STRING_H
#include "basic_string.h"
#endif

namespace thor
{

namespace system
{

size_type get_process_id();
bool get_process_path(string& out);     // utf-8
bool get_process_path(wstring& out);
bool get_command_line(string& out);     // utf-8
bool get_command_line(wstring& out);

void sleep(size_type ms);
void yield();

size_type get_cpu_count();

void get_os_version(string& str);
void get_os_version(wstring& str);

void get_os_name(string& str);
void get_os_name(wstring& str);

bool env_get(const char* var, string& out);
bool env_get(const wchar_t* var, wstring& out);
bool env_set(const char* var, const char* val);
bool env_set(const wchar_t* var, const wchar_t* val);

bool get_machine_name(string& out);     // utf-8
bool get_machine_name(wstring& out);
bool get_local_ip(string& out);
bool get_local_ip(wstring& out);

void get_console_title(string& out);
void get_console_title(wstring& out);
void set_console_title(const char* val);
void set_console_title(const wchar_t* val);

void get_working_dir(string& out);
void get_working_dir(wstring& out);
bool set_working_dir(const char* dir);
bool set_working_dir(const wchar_t* dir);

void get_module_path(string& out);
void get_module_path(wstring& out);
bool set_module_path(const char* path);
bool set_module_path(const wchar_t* path);

void get_absolute_path(const char* relative, string& absolute);
void get_absolute_path(const wchar_t* relative, wstring& absolute);

inline bool is_little_endian() { size_type s = 1; return (*(byte*)&s) == 1; }
inline bool is_big_endian() { return !is_little_endian(); }

}

}

#endif