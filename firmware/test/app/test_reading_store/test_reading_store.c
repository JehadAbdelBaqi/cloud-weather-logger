#include <stdbool.h>
#include <stdint.h>

#include <unity.h>

#include "config.h"
#include "reading.h"
#include "reading_store.h"

/**
 * @brief Puts one reading in the store, with a temperature of the caller's choosing to tell it by.
 *
 * @param temperature The reading's temperature
 * @return true if the store took the reading
 */
static bool put_reading(int32_t temperature)
{
    const reading_t reading = { .temperature = temperature, .pressure = 101325, .humidity = 4550 };

    return reading_store_put(&reading, 1790812800);
}

/**
 * @brief Gives the sequence number of the oldest reading in the store.
 *
 * @return The sequence number
 */
static uint32_t oldest_seq(void)
{
    stored_reading_t stored;

    reading_store_oldest(&stored);
    return stored.seq;
}

/** Runs before each test: empties the store. Its sequence numbers carry on from test to test. */
void setUp(void)
{
    while (!reading_store_is_empty()) {
        reading_store_remove_oldest();
    }
}

/** Runs after each test. Nothing to clear up. */
void tearDown(void)
{
}

/** An empty store has no oldest reading to give. */
static void test_empty_store_gives_no_reading(void)
{
    stored_reading_t stored;

    TEST_ASSERT_TRUE(reading_store_is_empty());
    TEST_ASSERT_FALSE(reading_store_oldest(&stored));
}

/** A reading put in comes back as it was, with the time it was given. */
static void test_reading_put_in_is_given_back(void)
{
    const reading_t reading = { .temperature = -512, .pressure = 101325, .humidity = 4550 };
    stored_reading_t stored;

    TEST_ASSERT_TRUE(reading_store_put(&reading, 1790812800));
    TEST_ASSERT_TRUE(reading_store_oldest(&stored));

    TEST_ASSERT_EQUAL_UINT32(1790812800, stored.time);
    TEST_ASSERT_EQUAL_INT32(-512, stored.reading.temperature);
    TEST_ASSERT_EQUAL_UINT32(101325, stored.reading.pressure);
    TEST_ASSERT_EQUAL_UINT32(4550, stored.reading.humidity);
}

/** Giving the oldest reading leaves it in the store. */
static void test_oldest_reading_stays_in_the_store(void)
{
    stored_reading_t stored;

    put_reading(1);
    reading_store_oldest(&stored);

    TEST_ASSERT_FALSE(reading_store_is_empty());
    TEST_ASSERT_TRUE(reading_store_oldest(&stored));
}

/** Each reading stored gets the sequence number after the last one's. */
static void test_sequence_numbers_go_up_by_one(void)
{
    uint32_t first;

    put_reading(1);
    put_reading(2);
    first = oldest_seq();
    reading_store_remove_oldest();

    TEST_ASSERT_EQUAL_UINT32(first + 1, oldest_seq());
}

/** Readings leave the store oldest first. */
static void test_readings_leave_oldest_first(void)
{
    stored_reading_t stored;

    put_reading(1);
    put_reading(2);

    reading_store_oldest(&stored);
    TEST_ASSERT_EQUAL_INT32(1, stored.reading.temperature);
    reading_store_remove_oldest();
    reading_store_oldest(&stored);
    TEST_ASSERT_EQUAL_INT32(2, stored.reading.temperature);
}

/** Removing by the oldest reading's sequence number takes it out. */
static void test_remove_by_the_oldest_sequence_number(void)
{
    put_reading(1);

    TEST_ASSERT_TRUE(reading_store_remove(oldest_seq()));
    TEST_ASSERT_TRUE(reading_store_is_empty());
}

/** A sequence number that is not the oldest reading's removes nothing. */
static void test_remove_by_another_sequence_number_removes_nothing(void)
{
    stored_reading_t stored;

    put_reading(1);
    put_reading(2);

    TEST_ASSERT_FALSE(reading_store_remove(oldest_seq() + 1));
    reading_store_oldest(&stored);
    TEST_ASSERT_EQUAL_INT32(1, stored.reading.temperature);
}

/** A full store turns the next reading down. */
static void test_full_store_refuses_a_reading(void)
{
    for (uint32_t i = 0; i < READING_STORE_CAPACITY; i++) {
        TEST_ASSERT_TRUE(put_reading((int32_t)i));
    }

    TEST_ASSERT_TRUE(reading_store_is_full());
    TEST_ASSERT_FALSE(put_reading(-1));
}

/** The store carries on past its last place: readings still leave in order after it has wrapped. */
static void test_order_is_kept_past_the_last_place(void)
{
    stored_reading_t stored;

    // Fill it, free three places at the front, then fill those: the newest three sit at the start
    for (uint32_t i = 0; i < READING_STORE_CAPACITY; i++) {
        put_reading((int32_t)i);
    }
    for (uint32_t i = 0; i < 3; i++) {
        reading_store_remove_oldest();
    }
    for (uint32_t i = 0; i < 3; i++) {
        TEST_ASSERT_TRUE(put_reading((int32_t)(READING_STORE_CAPACITY + i)));
    }

    for (uint32_t i = 3; i < READING_STORE_CAPACITY + 3; i++) {
        TEST_ASSERT_TRUE(reading_store_oldest(&stored));
        TEST_ASSERT_EQUAL_INT32((int32_t)i, stored.reading.temperature);
        reading_store_remove_oldest();
    }
    TEST_ASSERT_TRUE(reading_store_is_empty());
}

/** Runs every test in this file. */
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_empty_store_gives_no_reading);
    RUN_TEST(test_reading_put_in_is_given_back);
    RUN_TEST(test_oldest_reading_stays_in_the_store);

    RUN_TEST(test_sequence_numbers_go_up_by_one);
    RUN_TEST(test_readings_leave_oldest_first);

    RUN_TEST(test_remove_by_the_oldest_sequence_number);
    RUN_TEST(test_remove_by_another_sequence_number_removes_nothing);

    RUN_TEST(test_full_store_refuses_a_reading);
    RUN_TEST(test_order_is_kept_past_the_last_place);

    return UNITY_END();
}
