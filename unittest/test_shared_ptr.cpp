#include "test_common.h"

#include "../shared_ptr.h"
#include "../auto_ptr.h"

#pragma warning(disable : 4800) // forcing value to bool 'true' or 'false' (performance warning)

using namespace thor;

struct sptr_test_base
{
    static atomic_integer<size_type> constructed;
    static atomic_integer<size_type> destructed;

    virtual void foo() {}

    sptr_test_base()
    {
        ++constructed;
    }
    virtual ~sptr_test_base()
    {
        ++destructed;
    }
};
atomic_integer<size_type> sptr_test_base::constructed(0);
atomic_integer<size_type> sptr_test_base::destructed(0);


struct sptr_test : public sptr_test_base
{
    static atomic_integer<size_type> constructed;
    static atomic_integer<size_type> destructed;

    sptr_test() { ++constructed; }
    ~sptr_test() { ++destructed; }
};
atomic_integer<size_type> sptr_test::constructed(0);
atomic_integer<size_type> sptr_test::destructed(0);

TEST(shared_ptr, initial)
{
    {
        shared_ptr<sptr_test> ptr3(new sptr_test);
        EXPECT_EQ(sptr_test_base::constructed, 1);
        EXPECT_EQ(sptr_test_base::destructed, 0);
        EXPECT_EQ(sptr_test::constructed, 1);
        EXPECT_EQ(sptr_test::destructed, 0);
        shared_ptr<sptr_test_base> ptr(ptr3);
        ptr = new sptr_test;
        EXPECT_EQ(sptr_test::constructed, 2);
        EXPECT_EQ(sptr_test::destructed, 0);
        EXPECT_EQ(sptr_test_base::constructed, 2);
        EXPECT_EQ(sptr_test_base::destructed, 0);
        weak_ptr<sptr_test> weak;
        weak = ptr3;
        EXPECT_TRUE(weak.lock());
        shared_ptr<sptr_test> ptr2(weak.lock());
        if (ptr2)
        {
            ptr2->foo();
        }
    }

    EXPECT_EQ(sptr_test::destructed, 2);
    EXPECT_EQ(sptr_test_base::destructed, 2);

    // Comparison tests
    {
        shared_ptr<sptr_test> ptr1, ptr2;
        sptr_test* p(0);
        EXPECT_FALSE(ptr1);
        EXPECT_EQ(ptr1, ptr2);
        EXPECT_EQ(ptr1, p);
    }
}

TEST(auto_ptr, initial)
{
    auto_ptr<sptr_test> ptr(new sptr_test);
    auto_ptr<sptr_test> ptr2 = ptr;
}