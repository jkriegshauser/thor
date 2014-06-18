#include "gtest/gtest.h"

#include "../ref_counted.h"

template<typename T_REF_POLICY, typename T_DELETE_POLICY, thor::size_type initial> class refcounted_harness : public thor::ref_counted<T_REF_POLICY, T_DELETE_POLICY, initial>
{
public:
    static bool created, deleted;
    refcounted_harness()
    {
        created = true; deleted = false;
    }
    ~refcounted_harness()
    {
        deleted = true;
    }
};

template<typename T_REF_POLICY, typename T_DELETE_POLICY, thor::size_type initial> class refcounted_harness_notify : public thor::ref_counted<T_REF_POLICY, T_DELETE_POLICY, initial>
{
public:
    static bool created, deleted, zero_refcount;
    refcounted_harness_notify()
    {
        created = true; deleted = false;
    }
    ~refcounted_harness_notify()
    {
        deleted = true;
    }

    virtual void on_zero_refcount()
    {
        zero_refcount = true;
        delete this;
    }
};

template<typename T_REF_POLICY, typename T_DELETE_POLICY, thor::size_type initial> bool refcounted_harness<T_REF_POLICY, T_DELETE_POLICY, initial>::created = false;
template<typename T_REF_POLICY, typename T_DELETE_POLICY, thor::size_type initial> bool refcounted_harness<T_REF_POLICY, T_DELETE_POLICY, initial>::deleted = false;

template<typename T_REF_POLICY, typename T_DELETE_POLICY, thor::size_type initial> bool refcounted_harness_notify<T_REF_POLICY, T_DELETE_POLICY, initial>::created = false;
template<typename T_REF_POLICY, typename T_DELETE_POLICY, thor::size_type initial> bool refcounted_harness_notify<T_REF_POLICY, T_DELETE_POLICY, initial>::deleted = false;
template<typename T_REF_POLICY, typename T_DELETE_POLICY, thor::size_type initial> bool refcounted_harness_notify<T_REF_POLICY, T_DELETE_POLICY, initial>::zero_refcount = false;

typedef refcounted_harness<thor::policy::thread_safe_ref_count, thor::policy::delete_this_on_zero_refcount, 0> test_refcounted;
typedef refcounted_harness<thor::policy::thread_safe_ref_count, thor::policy::delete_this_on_zero_refcount, 1> test_refcounted1;
typedef refcounted_harness_notify<thor::policy::unsafe_ref_count, thor::policy::notify_on_zero_refcount, 0> unsafe_refcounted;

TEST(ref_counted, initial)
{
    test_refcounted* t = new test_refcounted;
    EXPECT_TRUE(test_refcounted::created);
    EXPECT_FALSE(test_refcounted::deleted);
    EXPECT_EQ(0, t->get_ref_count());
    t->add_ref();
    EXPECT_EQ(1, t->get_ref_count());
    t->add_ref();
    EXPECT_EQ(2, t->get_ref_count());
    t->release();
    EXPECT_EQ(1, t->get_ref_count());
    t->release();
    EXPECT_TRUE(test_refcounted::deleted);

    test_refcounted1* t1 = new test_refcounted1;
    EXPECT_TRUE(test_refcounted1::created);
    EXPECT_FALSE(test_refcounted1::deleted);
    EXPECT_EQ(1, t1->get_ref_count());
    t1->add_ref();
    EXPECT_EQ(2, t1->get_ref_count());
    t1->release();
    EXPECT_EQ(1, t1->get_ref_count());
    t1->release();
    EXPECT_TRUE(test_refcounted1::deleted);

    unsafe_refcounted* u = new unsafe_refcounted;
    EXPECT_TRUE(unsafe_refcounted::created);
    EXPECT_FALSE(unsafe_refcounted::deleted);
    EXPECT_FALSE(unsafe_refcounted::zero_refcount);
    u->add_ref();
    EXPECT_EQ(1, u->get_ref_count());
    u->release();
    EXPECT_TRUE(unsafe_refcounted::deleted);
    EXPECT_TRUE(unsafe_refcounted::zero_refcount);
}

TEST(ref_counted, test_ref_pointer)
{
    thor::ref_pointer<test_refcounted> t = new test_refcounted;
    EXPECT_TRUE(test_refcounted::created);
    EXPECT_FALSE(test_refcounted::deleted);
    EXPECT_EQ(1, t->get_ref_count());

    thor::ref_pointer<test_refcounted> t2 = t;
    EXPECT_EQ(2, t->get_ref_count());

    EXPECT_TRUE(t2 == t);

    t = 0;
    EXPECT_EQ(1, t2->get_ref_count());
    EXPECT_TRUE(t < t2);
    EXPECT_TRUE(t.get() < t2);
    EXPECT_TRUE(t <= t2);
    EXPECT_TRUE(t <= t2.get());
    EXPECT_TRUE(t2 > t);
    EXPECT_TRUE(t2 >= t);
    EXPECT_TRUE(t2 != t);
    t2 = 0;
    EXPECT_TRUE(t2 == 0);
    EXPECT_TRUE(test_refcounted::deleted);
}