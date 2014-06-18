#include "map.h"
#include "test_common.h"

template <class Key, class Value> bool test_map()
{
    bool b = true;

    typedef thor::map<Key, Value> map;

    map m;
    EXPECT_TRUE(m.begin() == m.end());
    EXPECT_TRUE(m.rbegin() == m.rend());
    EXPECT_TRUE(m.size() == 0);
    EXPECT_TRUE(m.empty());

    {
        thor::pair<map::iterator, bool> result = m.insert(map::value_type(Key(0), Value()));
        EXPECT_TRUE((*result.first).first == Key(0));
        EXPECT_TRUE((*result.first).second == Value());
        // (*result.first).first = Key(1); // Should fail to compile
        (*result.first).second = Value();
        EXPECT_TRUE(result.second == true);
        EXPECT_TRUE(result.first++ != m.end());
        EXPECT_TRUE(result.first == m.end());
        EXPECT_TRUE(!m.empty());
        EXPECT_TRUE(m.size() == 1);
        result = m.insert(map::value_type(Key(0), Value()));
        EXPECT_TRUE((*result.first).first == Key(0));
        EXPECT_TRUE((*result.first).second == Value());
        EXPECT_TRUE(result.second == false);
        EXPECT_TRUE(result.first != m.end());
        EXPECT_TRUE(++result.first == m.end());
        EXPECT_TRUE(!m.empty());
        EXPECT_TRUE(m.size() == 1);
    }

    {
        map::iterator iter = m.find(Key(1));
        EXPECT_TRUE(iter == m.end());
        iter = m.find(Key(0));
        EXPECT_TRUE(iter != m.end());
        EXPECT_TRUE((*iter).first == Key(0));
        EXPECT_TRUE((*iter).second == Value());

        EXPECT_TRUE(m.count(Key(1)) == 0);
        EXPECT_TRUE(m.count(Key(0)) == 1);

        iter = m.lower_bound(Key(1));
        EXPECT_TRUE(iter == m.end());
        iter = m.upper_bound(Key(0));
        EXPECT_TRUE(iter == m.end());
        thor::pair<map::const_iterator, map::const_iterator> result = m.equal_range(Key(0));
        EXPECT_TRUE(result.first != result.second);
    }

    {
        // Test constructors
        map::key_compare comp;
        const map M1(comp);
        const map M2(m.begin(), m.end(), comp);
    }

    {
        const map k(m);
        EXPECT_TRUE(k.size() == m.size());
        EXPECT_TRUE(k.empty() == m.empty());

        m = k;

        EXPECT_TRUE(m == k);
        
        EXPECT_TRUE(k.count(Key(1)) == 0);
        EXPECT_TRUE(k.count(Key(0)) == 1);
        
        // map::iterator iter = k.find(Key(0)); // Should fail to compile
        
        map::const_iterator iter = k.find(Key(0));
        
        // (*iter).second = Value(); // Should fail to compile
        
        iter = k.lower_bound(Key(1));
        EXPECT_TRUE(iter == k.end());
        iter = k.upper_bound(Key(0));
        EXPECT_TRUE(iter == k.end());
        thor::pair<map::const_iterator, map::const_iterator> result = k.equal_range(Key(0));
        EXPECT_TRUE(result.first != result.second);
    }

    m.clear();
    EXPECT_TRUE(m.size() == 0);
    EXPECT_TRUE(m.empty());
    EXPECT_TRUE(m.begin() == m.end());
    EXPECT_TRUE(m.rbegin() == m.rend());

    // Iteration tests
    for (int i = 0; i < 1000; i += 10)
    {
        m.insert(map::value_type(Key(i), Value()));
    }

    {
        map m2;
        m2.insert_range(++m.begin(), --m.end());
        EXPECT_TRUE(m2.size() == (m.size() - 2));
    }

    for (int i = 0; i < 1000; ++i)
    {
        map::iterator l(m.lower_bound(Key(i)));
        if (l == m.end())
        {
            EXPECT_TRUE((*--m.end()).first < i);
        }
        else
        {
            EXPECT_TRUE(!((*l).first < Key(i)));
        }
        map::iterator u(m.upper_bound(Key(i)));
        if (u == m.end())
        {
            EXPECT_TRUE(!(Key(i) < (*--m.end()).first));
        }
        else
        {
            EXPECT_TRUE(Key(i) < (*u).first);
        }
        thor::pair<map::iterator, map::iterator> e = m.equal_range(Key(i));
        EXPECT_TRUE(e.first == l);
        EXPECT_TRUE(e.second == u);
    }

    thor::size_type size = 0;
    Key last = Key();
    for (map::iterator iter(m.begin()); iter != m.end(); ++iter)
    {
        ++size;
        EXPECT_TRUE(m.count((*iter).first) == 1);
        EXPECT_TRUE(iter == m.begin() || last < (*iter).first);
        last = (*iter).first;
    }
    EXPECT_TRUE(size == m.size());

    size = 0;
    for (map::reverse_iterator iter(m.rbegin()); iter != m.rend(); ++iter)
    {
        ++size;
        EXPECT_TRUE(iter == m.rbegin() || (*iter).first < last);
        last = (*iter).first;
    }
    EXPECT_TRUE(size == m.size());

    EXPECT_TRUE(--(++m.begin()) == m.begin());
    EXPECT_TRUE(++(--m.end()) == m.end());

    map m2;
    m2[Key(-1)] = Value();
    m2[Key(-1)] = Value();
    m2[Key(-2)] = Value();
    m2[Key(-2)] = Value();
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

    map m3;
    m.swap(m3);
    EXPECT_TRUE(m.empty());
    EXPECT_TRUE(m.begin() == m.end());
    EXPECT_TRUE(m3.size() == 1);
    EXPECT_TRUE(m3.begin() != m3.end());

    m3.erase(m3.begin(), m3.end());
    EXPECT_TRUE(m3.empty());

    return b;
}

