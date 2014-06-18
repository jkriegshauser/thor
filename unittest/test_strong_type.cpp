#include "test_common.h"

#include "../strong_type.h"

THOR_DECLARE_STRONG_TYPE(int, StrongTypeTest);

TEST(strong_type, initial)
{
    StrongTypeTest st;
    EXPECT_EQ(0, st.value());

    // st = 5; // Error by design
    st = StrongTypeTest(5);
    StrongTypeTest st2 = st;
    StrongTypeTest st3(st);
    EXPECT_EQ(5, st.value());
    EXPECT_EQ(5, st.cvalue());

    int ival = st.coerce<int>();
    EXPECT_EQ(5, ival);
    signed int sival = st.coerce<signed int>();
    EXPECT_EQ(5, sival);
    // unsigned uval = st.coerce<unsigned>(); // Error by design
    // int64 i64val = st.coerce<int64>(); // Error by design
}

template<class T> struct init_to_5_policy
{
    void operator () (T& val) { val = T(5); }
};

THOR_DECLARE_STRONG_TYPE_INIT(int, StrongTypeTest5, init_to_5_policy);

TEST(strong_type, init_policy)
{
    StrongTypeTest5 st;
    EXPECT_EQ(5, st.value());

    st = StrongTypeTest5(6);
    EXPECT_EQ(6, st.value());

    //st = StrongTypeTest(1); // Error by design
}