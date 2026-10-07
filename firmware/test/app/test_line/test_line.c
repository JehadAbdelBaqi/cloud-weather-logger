#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <unity.h>

#include "line.h"
#include "protocol.h"

// The line being built, started afresh for each test
static char line[LINE_BUFFER_SIZE];

/** Runs before each test. Starts with an empty buffer. */
void setUp(void)
{
    memset(line, 0, sizeof(line));
}

/** Runs after each test. Nothing to clear up. */
void tearDown(void)
{
}

/**
 * @brief Fills the line being built with a letter and one long field, up to a given length.
 *
 * @param length How long the line is to be, in characters
 */
static void fill_line_to(size_t length)
{
    line_start(line, 'T');
    line[1] = PROTOCOL_FIELD_SEPARATOR;
    memset(&line[2], 'a', length - 2);
    line[length] = '\0';
}

/** A line starts as its letter alone. */
static void test_start_is_the_letter_alone(void)
{
    line_start(line, 'W');

    TEST_ASSERT_EQUAL_STRING("W", line);
}

/** Starting a line again throws away what was in the buffer. */
static void test_start_replaces_an_earlier_line(void)
{
    line_start(line, 'W');
    line_add_field(line, "old");

    line_start(line, 'R');

    TEST_ASSERT_EQUAL_STRING("R", line);
}

/** A field goes on the end with a separator before it. */
static void test_field_is_added_after_a_separator(void)
{
    line_start(line, 'R');

    TEST_ASSERT_TRUE(line_add_field(line, "7"));
    TEST_ASSERT_TRUE(line_add_field(line, "ok"));

    TEST_ASSERT_EQUAL_STRING("R,7,ok", line);
}

/** A field with nothing in it still gets its separator. */
static void test_empty_field_is_added(void)
{
    line_start(line, 'R');

    TEST_ASSERT_TRUE(line_add_field(line, ""));

    TEST_ASSERT_EQUAL_STRING("R,", line);
}

/** A number is added in decimal. */
static void test_number_is_added_in_decimal(void)
{
    line_start(line, 'W');

    TEST_ASSERT_TRUE(line_add_number(line, 102137));

    TEST_ASSERT_EQUAL_STRING("W,102137", line);
}

/** A negative number is added with its sign. */
static void test_negative_number_is_added_with_its_sign(void)
{
    line_start(line, 'W');

    TEST_ASSERT_TRUE(line_add_signed(line, -512));

    TEST_ASSERT_EQUAL_STRING("W,-512", line);
}

/** A whole reading line is built field by field. */
static void test_a_reading_line(void)
{
    line_start(line, PROTOCOL_LETTER_READING);
    line_add_number(line, 1042);
    line_add_number(line, 1790812800);
    line_add_signed(line, 2140);
    line_add_number(line, 101325);
    line_add_number(line, 4550);

    TEST_ASSERT_EQUAL_STRING("W,1042,1790812800,2140,101325,4550", line);
}

/** A field that brings the line to exactly the limit is added. */
static void test_field_that_fills_the_line_is_added(void)
{
    fill_line_to(LINK_LINE_MAX_LEN - 3);

    TEST_ASSERT_TRUE(line_add_field(line, "ok"));

    TEST_ASSERT_EQUAL_size_t(LINK_LINE_MAX_LEN, strlen(line));
}

/** A field that would take the line past the limit is not added, and the line is left as it was. */
static void test_field_past_the_limit_is_not_added(void)
{
    fill_line_to(LINK_LINE_MAX_LEN - 2);

    TEST_ASSERT_FALSE(line_add_field(line, "ok"));

    TEST_ASSERT_EQUAL_size_t(LINK_LINE_MAX_LEN - 2, strlen(line));
}

/** A number that would take the line past the limit is not added either. */
static void test_number_past_the_limit_is_not_added(void)
{
    fill_line_to(LINK_LINE_MAX_LEN - 2);

    TEST_ASSERT_FALSE(line_add_number(line, 1042));
    TEST_ASSERT_FALSE(line_add_signed(line, -512));

    TEST_ASSERT_EQUAL_size_t(LINK_LINE_MAX_LEN - 2, strlen(line));
}

