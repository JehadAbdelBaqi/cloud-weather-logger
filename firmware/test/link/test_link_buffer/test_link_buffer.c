#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <unity.h>

#include "link_buffer.h"

static link_buffer_t buffer;

/** Runs before each test: starts with an empty buffer. */
void setUp(void)
{
    memset(&buffer, 0, sizeof(buffer));
}

/** Runs after each test. Nothing to clear up. */
void tearDown(void)
{
}

/** Nothing can be taken out of an empty buffer. */
static void test_empty_buffer_gives_nothing(void)
{
    uint8_t byte = 0;

    TEST_ASSERT_FALSE(link_buffer_take(&buffer, &byte));
}

/** A byte put in comes back out. */
static void test_a_byte_put_in_is_taken_out(void)
{
    uint8_t byte = 0;

    TEST_ASSERT_TRUE(link_buffer_put(&buffer, 'L'));
    TEST_ASSERT_TRUE(link_buffer_take(&buffer, &byte));
    TEST_ASSERT_EQUAL_UINT8('L', byte);
}

/** Once its byte is taken, the buffer is empty again. */
static void test_buffer_is_empty_after_the_last_byte_is_taken(void)
{
    uint8_t byte = 0;

    link_buffer_put(&buffer, 'L');
    link_buffer_take(&buffer, &byte);

    TEST_ASSERT_FALSE(link_buffer_take(&buffer, &byte));
}

/** Bytes come out in the order they went in. */
static void test_bytes_come_out_in_order(void)
{
    const char *sent = "L,W,12,ok\n";
    char received[16] = "";
    uint8_t byte = 0;
    size_t count = 0;

    for (size_t i = 0; i < strlen(sent); i++) {
        link_buffer_put(&buffer, (uint8_t)sent[i]);
    }
    while (link_buffer_take(&buffer, &byte)) {
        received[count] = (char)byte;
        count++;
    }

    TEST_ASSERT_EQUAL_STRING(sent, received);
}

/** A full buffer turns the next byte down, and holds one byte fewer than it has places. */
static void test_full_buffer_drops_the_next_byte(void)
{
    for (uint32_t i = 0; i < LINK_BUFFER_SIZE - 1; i++) {
        TEST_ASSERT_TRUE(link_buffer_put(&buffer, (uint8_t)i));
    }

    TEST_ASSERT_FALSE(link_buffer_put(&buffer, 0xFF));
}

/** The bytes already in a full buffer are kept when another is turned down. */
static void test_full_buffer_keeps_what_it_holds(void)
{
    uint8_t byte = 0;

    for (uint32_t i = 0; i < LINK_BUFFER_SIZE - 1; i++) {
        link_buffer_put(&buffer, (uint8_t)i);
    }
    link_buffer_put(&buffer, 0xFF);

    for (uint32_t i = 0; i < LINK_BUFFER_SIZE - 1; i++) {
        TEST_ASSERT_TRUE(link_buffer_take(&buffer, &byte));
        TEST_ASSERT_EQUAL_UINT8((uint8_t)i, byte);
    }
    TEST_ASSERT_FALSE(link_buffer_take(&buffer, &byte));
}

/** Taking a byte out of a full buffer makes room for one more. */
static void test_taking_a_byte_makes_room(void)
{
    uint8_t byte = 0;

    for (uint32_t i = 0; i < LINK_BUFFER_SIZE - 1; i++) {
        link_buffer_put(&buffer, (uint8_t)i);
    }
    link_buffer_take(&buffer, &byte);

    TEST_ASSERT_TRUE(link_buffer_put(&buffer, 0xFF));
}

/** The buffer carries on past its last place: many more bytes than it has places pass through. */
static void test_bytes_pass_through_past_the_end(void)
{
    uint8_t byte = 0;

    for (uint32_t i = 0; i < 3 * LINK_BUFFER_SIZE; i++) {
        TEST_ASSERT_TRUE(link_buffer_put(&buffer, (uint8_t)i));
        TEST_ASSERT_TRUE(link_buffer_take(&buffer, &byte));
        TEST_ASSERT_EQUAL_UINT8((uint8_t)i, byte);
    }
}

/** Runs every test in this file. */
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_empty_buffer_gives_nothing);
    RUN_TEST(test_a_byte_put_in_is_taken_out);
    RUN_TEST(test_buffer_is_empty_after_the_last_byte_is_taken);
    RUN_TEST(test_bytes_come_out_in_order);

    RUN_TEST(test_full_buffer_drops_the_next_byte);
    RUN_TEST(test_full_buffer_keeps_what_it_holds);
    RUN_TEST(test_taking_a_byte_makes_room);
    RUN_TEST(test_bytes_pass_through_past_the_end);

    return UNITY_END();
}
