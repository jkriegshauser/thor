#include "test_common.h"
#include "../embedded_hash_multimap.h"

#include "../basic_string.h"

using namespace thor;

template <class Key, class T, embedded_hash_multimap_link<Key, T> T::*LINK, class T_PARTITION_POLICY> void test_ehmmap()
{
    typedef embedded_hash_multimap<Key, T, LINK, thor::hash<Key>, T_PARTITION_POLICY> map;

    {
        map m;
        EXPECT_TRUE(m.empty());
        EXPECT_TRUE(m.size() == 0);
        EXPECT_TRUE(m.begin() == m.end());

        T* entry = new T;
        m.insert("Hello", entry);
        EXPECT_TRUE(m.find("Hello").m_node == entry);
        m.delete_all();
    }
}

struct MyMultimapTest
{
private:
    char c[100];
public:
    embedded_hash_multimap_link<thor::basic_string<char>, MyMultimapTest> link;
};

TEST(test_ehmmap, test_ehmmap)
{
    test_ehmmap<string, MyMultimapTest, &MyMultimapTest::link, thor::policy::base2_partition>();
    test_ehmmap<string, MyMultimapTest, &MyMultimapTest::link, thor::policy::prime_number_partition>();
}