/** The first field is the one after the letter. */
static void test_first_field(void)
{
    size_t length = 0;
    const char *field = line_field("C,7,rate 10", 0, &length);

    TEST_ASSERT_EQUAL_STRING("7,rate 10", field);
    TEST_ASSERT_EQUAL_size_t(1, length);
}

/** The last field runs to the end of the line. */
static void test_last_field(void)
{
    size_t length = 0;
    const char *field = line_field("C,7,rate 10", 1, &length);

    TEST_ASSERT_EQUAL_STRING("rate 10", field);
    TEST_ASSERT_EQUAL_size_t(7, length);
}

/** A field with nothing in it is found, with a length of 0. */
static void test_empty_field_is_found(void)
{
    size_t length = 99;
    const char *field = line_field("C,,help", 0, &length);

    TEST_ASSERT_NOT_NULL(field);
    TEST_ASSERT_EQUAL_size_t(0, length);
}

/** A separator at the very end leaves an empty last field. */
static void test_empty_last_field_is_found(void)
{
    size_t length = 99;
    const char *field = line_field("C,7,", 1, &length);

    TEST_ASSERT_NOT_NULL(field);
    TEST_ASSERT_EQUAL_size_t(0, length);
}

/** A field past the last one is not there. */
static void test_field_past_the_end_is_not_found(void)
{
    size_t length = 0;

    TEST_ASSERT_NULL(line_field("C,7,help", 2, &length));
}

/** A line that is a letter alone has no fields. */
static void test_letter_alone_has_no_fields(void)
{
    size_t length = 0;

    TEST_ASSERT_NULL(line_field("C", 0, &length));
}

/** An empty line has no fields. */
static void test_empty_line_has_no_fields(void)
{
    size_t length = 0;

    TEST_ASSERT_NULL(line_field("", 0, &length));
}

/** A line carries a word when the word is all that follows its letter. */
static void test_word_is_found(void)
{
    TEST_ASSERT_TRUE(message_word_is("H,ack", "ack"));
}

/** The letter of the line makes no difference. */
static void test_word_is_found_whatever_the_letter(void)
{
    TEST_ASSERT_TRUE(message_word_is("X,ack", "ack"));
}

/** A different word is not a match. */
static void test_different_word_is_not_a_match(void)
{
    TEST_ASSERT_FALSE(message_word_is("H,request", "ack"));
}

/** A word that only starts the same is not a match. */
static void test_longer_word_is_not_a_match(void)
{
    TEST_ASSERT_FALSE(message_word_is("H,acknowledged", "ack"));
}

/** The start of a word is not a match for the whole word. */
static void test_shorter_word_is_not_a_match(void)
{
    TEST_ASSERT_FALSE(message_word_is("H,ac", "ack"));
}

/** A word with another field after it is not a match. */
static void test_word_followed_by_a_field_is_not_a_match(void)
{
    TEST_ASSERT_FALSE(message_word_is("H,ack,7", "ack"));
}

/** A word in a later field is not a match: it has to follow the letter. */
static void test_word_in_a_later_field_is_not_a_match(void)
{
    TEST_ASSERT_FALSE(message_word_is("C,7,ack", "ack"));
}

/** A line that is a letter alone carries no word. */
static void test_letter_alone_carries_no_word(void)
{
    TEST_ASSERT_FALSE(message_word_is("H", "ack"));
}

/** An empty line carries no word. */
static void test_empty_line_carries_no_word(void)
{
    TEST_ASSERT_FALSE(message_word_is("", "ack"));
}

/** A field is copied out with an ending of its own. */
static void test_copy_field(void)
{
    char text[8];

    TEST_ASSERT_TRUE(line_copy_field("C,7,rate 10", 0, text, sizeof(text)));
    TEST_ASSERT_EQUAL_STRING("7", text);
}

