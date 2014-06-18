#include "list.h"
#include "test_common.h"

template <class T> class ListValidator
{
public:
    bool operator () (T& t)
    {
        return t.validate();
    }
};

template <typename T, thor_size_type T_PREALLOC> bool test_list()
{
    typedef thor::list<T, T_PREALLOC> list;

    {
        list l;
        EXPECT_TRUE(l.size() == 0);
        EXPECT_TRUE(l.empty());
    }

    {
        const list l(5U, T());
        EXPECT_TRUE(l.size() == 5);
        list::size_type t = 0;
        for (list::const_iterator iter(l.begin()); iter != l.end(); ++iter)
        {
            ++t;
        }
        EXPECT_TRUE(t == 5);
        EXPECT_TRUE(!l.empty());

        list l2(l);
        EXPECT_TRUE(l2.size() == l.size());
        l2.validate();

        list l3(2); l3 = l; l3.validate();
        list l4(7); l4 = l; l4.validate();

        {
            T& t = l3.front(); t = T();
            const T& ct = l.front(); THOR_UNUSED(ct);
        }
        {
            T& t = l3.back(); t = T();
            const T& ct = l.back(); THOR_UNUSED(ct);
        }
    }

    {
        list l(5);
        EXPECT_TRUE(l.size() == 5);
        list::size_type t = 0;
        for (list::iterator iter(l.begin()); iter != l.end(); ++iter)
        {
            *iter = T();
            ++t;
        }
        EXPECT_TRUE(t == 5);
    }

    return true;
}

template <thor_size_type T_PREALLOC> bool test_list_extensions()
{
    // Test pointer extensions
    test_pointer_types<thor::list<s*, T_PREALLOC>, ListValidator<thor::list<s*, T_PREALLOC> > >();

    // Test push_back extensions
    test_push_back<thor::list<nocopy, T_PREALLOC>, ListValidator<thor::list<nocopy, T_PREALLOC> > >(0, 1.f, 2.0, 3, '4');

    // Test push_front extensions
    test_push_front<thor::list<nocopy, T_PREALLOC>, ListValidator<thor::list<nocopy, T_PREALLOC> > >(0, 1.f, 2.0, 3, '4');

    // Test insert extensions
    test_insert<thor::list<s, T_PREALLOC>, ListValidator<thor::list<s, T_PREALLOC> > >(0, 1.f, 2.0, 3, '4');

    return true;
}

template <class T, class U> bool test_list_swap()
{
    bool b = true;
    T l1;
    l1.push_back(1);
    l1.push_back(2);
    U l2;
    l2.push_front(4);
    l2.push_front(3);
    l1.swap(l2);
    thor::swap(l1, l2);
    l1.swap(l2);
    l1.validate();
    l2.validate();
    EXPECT_TRUE(l1.front() == 3);
    EXPECT_TRUE(l1.back()  == 4);
    EXPECT_TRUE(l2.front() == 1);
    EXPECT_TRUE(l2.back()  == 2);
    return b;
}

template <class T, class U> bool test_splice()
{
    bool b = true;
    T l1;
    l1.push_back(3); // 3
    l1.push_back(4); // 3 4
    U l2;
    l2.push_front(2); // 2
    l2.push_front(1); // 1 2

    // Splice
    T l3(5); // 0 0 0 0 0
    l1.splice(l1.begin(), l2); // 1 2 3 4 / -
    l1.validate();
    l2.validate();
    EXPECT_TRUE(l1.size() == 4);
    EXPECT_TRUE(l2.size() == 0);

    l2.splice(l2.begin(), l1, l1.begin()); // 1 / 2 3 4
    l1.validate();
    l2.validate();
    EXPECT_TRUE(l2.size() == 1);
    EXPECT_TRUE(l1.size() == 3);

    T::iterator iter(l3.begin()); ++iter;
    l3.splice(iter, l1, l1.begin(), --l1.end()); // 0 2 3 (iter)0 0 0 0 / 4
    l3.validate();
    l1.validate();
    EXPECT_TRUE(l1.size() == 1);
    EXPECT_TRUE(l3.size() == 7);
    --iter;
    EXPECT_TRUE(*iter == 3);

    // Test remove()
    l3.insert(--l3.end(), 3);
    l3.remove(3);
    EXPECT_TRUE(l3.size() == 6);
    l3.validate();

    // Test sort()
    l3.sort();
    l3.validate();

    l3.sort(thor::greater<int>());
    l3.validate();

    // Test unique()
    l3.unique();
    l3.validate();

    // Test move()
    l3.push_back(4);
    l3.push_back(5); // 2 0 4 5
    l3.move(--l3.end(), ++l3.begin()); // 2 5 0 4
    l3.validate();
    l3.move(l3.begin(), l3.end()); // 5 0 4 2
    l3.validate();
    EXPECT_TRUE(l3.size() == 4);
    iter = l3.begin();
    EXPECT_TRUE(*iter++ == 5);
    EXPECT_TRUE(*iter++ == 0);
    EXPECT_TRUE(*iter++ == 4);
    EXPECT_TRUE(*iter++ == 2);
    EXPECT_TRUE(iter == l3.end());

    return b;
}

TEST(test_list, test_list)
{
    test_list<int, 0>();
    test_list<int, 5>();
    test_list<s, 0>();
    test_list<s, 5>();
    test_list<aligntest, 0>();
    test_list<aligntest, 5>();
    test_list_extensions<0>();
    test_list_extensions<3>();
    test_list_extensions<10>();
    test_resize<thor::list<int>, ListValidator<thor::list<int> > >(100);
    test_resize<thor::list<int, 10>, ListValidator<thor::list<int, 10> > >(100);

    {
        thor::list<int> list(10);
        test_erase<ListValidator<thor::list<int> > >(list);
    }
    {
        thor::list<s, 5> list(10);
        test_erase<ListValidator<thor::list<s, 5> > >(list);
    }

    // swap test
    test_list_swap<thor::list<int>, thor::list<int> >();
    test_list_swap<thor::list<int>, thor::list<int, 5> >();
    test_list_swap<thor::list<int, 5>, thor::list<int> >();
    test_list_swap<thor::list<int, 5>, thor::list<int, 5> >();

    // splice test
    test_splice<thor::list<int>, thor::list<int> >();
    test_splice<thor::list<int>, thor::list<int, 5> >();
    test_splice<thor::list<int, 5>, thor::list<int> >();
    test_splice<thor::list<int, 5>, thor::list<int, 5> >();
}