#include "hash_set.h"
#include "test_common.h"

template <class Key> bool test_hash_set()
{
    bool b = true;

    typedef thor::hash_set<Key> set;

    set m;
    EXPECT_TRUE(m.begin() == m.end());
    EXPECT_TRUE(m.size() == 0);
    EXPECT_TRUE(m.empty());

    {
        thor::pair<set::iterator, bool> result = m.insert(set::value_type(Key(0)));
        EXPECT_TRUE((*result.first) == Key(0));
        // (*result.first) = Key(1); // Should fail to compile
        EXPECT_TRUE(result.second == true);
        EXPECT_TRUE(result.first++ != m.end());
        EXPECT_TRUE(result.first == m.end());
        EXPECT_TRUE(!m.empty());
        EXPECT_TRUE(m.size() == 1);
        result = m.insert(set::value_type(Key(0)));
        EXPECT_TRUE((*result.first) == Key(0));
        EXPECT_TRUE(result.second == false);
        EXPECT_TRUE(result.first != m.end());
        EXPECT_TRUE(++result.first == m.end());
        EXPECT_TRUE(!m.empty());
        EXPECT_TRUE(m.size() == 1);
    }

    {
        set::iterator iter = m.find(Key(1));
        EXPECT_TRUE(iter == m.end());
        iter = m.find(Key(0));
        EXPECT_TRUE(iter != m.end());
        EXPECT_TRUE((*iter) == Key(0));

        EXPECT_TRUE(m.count(Key(1)) == 0);
        EXPECT_TRUE(m.count(Key(0)) == 1);

        thor::pair<set::const_iterator, set::const_iterator> result = m.equal_range(Key(0));
        EXPECT_TRUE(result.first != result.second);
    }

    {
        const set k(m);
        EXPECT_TRUE(k.size() == m.size());
        EXPECT_TRUE(k.empty() == m.empty());

        m = k;

        EXPECT_TRUE(m == k);

        EXPECT_TRUE(k.count(Key(1)) == 0);
        EXPECT_TRUE(k.count(Key(0)) == 1);

        // set::iterator iter = k.find(Key(0)); // Should fail to compile

        set::const_iterator iter = k.find(Key(0));

        // (*iter) = Key(); // Should fail to compile

        thor::pair<set::const_iterator, set::const_iterator> result = k.equal_range(Key(0));
        EXPECT_TRUE(result.first != result.second);
    }

    m.clear();
    EXPECT_TRUE(m.size() == 0);
    EXPECT_TRUE(m.empty());
    EXPECT_TRUE(m.begin() == m.end());

    // Iteration tests
    for (int i = 0; i < 1000; i += 10)
    {
        m.insert(set::value_type(Key(i)));
    }

    {
        set m2;
        m2.insert(++m.begin(), --m.end());
        EXPECT_TRUE(m2.size() == (m.size() - 2));
    }

    for (int i = 0; i < 1000; i += 10)
    {
        thor::pair<set::iterator, set::iterator> e = m.equal_range(Key(i));
        EXPECT_TRUE(thor::distance(e.first, e.second) == 1);
    }

    thor::size_type size = 0;
    for (set::iterator iter(m.begin()); iter != m.end(); ++iter)
    {
        ++size;
        EXPECT_TRUE(m.count((*iter)) == 1);
    }
    EXPECT_TRUE(size == m.size());

    EXPECT_TRUE(--(++m.begin()) == m.begin());
    EXPECT_TRUE(++(--m.end()) == m.end());

    set m2;
    m2.insert(Key(-1));
    m2.insert(Key(-1));
    m2.insert(Key(-2));
    m2.insert(Key(-2));
    m2.swap(m);
    EXPECT_TRUE(m != m2);
    EXPECT_TRUE(m.size() == 2);
    EXPECT_TRUE(m2.size() == 100);

    EXPECT_TRUE(--(++m.begin()) == m.begin());
    EXPECT_TRUE(++(--m.end()) == m.end());
    EXPECT_TRUE(--(++m2.begin()) == m2.begin());
    EXPECT_TRUE(++(--m2.end()) == m2.end());

    m2.erase(++m2.begin(), --m2.end());
    while (!m2.empty())
    {
        m2.erase(m2.begin());
    }

    m.erase(Key(-1));
    EXPECT_TRUE(m.size() == 1);

    set m3;
    m.swap(m3);
    EXPECT_TRUE(m.empty());
    EXPECT_TRUE(m.begin() == m.end());
    EXPECT_TRUE(m3.size() == 1);
    EXPECT_TRUE(m3.begin() != m3.end());

    m3.erase(m3.begin(), m3.end());
    EXPECT_TRUE(m3.empty());

    return b;
}

