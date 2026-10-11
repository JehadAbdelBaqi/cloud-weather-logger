#include <stdbool.h>
#include <string.h>

#include <unity.h>

#include "config.h"
#include "reply_store.h"

/**
 * @brief Fills the store: puts in replies until it takes no more.
 */
static void fill_the_store(void)
{
    while (reply_store_put("1", "ok")) {
    }
}

/** Runs before each test: empties the reply store. */
void setUp(void)
{
    while (!reply_store_is_empty()) {
        reply_store_remove_oldest();
    }
}

/** Runs after each test. Nothing to clear up. */
void tearDown(void)
{
}

/** An empty store gives no reply, and leaves the caller's reply as it was. */
static void test_empty_store_gives_no_reply(void)
{
    stored_reply_t stored = { .id = "old", .result = "old" };

    TEST_ASSERT_FALSE(reply_store_oldest(&stored));
    TEST_ASSERT_EQUAL_STRING("old", stored.id);
}

/** A reply put in is given back with its ID and its result. */
static void test_reply_put_in_is_given_back(void)
{
    stored_reply_t stored;

    TEST_ASSERT_TRUE(reply_store_put("7", "ok"));

    TEST_ASSERT_TRUE(reply_store_oldest(&stored));
    TEST_ASSERT_EQUAL_STRING("7", stored.id);
    TEST_ASSERT_EQUAL_STRING("ok", stored.result);
}

/** Asking for the oldest reply leaves it in the store. */
static void test_oldest_reply_stays_in_the_store(void)
{
    stored_reply_t stored;

    reply_store_put("7", "ok");
    reply_store_oldest(&stored);

    TEST_ASSERT_FALSE(reply_store_is_empty());
}

/** Replies leave the store oldest first. */
static void test_replies_leave_oldest_first(void)
{
    stored_reply_t stored;

    reply_store_put("7", "ok");
    reply_store_put("8", "err unknown command");
    reply_store_remove_oldest();

    reply_store_oldest(&stored);
    TEST_ASSERT_EQUAL_STRING("8", stored.id);
    TEST_ASSERT_EQUAL_STRING("err unknown command", stored.result);
}

/** The oldest reply is removed by its ID. */
static void test_remove_by_the_oldest_id(void)
{
    reply_store_put("7", "ok");

    TEST_ASSERT_TRUE(reply_store_remove("7"));
    TEST_ASSERT_TRUE(reply_store_is_empty());
}

/** An ID that is not the oldest reply's removes nothing. */
static void test_remove_by_another_id_removes_nothing(void)
{
    reply_store_put("7", "ok");
    reply_store_put("8", "ok");

    TEST_ASSERT_FALSE(reply_store_remove("8"));
    TEST_ASSERT_FALSE(reply_store_remove("9"));

    TEST_ASSERT_TRUE(reply_store_remove("7"));
}

/** A full store refuses a reply. */
static void test_full_store_refuses_a_reply(void)
{
    fill_the_store();

    TEST_ASSERT_TRUE(reply_store_is_full());
    TEST_ASSERT_FALSE(reply_store_put("7", "ok"));
}

/** The longest ID is stored; one character longer is refused. */
static void test_id_that_does_not_fit_is_refused(void)
{
    char id[REPLY_ID_SIZE + 1];

    memset(id, 'a', sizeof(id));
    id[REPLY_ID_SIZE] = '\0';
    TEST_ASSERT_FALSE(reply_store_put(id, "ok"));

    id[REPLY_ID_SIZE - 1] = '\0';
    TEST_ASSERT_TRUE(reply_store_put(id, "ok"));
}

/** The longest result is stored; one character longer is refused. */
static void test_result_that_does_not_fit_is_refused(void)
{
    char result[REPLY_RESULT_SIZE + 1];

    memset(result, 'a', sizeof(result));
    result[REPLY_RESULT_SIZE] = '\0';
    TEST_ASSERT_FALSE(reply_store_put("7", result));

    result[REPLY_RESULT_SIZE - 1] = '\0';
    TEST_ASSERT_TRUE(reply_store_put("7", result));
}

/** A reply that is refused takes no place in the store. */
static void test_refused_reply_takes_no_place(void)
{
    char id[REPLY_ID_SIZE + 1];

    memset(id, 'a', sizeof(id));
    id[REPLY_ID_SIZE] = '\0';
    reply_store_put(id, "ok");

    TEST_ASSERT_TRUE(reply_store_is_empty());
}

/** Runs every test in this file. */
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_empty_store_gives_no_reply);
    RUN_TEST(test_reply_put_in_is_given_back);
    RUN_TEST(test_oldest_reply_stays_in_the_store);
    RUN_TEST(test_replies_leave_oldest_first);

    RUN_TEST(test_remove_by_the_oldest_id);
    RUN_TEST(test_remove_by_another_id_removes_nothing);

    RUN_TEST(test_full_store_refuses_a_reply);
    RUN_TEST(test_id_that_does_not_fit_is_refused);
    RUN_TEST(test_result_that_does_not_fit_is_refused);
    RUN_TEST(test_refused_reply_takes_no_place);

    return UNITY_END();
}
