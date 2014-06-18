#include "gtest/gtest.h"

#include "../vector.h"
#include "../list.h"
#include "../algorithm.h"

static const int A[] = {1, 1, 1, 2, 2, 3, 4, 4, 4, 5, 6, 7, 8, 8, 8};
static const size_t N = sizeof(A)/sizeof(A[0]);

static const int A2[] = {1, 2, 3, 4, 5, 6, 7, 8};
static const size_t N2 = sizeof(A2)/sizeof(A2[0]);

TEST(algorithms, test_copy)
{
    thor::vector<int> V1, V2;
    V1.insert(V1.end(), A, A + N);
    V2.resize(V1.size());

    thor::copy(V1.begin(), V1.end(), V2.begin());

    for (size_t i = 0; i != N; ++i)
    {
        EXPECT_TRUE(V1[i] == A[i]);
        EXPECT_TRUE(V2[i] == A[i]);
    }

    int B[N] = {};
    thor::copy(A, A+N, B);

    for (size_t i = 0; i != N; ++i)
    {
        EXPECT_TRUE(A[i] == B[i]);
    }
}

TEST(algoritms, test_unique)
{
    thor::vector<int> V(A2, A2 + N2);
    V.erase(thor::unique(V.begin(), V.end()), V.end());
    EXPECT_TRUE(V.size() == N2);

    thor::vector<int> V2(A, A + N);
    V2.erase(thor::unique(V2.begin(), V2.end()), V2.end());
    EXPECT_TRUE(V2.size() != N);

    V2.assign(A, A+N);
    V2.erase(thor::unique(V2.begin(), V2.end(), thor::equal_to<int>()), V2.end());
    EXPECT_TRUE(V2.size() != N);
}

template <class T> class adder : public thor::unary_function<T, void>
{
public:
    int count;
    adder() : count(0) {}

    void operator () (const T& t)
    {
        count += t;
    }
};

TEST(algorithms, test_foreach)
{
    thor::vector<int> V(A2, A2+N2);
    int total = thor::for_each(V.begin(), V.end(), adder<int>()).count;
    EXPECT_TRUE(total == 36);

    total = thor::for_each(A2, A2+N2, adder<int>()).count;
    EXPECT_TRUE(total == 36);
}

class MyRand
{
public:
    thor::size_type operator () (thor::size_type n)
    {
        return rand() % n;
    }
};

TEST(algorithms, test_shuffle)
{
    thor::vector<int> V(A2, A2+N2);
    thor::random_shuffle(V.begin(), V.end());

    int A3[N2];
    thor::copy(A2, A2+N2, A3);
    thor::random_shuffle(A3, A3+N2, MyRand());
}

TEST(algorithms, test_reverse)
{
    thor::vector<int> V(A2, A2+N2);
    thor::reverse(V.begin(), V.end());
    int i = 0;
    for (thor::vector<int>::reverse_iterator iter(V.rbegin()); iter != V.rend(); ++iter, ++i)
    {
        EXPECT_TRUE(*iter == A2[i]);
    }

    thor::vector<int> V2(A, A+N);
    thor::reverse(V2.begin(), V2.end());
    i = 0;
    for (thor::vector<int>::reverse_iterator iter(V2.rbegin()); iter != V2.rend(); ++iter, ++i)
    {
        EXPECT_TRUE(*iter == A[i]);
    }

    int A3[N2];
    thor::copy(A2, A2+N2, A3);
    thor::reverse(A3, A3+N2);
    for (i = 0; i != N2; ++i)
    {
        EXPECT_TRUE(A3[i] == A2[N2-i-1]);
    }

    thor::list<int> L(A2, A2+N2);
    thor::reverse(L.begin(), L.end());
    i = 0;
    for (thor::list<int>::reverse_iterator iter(L.rbegin()); iter != L.rend(); ++iter, ++i)
    {
        EXPECT_TRUE(*iter == A2[i]);
    }
}

struct evenpred
{
    bool operator () (int i) const
    {
        return (i % 2) == 0;
    }
};

TEST(algorithms, test_remove)
{
    thor::vector<int> V;
    V.push_back(2);
    V.push_back(3);
    V.push_back(1);
    V.push_back(4);
    V.push_back(1);
    V.push_back(5);
    V.push_back(9);

    thor::vector<int> V2(V);
    V2.erase(thor::remove(V2.begin(), V2.end(), 4), V2.end());
    EXPECT_TRUE(V2.size() == 6);

    V2 = V;
    V2.erase(thor::remove_if(V2.begin(), V2.end(), evenpred()), V2.end());
    EXPECT_TRUE(V2.size() == 5);
}