template <class Key> bool test_hash_multiset()
{
    bool b = true;

    typedef thor::hash_multiset<Key> set;

    set m;
    EXPECT_TRUE(m.begin() == m.end());
    EXPECT_TRUE(m.size() == 0);
    EXPECT_TRUE(m.empty());

    {
        set::iterator result = m.insert(set::value_type(Key(0)));
        EXPECT_TRUE((*result) == Key(0));
        // (*result) = Key(1); // Should fail to compile
        EXPECT_TRUE(result++ != m.end());
        EXPECT_TRUE(result == m.end());
        EXPECT_TRUE(!m.empty());
        EXPECT_TRUE(m.size() == 1);
        result = m.insert(set::value_type(Key(0)));
        EXPECT_TRUE((*result) == Key(0));
        EXPECT_TRUE(result != m.end());
        EXPECT_TRUE(++result == m.end());
        EXPECT_TRUE(!m.empty());
        EXPECT_TRUE(m.size() == 2);
    }

    {
        set::iterator iter = m.find(Key(1));
        EXPECT_TRUE(iter == m.end());
        iter = m.find(Key(0));
        EXPECT_TRUE(iter != m.end());
        EXPECT_TRUE((*iter) == Key(0));

        EXPECT_TRUE(m.count(Key(1)) == 0);
        EXPECT_TRUE(m.count(Key(0)) == 2);

        thor::pair<set::const_iterator, set::const_iterator> result = m.equal_range(Key(0));
        EXPECT_TRUE(result.first != result.second);
    }

    {
        const set k(m);
        EXPECT_TRUE(k.size() == m.size());
        EXPECT_TRUE(k.empty() == m.empty());

        m = k;

        EXPECT_TRUE(m == k);

        EXPECT_TRUE(k.count(Key(1)) == 0);
        EXPECT_TRUE(k.count(Key(0)) == 2);

        // set::iterator iter = k.find(Key(0)); // Should fail to compile

        set::const_iterator iter = k.find(Key(0));

        // (*iter) = Key(); // Should fail to compile

        thor::pair<set::const_iterator, set::const_iterator> result = k.equal_range(Key(0));
        EXPECT_TRUE(result.first != result.second);
    }

    m.clear();
    EXPECT_TRUE(m.size() == 0);
    EXPECT_TRUE(m.empty());
    EXPECT_TRUE(m.begin() == m.end());

    // Iteration tests
    for (int i = 0; i < 1000; i += 10)
    {
        m.insert(set::value_type(Key(i)));
        m.insert(set::value_type(Key(i)));
    }

    {
        set m2;
        m2.insert(++m.begin(), --m.end());
        EXPECT_TRUE(m2.size() == (m.size() - 2));
    }

    for (int i = 0; i < 1000; i += 10)
    {
        thor::pair<set::iterator, set::iterator> e = m.equal_range(Key(i));
        EXPECT_TRUE(thor::distance(e.first, e.second) == 2);
    }

    thor::size_type size = 0;
    for (set::iterator iter(m.begin()); iter != m.end(); ++iter)
    {
        ++size;
        EXPECT_TRUE(m.count((*iter)) == 2);
    }
    EXPECT_TRUE(size == m.size());

    EXPECT_TRUE(--(++m.begin()) == m.begin());
    EXPECT_TRUE(++(--m.end()) == m.end());

    set m2;
    m2.insert(set::value_type(Key(-1)));
    m2.insert(set::value_type(Key(-1)));
    m2.insert(set::value_type(Key(-2)));
    m2.insert(set::value_type(Key(-2)));
    m2.swap(m);
    EXPECT_TRUE(m != m2);
    EXPECT_TRUE(m.size() == 4);
    EXPECT_TRUE(m2.size() == 200);

    EXPECT_TRUE(--(++m.begin()) == m.begin());
    EXPECT_TRUE(++(--m.end()) == m.end());
    EXPECT_TRUE(--(++m2.begin()) == m2.begin());
    EXPECT_TRUE(++(--m2.end()) == m2.end());

    m2.erase(++m2.begin(), --m2.end());
    while (!m2.empty())
    {
        m2.erase(m2.begin());
    }

    m.erase(Key(-1));
    EXPECT_TRUE(m.size() == 2);

    set m3;
    m.swap(m3);
    EXPECT_TRUE(m.empty());
    EXPECT_TRUE(m.begin() == m.end());
    m3.erase(m3.begin(), m3.end());
    EXPECT_TRUE(m3.empty());

    return b;
}

TEST(test_hashset, test_hashset)
{
    test_hash_set<int>();
    test_hash_set<s>();
    test_hash_set<aligntest>();

    test_hash_multiset<int>();
    test_hash_multiset<s>();
    test_hash_multiset<aligntest>();

    // Test hash early-out
    {
        THOR_COMPILETIME_ASSERT(sizeof(long long) == 8, InvalidSize);
        typedef thor::hash_multiset<long long> set;
        set m;
        set::iterator i;

        i = m.insert(0x800000001);
        i = m.insert(0x10000009);
        i = m.insert(0x100000008);
        i = m.insert(0x9);
        i = m.insert(0x900000000);
        // Test move
        EXPECT_TRUE((*--m.end()) == 0x900000000);
        m.move(i, m.begin());
        EXPECT_TRUE((*m.begin()) == 0x900000000);
        i = m.find(0x800000001);
        i = m.find(0x10000009);
        i = m.find(0x10000009);
        i = m.find(0x100000008);
        i = m.find(0x9);
        i = m.find(0x900000000);
        m.erase(0x100000008);
        EXPECT_TRUE(m.end() == m.find(0x100000008));
        m.erase(0x10000009);
        EXPECT_TRUE(m.end() == m.find(0x10000009));
        m.erase(0x9);
        EXPECT_TRUE(m.end() == m.find(0x9));
        m.erase(0x900000000);
        EXPECT_TRUE(m.end() == m.find(0x900000000));
        m.erase(0x800000001);
        EXPECT_TRUE(m.end() == m.find(0x800000001));
        EXPECT_TRUE(m.empty());
    }
}