#include <stdint.h>

#include <unity.h>

#include "format_number.h"
#include "message.h"
#include "message_helpers.h"
#include "messages.h"
#include "protocol.h"
#include "reading_store.h"
#include "reading_to_message.h"

// Room for the text of any message built in these tests
#define TEXT_SIZE 64

/** Runs before each test. Nothing to set up. */
void setUp(void)
{
}

/** Runs after each test. Nothing to clear up. */
void tearDown(void)
{
}

/** A message starts as its letter and nothing after it. */
static void test_start_is_the_letter_alone(void)
{
    char text[TEXT_SIZE] = "old text";

    message_start(text, 'W');

    TEST_ASSERT_EQUAL_STRING("W", text);
}

/** A field goes on the end, with a separator before it. */
static void test_field_is_added_after_a_separator(void)
{
    char text[TEXT_SIZE];

    message_start(text, 'R');
    message_add_field(text, "7");
    message_add_field(text, "ok");

    TEST_ASSERT_EQUAL_STRING("R,7,ok", text);
}

/** An empty field still gets its separator. */
static void test_empty_field_is_added(void)
{
    char text[TEXT_SIZE];

    message_start(text, 'R');
    message_add_field(text, "");
    message_add_field(text, "ok");

    TEST_ASSERT_EQUAL_STRING("R,,ok", text);
}

/** The handshake message is constructed as its letter and its one word. */
static void test_construct_the_handshake(void)
{
    char text[TEXT_SIZE];

    construct_message(text, handshake_message);

    TEST_ASSERT_EQUAL_STRING("H,request", text);
}

/** A message with no fields is constructed as its letter alone. */
static void test_construct_with_no_fields(void)
{
    char text[TEXT_SIZE];
    message_t message = { .letter = 'X', .number_of_fields = 0 };

    construct_message(text, message);

    TEST_ASSERT_EQUAL_STRING("X", text);
}

/** Only as many fields as the message says it has are constructed. */
static void test_construct_stops_at_the_number_of_fields(void)
{
    char text[TEXT_SIZE];
    message_t message = {
        .letter = 'X',
        .number_of_fields = 2,
        .fields = { .text = { "one", "two", "three" } },
    };

    construct_message(text, message);

    TEST_ASSERT_EQUAL_STRING("X,one,two", text);
}

/** Filling a message puts the values in its fields, in order. */
static void test_fill_puts_the_values_in_the_fields(void)
{
    char text[TEXT_SIZE];
    message_t message = { .letter = 'R', .number_of_fields = 2 };
    const char *const values[] = { "7", "ok" };

    fill_message(&message, values);
    construct_message(text, message);

    TEST_ASSERT_EQUAL_STRING("R,7,ok", text);
}

/** A message filled from numbers is constructed with those numbers as text. */
static void test_fill_from_numbers(void)
{
    char text[TEXT_SIZE];
    message_t message = { .letter = 'X', .number_of_fields = 3 };
    const int32_t numbers[] = { 5, -20, 300 };
    char texts[3][FORMAT_SIGNED_SIZE];

    fill_message_fields_from_numbers(&message, numbers, texts);
    construct_message(text, message);

    TEST_ASSERT_EQUAL_STRING("X,5,-20,300", text);
}

/** A stored reading becomes the reading message: seq, time, temperature, pressure, humidity. */
static void test_a_stored_reading_becomes_the_reading_message(void)
{
    char text[TEXT_SIZE];
    char texts[PROTOCOL_FIELDS_READING][FORMAT_SIGNED_SIZE];
    const stored_reading_t stored = {
        .seq = 1042,
        .time = 1790812800,
        .reading = { .temperature = -512, .pressure = 101325, .humidity = 4550 },
    };

    reading_to_message(&stored, &reading_message, texts);
    construct_message(text, reading_message);

    TEST_ASSERT_EQUAL_STRING("W,1042,1790812800,-512,101325,4550", text);
}

/** Runs every test in this file. */
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_start_is_the_letter_alone);
    RUN_TEST(test_field_is_added_after_a_separator);
    RUN_TEST(test_empty_field_is_added);

    RUN_TEST(test_construct_the_handshake);
    RUN_TEST(test_construct_with_no_fields);
    RUN_TEST(test_construct_stops_at_the_number_of_fields);

    RUN_TEST(test_fill_puts_the_values_in_the_fields);
    RUN_TEST(test_fill_from_numbers);

    RUN_TEST(test_a_stored_reading_becomes_the_reading_message);

    return UNITY_END();
}
