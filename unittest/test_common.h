#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include "gtest/gtest.h"
#include "basetypes.h"
#include "hash_funcs.h"

#include <assert.h>

struct Constructed
{
    bool constructed;
    bool destructed;
    Constructed() : constructed(true), destructed(false) {}
    ~Constructed() { assert(constructed); destructed = true; }
};

struct s
{
    Constructed c;
    int params;
    int* test;

    thor_byte __unused[64]; // Just make the class much larger.

    s() { params = 0; test = new int(0); };
    s(int i) { params = 1; test = new int(i); };
    s(int i, float) { params = 2; test = new int(i); };
    s(int i, float, double) { params = 3; test = new int(i); };
    s(int i, float, double, long) { params = 4; test = new int(i); };
    s(int i, float, double, long, char) { params = 5; test = new int(i); };
    s(const s& rhs) : c(), params(rhs.params), test(new int(*rhs.test)) {}
    virtual ~s() { delete test; }

    s& operator = (const s& rhs)
    {
        params = rhs.params;
        delete test;
        test = new int(*rhs.test);
        return *this;
    }

    bool operator < (const s& S) const
    {
        return *test < *S.test;
    }

    void foo() const { ASSERT_TRUE(c.constructed && !c.destructed); };
};

inline bool operator == ( const s& lhs, const s& rhs )
{
    return *lhs.test == *rhs.test;
}

struct nocopy : public s
{
    // Not implemented; link will fail if these are called
    nocopy(const nocopy&);
    nocopy& operator = (const nocopy&);

    nocopy() : s() {}
    nocopy(int i) : s(i) {}
    nocopy(int i, float f) : s(i, f) {}
    nocopy(int i, float f, double d) : s(i, f, d) {}
    nocopy(int i, float f, double d, long l) : s(i, f, d, l) {}
    nocopy(int i, float f, double d, long l, char c) : s(i, f, d, l, c) {}
};

__declspec(align(32)) struct aligntest
{
    aligntest()
    {
        padding[0] = 0;
        assert(((size_t)this % 32) == 0);
        THOR_COMPILETIME_ASSERT(sizeof(aligntest) == 32, SizeError);
    }
    aligntest(int i)
    {
        padding[0] = i;
        assert(((size_t)this % 32) == 0);
        THOR_COMPILETIME_ASSERT(sizeof(aligntest) == 32, SizeError);
    }

    bool operator < (const aligntest& rhs) const
    {
        return padding[0] < rhs.padding[0];
    }

    int padding[8];
};

inline bool operator == ( const aligntest& lhs, const aligntest& rhs )
{
    return lhs.padding[0] == rhs.padding[0];
}

// Hash functions
namespace thor
{
    template<> struct hash<s>
    {
        size_type operator ()(const s& S) const
        {
            return size_type(*S.test);
        }
    };

    template<> struct hash<aligntest>
    {
        size_type operator ()(const aligntest& A) const
        {
            return size_type(A.padding[0]);
        }
    };
}

template <class T> class NoValidate
{
public:
    bool operator () (T&)
    {
        return true;
    }
};

template <class Container, class Validator> bool test_pointer_types()
{
    // Test pointer extensions
    bool b = true;
    
    Validator v;
    Container c;
    c.push_back((Container::value_type)NULL); // c.push_back(NULL) doesn't compile
    c.push_back((Container::value_type)0); // c.push_back(0) doesn't compile
    c.push_back(new s);
    c.delete_all();
    b &= v(c);
    EXPECT_TRUE(c.size() == 0);
    c.push_back(new s);
    c.erase_and_delete(c.begin());
    b &= v(c);
    EXPECT_TRUE(c.size() == 0);

    c.push_back(new s);
    c.push_back(new s);
    c.push_back(new s);
    c.pop_front_delete();
    b &= v(c);
    EXPECT_TRUE(c.size() == 2);
    c.pop_back_delete();
    b &= v(c);
    EXPECT_TRUE(c.size() == 1);
    c.delete_all();
    b &= v(c);

    return b;
}

