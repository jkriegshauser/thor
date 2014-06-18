#include "test_common.h"

#include "../file.h"

using namespace thor;

TEST(file, test)
{
    thor::file::properties p, p2;

    EXPECT_TRUE(thor::file::exists("c:/windows/system32", &p));
    EXPECT_TRUE(thor::file::exists(L"c:/windows\\\\system32", &p));
    EXPECT_TRUE(thor::file::exists("c:\\windows\\\\/system32\\winver.exe", &p));
    EXPECT_TRUE(thor::file::exists(L"c:/windows\\/\\system32/winver.exe", &p));

    FILE* temp = 0;
    ::fopen_s(&temp, "test.txt", "a+");
    ASSERT_TRUE(temp != 0);
    ::fputs("Hello World!\n", temp);
    ::fclose(temp); temp = 0;

    p = file::properties();
    p.hidden = true;
    p.create_time = time::seconds_adjust(time::seconds_now(), -100000);
    p.mod_time = time::seconds_adjust(time::seconds_now(), -1000000);
    EXPECT_TRUE(file::set_properties(L"test.txt", p));

    EXPECT_TRUE(file::exists(L"test.txt", &p2));
    EXPECT_TRUE(p2.hidden);
    EXPECT_FALSE(p2.directory);
    EXPECT_FALSE(p2.read_only);
    EXPECT_EQ(p2.mod_time.cvalue(), p.mod_time.cvalue());
    EXPECT_EQ(p2.create_time.cvalue(), p.create_time.cvalue());

    EXPECT_TRUE(file::move(L"test.txt", L"test2.txt"));
    EXPECT_FALSE(file::remove(L"test.txt"));
    EXPECT_TRUE(file::remove(L"test2.txt"));
}