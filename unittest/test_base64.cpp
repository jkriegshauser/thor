#include "test_common.h"

#include "../base64.h"

TEST(base64, initial)
{
    FILE* pFile = 0;
    ::fopen_s(&pFile, "debug/unittest.exe", "rb");
    ASSERT_TRUE(pFile != 0);

    thor::vector<byte> data;
    ::fseek(pFile, 0, SEEK_END);
    long size = ::ftell(pFile);
    data.resize(size);
    ::fseek(pFile, 0, SEEK_SET);
    ASSERT_EQ(data.size(), ::fread(&data[0], 1, data.size(), pFile));

    // Regular mode
    {
        thor::vector<byte> data2;
        thor::string output;
        thor::base64_encode(false, data, output);
        EXPECT_NE(0U, output.length());

        thor::base64_decode(data2, output);
        EXPECT_NE(0U, data2.size());

        EXPECT_TRUE(data == data2);
    }

    // URL mode
    {
        thor::vector<byte> data2;
        thor::string output;
        thor::base64_encode(true, data, output);
        EXPECT_NE(0U, output.length());

        thor::base64_decode(data2, output);
        EXPECT_NE(0U, data2.size());

        EXPECT_TRUE(data == data2);
    }
}