template <class Key, class Value> bool test_multimap()
{
    bool b = true;

    typedef thor::multimap<Key, Value> map;

    map m;
    EXPECT_TRUE(m.begin() == m.end());
    EXPECT_TRUE(m.rbegin() == m.rend());
    EXPECT_TRUE(m.size() == 0);
    EXPECT_TRUE(m.empty());

    {
        map::iterator result = m.insert(map::value_type(Key(0), Value()));
        EXPECT_TRUE((*result).first == Key(0));
        EXPECT_TRUE((*result).second == Value());
        // (*result).first = Key(1); // Should fail to compile
        (*result).second = Value();
        EXPECT_TRUE(result++ != m.end());
        EXPECT_TRUE(result == m.end());
        EXPECT_TRUE(!m.empty());
        EXPECT_TRUE(m.size() == 1);
        result = m.insert(map::value_type(Key(0), Value()));
        EXPECT_TRUE((*result).first == Key(0));
        EXPECT_TRUE((*result).second == Value());
        EXPECT_TRUE(result != m.end());
        EXPECT_TRUE(++result == m.end());
        EXPECT_TRUE(!m.empty());
        EXPECT_TRUE(m.size() == 2);
    }

    {
        map::iterator iter = m.find(Key(1));
        EXPECT_TRUE(iter == m.end());
        iter = m.find(Key(0));
        EXPECT_TRUE(iter != m.end());
        EXPECT_TRUE((*iter).first == Key(0));
        EXPECT_TRUE((*iter).second == Value());

        EXPECT_TRUE(m.count(Key(1)) == 0);
        EXPECT_TRUE(m.count(Key(0)) == 2);

        iter = m.lower_bound(Key(1));
        EXPECT_TRUE(iter == m.end());
        iter = m.upper_bound(Key(0));
        EXPECT_TRUE(iter == m.end());
        thor::pair<map::const_iterator, map::const_iterator> result = m.equal_range(Key(0));
        EXPECT_TRUE(result.first != result.second);
    }

    {
        // Test constructors
        map::key_compare comp;
        const map M1(comp);
        const map M2(m.begin(), m.end(), comp);
    }

    {
        const map k(m);
        EXPECT_TRUE(k.size() == m.size());
        EXPECT_TRUE(k.empty() == m.empty());

        m = k;

        EXPECT_TRUE(m == k);

        EXPECT_TRUE(k.count(Key(1)) == 0);
        EXPECT_TRUE(k.count(Key(0)) == 2);

        // map::iterator iter = k.find(Key(0)); // Should fail to compile

        map::const_iterator iter = k.find(Key(0));

        // (*iter).second = Value(); // Should fail to compile

        iter = k.lower_bound(Key(1));
        EXPECT_TRUE(iter == k.end());
        iter = k.upper_bound(Key(0));
        EXPECT_TRUE(iter == k.end());
        thor::pair<map::const_iterator, map::const_iterator> result = k.equal_range(Key(0));
        EXPECT_TRUE(result.first != result.second);
    }

    m.clear();
    EXPECT_TRUE(m.size() == 0);
    EXPECT_TRUE(m.empty());
    EXPECT_TRUE(m.begin() == m.end());
    EXPECT_TRUE(m.rbegin() == m.rend());

    // Iteration tests
    for (int i = 0; i < 1000; i += 10)
    {
        m.insert(map::value_type(Key(i), Value()));
        m.insert(map::value_type(Key(i), Value()));
    }

    {
        map m2;
        m2.insert_range(++m.begin(), --m.end());
        EXPECT_TRUE(m2.size() == (m.size() - 2));
    }

    for (int i = 0; i < 1000; ++i)
    {
        map::iterator l(m.lower_bound(Key(i)));
        if (l == m.end())
        {
            EXPECT_TRUE((*--m.end()).first < i);
        }
        else
        {
            EXPECT_TRUE(!((*l).first < Key(i)));
        }
        map::iterator u(m.upper_bound(Key(i)));
        if (u == m.end())
        {
            EXPECT_TRUE(!(Key(i) < (*--m.end()).first));
        }
        else
        {
            EXPECT_TRUE(Key(i) < (*u).first);
        }
        thor::pair<map::iterator, map::iterator> e = m.equal_range(Key(i));
        EXPECT_TRUE(e.first == l);
        EXPECT_TRUE(e.second == u);
    }

    thor::size_type size = 0;
    Key last = Key();
    for (map::iterator iter(m.begin()); iter != m.end(); ++iter)
    {
        ++size;
        EXPECT_TRUE(m.count((*iter).first) == 2);
        EXPECT_TRUE(iter == m.begin() || last <= (*iter).first);
        last = (*iter).first;
    }
    EXPECT_TRUE(size == m.size());

    size = 0;
    for (map::reverse_iterator iter(m.rbegin()); iter != m.rend(); ++iter)
    {
        ++size;
        EXPECT_TRUE(iter == m.rbegin() || (*iter).first <= last);
        last = (*iter).first;
    }
    EXPECT_TRUE(size == m.size());

    EXPECT_TRUE(--(++m.begin()) == m.begin());
    EXPECT_TRUE(++(--m.end()) == m.end());

    map m2;
    m2.insert(map::value_type(Key(-1)));
    m2.insert(map::value_type(Key(-1)));
    m2.insert(map::value_type(Key(-2)));
    m2.insert(map::value_type(Key(-2)));
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

    map m3;
    m.swap(m3);
    EXPECT_TRUE(m.empty());
    EXPECT_TRUE(m.begin() == m.end());
    m3.erase(m3.begin(), m3.end());
    EXPECT_TRUE(m3.empty());

    return b;
}

