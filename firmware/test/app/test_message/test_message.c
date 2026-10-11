#include <stdint.h>

#include <unity.h>

#include "format_number.h"
#include "message_build.h"
#include "message_fields.h"
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

/** The handshake message is constructed as its letter and its one word. */
static void test_construct_the_handshake(void)
{
    char text[TEXT_SIZE];

    construct_message(text, PROTOCOL_FIELD_SEPARATOR, handshake_message);

    TEST_ASSERT_EQUAL_STRING("H,request", text);
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
    construct_message(text, PROTOCOL_FIELD_SEPARATOR, reading_message);

    TEST_ASSERT_EQUAL_STRING("W,1042,1790812800,-512,101325,4550", text);
}

/** A reading's values become text in the order every reading sends them. */
static void test_reading_values_become_text(void)
{
    char texts[READING_VALUE_COUNT][FORMAT_SIGNED_SIZE];
    const reading_t reading = { .temperature = -512, .pressure = 101325, .humidity = 4550 };

    reading_values_to_text(1790812800, &reading, texts);

    TEST_ASSERT_EQUAL_STRING("1790812800", texts[0]);
    TEST_ASSERT_EQUAL_STRING("-512", texts[1]);
    TEST_ASSERT_EQUAL_STRING("101325", texts[2]);
    TEST_ASSERT_EQUAL_STRING("4550", texts[3]);
}

/** The reply message is constructed as its letter, the command's ID and the result. */
static void test_construct_a_reply(void)
{
    char text[TEXT_SIZE];
    const char *const values[] = { "7", PROTOCOL_REPLY_OK };

    fill_message(&reply_message, values);
    construct_message(text, PROTOCOL_FIELD_SEPARATOR, reply_message);

    TEST_ASSERT_EQUAL_STRING("R,7,ok", text);
}

/** Runs every test in this file. */
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_construct_the_handshake);

    RUN_TEST(test_a_stored_reading_becomes_the_reading_message);
    RUN_TEST(test_reading_values_become_text);

    RUN_TEST(test_construct_a_reply);

    return UNITY_END();
}
