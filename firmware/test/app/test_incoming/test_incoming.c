#include <stdbool.h>
#include <stdint.h>

#include <unity.h>

#include "format_number.h"
#include "incoming.h"
#include "protocol.h"
#include "reading.h"
#include "reading_store.h"
#include "routing.h"

/**
 * @brief Puts one reading in the store and gives its sequence number.
 *
 * @return The sequence number the store gave the reading
 */
static uint32_t store_one_reading(void)
{
    const reading_t reading = { .temperature = 2140, .pressure = 101325, .humidity = 4550 };
    stored_reading_t stored;

    reading_store_put(&reading, 1790812800);
    reading_store_oldest(&stored);
    return stored.seq;
}

/** Runs before each test: empties the reading store. */
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

/** A whole response is read into its three fields. */
static void test_response_is_read_into_its_fields(void)
{
    response_t response;
    bool awaiting_ack = true;

    TEST_ASSERT_TRUE(read_response("L,W,12,ok", &awaiting_ack, &response));
    TEST_ASSERT_EQUAL_STRING("W", response.letter);
    TEST_ASSERT_EQUAL_STRING("12", response.first_field);
    TEST_ASSERT_EQUAL_STRING("ok", response.status);
}

/** Reading a response sets the awaiting flag to false: its answer has arrived. */
static void test_response_clears_the_awaiting_flag(void)
{
    response_t response;
    bool awaiting_ack = true;

    read_response("L,H,request,ok", &awaiting_ack, &response);

    TEST_ASSERT_FALSE(awaiting_ack);
}

/** A response with a field missing is not read. */
static void test_response_with_a_field_missing(void)
{
    response_t response;
    bool awaiting_ack = true;

    TEST_ASSERT_FALSE(read_response("L,W,12", &awaiting_ack, &response));
    TEST_ASSERT_FALSE(read_response("L", &awaiting_ack, &response));
}

/** An ok status sets the state to ok. */
static void test_status_ok_sets_the_ok_state(void)
{
    state_key_t state = STATE_NO_RESPONSE;

    handle_status(PROTOCOL_REPLY_OK, &state);

    TEST_ASSERT_EQUAL(STATE_OK, state);
}

/** Any other status sets the state to no response. */
static void test_status_not_ok_sets_the_no_response_state(void)
{
    state_key_t state = STATE_OK;

    handle_status(PROTOCOL_RESPONSE_NO_WIFI, &state);

    TEST_ASSERT_EQUAL(STATE_NO_RESPONSE, state);
}

/** With the state ok, the reading that was answered is removed from the store. */
static void test_answered_reading_is_removed_when_ok(void)
{
    char seq_text[FORMAT_NUMBER_SIZE];

    format_number(store_one_reading(), seq_text);

    remove_reading_if_ok(seq_text, STATE_OK);

    TEST_ASSERT_TRUE(reading_store_is_empty());
}

/** With the state not ok, the reading stays in the store, to be sent again. */
static void test_reading_stays_when_not_ok(void)
{
    char seq_text[FORMAT_NUMBER_SIZE];

    format_number(store_one_reading(), seq_text);

    remove_reading_if_ok(seq_text, STATE_NO_RESPONSE);

    TEST_ASSERT_FALSE(reading_store_is_empty());
}

/** A sequence number that is not a number removes nothing. */
static void test_reading_stays_when_the_seq_is_not_a_number(void)
{
    store_one_reading();

    remove_reading_if_ok("abc", STATE_OK);

    TEST_ASSERT_FALSE(reading_store_is_empty());
}

/** Runs every test in this file. */
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_response_is_read_into_its_fields);
    RUN_TEST(test_response_clears_the_awaiting_flag);
    RUN_TEST(test_response_with_a_field_missing);

    RUN_TEST(test_status_ok_sets_the_ok_state);
    RUN_TEST(test_status_not_ok_sets_the_no_response_state);

    RUN_TEST(test_answered_reading_is_removed_when_ok);
    RUN_TEST(test_reading_stays_when_not_ok);
    RUN_TEST(test_reading_stays_when_the_seq_is_not_a_number);

    return UNITY_END();
}
