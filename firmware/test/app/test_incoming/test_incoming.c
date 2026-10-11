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

/** Field 0 is the first one after the letter. */
static void test_field_first(void)
{
    char field[8];

    TEST_ASSERT_TRUE(get_field("L,W,12,ok", 0, field, sizeof(field)));
    TEST_ASSERT_EQUAL_STRING("W", field);
}

/** A field in the middle stops at the next separator. */
static void test_field_middle(void)
{
    char field[8];

    TEST_ASSERT_TRUE(get_field("L,W,12,ok", 1, field, sizeof(field)));
    TEST_ASSERT_EQUAL_STRING("12", field);
}

/** The last field runs to the end of the message. */
static void test_field_last(void)
{
    char field[8];

    TEST_ASSERT_TRUE(get_field("L,W,12,ok", 2, field, sizeof(field)));
    TEST_ASSERT_EQUAL_STRING("ok", field);
}

/** An empty field is a field: it is copied as empty text. */
static void test_field_empty(void)
{
    char field[8] = "old";

    TEST_ASSERT_TRUE(get_field("L,W,,ok", 1, field, sizeof(field)));
    TEST_ASSERT_EQUAL_STRING("", field);
}

/** A field past the end of the message is not there. */
static void test_field_past_the_end(void)
{
    char field[8];

    TEST_ASSERT_FALSE(get_field("L,W,12,ok", 3, field, sizeof(field)));
}

/** A message that is its letter alone has no fields. */
static void test_field_of_a_letter_alone(void)
{
    char field[8];

    TEST_ASSERT_FALSE(get_field("L", 0, field, sizeof(field)));
}

/** A field that fills the buffer, with its ending '\0', is copied. */
static void test_field_that_fills_the_buffer(void)
{
    char field[4];

    TEST_ASSERT_TRUE(get_field("L,abc,ok", 0, field, sizeof(field)));
    TEST_ASSERT_EQUAL_STRING("abc", field);
}

/** A field one character too long for the buffer is not copied. */
static void test_field_too_long_for_the_buffer(void)
{
    char field[4];

    TEST_ASSERT_FALSE(get_field("L,abcd,ok", 0, field, sizeof(field)));
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

/** The sequence number of some other reading removes nothing. */
static void test_reading_stays_when_the_seq_is_another_readings(void)
{
    char seq_text[FORMAT_NUMBER_SIZE];

    format_number(store_one_reading() + 1, seq_text);

    remove_reading_if_ok(seq_text, STATE_OK);

    TEST_ASSERT_FALSE(reading_store_is_empty());
}

/** Runs every test in this file. */
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_field_first);
    RUN_TEST(test_field_middle);
    RUN_TEST(test_field_last);
    RUN_TEST(test_field_empty);
    RUN_TEST(test_field_past_the_end);
    RUN_TEST(test_field_of_a_letter_alone);
    RUN_TEST(test_field_that_fills_the_buffer);
    RUN_TEST(test_field_too_long_for_the_buffer);

    RUN_TEST(test_response_is_read_into_its_fields);
    RUN_TEST(test_response_clears_the_awaiting_flag);
    RUN_TEST(test_response_with_a_field_missing);

    RUN_TEST(test_status_ok_sets_the_ok_state);
    RUN_TEST(test_status_not_ok_sets_the_no_response_state);

    RUN_TEST(test_answered_reading_is_removed_when_ok);
    RUN_TEST(test_reading_stays_when_not_ok);
    RUN_TEST(test_reading_stays_when_the_seq_is_not_a_number);
    RUN_TEST(test_reading_stays_when_the_seq_is_another_readings);

    return UNITY_END();
}
