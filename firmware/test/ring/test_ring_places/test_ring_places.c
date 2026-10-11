#include <stdbool.h>
#include <stdint.h>

#include <unity.h>

#include "ring_places.h"

// How many places the ring in these tests has
#define CAPACITY 3

static ring_t ring;

/** Runs before each test: starts with an empty ring. */
void setUp(void)
{
    ring = (ring_t){ .capacity = CAPACITY };
}

/** Runs after each test. Nothing to clear up. */
void tearDown(void)
{
}

/**
 * @brief Adds one item to the ring and gives the place it was given.
 *
 * @return The place
 */
static uint32_t add_one(void)
{
    uint32_t place = 0;

    ring_add(&ring, &place);
    return place;
}

/** A new ring is empty and not full. */
static void test_new_ring_is_empty(void)
{
    TEST_ASSERT_TRUE(ring_is_empty(&ring));
    TEST_ASSERT_FALSE(ring_is_full(&ring));
}

/** Items are given the places in order, from the first. */
static void test_places_are_given_in_order(void)
{
    TEST_ASSERT_EQUAL_UINT32(0, add_one());
    TEST_ASSERT_EQUAL_UINT32(1, add_one());
    TEST_ASSERT_EQUAL_UINT32(2, add_one());
}

/** A ring holding an item is not empty. */
static void test_ring_with_an_item_is_not_empty(void)
{
    add_one();

    TEST_ASSERT_FALSE(ring_is_empty(&ring));
}

/** The ring is full once every place is taken. */
static void test_full_when_every_place_is_taken(void)
{
    add_one();
    add_one();
    add_one();

    TEST_ASSERT_TRUE(ring_is_full(&ring));
}

/** A full ring takes no more, and leaves the caller's place as it was. */
static void test_full_ring_takes_no_more(void)
{
    uint32_t place = 99;

    add_one();
    add_one();
    add_one();

    TEST_ASSERT_FALSE(ring_add(&ring, &place));
    TEST_ASSERT_EQUAL_UINT32(99, place);
}

/** The oldest item is the first one added. */
static void test_oldest_is_the_first_added(void)
{
    add_one();
    add_one();

    TEST_ASSERT_EQUAL_UINT32(0, ring_oldest_place(&ring));
}

/** Removing the oldest makes the next one the oldest. */
static void test_removing_makes_the_next_the_oldest(void)
{
    add_one();
    add_one();

    ring_remove_oldest(&ring);

    TEST_ASSERT_EQUAL_UINT32(1, ring_oldest_place(&ring));
}

/** Removing from an empty ring does nothing. */
static void test_removing_from_an_empty_ring_does_nothing(void)
{
    ring_remove_oldest(&ring);

    TEST_ASSERT_TRUE(ring_is_empty(&ring));
    TEST_ASSERT_EQUAL_UINT32(0, add_one());
}

/** After the last place, the next item goes into the first place again. */
static void test_places_go_round_to_the_start(void)
{
    add_one();
    add_one();
    add_one();
    ring_remove_oldest(&ring);

    TEST_ASSERT_EQUAL_UINT32(0, add_one());
}

/** After the last place, the oldest goes round to the first place again. */
static void test_oldest_goes_round_to_the_start(void)
{
    add_one();
    add_one();
    add_one();
    ring_remove_oldest(&ring);
    ring_remove_oldest(&ring);
    add_one();
    ring_remove_oldest(&ring);

    TEST_ASSERT_EQUAL_UINT32(0, ring_oldest_place(&ring));
}

/** Runs every test in this file. */
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_new_ring_is_empty);
    RUN_TEST(test_places_are_given_in_order);
    RUN_TEST(test_ring_with_an_item_is_not_empty);

    RUN_TEST(test_full_when_every_place_is_taken);
    RUN_TEST(test_full_ring_takes_no_more);

    RUN_TEST(test_oldest_is_the_first_added);
    RUN_TEST(test_removing_makes_the_next_the_oldest);
    RUN_TEST(test_removing_from_an_empty_ring_does_nothing);

    RUN_TEST(test_places_go_round_to_the_start);
    RUN_TEST(test_oldest_goes_round_to_the_start);

    return UNITY_END();
}
