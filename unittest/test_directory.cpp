#include "test_common.h"

#include "../directory.h"

using namespace thor;

TEST(directory, listing)
{
    dir::listing list(".");
    if (list.valid())
    {
        for (dir::listing::const_iterator iter(list.begin()); iter != list.end(); ++iter)
        {
            // printf("%s\\%s\t%llu\n", (*iter).base_path.c_str(), (*iter).name.c_str(), (*iter).size);
        }
    }

    dir::wlisting wlist(L".");
    if (wlist.valid())
    {
        for (dir::wlisting::const_iterator iter(wlist.begin()); iter != wlist.end(); ++iter)
        {
            // printf("%S\\%S\t%llu\n", (*iter).base_path.c_str(), (*iter).name.c_str(), (*iter).size);
        }
    }

    EXPECT_TRUE(dir::create("test_directory/my_test_directory"));
    EXPECT_TRUE(dir::exists("test_directory/my_test_directory"));
    EXPECT_TRUE(dir::create(L"my unicode test directory/my test directory"));
    EXPECT_TRUE(dir::exists(L"my unicode test directory/my test directory"));
    EXPECT_GT(dir::freespace("test_directory/my_test_directory"), 0);
    EXPECT_GT(dir::freespace(L"my unicode test directory/my test directory"), 0);
    EXPECT_FALSE(dir::remove("test_directory"));
    EXPECT_FALSE(dir::remove(L"my unicode test directory"));
    EXPECT_TRUE(dir::remove("test_directory", false));
    EXPECT_TRUE(dir::remove(L"my unicode test directory", false));
    EXPECT_FALSE(dir::exists("test_directory"));
    EXPECT_FALSE(dir::exists(L"my unicode test directory"));
}