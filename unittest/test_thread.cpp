#include "gtest/gtest.h"

#include "../thread.h"

class test_thread : public thor::thread
{
public:
    static thor::atomic_integer<int> test;

    test_thread() : thor::thread("test_thread") { test = 0; }

protected:
    void execute()
    {
        ++test;
    }
};

thor::atomic_integer<int> test_thread::test;

TEST(thread, initial)
{
    {
        thor::ref_pointer<test_thread> p;
        p = new test_thread;
        p->start();
        EXPECT_TRUE(p->get_thread_id() != thor::thread_id::get_current_thread());
        p->join();
        EXPECT_EQ(thor::thread::state_finished, p->get_thread_state());
        EXPECT_TRUE(1 == p->test);
    }

    {
        test_thread* p = new test_thread;
        p->start();
        int count = 0;
        while (test_thread::test == 0)
        {
            ++count;
        }
        EXPECT_EQ(1, test_thread::test.get());
    }
}