#include <stdint.h>

#include <unity.h>

#include "bme280_raw.h"

/** Runs before each test. Nothing to set up. */
void setUp(void)
{
}

/** Runs after each test. Nothing to clear up. */
void tearDown(void)
{
}

/** Pressure is the first three bytes: 8 bits, 8 bits, then the top 4 bits of the third. */
static void test_pressure_from_the_first_three_bytes(void)
{
    const uint8_t data[BME280_RAW_LENGTH] = { 0x12, 0x34, 0x56, 0, 0, 0, 0, 0 };
    bme280_raw_t raw;

    bme280_raw_from_bytes(data, &raw);

    TEST_ASSERT_EQUAL_HEX32(0x12345, raw.pressure);
}

/** Temperature is the next three bytes, joined the same way. */
static void test_temperature_from_the_next_three_bytes(void)
{
    const uint8_t data[BME280_RAW_LENGTH] = { 0, 0, 0, 0xAB, 0xCD, 0xEF, 0, 0 };
    bme280_raw_t raw;

    bme280_raw_from_bytes(data, &raw);

    TEST_ASSERT_EQUAL_HEX32(0xABCDE, raw.temperature);
}

/** Humidity is the last two bytes, high byte first. */
static void test_humidity_from_the_last_two_bytes(void)
{
    const uint8_t data[BME280_RAW_LENGTH] = { 0, 0, 0, 0, 0, 0, 0x6E, 0x8C };
    bme280_raw_t raw;

    bme280_raw_from_bytes(data, &raw);

    TEST_ASSERT_EQUAL_HEX16(0x6E8C, raw.humidity);
}

/** The low 4 bits of a third byte are not part of the value. */
static void test_low_bits_of_the_third_byte_are_dropped(void)
{
    const uint8_t data[BME280_RAW_LENGTH] = { 0, 0, 0x0F, 0, 0, 0x0F, 0, 0 };
    bme280_raw_t raw;

    bme280_raw_from_bytes(data, &raw);

    TEST_ASSERT_EQUAL_HEX32(0, raw.pressure);
    TEST_ASSERT_EQUAL_HEX32(0, raw.temperature);
}

/** One value's bytes do not leak into another value. */
static void test_values_do_not_overlap(void)
{
    const uint8_t data[BME280_RAW_LENGTH] = { 0xFF, 0xFF, 0xFF, 0, 0, 0, 0xFF, 0xFF };
    bme280_raw_t raw;

    bme280_raw_from_bytes(data, &raw);

    TEST_ASSERT_EQUAL_HEX32(0xFFFFF, raw.pressure);
    TEST_ASSERT_EQUAL_HEX32(0, raw.temperature);
    TEST_ASSERT_EQUAL_HEX16(0xFFFF, raw.humidity);
}

/** Every bit set gives the largest values: 20 bits, 20 bits and 16 bits. */
static void test_largest_values(void)
{
    const uint8_t data[BME280_RAW_LENGTH] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    bme280_raw_t raw;

    bme280_raw_from_bytes(data, &raw);

    TEST_ASSERT_EQUAL_HEX32(0xFFFFF, raw.pressure);
    TEST_ASSERT_EQUAL_HEX32(0xFFFFF, raw.temperature);
    TEST_ASSERT_EQUAL_HEX16(0xFFFF, raw.humidity);
}

/** Runs every test in this file. */
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_pressure_from_the_first_three_bytes);
    RUN_TEST(test_temperature_from_the_next_three_bytes);
    RUN_TEST(test_humidity_from_the_last_two_bytes);

    RUN_TEST(test_low_bits_of_the_third_byte_are_dropped);
    RUN_TEST(test_values_do_not_overlap);
    RUN_TEST(test_largest_values);

    return UNITY_END();
}