TEST(test_map, test_map)
{
    test_map<int, int>();
    test_map<int, s>();
    test_map<int, aligntest>();
    // test_map<int, nocopy>(); // Link will fail

    // Test insert extensions
    {
        typedef thor::map<int, nocopy> map;
        map m;
        map::iterator i;

        i = m.insert(-1);
        i = m.insert(-1);
        EXPECT_TRUE((*i).first == -1);
        EXPECT_TRUE((*i).second.params == 0);
        i = m.insert(1, 1);
        i = m.insert(1, 1);
        EXPECT_TRUE((*i).first == 1);
        EXPECT_TRUE((*i).second.params == 1);
        i = m.insert(2, 1, 2.f);
        i = m.insert(2, 1, 2.f);
        EXPECT_TRUE((*i).first == 2);
        EXPECT_TRUE((*i).second.params == 2);
        i = m.insert(3, 1, 2.f, 3.0);
        i = m.insert(3, 1, 2.f, 3.0);
        EXPECT_TRUE((*i).first == 3);
        EXPECT_TRUE((*i).second.params == 3);
        i = m.insert(4, 1, 2.f, 3.0, 4);
        i = m.insert(4, 1, 2.f, 3.0, 4);
        EXPECT_TRUE((*i).first == 4);
        EXPECT_TRUE((*i).second.params == 4);
        map::data_type *p = new (m.insert_placement(5)) map::data_type(1, 2.f, 3.0, 4, '5');
        p = new (m.insert_placement(5)) map::data_type(1, 2.f, 3.0, 4, '5');
        EXPECT_TRUE(p->params == 5);
        EXPECT_TRUE(m.size() == 6);
    }

    test_multimap<int, int>();
    test_multimap<int, s>();
    test_multimap<int, aligntest>();
    // test_multimap<int, nocopy>(); // Link will fail

    // Test insert extensions
    {
        typedef thor::multimap<int, nocopy> map;
        map m;
        map::iterator i;

        i = m.insert(-1);
        i = m.insert(-1);
        EXPECT_TRUE((*i).first == -1);
        EXPECT_TRUE((*i).second.params == 0);
        i = m.insert(1, 1);
        i = m.insert(1, 1);
        EXPECT_TRUE((*i).first == 1);
        EXPECT_TRUE((*i).second.params == 1);
        i = m.insert(2, 1, 2.f);
        i = m.insert(2, 1, 2.f);
        EXPECT_TRUE((*i).first == 2);
        EXPECT_TRUE((*i).second.params == 2);
        i = m.insert(3, 1, 2.f, 3.0);
        i = m.insert(3, 1, 2.f, 3.0);
        EXPECT_TRUE((*i).first == 3);
        EXPECT_TRUE((*i).second.params == 3);
        i = m.insert(4, 1, 2.f, 3.0, 4);
        i = m.insert(4, 1, 2.f, 3.0, 4);
        EXPECT_TRUE((*i).first == 4);
        EXPECT_TRUE((*i).second.params == 4);
        map::data_type *p = new (m.insert_placement(5)) map::data_type(1, 2.f, 3.0, 4, '5');
        p = new (m.insert_placement(5)) map::data_type(1, 2.f, 3.0, 4, '5');
        EXPECT_TRUE(p->params == 5);
        EXPECT_TRUE(m.size() == 12);
    }
}