/** The last field is copied up to the end of the line. */
static void test_copy_last_field(void)
{
    char text[8];

    TEST_ASSERT_TRUE(line_copy_field("C,7,rate 10", 1, text, sizeof(text)));
    TEST_ASSERT_EQUAL_STRING("rate 10", text);
}

/** A field that exactly fills the buffer, with its ending, is copied. */
static void test_copy_field_that_just_fits(void)
{
    char text[4];

    TEST_ASSERT_TRUE(line_copy_field("C,abc,help", 0, text, sizeof(text)));
    TEST_ASSERT_EQUAL_STRING("abc", text);
}

/** A field too long for the buffer is not copied, not cut short. */
static void test_copy_field_too_long(void)
{
    char text[4];

    TEST_ASSERT_FALSE(line_copy_field("C,abcd,help", 0, text, sizeof(text)));
}

/** A field the line does not have is not copied. */
static void test_copy_missing_field(void)
{
    char text[8];

    TEST_ASSERT_FALSE(line_copy_field("C", 0, text, sizeof(text)));
    TEST_ASSERT_FALSE(line_copy_field("C,7", 1, text, sizeof(text)));
}

/** A field with nothing in it is copied as an empty text. */
static void test_copy_empty_field(void)
{
    char text[8] = "x";

    TEST_ASSERT_TRUE(line_copy_field("C,,help", 0, text, sizeof(text)));
    TEST_ASSERT_EQUAL_STRING("", text);
}

/** A text that is the word and nothing else starts with it, and nothing follows. */
static void test_take_word_alone(void)
{
    const char *rest = "unset";

    TEST_ASSERT_TRUE(line_take_word("read", "read", &rest));
    TEST_ASSERT_NULL(rest);
}

/** After the word and a space, the rest of the text is found. */
static void test_take_word_with_a_value(void)
{
    const char *rest = NULL;

    TEST_ASSERT_TRUE(line_take_word("rate 10", "rate", &rest));
    TEST_ASSERT_EQUAL_STRING("10", rest);
}

/** A word and a space with nothing after them leaves an empty rest. */
static void test_take_word_with_an_empty_value(void)
{
    const char *rest = NULL;

    TEST_ASSERT_TRUE(line_take_word("rate ", "rate", &rest));
    TEST_ASSERT_EQUAL_STRING("", rest);
}

/** A longer word that only starts the same is not the word. */
static void test_take_word_rejects_a_longer_word(void)
{
    const char *rest = NULL;

    TEST_ASSERT_FALSE(line_take_word("rated 10", "rate", &rest));
    TEST_ASSERT_FALSE(line_take_word("reader", "read", &rest));
}

/** A different word, a shorter one and an empty text are not the word. */
static void test_take_word_rejects_other_text(void)
{
    const char *rest = NULL;

    TEST_ASSERT_FALSE(line_take_word("help", "read", &rest));
    TEST_ASSERT_FALSE(line_take_word("rat", "rate", &rest));
    TEST_ASSERT_FALSE(line_take_word("", "rate", &rest));
}

/** Capital letters are not the same word. */
static void test_take_word_is_case_sensitive(void)
{
    const char *rest = NULL;

    TEST_ASSERT_FALSE(line_take_word("READ", "read", &rest));
}

/** Digits are read as a number. */
static void test_read_number(void)
{
    uint32_t value = 99;

    TEST_ASSERT_TRUE(line_read_number("0", &value));
    TEST_ASSERT_EQUAL_UINT32(0, value);
    TEST_ASSERT_TRUE(line_read_number("10", &value));
    TEST_ASSERT_EQUAL_UINT32(10, value);
    TEST_ASSERT_TRUE(line_read_number("3600", &value));
    TEST_ASSERT_EQUAL_UINT32(3600, value);
}

/** Zeros in front do not change the number. */
static void test_read_number_with_leading_zeros(void)
{
    uint32_t value = 0;

    TEST_ASSERT_TRUE(line_read_number("0010", &value));
    TEST_ASSERT_EQUAL_UINT32(10, value);
}

