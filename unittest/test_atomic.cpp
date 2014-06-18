#include "test_common.h"

#include "../atomic_integer.h"
#include "../atomic_pointer.h"

template<typename T> void perform_test()
{
    typedef __declspec(align(8)) thor::atomic_integer<T> atomic_int;
    atomic_int i;
    EXPECT_TRUE(i == 0);
    EXPECT_TRUE(0 == i);
    EXPECT_FALSE(i < 0);
    EXPECT_TRUE(i <= 0);
    EXPECT_TRUE(i >= 0);
    EXPECT_FALSE(i > 0);
    EXPECT_FALSE(i != 0);
    EXPECT_FALSE(0 != i);
    EXPECT_FALSE(0 > i);
    EXPECT_FALSE(0 < i);
    EXPECT_TRUE(0 >= i);
    EXPECT_TRUE(0 <= i);

    EXPECT_TRUE(++i == 1);
    EXPECT_TRUE(i++ == 1);
    EXPECT_TRUE(i == 2);
    i = 3;
    EXPECT_TRUE(i == 3);
    EXPECT_TRUE(--i == 2);
    EXPECT_TRUE(i-- == 2);
    EXPECT_TRUE(i == 1);
    atomic_int i2(4);
    atomic_int i3(i2);
    EXPECT_TRUE(i3 == 4);
    i = 4;
    EXPECT_TRUE(i == 4);
    i.set(5);
    EXPECT_TRUE(i == 5);
    EXPECT_TRUE(i.get() == 5);
    EXPECT_TRUE(i.exchange(10) == 5);
    EXPECT_TRUE(i == 10);
    EXPECT_TRUE(i.compare_exchange(5, 5) == 10);
    EXPECT_TRUE(i == 10);
    EXPECT_TRUE(i.compare_exchange(5, 10) == 10);
    EXPECT_TRUE(i == 5);

    i += 10;
    EXPECT_TRUE(i == 15);
    i -= 10;
    EXPECT_TRUE(i == 5);
}

TEST(atomic_integer, int)
{
    perform_test<int>();
}

TEST(atomic_integer, short)
{
    perform_test<short>();
}

TEST(atomic_integer, char)
{
    perform_test<char>();
}

TEST(atomic_integer, __int64)
{
    perform_test<__int64>();
}

template<typename T> void perform_pointer_test()
{
    typedef __declspec(align(8)) thor::atomic_pointer<T> atomic_pointer;
    atomic_pointer p;
    EXPECT_TRUE(p == 0);

    p.exchange((T*)0x1234);
    ++p;
    EXPECT_TRUE((thor_size_type)p.get() == (0x1234 + sizeof(T)));

    p.set((T*)0x123456);

    atomic_pointer p2 = p;
    atomic_pointer p3(p2);
    atomic_pointer p4((T*)0x1234);

    p2 = (T*)0x2345;

    ++p4;
    p4++;
    --p4;
    p4--;

    T* raw = p;
    const T* raw2 = const_cast<const atomic_pointer&>(p);
    THOR_UNUSED(raw); THOR_UNUSED(raw2);

    p.compare_exchange(0, 0);
}

TEST(atomic_pointer, int)
{
    perform_pointer_test<int>();
}

TEST(atomic_pointer, char)
{
    perform_pointer_test<char>();
}

TEST(atomic_pointer, __int64)
{
    perform_pointer_test<__int64>();
}

TEST(atomic_pointer, s)
{
    perform_pointer_test<s>();
}

// TODO: Thread test