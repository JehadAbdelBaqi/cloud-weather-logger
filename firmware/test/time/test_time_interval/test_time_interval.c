#include <stdint.h>

#include <unity.h>

#include "time_interval.h"

/** Runs before each test. Nothing to set up. */
void setUp(void)
{
}

/** Runs after each test. Nothing to clear up. */
void tearDown(void)
{
}

/** Before the interval is up, it has not passed. */
static void test_not_passed_before_the_interval(void)
{
    TEST_ASSERT_FALSE(time_interval_passed(1499, 1000, 500));
}

/** At exactly the interval, it has passed. */
static void test_passed_at_the_interval(void)
{
    TEST_ASSERT_TRUE(time_interval_passed(1500, 1000, 500));
}

/** After the interval, it has passed. */
static void test_passed_after_the_interval(void)
{
    TEST_ASSERT_TRUE(time_interval_passed(9000, 1000, 500));
}

/** No time gone by at all: an interval has not passed. */
static void test_not_passed_at_the_same_moment(void)
{
    TEST_ASSERT_FALSE(time_interval_passed(1000, 1000, 500));
}

/** An interval of zero has always passed. */
static void test_zero_interval_has_always_passed(void)
{
    TEST_ASSERT_TRUE(time_interval_passed(1000, 1000, 0));
}

/** The counter wrapped round to 0 in between: 16 ms before the wrap and 16 ms after are 32 ms. */
static void test_counter_wrapped_passed(void)
{
    TEST_ASSERT_TRUE(time_interval_passed(0x00000010, 0xFFFFFFF0, 32));
}

/** The counter wrapped round to 0 in between, and the interval is not up yet. */
static void test_counter_wrapped_not_passed(void)
{
    TEST_ASSERT_FALSE(time_interval_passed(0x00000010, 0xFFFFFFF0, 33));
}

/** Runs every test in this file. */
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_not_passed_before_the_interval);
    RUN_TEST(test_passed_at_the_interval);
    RUN_TEST(test_passed_after_the_interval);
    RUN_TEST(test_not_passed_at_the_same_moment);
    RUN_TEST(test_zero_interval_has_always_passed);

    RUN_TEST(test_counter_wrapped_passed);
    RUN_TEST(test_counter_wrapped_not_passed);

    return UNITY_END();
}
