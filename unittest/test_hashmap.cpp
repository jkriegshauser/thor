#include "hash_map.h"
#include "test_common.h"
#include <time.h>

template <class Key, class Value> bool test_hash_map()
{
    bool b = true;

    typedef thor::hash_map<Key, Value> map;

    map m;
    EXPECT_TRUE(m.begin() == m.end());
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

        thor::pair<map::const_iterator, map::const_iterator> result = m.equal_range(Key(0));
        EXPECT_TRUE(result.first != result.second);
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

        thor::pair<map::const_iterator, map::const_iterator> result = k.equal_range(Key(0));
        EXPECT_TRUE(result.first != result.second);
    }

    m.clear();
    EXPECT_TRUE(m.size() == 0);
    EXPECT_TRUE(m.empty());
    EXPECT_TRUE(m.begin() == m.end());

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
        thor::pair<map::iterator, map::iterator> e = m.equal_range(Key(i));
        EXPECT_TRUE(thor::distance(e.first, e.second) == (thor_diff_type)m.count(Key(i)));
    }

    thor::size_type size = 0;
    for (map::iterator iter(m.begin()); iter != m.end(); ++iter)
    {
        ++size;
        EXPECT_TRUE(m.count((*iter).first) == 1);
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
    // EXPECT_TRUE(m != m2);
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

template <class Key, class Value> bool test_hash_multimap()
{
    bool b = true;

    typedef thor::hash_multimap<Key, Value> map;

    map m;
    EXPECT_TRUE(m.begin() == m.end());
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

        thor::pair<map::const_iterator, map::const_iterator> result = m.equal_range(Key(0));
        EXPECT_TRUE(result.first != result.second);
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

        thor::pair<map::const_iterator, map::const_iterator> result = k.equal_range(Key(0));
        EXPECT_TRUE(result.first != result.second);
    }

    m.clear();
    EXPECT_TRUE(m.size() == 0);
    EXPECT_TRUE(m.empty());
    EXPECT_TRUE(m.begin() == m.end());

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
        thor::pair<map::iterator, map::iterator> e = m.equal_range(Key(i));
        EXPECT_TRUE(distance(e.first, e.second) == (thor_diff_type)m.count(Key(i)));
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

    EXPECT_TRUE(--(++m.begin()) == m.begin());
    EXPECT_TRUE(++(--m.end()) == m.end());

    map m2;
    m2.insert(map::value_type(Key(-1)));
    m2.insert(map::value_type(Key(-1)));
    m2.insert(map::value_type(Key(-2)));
    m2.insert(map::value_type(Key(-2)));
    m2.swap(m);
    // EXPECT_TRUE(m != m2);
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

template <class T> thor::size_type get_hash(const T& t)
{
    return thor::hash<T>() (t);
}

struct UniqueNumHasher
{
    thor::size_type operator () (const unsigned& p) const { return p; }
};

typedef thor::hash_map<unsigned, unsigned char, UniqueNumHasher> PlayerGroupMap;

struct Match
{
    unsigned someinfo;

    PlayerGroupMap players;
    PlayerGroupMap groups;

    class PlayerCountSort
    {
    public:
        bool operator() ( const Match ma, const Match mb ) const
        {
            return ma.players.size() >  mb.players.size();
        }
    };
};

TEST(test_hashmap, test_hashmap)
{
    srand((unsigned int)time(0));
#if 0
    for (int i = 0; i < 100; ++i)
    {
        thor::vector<Match> matchvec;
        int matchcount = (rand() % 10) + 20; // between 20 - 30
        for (int j = 0; j < matchcount; ++j)
        {
            Match m;
            int playercount = (rand() % 18) + 2; // between 2 and 20
            for (int k = 0; k < playercount; ++k)
            {
                m.players.insert(rand(), (unsigned char)(rand() % 2));
            }
            matchvec.push_back(m);
        }

        thor::sort(matchvec.begin(), matchvec.end(), Match::PlayerCountSort());
    }
#endif

    // Test out hash functions
    int* p = new int(30);
    const int* p2 = new int(40);
    const int* const p3 = new int(50);
    EXPECT_TRUE(get_hash(p) != ((thor_size_type)p));
    EXPECT_TRUE(get_hash(p2) != ((thor_size_type)p2));
    EXPECT_TRUE(get_hash(p3) != ((thor_size_type)p3));
    delete p;
    delete p2;
    delete p3;

    EXPECT_TRUE(get_hash(30) == 30);
    const char* str = "asdfasdf";
    EXPECT_TRUE(get_hash(str) != 0);
    const wchar_t* wstr = L"asdfasdf";
    EXPECT_TRUE(get_hash(wstr) != 0);

    test_hash_map<int, int>();
    test_hash_map<int, s>();
    test_hash_map<int, aligntest>();
    // test_hash_map<int, nocopy>(); // Link will fail

    // Test insert extensions
    {
        typedef thor::hash_map<int, nocopy> map;
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

    test_hash_multimap<int, int>();
    test_hash_multimap<int, s>();
    test_hash_multimap<int, aligntest>();
    // test_multimap<int, nocopy>(); // Link will fail

    // Test insert extensions
    {
        typedef thor::hash_multimap<int, nocopy> map;
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

    // Test hash early-out
    {
        THOR_COMPILETIME_ASSERT(sizeof(long long) == 8, InvalidSize);
        typedef thor::hash_map<long long, nocopy> map;
        map m;
        map::iterator i;

        i = m.insert(0x800000001);
        i = m.insert(0x10000009);
        i = m.insert(0x100000008);
        i = m.insert(0x9);
        i = m.insert(0x900000000);
        // Test move
        EXPECT_TRUE((*--m.end()).first == 0x900000000);
        m.move(i, m.begin());
        EXPECT_TRUE((*m.begin()).first == 0x900000000);
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