template <class Container, class Validator, class T1, class T2, class T3, class T4, class T5>
bool test_push_back(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5)
{
    bool b = true;

    Container c;
    Validator v;
    
    // Test various push_back derivatives:
    c.push_back();
    EXPECT_TRUE(c.back().params == 0);
    b &= v(c);
    c.push_back(t1);
    EXPECT_TRUE(c.back().params == 1);
    b &= v(c);
    c.push_back(t1, t2);
    EXPECT_TRUE(c.back().params == 2);
    b &= v(c);
    c.push_back(t1, t2, t3);
    EXPECT_TRUE(c.back().params == 3);
    b &= v(c);
    c.push_back(t1, t2, t3, t4);
    EXPECT_TRUE(c.back().params == 4);
    b &= v(c);
    new (c.push_back_placement()) Container::value_type(t1, t2, t3, t4, t5);
    EXPECT_TRUE(c.back().params == 5);
    b &= v(c);

    return b;
}

template <class Container, class Validator, class T1, class T2, class T3, class T4, class T5>
bool test_push_front(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5)
{
    bool b = true;

    Container c;
    Validator v;

    // Test various push_back derivatives:
    c.push_front();
    EXPECT_TRUE(c.front().params == 0);
    b &= v(c);
    c.push_front(t1);
    EXPECT_TRUE(c.front().params == 1);
    b &= v(c);
    c.push_front(t1, t2);
    EXPECT_TRUE(c.front().params == 2);
    b &= v(c);
    c.push_front(t1, t2, t3);
    EXPECT_TRUE(c.front().params == 3);
    b &= v(c);
    c.push_front(t1, t2, t3, t4);
    EXPECT_TRUE(c.front().params == 4);
    b &= v(c);
    new (c.push_front_placement()) Container::value_type(t1, t2, t3, t4, t5);
    EXPECT_TRUE(c.front().params == 5);
    b &= v(c);

    return b;
}

template <class Container, class Validator, class T1, class T2, class T3, class T4, class T5>
bool test_insert(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5)
{
    bool b = true;

    // Test various insert derivatives:
    Container c;
    Validator v;
    c.insert(c.begin());
    EXPECT_TRUE(c.front().params == 0);
    b &= v(c);
    c.insert(c.begin(), t1);
    EXPECT_TRUE(c.front().params == 1);
    b &= v(c);
    c.insert(c.begin(), t1, t2);
    EXPECT_TRUE(c.front().params == 2);
    b &= v(c);
    c.insert(c.begin(), t1, t2, t3);
    EXPECT_TRUE(c.front().params == 3);
    b &= v(c);
    c.insert(c.begin(), t1, t2, t3, t4);
    EXPECT_TRUE(c.front().params == 4);
    b &= v(c);
    new (c.insert_placement(c.begin())) Container::value_type(t1, t2, t3, t4, t5);
    EXPECT_TRUE(c.front().params == 5);
    b &= v(c);

    c.insert(c.begin(), c.begin(), c.end());
    EXPECT_TRUE(c.size() == 12);
    b &= v(c);

    c.insert(++c.begin(), Container::size_type(5), Container::value_type(t1));
    EXPECT_TRUE(c.size() == 17);
    b &= v(c);

    return b;
}

template <class Validator, class Container> bool test_erase(Container& c)
{
    bool b = true;

    Validator v;

    EXPECT_TRUE(c.size() > 3);
    typename Container::size_type size = c.size();
    THOR_UNUSED(size);

    c.erase(++c.begin());
    EXPECT_TRUE(c.size() == --size);
    b &= v(c);

    c.erase(++c.begin(), --c.end());
    EXPECT_TRUE(c.size() <= 2);
    b &= v(c);

    c.clear();
    EXPECT_TRUE(c.empty());
    b &= v(c);

    return b;
}

template <class Container, class Validator, class T> bool test_resize(const T& t = T())
{
    bool b = true;

    Container c;
    Validator v;
    for (thor_size_type i = 0; i < 21; i += 3)
    {
        c.resize(i, t);
        b &= v(c);
    }
    for (int i = 21; i >= 0; i -= 3)
    {
        c.resize((Container::size_type)i, t);
        b &= v(c);
    }

    return b;
}

#endif