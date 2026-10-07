#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <unity.h>

#include "bme280_calibration.h"

// The sensor's two runs of calibration bytes, and the values made from them
static uint8_t a[BME280_CALIBRATION_A_LENGTH];
static uint8_t b[BME280_CALIBRATION_B_LENGTH];
static bme280_calibration_t calibration;

/** Runs before each test. Starts with every byte zero. */
void setUp(void)
{
    memset(a, 0, sizeof(a));
    memset(b, 0, sizeof(b));
    memset(&calibration, 0, sizeof(calibration));
}

/** Runs after each test. Nothing to clear up. */
void tearDown(void)
{
}

/**
 * @brief Puts a 16-bit number into the bytes the way the sensor stores it, low byte first.
 *
 * @param bytes The bytes
 * @param at    Where the low byte goes
 * @param value The number, which may be negative
 */
static void put16(uint8_t *bytes, size_t at, int32_t value)
{
    uint16_t bits = (uint16_t)value;

    bytes[at] = (uint8_t)(bits & 0xFF);
    bytes[at + 1] = (uint8_t)(bits >> 8);
}

/** Two bytes are joined low byte first. */
static void test_low_byte_comes_first(void)
{
    a[0] = 0x84;
    a[1] = 0x6E;

    bme280_calibration_from_bytes(a, b, &calibration);

    TEST_ASSERT_EQUAL_UINT16(0x6E84, calibration.t1);
}

/** An unsigned value with its top bit set stays positive. */
static void test_unsigned_value_stays_positive(void)
{
    put16(a, 0, 0xFFFF);
    put16(a, 6, 0xFFFF);

    bme280_calibration_from_bytes(a, b, &calibration);

    TEST_ASSERT_EQUAL_UINT16(65535, calibration.t1);
    TEST_ASSERT_EQUAL_UINT16(65535, calibration.p1);
}

/** A signed value with its top bit set is negative. */
static void test_signed_value_can_be_negative(void)
{
    put16(a, 2, 0xFFFF);

    bme280_calibration_from_bytes(a, b, &calibration);

    TEST_ASSERT_EQUAL_INT16(-1, calibration.t2);
}

/** The three temperature values come from the first six bytes. */
static void test_temperature_values(void)
{
    put16(a, 0, 28292);
    put16(a, 2, 26587);
    put16(a, 4, 50);

    bme280_calibration_from_bytes(a, b, &calibration);

    TEST_ASSERT_EQUAL_UINT16(28292, calibration.t1);
    TEST_ASSERT_EQUAL_INT16(26587, calibration.t2);
    TEST_ASSERT_EQUAL_INT16(50, calibration.t3);
}

/** The nine pressure values each come from their own two bytes, after the temperature values. */
static void test_pressure_values(void)
{
    put16(a, 6, 36477);
    put16(a, 8, -10685);
    put16(a, 10, 3024);
    put16(a, 12, 2855);
    put16(a, 14, 140);
    put16(a, 16, -7);
    put16(a, 18, 15500);
    put16(a, 20, -14600);
    put16(a, 22, 6000);

    bme280_calibration_from_bytes(a, b, &calibration);

    TEST_ASSERT_EQUAL_UINT16(36477, calibration.p1);
    TEST_ASSERT_EQUAL_INT16(-10685, calibration.p2);
    TEST_ASSERT_EQUAL_INT16(3024, calibration.p3);
    TEST_ASSERT_EQUAL_INT16(2855, calibration.p4);
    TEST_ASSERT_EQUAL_INT16(140, calibration.p5);
    TEST_ASSERT_EQUAL_INT16(-7, calibration.p6);
    TEST_ASSERT_EQUAL_INT16(15500, calibration.p7);
    TEST_ASSERT_EQUAL_INT16(-14600, calibration.p8);
    TEST_ASSERT_EQUAL_INT16(6000, calibration.p9);
}

/** The first humidity value is the last byte of the first run. */
static void test_h1_is_the_last_byte_of_the_first_run(void)
{
    a[BME280_CALIBRATION_A_LENGTH - 1] = 75;

    bme280_calibration_from_bytes(a, b, &calibration);

    TEST_ASSERT_EQUAL_UINT8(75, calibration.h1);
}

/** The second and third humidity values start the second run: two bytes, then one. */
static void test_h2_and_h3(void)
{
    put16(b, 0, 362);
    b[2] = 9;

    bme280_calibration_from_bytes(a, b, &calibration);

    TEST_ASSERT_EQUAL_INT16(362, calibration.h2);
    TEST_ASSERT_EQUAL_UINT8(9, calibration.h3);
}

/** h4 and h5 are 12 bits each and share a byte: its low half ends h4, its high half starts h5. */
static void test_h4_and_h5_share_a_byte(void)
{
    b[3] = 0x14;
    b[4] = 0x2A;
    b[5] = 0x03;

    bme280_calibration_from_bytes(a, b, &calibration);

    TEST_ASSERT_EQUAL_HEX16(0x14A, calibration.h4);
    TEST_ASSERT_EQUAL_HEX16(0x032, calibration.h5);
}

/** h4 and h5 are negative when the top bit of their own byte is set. */
static void test_h4_and_h5_can_be_negative(void)
{
    b[3] = 0x80;
    b[4] = 0xF0;
    b[5] = 0xFF;

    bme280_calibration_from_bytes(a, b, &calibration);

    TEST_ASSERT_EQUAL_INT16(-2048, calibration.h4);
    TEST_ASSERT_EQUAL_INT16(-1, calibration.h5);
}

/** The last humidity value is one signed byte. */
static void test_h6_is_a_signed_byte(void)
{
    b[6] = 0xE2;

    bme280_calibration_from_bytes(a, b, &calibration);

    TEST_ASSERT_EQUAL_INT8(-30, calibration.h6);
}

/** Runs every test in this file. */
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_low_byte_comes_first);
    RUN_TEST(test_unsigned_value_stays_positive);
    RUN_TEST(test_signed_value_can_be_negative);

    RUN_TEST(test_temperature_values);
    RUN_TEST(test_pressure_values);

    RUN_TEST(test_h1_is_the_last_byte_of_the_first_run);
    RUN_TEST(test_h2_and_h3);
    RUN_TEST(test_h4_and_h5_share_a_byte);
    RUN_TEST(test_h4_and_h5_can_be_negative);
    RUN_TEST(test_h6_is_a_signed_byte);

    return UNITY_END();
}
