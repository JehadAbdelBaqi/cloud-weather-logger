#include <stdint.h>

#include <unity.h>

#include "format_number.h"

/** Runs before each test. Nothing to set up. */
void setUp(void)
{
}

/** Runs after each test. Nothing to clear up. */
void tearDown(void)
{
}

/** Zero is one digit, not an empty text. */
static void test_number_zero(void)
{
    char text[FORMAT_NUMBER_SIZE];

    format_number(0, text);

    TEST_ASSERT_EQUAL_STRING("0", text);
}

/** A number of several digits comes out highest digit first. */
static void test_number_several_digits(void)
{
    char text[FORMAT_NUMBER_SIZE];

    format_number(102137, text);

    TEST_ASSERT_EQUAL_STRING("102137", text);
}

/** The largest number fits: 10 digits, the most FORMAT_NUMBER_SIZE has room for. */
static void test_number_largest(void)
{
    char text[FORMAT_NUMBER_SIZE];

    format_number(UINT32_MAX, text);

    TEST_ASSERT_EQUAL_STRING("4294967295", text);
}

/** A number that is not negative has no sign. */
static void test_signed_positive(void)
{
    char text[FORMAT_SIGNED_SIZE];

    format_signed(2350, text);

    TEST_ASSERT_EQUAL_STRING("2350", text);
}

/** Zero has no sign. */
static void test_signed_zero(void)
{
    char text[FORMAT_SIGNED_SIZE];

    format_signed(0, text);

    TEST_ASSERT_EQUAL_STRING("0", text);
}

/** A negative number starts with a '-'. */
static void test_signed_negative(void)
{
    char text[FORMAT_SIGNED_SIZE];

    format_signed(-512, text);

    TEST_ASSERT_EQUAL_STRING("-512", text);
}

/** The largest positive number fits. */
static void test_signed_largest(void)
{
    char text[FORMAT_SIGNED_SIZE];

    format_signed(INT32_MAX, text);

    TEST_ASSERT_EQUAL_STRING("2147483647", text);
}

/** The most negative number fits: a '-' and 10 digits, the most FORMAT_SIGNED_SIZE has room for. */
static void test_signed_most_negative(void)
{
    char text[FORMAT_SIGNED_SIZE];

    format_signed(INT32_MIN, text);

    TEST_ASSERT_EQUAL_STRING("-2147483648", text);
}

/** A byte comes out as "0x" and two digits. */
static void test_hex_byte(void)
{
    char text[FORMAT_HEX_BYTE_SIZE];

    format_hex_byte(0x60, text);

    TEST_ASSERT_EQUAL_STRING("0x60", text);
}

/** A byte below 0x10 keeps its leading zero. */
static void test_hex_byte_leading_zero(void)
{
    char text[FORMAT_HEX_BYTE_SIZE];

    format_hex_byte(0x0F, text);

    TEST_ASSERT_EQUAL_STRING("0x0F", text);
}

/** Zero is "0x00". */
static void test_hex_byte_zero(void)
{
    char text[FORMAT_HEX_BYTE_SIZE];

    format_hex_byte(0x00, text);

    TEST_ASSERT_EQUAL_STRING("0x00", text);
}

/** The digits above 9 are capital letters. */
static void test_hex_byte_letters(void)
{
    char text[FORMAT_HEX_BYTE_SIZE];

    format_hex_byte(0xAB, text);

    TEST_ASSERT_EQUAL_STRING("0xAB", text);
}

/** The largest byte is "0xFF". */
static void test_hex_byte_largest(void)
{
    char text[FORMAT_HEX_BYTE_SIZE];

    format_hex_byte(0xFF, text);

    TEST_ASSERT_EQUAL_STRING("0xFF", text);
}

/** Runs every test in this file. */
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_number_zero);
    RUN_TEST(test_number_several_digits);
    RUN_TEST(test_number_largest);

    RUN_TEST(test_signed_positive);
    RUN_TEST(test_signed_zero);
    RUN_TEST(test_signed_negative);
    RUN_TEST(test_signed_largest);
    RUN_TEST(test_signed_most_negative);

    RUN_TEST(test_hex_byte);
    RUN_TEST(test_hex_byte_leading_zero);
    RUN_TEST(test_hex_byte_zero);
    RUN_TEST(test_hex_byte_letters);
    RUN_TEST(test_hex_byte_largest);

    return UNITY_END();
}
