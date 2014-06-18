#include "test_common.h"

#include "../time_util.h"
#include "../system.h"
#include "../thread.h"

using namespace thor::time;
using namespace thor;

TEST(time_util, initial)
{
    seconds now_s = seconds_now();
    milliseconds now_ms = milliseconds_now();
    microseconds now_us = microseconds_now();
    nanoseconds now_ns = nanoseconds_now();
    thor::system::sleep(1000);
    seconds later_s = seconds_now();
    milliseconds later_ms = milliseconds_now();
    microseconds later_us = microseconds_now();
    nanoseconds later_ns = nanoseconds_now();
    EXPECT_TRUE(later_s == seconds_adjust(now_s, 1)) << "Elapsed (sec): " << seconds_diff(now_s, later_s);
    EXPECT_TRUE(later_ms > milliseconds_adjust(now_ms, 950) &&
                later_ms < milliseconds_adjust(now_ms, 1050)) << "Elapsed (msec): " << milliseconds_diff(now_ms, later_ms);
    EXPECT_TRUE(later_us > microseconds(now_us.cvalue() + 950000) &&
                later_us < microseconds(now_us.cvalue() + 1050000)) << "Elapsed (usec): " << microseconds_diff(now_us, later_us);
    EXPECT_TRUE(later_ns > nanoseconds(now_ns.cvalue() + 950000000) &&
                later_ns < nanoseconds(now_ns.cvalue() + 1050000000)) << "Elapsed (nsec): " << nanoseconds_diff(now_ns, later_ns);
}

#define DEFINE_TIME_THREAD(TimeClass) \
class TimeClass##_test_thread : public thor::thread \
{ \
protected: \
    void execute() { while (!is_stop_requested()) { current = TimeClass##_now(); ++iterations; } } \
public: \
    TimeClass##_test_thread() : thor::thread(#TimeClass "_test_thread"), iterations(0) {} \
    TimeClass current; \
    uint64 iterations; \
}

DEFINE_TIME_THREAD(seconds);
DEFINE_TIME_THREAD(milliseconds);
DEFINE_TIME_THREAD(microseconds);
DEFINE_TIME_THREAD(nanoseconds);

TEST(time_util, thread_test)
{
    // Test two threads hammering away at milliseconds_now() to verify that the value only increases by the correct amount
    ref_pointer<milliseconds_test_thread> threads[2];

    // Start suspended
    threads[0] = new milliseconds_test_thread;
    threads[0]->start(true);
    threads[1] = new milliseconds_test_thread;
    threads[1]->start(true);

    milliseconds start = milliseconds_now();
    threads[0]->resume(); threads[1]->resume();
    thor::system::sleep(1000);
    threads[0]->stop(); threads[1]->stop();
    milliseconds end = milliseconds_now();
    EXPECT_TRUE(end >= milliseconds_adjust(start, 950) && end <= milliseconds_adjust(start, 1050)) << "Elapsed (msec): " << milliseconds_diff(start, end);
}

TEST(time_util, count_test)
{
    ref_pointer<seconds_test_thread> secthread = new seconds_test_thread;
    secthread->start(true); secthread->resume();
    thor::system::sleep(250);
    secthread->stop(); secthread->wait();

    ref_pointer<milliseconds_test_thread> msthread = new milliseconds_test_thread;
    msthread->start(true); msthread->resume();
    thor::system::sleep(250);
    msthread->stop(); msthread->wait();

    ref_pointer<microseconds_test_thread> usthread = new microseconds_test_thread;
    usthread->start(true); usthread->resume();
    thor::system::sleep(250);
    usthread->stop(); usthread->wait();

    ref_pointer<nanoseconds_test_thread> nsthread = new nanoseconds_test_thread;
    nsthread->start(true); nsthread->resume();
    thor::system::sleep(250);
    nsthread->stop(); nsthread->wait();

    printf("seconds_now() calls per second: %lld\n"
        "milliseconds_now() calls per second: %lld\n"
        "microseconds_now() calls per second: %lld\n"
        "nanoseconds_now() calls per second: %lld\n",
        secthread->iterations * 4,
        msthread->iterations * 4,
        usthread->iterations * 4,
        nsthread->iterations * 4);

    int final = 0; THOR_UNUSED(final);
}