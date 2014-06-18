#include "test_common.h"

#include "../thor/priority_queue.h"
#include "../thor/deque.h"

template<typename Container> void perform_test()
{
    typedef thor::priority_queue<int, Container> priority_queue;
    {
        priority_queue test;
        EXPECT_TRUE(test.empty());
        EXPECT_EQ(0, test.size());
    }

    {
        priority_queue::compare_type comp;
        priority_queue pqtest(comp);
        EXPECT_TRUE(pqtest.empty());
        EXPECT_EQ(0, pqtest.size());
    }

    int myints[] = {10, 20, 30, 5, 15};
    {
        priority_queue test(myints, myints + 5);
        priority_queue test3(myints, myints + 5, priority_queue::compare_type());
        EXPECT_FALSE(test.empty());
        EXPECT_EQ(5, test.size());
        EXPECT_EQ(30, test.top());

        test.pop();
        EXPECT_EQ(4, test.size());
        EXPECT_EQ(20, test.top());

        test.push();
        EXPECT_EQ(5, test.size());
        EXPECT_EQ(20, test.top());

        test.push(99);
        EXPECT_EQ(6, test.size());
        EXPECT_EQ(99, test.top());

        priority_queue test2(test);
        EXPECT_EQ(6, test2.size());
        EXPECT_EQ(99, test2.top());
    }
}

TEST(priority_queue, with_vector)
{
    perform_test<thor::vector<int> >();
}

TEST(priority_queue, with_deque)
{
    perform_test<thor::deque<int> >();
}