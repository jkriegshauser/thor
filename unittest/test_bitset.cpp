#include "test_common.h"

#include "../bitset.h"

using namespace thor;

template<unsigned N> void test_bitset()
{
    typedef thor::bitset<N> bitset;
    
    {
        bitset test;
        test[0] = true;
        EXPECT_TRUE(test[0]);
        EXPECT_TRUE(const_cast<const bitset&>(test)[0]); // test the const operator []
    }

    {
        bitset test((unsigned long)0x12345678);
        bitset test2(test);
        bitset test3; test3 = test;
        EXPECT_TRUE(test == test3);
        EXPECT_TRUE(test == test2);
        const bool matchvals[] = {
            0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0
        };
        for (bitset::size_type i = 0; i < (sizeof(matchvals)/sizeof(matchvals[0])); ++i)
        {
            EXPECT_TRUE(test.size() <= i || test.test(i) == matchvals[i]) << i;
        }
    }

    {
        bitset test(thor::string("00010010001101000101011001111000"), 0, THOR_SUPPRESS_WARNING(N == 0) ? size_type(-1) : N);
        if (test.size() < 4)
        {
            EXPECT_FALSE(test.any());
            EXPECT_TRUE(test.none());
        }
        else
        {
            EXPECT_TRUE(test.any());
            EXPECT_FALSE(test.none());
        }
        unsigned long val = (unsigned long)0x12345678;
        if (test.size() < (8 * sizeof(unsigned long)))
        {
            // Mask off unused bits
            unsigned long mask = (1ul << ((sizeof(unsigned long)*8) - test.size())) - 1;
            val &= ~mask;
        }
        EXPECT_EQ(test.to_ulong(), val) << val;
    }
    {
        bitset test(thor::wstring(L"00010010001101000101011001111000"), 0, THOR_SUPPRESS_WARNING(N == 0) ? size_type(-1) : N);
        if (test.size() < 4)
        {
            EXPECT_FALSE(test.any());
            EXPECT_TRUE(test.none());
        }
        else
        {
            EXPECT_TRUE(test.any());
            EXPECT_FALSE(test.none());
        }
        unsigned long val = (unsigned long)0x12345678;
        if (test.size() < (8 * sizeof(unsigned long)))
        {
            // Mask off unused bits
            unsigned long mask = (1ul << ((sizeof(unsigned long)*8) - test.size())) - 1;
            val &= ~mask;
        }
        EXPECT_EQ(test.to_ulong(), val) << val;
    }

    if (THOR_SUPPRESS_WARNING(N == 0 || N >= 32))
    {
        bitset b1(0xcdcdcdcd), b2(0x8b4b4b41);
        EXPECT_EQ(0x32323232, (~b1).to_ulong());
        EXPECT_EQ(0x89494941, (b1 & b2).to_ulong());
        EXPECT_EQ(0xcfcfcfcd, (b1 | b2).to_ulong());
        EXPECT_EQ(0x4686868c, (b1 ^ b2).to_ulong());
        EXPECT_EQ(0x66e6e6e6, (b1 >> 1).to_ulong());
        EXPECT_EQ(0x9b9b9b9a, (b1 << 1).to_ulong());
    }

    if (THOR_SUPPRESS_WARNING(N == 0 || N >=32))
    {
        bitset test(0);
        EXPECT_TRUE(test.none());
        EXPECT_FALSE(test.any());
        test.set();
        EXPECT_TRUE(test.any());
        EXPECT_TRUE(test.all());
        EXPECT_FALSE(test.none());
        EXPECT_EQ(thor::string::npos, test.to_string<char>().find('0'));
    }

    const bitset::size_type count = (N == 0 ? (8*sizeof(unsigned long)) : N);
    {
        bitset test(0);
        test.set();
        EXPECT_EQ(count, test.count());
        test >>= (count - 1);
        EXPECT_EQ(1, test.count());
    }

    if (THOR_SUPPRESS_WARNING(count > 1))
    {
        bitset test(0);
        test.set(0);
        test.set(count - 1);
        EXPECT_EQ(2, test.count());
        test.set(0, 0);
        test.set(count - 1, 0);
        EXPECT_EQ(0, test.count());
        test.set(0);
        test.set(count - 1);
        EXPECT_EQ(2, test.count());
        test.reset(0);
        test.reset(count - 1);
        EXPECT_EQ(0, test.count());
        EXPECT_FALSE(test.test(0));
        EXPECT_FALSE(test.test(count - 1));
        test.set(0);
        test.set(count - 1);
        EXPECT_TRUE(test.test(0));
        EXPECT_TRUE(test.test(count - 1));

        bitset test2(0);
        test2.set(0);
        test2.set(count - 1);
        test2.rotate_left(1);
        EXPECT_EQ(2, test2.count());
        EXPECT_TRUE(test2.test(count - 1));
        EXPECT_TRUE(test2.test(count - 2));
        test2.rotate_right(2);
        EXPECT_EQ(2, test2.count());
        EXPECT_TRUE(test2.test(0));
        EXPECT_TRUE(test2.test(1));
    }
}

TEST(bitset, initial)
{
    test_bitset<1>();
    test_bitset<12>();
    test_bitset<31>();
    test_bitset<32>();
    test_bitset<33>();
    test_bitset<63>();
    test_bitset<64>();
    test_bitset<65>();
    test_bitset<66>();
    test_bitset<0>();
}