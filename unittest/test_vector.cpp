#include "test_common.h"
#include "vector.h"

template <class Iter> void test_iterator(Iter iter)
{
    Iter i = iter;
    ++i;
    i++;
    i--;
    --i;
    i += 2;
    i - 2;
    i -= 2;
    i + 2;
    EXPECT_TRUE(i == iter);
    EXPECT_TRUE(i - iter == 0);
}

template <class T> bool test_inheritance(thor::vector<T>& v)
{
    typedef thor::vector<T> vec;
    v.push_back();
    v.insert(v.begin());

    return true;
}

template <class T, unsigned T_PREALLOC> bool test_vector()
{
    typedef thor::vector<T, T_PREALLOC> vec;
    EXPECT_TRUE(vec().size() == 0);
    EXPECT_TRUE(vec().capacity() == 0);
    {
        vec v;
        EXPECT_TRUE(v.empty());
        EXPECT_TRUE(v.begin() == v.end());
        EXPECT_TRUE(v.rbegin() == v.rend());

        v.push_back(T());
        v.push_back(T());
        v.push_back(T());
        EXPECT_TRUE(v.size() == 3);
        EXPECT_TRUE(v.capacity() >= 3);
        EXPECT_TRUE(v.begin() != v.end());
        EXPECT_TRUE(v.rbegin() != v.rend());
        EXPECT_TRUE(v[0] == T());
        EXPECT_TRUE(v.at(1) == T());

        {
            // Test iterator functions
            test_iterator(const_cast<const vec&>(v).begin());
            test_iterator(const_cast<const vec&>(v).rbegin());
            test_iterator(v.begin());
            test_iterator(v.rbegin());
        }

        size_t t = 0;
        for (vec::iterator iter(v.begin()); iter != v.end(); ++iter)
        {
            *iter = T();
            ++t;
        }
        EXPECT_TRUE(t == v.size());
        
        t = 0;
        for (vec::reverse_iterator iter(v.rbegin()); iter != v.rend(); ++iter)
        {
            // test iterator functions
            *iter = T();
            ++t;
        }
        EXPECT_TRUE(t == v.size());

        {
            vec v2(v.begin(), v.end());
            EXPECT_TRUE(v2.size() == v.size());
            if (THOR_SUPPRESS_WARNING(T_PREALLOC) == 0)
            {
                EXPECT_TRUE(v2.capacity() == v.size());
            }
            else
            {
                EXPECT_TRUE(v2.capacity() == T_PREALLOC);
            }
        }

        v = vec();
        EXPECT_TRUE(v.size() == 0);
        EXPECT_TRUE(v.capacity() >= 0);

        v.reserve(100);
        EXPECT_TRUE(v.size() == 0);
        EXPECT_TRUE(v.capacity() == 100);

        v.resize(150);
        EXPECT_TRUE(v.size() == 150);
        EXPECT_TRUE(v.capacity() >= 150);
        EXPECT_TRUE(v.front() == T());
        EXPECT_TRUE(v.back() == T());

        v.resize(5, T());
        EXPECT_TRUE(v.size() == 5);

        v.assign(3U, T());
        EXPECT_TRUE(v.size() == 3);
    }
    {
        for (thor::size_type i = 0; i != 100; ++i)
        {
            vec v(i);
            EXPECT_TRUE(v.size() == i);
            EXPECT_TRUE(v.capacity() == T_PREALLOC || v.capacity() == i);

            vec v2(i, T());
            EXPECT_TRUE(v2.size() == i);
            EXPECT_TRUE(v.capacity() == T_PREALLOC || v.capacity() == i);

            vec v3(v);
            EXPECT_TRUE(v3.size() == v.size());
            EXPECT_TRUE(v.capacity() == T_PREALLOC || v.capacity() == i);
        }
    }
    {
        const vec v(3);
        EXPECT_TRUE(!v.empty());
        EXPECT_TRUE(v.begin() != v.end());
        EXPECT_TRUE(v.rbegin() != v.rend());
        EXPECT_TRUE(v.front() == T());
        EXPECT_TRUE(v.back() == T());
        vec v1(v.begin(), v.end());
        vec v2(v.rbegin(), v.rend());
        v1.swap(v2);

        v1.assign(v.begin(), v.end());
        EXPECT_TRUE(v1.size() == v.size());

        v1.insert(v1.end(), T());
        EXPECT_TRUE(v1.size() == v.size() + 1);
        v1.insert(v1.end(), v.begin(), v.end());
        EXPECT_TRUE(v1.size() == 2 * v.size() + 1);
        v1.insert(v1.end(), vec::size_type(2), T());
        EXPECT_TRUE(v1.size() == 2 * v.size() + 3);
        v1.erase(v1.begin());
        EXPECT_TRUE(v1.size() == 2 * v.size() + 2);
        v1.erase(v1.begin(), v1.end());
        EXPECT_TRUE(v1.size() == 0);

        v2.clear();
        EXPECT_TRUE(v2.size() == 0);
    }

    {
        //Extension tests
        vec v;
        v.push_back();
        v.push_back();
        v.reduce(1);
        EXPECT_TRUE(v.size() == 2);
        v.pop_back();
        v.reduce(1);
        EXPECT_TRUE(v.size() == 1);
        EXPECT_TRUE(v.capacity() == 1 || v.capacity() == T_PREALLOC);

        v.clear();
        EXPECT_TRUE(v.size() == 0);
        EXPECT_TRUE(v.capacity() == 1 || v.capacity() == T_PREALLOC);
        v.reduce();
        EXPECT_TRUE(v.capacity() == 0);
    }
    return true;
}

