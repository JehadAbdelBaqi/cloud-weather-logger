#include <stdint.h>

#include <unity.h>

#include "format_number.h"
#include "message_build.h"
#include "message_fields.h"

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

/** Filling a message puts the values in its fields, in order. */
static void test_fill_puts_the_values_in_the_fields(void)
{
    char text[TEXT_SIZE];
    message_t message = { .letter = 'R', .number_of_fields = 2 };
    const char *const values[] = { "7", "ok" };

    fill_message(&message, values);
    construct_message(text, ',', message);

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
    construct_message(text, ',', message);

    TEST_ASSERT_EQUAL_STRING("X,5,-20,300", text);
}

/** Field 0 is the first one after the letter. */
static void test_field_first(void)
{
    char field[8];

    TEST_ASSERT_TRUE(get_field("L,W,12,ok", ',', 0, field, sizeof(field)));
    TEST_ASSERT_EQUAL_STRING("W", field);
}

/** A field in the middle stops at the next separator. */
static void test_field_middle(void)
{
    char field[8];

    TEST_ASSERT_TRUE(get_field("L,W,12,ok", ',', 1, field, sizeof(field)));
    TEST_ASSERT_EQUAL_STRING("12", field);
}

/** The last field runs to the end of the message. */
static void test_field_last(void)
{
    char field[8];

    TEST_ASSERT_TRUE(get_field("L,W,12,ok", ',', 2, field, sizeof(field)));
    TEST_ASSERT_EQUAL_STRING("ok", field);
}

/** An empty field is a field: it is copied as empty text. */
static void test_field_empty(void)
{
    char field[8] = "old";

    TEST_ASSERT_TRUE(get_field("L,W,,ok", ',', 1, field, sizeof(field)));
    TEST_ASSERT_EQUAL_STRING("", field);
}

/** A field past the end of the message is not there. */
static void test_field_past_the_end(void)
{
    char field[8];

    TEST_ASSERT_FALSE(get_field("L,W,12,ok", ',', 3, field, sizeof(field)));
}

/** A message that is its letter alone has no fields. */
static void test_field_of_a_letter_alone(void)
{
    char field[8];

    TEST_ASSERT_FALSE(get_field("L", ',', 0, field, sizeof(field)));
}

/** A field that fills the buffer, with its ending '\0', is copied. */
static void test_field_that_fills_the_buffer(void)
{
    char field[4];

    TEST_ASSERT_TRUE(get_field("L,abc,ok", ',', 0, field, sizeof(field)));
    TEST_ASSERT_EQUAL_STRING("abc", field);
}

/** A field one character too long for the buffer is not copied. */
static void test_field_too_long_for_the_buffer(void)
{
    char field[4];

    TEST_ASSERT_FALSE(get_field("L,abcd,ok", ',', 0, field, sizeof(field)));
}

/** Runs every test in this file. */
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_fill_puts_the_values_in_the_fields);
    RUN_TEST(test_fill_from_numbers);

    RUN_TEST(test_field_first);
    RUN_TEST(test_field_middle);
    RUN_TEST(test_field_last);
    RUN_TEST(test_field_empty);
    RUN_TEST(test_field_past_the_end);
    RUN_TEST(test_field_of_a_letter_alone);
    RUN_TEST(test_field_that_fills_the_buffer);
    RUN_TEST(test_field_too_long_for_the_buffer);

    return UNITY_END();
}