/** The largest number that fits in 32 bits is read. */
static void test_read_largest_number(void)
{
    uint32_t value = 0;

    TEST_ASSERT_TRUE(line_read_number("4294967295", &value));
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, value);
}

/** A number too large for 32 bits is refused, not wrapped round. */
static void test_read_number_too_large(void)
{
    uint32_t value = 0;

    TEST_ASSERT_FALSE(line_read_number("4294967296", &value));
    TEST_ASSERT_FALSE(line_read_number("99999999999999999999999", &value));
}

/** An empty text is not a number. */
static void test_read_number_from_nothing(void)
{
    uint32_t value = 0;

    TEST_ASSERT_FALSE(line_read_number("", &value));
}

/** Anything that is not a digit makes the text not a number. */
static void test_read_number_rejects_other_characters(void)
{
    uint32_t value = 0;

    TEST_ASSERT_FALSE(line_read_number("ten", &value));
    TEST_ASSERT_FALSE(line_read_number("10s", &value));
    TEST_ASSERT_FALSE(line_read_number("-5", &value));
    TEST_ASSERT_FALSE(line_read_number("1.5", &value));
    TEST_ASSERT_FALSE(line_read_number(" 10", &value));
    TEST_ASSERT_FALSE(line_read_number("10 ", &value));
}

/** Runs every test in this file. */
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_start_is_the_letter_alone);
    RUN_TEST(test_start_replaces_an_earlier_line);

    RUN_TEST(test_field_is_added_after_a_separator);
    RUN_TEST(test_empty_field_is_added);
    RUN_TEST(test_number_is_added_in_decimal);
    RUN_TEST(test_negative_number_is_added_with_its_sign);
    RUN_TEST(test_a_reading_line);

    RUN_TEST(test_field_that_fills_the_line_is_added);
    RUN_TEST(test_field_past_the_limit_is_not_added);
    RUN_TEST(test_number_past_the_limit_is_not_added);

    RUN_TEST(test_first_field);
    RUN_TEST(test_last_field);
    RUN_TEST(test_empty_field_is_found);
    RUN_TEST(test_empty_last_field_is_found);
    RUN_TEST(test_field_past_the_end_is_not_found);
    RUN_TEST(test_letter_alone_has_no_fields);
    RUN_TEST(test_empty_line_has_no_fields);

    RUN_TEST(test_word_is_found);
    RUN_TEST(test_word_is_found_whatever_the_letter);
    RUN_TEST(test_different_word_is_not_a_match);
    RUN_TEST(test_longer_word_is_not_a_match);
    RUN_TEST(test_shorter_word_is_not_a_match);
    RUN_TEST(test_word_followed_by_a_field_is_not_a_match);
    RUN_TEST(test_word_in_a_later_field_is_not_a_match);
    RUN_TEST(test_letter_alone_carries_no_word);
    RUN_TEST(test_empty_line_carries_no_word);

    RUN_TEST(test_copy_field);
    RUN_TEST(test_copy_last_field);
    RUN_TEST(test_copy_field_that_just_fits);
    RUN_TEST(test_copy_field_too_long);
    RUN_TEST(test_copy_missing_field);
    RUN_TEST(test_copy_empty_field);

    RUN_TEST(test_take_word_alone);
    RUN_TEST(test_take_word_with_a_value);
    RUN_TEST(test_take_word_with_an_empty_value);
    RUN_TEST(test_take_word_rejects_a_longer_word);
    RUN_TEST(test_take_word_rejects_other_text);
    RUN_TEST(test_take_word_is_case_sensitive);

    RUN_TEST(test_read_number);
    RUN_TEST(test_read_number_with_leading_zeros);
    RUN_TEST(test_read_largest_number);
    RUN_TEST(test_read_number_too_large);
    RUN_TEST(test_read_number_from_nothing);
    RUN_TEST(test_read_number_rejects_other_characters);

    return UNITY_END();
}
