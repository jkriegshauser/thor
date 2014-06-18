#include "../embedded_list.h"
#include "test_common.h"

using namespace thor;

template <class T> struct DeleteEntry
{
    void operator () (T* p) const
    {
        delete p;
    }
};

template <class T, embedded_list_link<T> T::*U> bool test_elist()
{
    typedef embedded_list<T,U> list;

    {
        list l;
        EXPECT_TRUE(l.size() == 0);
        EXPECT_TRUE(l.empty());
    }

    {
        list l;
        const list& l2 = l;
        l2.size();
        l.push_back(new T(0));
        l.push_front(new T(1));
        EXPECT_TRUE(l.size() == 2);
        EXPECT_TRUE(!l.empty());
        EXPECT_TRUE(l.front() == T(1));
        EXPECT_TRUE(l.back() == T(0));
        EXPECT_TRUE(l2.front() == T(1));
        EXPECT_TRUE(l2.back() == T(0));

        list::size_type t = 0;
        for (list::const_iterator iter(l.begin()); iter != l.end(); ++iter)
        {
            ++t;
            iter++;
            iter--;
            *iter;
        }
        EXPECT_TRUE(t == 2);
        t = 0;
        for (list::iterator iter(l.begin()); iter != l.end(); ++iter)
        {
            ++t;
            iter++;
            iter--;
            *iter;
        }
        EXPECT_TRUE(t == 2);
        t = 0;
        for (list::const_reverse_iterator iter(l.rbegin()); iter != l.rend(); ++iter)
        {
            ++t;
            iter++;
            iter--;
            *iter;
        }
        EXPECT_TRUE(t == 2);
        t = 0;
        for (list::reverse_iterator iter(l.rbegin()); iter != l.rend(); ++iter)
        {
            ++t;
            iter++;
            iter--;
            *iter;
        }
        EXPECT_TRUE(t == 2);
        delete l.pop_front();
        delete l.pop_back();
        EXPECT_TRUE(l.empty());
    }

    {
        list l;
        for (int i = 100; i > 0; --i)
        {
            l.push_back(new T(i));
        }
        l.push_back(new T(5));
        l.push_back(new T(5));
        l.sort();
        list::iterator iter, lastiter;
        for (iter = l.begin(); iter != l.end(); ++iter)
        {
            if (iter != l.begin())
            {
                EXPECT_TRUE(!(*iter < *lastiter));
            }
            lastiter = iter;
        }
        iter = l.unique();
        EXPECT_TRUE(iter != l.end());
        int count = 0;
        while (iter != l.end())
        {
            T* p = l.remove(iter++);
            delete p;
            ++count;
        }
        EXPECT_TRUE(count == 2);
        l.pop_front_delete();
        l.pop_back_delete();

        iter = l.insert(++l.begin(), new T(10101));
        l.pop_front_delete();
        EXPECT_TRUE(l.front() == T(10101));
        // l.delete_all();
        l.remove(l.begin(), DeleteEntry<T>());
        l.remove(--l.end(), l.end(), DeleteEntry<T>());
        l.remove_all(DeleteEntry<T>());
    }

    return true;
}

struct MyTest
{
private:
    char c[100];
public:
    embedded_list_link<MyTest> link;

    MyTest(int n) : i(n) {}

    int i;

    bool operator == (const MyTest& rhs) const
    {
        return i == rhs.i;
    }

    bool operator < (const MyTest& rhs) const
    {
        return i < rhs.i;
    }
};

TEST(test_elist, test_elist)
{
    test_elist<MyTest, &MyTest::link>();
}