template <unsigned T_PREALLOC> bool test_extensions()
{
    {
        // Test pointer extensions
        typedef thor::vector<s*, T_PREALLOC> vec;
        vec v;
        v.push_back(new s);
        v.delete_all();
        EXPECT_TRUE(v.size() == 0);
        v.push_back(new s);
        v.erase_and_delete(v.begin());
        EXPECT_TRUE(v.size() == 0);
    }

    {
        typedef thor::vector<s, T_PREALLOC> vec;
        vec v;
        v.push_back();

        // Test various push_back derivatives:
        v.push_back(0);
        v.push_back(0, 0.f);
        v.push_back(0, 0.f, 0.0);
        v.push_back(0, 0.f, 0.0, 0);
        new (v.push_back_placement()) s(0, 0.f, 0.0, 0, '\0');

        // Test various insert derivatives:
        v.insert(v.begin());
        v.insert(v.begin(), 0);
        v.insert(v.begin(), 0, 0.f);
        v.insert(v.begin(), 0, 0.f, 0.0);
        v.insert(v.begin(), 0, 0.f, 0.0, 0);
        new (v.insert_placement(v.begin())) s(0, 0.f, 0.0, 0, '\0');

        // Test iterator::operator ->
        v.begin()->foo();
        const_cast<const vec&>(v).begin()->foo();
    }

    {
        typedef thor::vector<int, T_PREALLOC> vec;
        vec v;
        v.push_back(0);
        v.push_back(1);
        v.push_back(2);
        EXPECT_TRUE(v.size() == 3);
        v.swap_and_pop(0);
        EXPECT_TRUE(v.size() == 2);
        EXPECT_TRUE(v[0] == 2);
        EXPECT_TRUE(v[1] == 1);

        // Make sure these compile
        vec::const_iterator iter = v.begin();
        iter = const_cast<const vec&>(v).begin();
    }

    return true;
}

TEST(test_vector, test_vector)
{
    test_vector<int, 0>();
    test_vector<s, 0>();
    test_vector<int, 5>();
    test_vector<aligntest, 0>();
    test_vector<aligntest, 5>();
    test_extensions<0>();
    test_extensions<5>();

    int a[] = { 1, 2, 3, 4, 5, 6 };
    thor::vector<int, 3> v1(a, &a[2]);
    test_inheritance(v1);
    v1.clear();
    v1.reduce(1);
    EXPECT_TRUE(v1.capacity() == 3);
    v1.assign(a, &a[2]);

    thor::vector<int> v2(&a[2], &a[4]);
    thor::vector<int, 10> v3(&a[4], &a[6]);

    // Test out swap functionality with preallocated containers
    v2.swap(v3);
    EXPECT_TRUE(v2[0] == 5);
    EXPECT_TRUE(v2[1] == 6);
    EXPECT_TRUE(v3[0] == 3);
    EXPECT_TRUE(v3[1] == 4);
    v3.swap(v2);
    EXPECT_TRUE(v2[0] == 3);
    EXPECT_TRUE(v2[1] == 4);
    EXPECT_TRUE(v3[0] == 5);
    EXPECT_TRUE(v3[1] == 6);
    v3.swap(v1);
    EXPECT_TRUE(v1[0] == 5);
    EXPECT_TRUE(v1[1] == 6);
    EXPECT_TRUE(v3[0] == 1);
    EXPECT_TRUE(v3[1] == 2);

    EXPECT_TRUE( v2 != v1 );
    v1 = v2;
    EXPECT_TRUE( v1 == v2 );
    EXPECT_TRUE( v1 <= v2 );
    EXPECT_TRUE( v1 >= v2 );
    EXPECT_TRUE( v1 > v3 );
    EXPECT_TRUE( v3 < v1 );
}