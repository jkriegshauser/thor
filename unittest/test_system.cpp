#include "gtest/gtest.h"

#include "../system.h"

using namespace thor::system;

TEST(system, initial)
{
    EXPECT_TRUE(get_process_id() != 0);

    thor::basic_string<char, 512> str;
    thor::basic_string<wchar_t, 512> wstr;

    EXPECT_TRUE(get_process_path(str));
    EXPECT_TRUE(get_process_path(wstr));
    EXPECT_FALSE(str.empty());
    EXPECT_FALSE(wstr.empty());
    EXPECT_EQ(str.length(), thor::string_length(str.c_str()));
    EXPECT_EQ(wstr.length(), thor::string_length(wstr.c_str()));

    EXPECT_TRUE(get_command_line(str));
    EXPECT_TRUE(get_command_line(wstr));
    EXPECT_FALSE(str.empty());
    EXPECT_FALSE(wstr.empty());
    EXPECT_EQ(str.length(), thor::string_length(str.c_str()));
    EXPECT_EQ(wstr.length(), thor::string_length(wstr.c_str()));

    sleep(1);
    yield();

    EXPECT_GE(get_cpu_count(), 1U);

    str.clear(); wstr.clear();
    get_os_version(str);
    get_os_version(wstr);
    EXPECT_FALSE(str.empty());
    EXPECT_FALSE(wstr.empty());
    EXPECT_EQ(str.length(), thor::string_length(str.c_str()));
    EXPECT_EQ(wstr.length(), thor::string_length(wstr.c_str()));

    str.clear(); wstr.clear();
    get_os_name(str);
    get_os_name(wstr);
    EXPECT_FALSE(str.empty());
    EXPECT_FALSE(wstr.empty());
    EXPECT_EQ(str.length(), thor::string_length(str.c_str()));
    EXPECT_EQ(wstr.length(), thor::string_length(wstr.c_str()));

    EXPECT_TRUE(env_set("UNITTEST", "Hello World"));
    EXPECT_TRUE(env_set(L"UNITTESTUNICODE", L"Hello World Unicode"));

    EXPECT_TRUE(env_get("UNITTEST", str));
    EXPECT_TRUE(env_get(L"UNITTESTUNICODE", wstr));
    EXPECT_FALSE(str.empty());
    EXPECT_FALSE(wstr.empty());
    EXPECT_EQ(str.length(), thor::string_length(str.c_str()));
    EXPECT_EQ(wstr.length(), thor::string_length(wstr.c_str()));
    
    EXPECT_TRUE(get_machine_name(str));
    EXPECT_TRUE(get_machine_name(wstr));
    EXPECT_FALSE(str.empty());
    EXPECT_FALSE(wstr.empty());
    EXPECT_EQ(str.length(), thor::string_length(str.c_str()));
    EXPECT_EQ(wstr.length(), thor::string_length(wstr.c_str()));

    EXPECT_TRUE(get_local_ip(str));
    EXPECT_TRUE(get_local_ip(wstr));
    EXPECT_FALSE(str.empty());
    EXPECT_FALSE(wstr.empty());
    EXPECT_EQ(str.length(), thor::string_length(str.c_str()));
    EXPECT_EQ(wstr.length(), thor::string_length(wstr.c_str()));

    str.clear(); wstr.clear();
    get_console_title(str);
    get_console_title(wstr);
    EXPECT_FALSE(str.empty());
    EXPECT_FALSE(wstr.empty());
    EXPECT_EQ(str.length(), thor::string_length(str.c_str()));
    EXPECT_EQ(wstr.length(), thor::string_length(wstr.c_str()));

    set_console_title("Ascii!");
    set_console_title(L"Unicode!");

    str.clear(); wstr.clear();
    get_working_dir(str);
    get_working_dir(wstr);
    EXPECT_FALSE(str.empty());
    EXPECT_FALSE(wstr.empty());
    EXPECT_EQ(str.length(), thor::string_length(str.c_str()));
    EXPECT_EQ(wstr.length(), thor::string_length(wstr.c_str()));
    EXPECT_TRUE(set_working_dir(str.c_str()));
    EXPECT_TRUE(set_working_dir(wstr.c_str()));

    str.clear(); wstr.clear();
    get_module_path(str);
    get_module_path(wstr);
    EXPECT_EQ(str.length(), thor::string_length(str.c_str()));
    EXPECT_EQ(wstr.length(), thor::string_length(wstr.c_str()));
    EXPECT_TRUE(set_module_path(str.c_str()));
    EXPECT_TRUE(set_module_path(wstr.c_str()));

    str.clear(); wstr.clear();
    get_absolute_path("..", str);
    get_absolute_path(L"..", wstr);
    EXPECT_FALSE(str.empty());
    EXPECT_FALSE(wstr.empty());
    EXPECT_EQ(str.length(), thor::string_length(str.c_str()));
    EXPECT_EQ(wstr.length(), thor::string_length(wstr.c_str()));
   
    EXPECT_TRUE(is_little_endian());
    EXPECT_FALSE(is_big_endian());
}