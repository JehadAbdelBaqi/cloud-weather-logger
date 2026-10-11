#include <unity.h>

#include "message_build.h"

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
    message_add_field(text, ',', "7");
    message_add_field(text, ',', "ok");

    TEST_ASSERT_EQUAL_STRING("R,7,ok", text);
}

/** An empty field still gets its separator. */
static void test_empty_field_is_added(void)
{
    char text[TEXT_SIZE];

    message_start(text, 'R');
    message_add_field(text, ',', "");
    message_add_field(text, ',', "ok");

    TEST_ASSERT_EQUAL_STRING("R,,ok", text);
}

/** A message with no fields is constructed as its letter alone. */
static void test_construct_with_no_fields(void)
{
    char text[TEXT_SIZE];
    message_t message = { .letter = 'X', .number_of_fields = 0 };

    construct_message(text, ',', message);

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

    construct_message(text, ',', message);

    TEST_ASSERT_EQUAL_STRING("X,one,two", text);
}

/** Runs every test in this file. */
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_start_is_the_letter_alone);
    RUN_TEST(test_field_is_added_after_a_separator);
    RUN_TEST(test_empty_field_is_added);

    RUN_TEST(test_construct_with_no_fields);
    RUN_TEST(test_construct_stops_at_the_number_of_fields);

    return UNITY_END();
}
