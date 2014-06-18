#include "test_common.h"
#include "../embedded_multimap.h"
#include "../basic_string.h"

using namespace thor;

template<class T> struct deleter
{
    void operator () (T* obj)
    {
        delete obj;
    }
};

template <class Key, class T, embedded_multimap_link<Key, T> T::*LINK> void test_emmap()
{
    typedef embedded_multimap<Key, T, LINK> map;

    {
        map m;
        EXPECT_TRUE(m.empty());
        EXPECT_EQ(0, m.size());
        EXPECT_TRUE(m.begin() == m.end());

        T* entry = new T;
        m.insert("Hello", entry);
        T& entry2 = *m.insert("Hello2", new T);
        T& entry2_2 = *m.insert("Hello2", new T);
        T& entry3 = *m.insert("Hello3", new T);
        THOR_UNUSED(entry2);
        THOR_UNUSED(entry2_2);
        THOR_UNUSED(entry3);

        int count = 0;
        for (map::iterator i = m.begin(); i != m.end(); ++i)
            ++count;
        EXPECT_EQ(4, count);

        count = 0;
        for (map::reverse_iterator i = m.rbegin(); i != m.rend(); ++i)
            ++count;
        EXPECT_EQ(4, count);

        count = 0;
        for (map::const_iterator i = m.begin(); i != m.end(); ++i)
            ++count;
        EXPECT_EQ(4, count);

        count = 0;
        for (map::const_reverse_iterator i = m.rbegin(); i != m.rend(); ++i)
            ++count;
        EXPECT_EQ(4, count);

        count = 0;
        for (map::const_iterator i = m.cbegin(); i != m.cend(); ++i)
            ++count;
        EXPECT_EQ(4, count);

        count = 0;
        for (map::const_reverse_iterator i = m.crbegin(); i != m.crend(); ++i)
            ++count;
        EXPECT_EQ(4, count);

        EXPECT_TRUE(entry->link.is_contained());
        EXPECT_STREQ("Hello", entry->link.key().c_str());
        m.remove(entry);
        EXPECT_FALSE(entry->link.is_contained());
        delete entry;
        entry = 0;
        EXPECT_EQ(3, m.size());

        m.insert("Hello", entry = new T);
        EXPECT_EQ(4, m.size());
        m.remove(m.begin());
        EXPECT_EQ(3, m.size());
        m.insert("Hello", entry);
        EXPECT_EQ(4, m.size());
        m.remove(entry, deleter<T>());
        EXPECT_EQ(3, m.size());
        m.insert("Hello", entry = new T);
        EXPECT_EQ(4, m.size());
        m.remove_delete(entry);
        EXPECT_EQ(3, m.size());
        m.insert("Hello", entry = new T);
        EXPECT_EQ(4, m.size());
        m.remove_delete(m.begin());
        EXPECT_EQ(3, m.size());
        m.insert("Hello", entry = new T);
        EXPECT_EQ(4, m.size());
        EXPECT_EQ(1, m.remove("Hello"));
        EXPECT_EQ(3, m.size());
        m.insert("Hello", entry);
        thor::pair<map::iterator, map::iterator> keyrange = m.equal_range("Hello");
        m.remove(keyrange.first, keyrange.second);
        EXPECT_EQ(3, m.size());
        m.insert("Hello", entry);
        string names[] = { "Hello", "HelloWorld" };
        m.remove(&names[0], &names[2]);
        EXPECT_EQ(3, m.size());
        m.insert("Hello", entry);
        m.remove_delete("Hello");
        EXPECT_EQ(3, m.size());
        m.insert("Hello", entry = new T);
        keyrange = m.equal_range("Hello");
        EXPECT_EQ(1, m.remove_delete(keyrange.first, keyrange.second));
        EXPECT_EQ(3, m.size());
        m.insert("Hello", entry = new T);
        EXPECT_EQ(1, m.remove_delete(&names[0], &names[2]));
        EXPECT_EQ(3, m.size());


        EXPECT_TRUE(m.find("Hello") == m.end());
        EXPECT_TRUE(const_cast<const map&>(m).find("Hello2") != m.end());
        EXPECT_EQ(2, m.count("Hello2"));

        map::iterator iter = m.lower_bound("Hello2");
        map::const_iterator citer = const_cast<const map&>(m).lower_bound("Hello2");
        map::iterator end = m.upper_bound("Hello2");
        map::const_iterator cend = const_cast<const map&>(m).upper_bound("Hello2");
        EXPECT_EQ(2, thor::distance(iter, end));
        EXPECT_EQ(2, thor::distance(citer, cend));
        thor::pair<map::iterator, map::iterator> range = m.equal_range("Hello2");
        thor::pair<map::const_iterator, map::const_iterator> crange = const_cast<const map&>(m).equal_range("Hello2");

        EXPECT_TRUE(range.first == iter);
        EXPECT_TRUE(range.second == end);
        EXPECT_TRUE(crange.first == citer);
        EXPECT_TRUE(crange.second == cend);

        m.delete_all();
        EXPECT_TRUE(m.empty());
        EXPECT_EQ(0, m.size());

        entry = new T;
        m.insert("Hello World", entry);
        map m2;
        m2.swap(m);
        EXPECT_EQ(0, m.size());
        EXPECT_TRUE(m.empty());
        EXPECT_EQ(1, m2.size());
        EXPECT_FALSE(m2.empty());

        m2.remove_all();
        delete entry;
        EXPECT_TRUE(m2.empty());
        EXPECT_EQ(0, m2.size());
    }
}

struct MyEmbeddedMultimapTest
{
private:
    char c[100];
public:
    embedded_multimap_link<basic_string<char>, MyEmbeddedMultimapTest> link;
};

TEST(test_emmap, initial)
{
    test_emmap<string, MyEmbeddedMultimapTest, &MyEmbeddedMultimapTest::link>();
}