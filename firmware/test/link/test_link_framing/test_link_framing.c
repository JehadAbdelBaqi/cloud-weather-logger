#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <unity.h>

#include "link_framing.h"

// The line being collected, started afresh for each test
static link_framing_t framing;

/** Runs before each test. Starts with nothing collected. */
void setUp(void)
{
    memset(&framing, 0, sizeof(framing));
}

/** Runs after each test. Nothing to clear up. */
void tearDown(void)
{
}

/**
 * @brief Hands a text to the framing one character at a time, as the UART would.
 *
 * @param text The characters
 * @return How many lines were completed
 */
static int feed(const char *text)
{
    int completed = 0;

    for (; *text != '\0'; text++) {
        if (link_framing_take_byte(&framing, (uint8_t)*text)) {
            completed++;
        }
    }
    return completed;
}

/**
 * @brief Hands the same character to the framing a number of times.
 *
 * @param character The character
 * @param count     How many times
 * @return How many lines were completed
 */
static int feed_repeated(char character, size_t count)
{
    int completed = 0;

    for (size_t i = 0; i < count; i++) {
        if (link_framing_take_byte(&framing, (uint8_t)character)) {
            completed++;
        }
    }
    return completed;
}

/** Characters alone do not make a line: nothing is complete until the line ending. */
static void test_no_line_before_the_line_ending(void)
{
    TEST_ASSERT_EQUAL_INT(0, feed("H,ack"));
}

/** A '\n' completes the line, which is handed over without it. */
static void test_newline_completes_the_line(void)
{
    TEST_ASSERT_EQUAL_INT(1, feed("H,ack\n"));
    TEST_ASSERT_EQUAL_STRING("H,ack", framing.line);
}

/** A line ending in '\r\n' gives the same line as one ending in '\n'. */
static void test_carriage_return_before_newline_is_dropped(void)
{
    TEST_ASSERT_EQUAL_INT(1, feed("H,ack\r\n"));
    TEST_ASSERT_EQUAL_STRING("H,ack", framing.line);
}

/** A '\r' does not complete a line by itself. */
static void test_carriage_return_alone_completes_nothing(void)
{
    TEST_ASSERT_EQUAL_INT(0, feed("H,ack\r"));
}

/** A '\r' in the middle of a line is skipped and the line carries on. */
static void test_carriage_return_inside_a_line_is_skipped(void)
{
    TEST_ASSERT_EQUAL_INT(1, feed("H,\rack\n"));
    TEST_ASSERT_EQUAL_STRING("H,ack", framing.line);
}

/** A line ending with nothing before it is not a line. */
static void test_empty_line_is_dropped(void)
{
    TEST_ASSERT_EQUAL_INT(0, feed("\n"));
    TEST_ASSERT_EQUAL_INT(0, feed("\r\n"));
}

/** Two lines one after the other are each handed over, the second replacing the first. */
static void test_two_lines_in_a_row(void)
{
    TEST_ASSERT_EQUAL_INT(1, feed("H,request\n"));
    TEST_ASSERT_EQUAL_STRING("H,request", framing.line);

    TEST_ASSERT_EQUAL_INT(1, feed("H,ack\n"));
    TEST_ASSERT_EQUAL_STRING("H,ack", framing.line);
}

/** A shorter line after a longer one carries nothing over from it. */
static void test_shorter_line_after_a_longer_one(void)
{
    feed("T,a long line\n");

    TEST_ASSERT_EQUAL_INT(1, feed("T,hi\n"));
    TEST_ASSERT_EQUAL_STRING("T,hi", framing.line);
}

/** A line after an empty one is handed over as usual. */
static void test_line_after_an_empty_line(void)
{
    feed("\n");

    TEST_ASSERT_EQUAL_INT(1, feed("H,ack\n"));
    TEST_ASSERT_EQUAL_STRING("H,ack", framing.line);
}

/** A line of exactly LINK_LINE_MAX_LEN characters is the longest one kept. */
static void test_longest_line_is_kept(void)
{
    feed_repeated('a', LINK_LINE_MAX_LEN);

    TEST_ASSERT_EQUAL_INT(1, feed("\n"));
    TEST_ASSERT_EQUAL_size_t(LINK_LINE_MAX_LEN, strlen(framing.line));
}

/** A line one character over the limit is dropped whole, not cut short. */
static void test_line_over_the_limit_is_dropped(void)
{
    feed_repeated('a', LINK_LINE_MAX_LEN + 1);

    TEST_ASSERT_EQUAL_INT(0, feed("\n"));
}

/** A line far over the limit is dropped too, and the framing is left ready for the next line. */
static void test_line_far_over_the_limit_is_dropped(void)
{
    feed_repeated('a', 10 * LINK_LINE_MAX_LEN);

    TEST_ASSERT_EQUAL_INT(0, feed("\n"));
    TEST_ASSERT_FALSE(framing.too_long);
    TEST_ASSERT_EQUAL_size_t(0, framing.length);
}

/** After a line that was too long, the next line is handed over as usual. */
static void test_line_after_one_over_the_limit(void)
{
    feed_repeated('a', LINK_LINE_MAX_LEN + 1);
    feed("\n");

    TEST_ASSERT_EQUAL_INT(1, feed("H,ack\n"));
    TEST_ASSERT_EQUAL_STRING("H,ack", framing.line);
}

/** Runs every test in this file. */
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_no_line_before_the_line_ending);
    RUN_TEST(test_newline_completes_the_line);

    RUN_TEST(test_carriage_return_before_newline_is_dropped);
    RUN_TEST(test_carriage_return_alone_completes_nothing);
    RUN_TEST(test_carriage_return_inside_a_line_is_skipped);

    RUN_TEST(test_empty_line_is_dropped);
    RUN_TEST(test_two_lines_in_a_row);
    RUN_TEST(test_shorter_line_after_a_longer_one);
    RUN_TEST(test_line_after_an_empty_line);

    RUN_TEST(test_longest_line_is_kept);
    RUN_TEST(test_line_over_the_limit_is_dropped);
    RUN_TEST(test_line_far_over_the_limit_is_dropped);
    RUN_TEST(test_line_after_one_over_the_limit);

    return UNITY_END();
}
