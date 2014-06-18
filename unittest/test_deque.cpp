#include "deque.h"
#include "test_common.h"

template <class T> bool verify_size(const T& t)
{
    T::size_type s = 0;
    for (T::const_iterator iter(t.begin());
         iter != t.end();
         ++iter)
    {
        ++s;
    }
    EXPECT_TRUE(s == t.size());
    return s == t.size();
}

template <class T> bool test_deque()
{
    typedef thor::deque<T> deque;

    deque D;
    EXPECT_TRUE(D.empty());
    EXPECT_TRUE(D.size() == 0);
    EXPECT_TRUE(D.max_size() != 0);
    EXPECT_TRUE(D.begin() == D.end());
    EXPECT_TRUE(D.rbegin() == D.rend());

    deque D2(257);

    const deque D3(259U, T());

    deque D4(D2);

    deque D5(++D3.begin(), --D3.end());
    EXPECT_TRUE(D5.size() == (D3.size() - 2));

    D4 = D5;

    D4.swap(D);

    thor::swap(D4, D);

    EXPECT_TRUE(D4[0] == T());
    D4[0] = T();
    EXPECT_TRUE(D4[D4.size() - 1] == T());
    D4[D4.size() - 1] = T();
    EXPECT_TRUE(D3[0] == T());
    EXPECT_TRUE(D3[D3.size() - 1] == T());

    EXPECT_TRUE(&*D3.rbegin() == &*(--D3.end()));
    EXPECT_TRUE(&*D3.begin() == &*(--D3.rend()));
    EXPECT_TRUE(D3[D3.size() - 2] == T());

    EXPECT_TRUE(D3.front() == T());
    EXPECT_TRUE(D3.back() == T());
    D4.front() = T();
    D4.back() = T();

    for (int i = 0; i != 1024; ++i)
    {
        D4.push_back(T());
        D4.push_front(T());
        D2.push_back(T());
        D.push_front(T());
    }

    for (int i = 0; i != 1024; ++i)
    {
        D4[i+200] = T();
    }

    for (int i = 0; i != 257; ++i)
    {
        D4.pop_front();
    }

    for (int i = 0; i != 257; ++i)
    {
        D4.pop_back();
    }

    D4.erase(D4.begin() + 2);
    D4.erase(D4.end() - 2);
    D4.erase(++D4.begin(), D4.begin() + 258);
    D4.erase(D4.end() - 258, --D4.end());
    D4.insert(D4.begin() + 2, D2.begin(), D2.end());
    D4.insert(D4.end() - 2, deque::size_type(258), T());
    D4.erase(D4.end() - 257, D4.end());

    D4.insert(D4.begin() + 1000, deque::size_type(2), T());

    EXPECT_TRUE(verify_size(D4));
    EXPECT_TRUE(((++D4.begin()) - (--D4.end())) == -(deque::difference_type)(D4.size()-2));
    EXPECT_TRUE(((--D4.end()) - (++D4.begin())) ==  (deque::difference_type)(D4.size()-2));
    EXPECT_TRUE(((++D4.rbegin()) - (--D4.rend())) == -(deque::difference_type)(D4.size()-2));
    EXPECT_TRUE(((--D4.rend()) - (++D4.rbegin())) ==  (deque::difference_type)(D4.size()-2));

    deque::size_type s = 0;
    for (deque::iterator iter(D4.begin()); iter != D4.end(); ++iter)
    {
        EXPECT_TRUE((D4.end() - iter) == (deque::difference_type)(D4.size() - s));
        EXPECT_TRUE((iter - D4.end()) == -(deque::difference_type)(D4.size() - s));
        EXPECT_TRUE((iter - D4.begin()) == (deque::difference_type)s);
        EXPECT_TRUE((D4.begin() - iter) == -((deque::difference_type)s));
        EXPECT_TRUE(*iter == T());
        deque::size_type count;
        EXPECT_TRUE(D4.get_contiguous(iter, count) == &(*iter));
        EXPECT_TRUE(count > 0 && count <= 256 /*deque::BlockCount*/);
        *iter = T();
        ++s;
    }
    EXPECT_TRUE(s == D4.size());

    s = 0;
    for (deque::reverse_iterator iter(D4.rbegin()); iter != D4.rend(); ++iter)
    {
        EXPECT_TRUE(*iter == T());
        *iter = T();
        ++s;
    }
    EXPECT_TRUE(s == D4.size());

    s = 0;
    for (deque::const_iterator iter(D3.begin()); iter != D3.end(); ++iter)
    {
        EXPECT_TRUE(*iter == T());
        // *iter = T(); // will fail to compile
        deque::size_type count;
        EXPECT_TRUE(D3.get_contiguous(iter, count) == &(*iter));
        EXPECT_TRUE(count > 0 && count <= 256 /*deque::BlockCount*/);
        ++s;
    }
    EXPECT_TRUE(s == D3.size());

    s = 0;
    for (deque::const_reverse_iterator iter(D3.rbegin()); iter != D3.rend(); ++iter)
    {
        EXPECT_TRUE(*iter == T());
        // *iter = T(); // will fail to compile
        ++s;
    }
    EXPECT_TRUE(s == D3.size());

    D4.resize(5);
    EXPECT_TRUE(D4.size() == 5);
    EXPECT_TRUE(verify_size(D4));
    D4.resize(1026);
    EXPECT_TRUE(D4.size() == 1026);
    EXPECT_TRUE(verify_size(D4));

    D4.clear();

    return true;
}

TEST(test_deque, test_deque)
{
    EXPECT_TRUE(test_deque<s>());
    EXPECT_TRUE(test_deque<int>());

    bool b = test_pointer_types<thor::deque<s*>, NoValidate<thor::deque<s*> > >();
    EXPECT_TRUE(b);

    b = test_push_back<thor::deque<s>, NoValidate<thor::deque<s> > >(0, 1.f, 2.0, 3, '4');
    EXPECT_TRUE(b);
    b = test_push_front<thor::deque<s>, NoValidate<thor::deque<s> > >(0, 1.f, 2.0, 3, '4');
    EXPECT_TRUE(b);
    b = test_insert<thor::deque<s>, NoValidate<thor::deque<s> > >(0, 1.f, 2.0, 3, '4');
    EXPECT_TRUE(b);
}