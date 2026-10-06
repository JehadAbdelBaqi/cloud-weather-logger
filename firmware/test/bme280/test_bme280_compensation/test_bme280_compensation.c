#include <stdint.h>

#include <unity.h>

#include "bme280_compensation.h"

// One sensor's calibration values, used by every test unless it says otherwise. The results
// expected from them were worked out separately, with the floating-point form of the formulas.
static const bme280_calibration_t example = {
    .t1 = 27504, .t2 = 26435, .t3 = -1000,
    .p1 = 36477, .p2 = -10685, .p3 = 3024, .p4 = 2855, .p5 = 140,
    .p6 = -7, .p7 = 15500, .p8 = -14600, .p9 = 6000,
    .h1 = 75, .h2 = 362, .h3 = 0, .h4 = 315, .h5 = 50, .h6 = 30,
};

// The fine temperature value the example gives at 25.08 degrees C, for the pressure and
// humidity tests
#define EXAMPLE_T_FINE 128422

/** Runs before each test. Nothing to set up. */
void setUp(void)
{
}

/** Runs after each test. Nothing to clear up. */
void tearDown(void)
{
}

/** A raw value at room temperature comes out in hundredths of a degree: 25.08 degrees C. */
static void test_temperature_at_room_temperature(void)
{
    int32_t t_fine = 0;

    TEST_ASSERT_EQUAL_INT32(2508, bme280_compensate_temperature(519888, &example, &t_fine));
}

/** The fine temperature value is handed back for the pressure and humidity formulas. */
static void test_temperature_hands_back_t_fine(void)
{
    int32_t t_fine = 0;

    bme280_compensate_temperature(519888, &example, &t_fine);

    TEST_ASSERT_EQUAL_INT32(EXAMPLE_T_FINE, t_fine);
}

/** A temperature below freezing is negative: -7.91 degrees C. */
static void test_temperature_below_freezing(void)
{
    int32_t t_fine = 0;

    TEST_ASSERT_EQUAL_INT32(-791, bme280_compensate_temperature(415000, &example, &t_fine));
}

/** A high raw value gives a high temperature: 50.11 degrees C. */
static void test_temperature_hot(void)
{
    int32_t t_fine = 0;

    TEST_ASSERT_EQUAL_INT32(5011, bme280_compensate_temperature(600000, &example, &t_fine));
}

/** Another sensor's calibration values give another result for much the same raw value. */
static void test_temperature_uses_the_calibration_values(void)
{
    const bme280_calibration_t other = { .t1 = 28292, .t2 = 26587, .t3 = 50 };
    int32_t t_fine = 0;

    TEST_ASSERT_EQUAL_INT32(2403, bme280_compensate_temperature(528500, &other, &t_fine));
}

/** A raw pressure comes out in whole pascals: 1006.53 hPa. */
static void test_pressure_near_sea_level(void)
{
    TEST_ASSERT_EQUAL_UINT32(100653, bme280_compensate_pressure(415148, &example, EXAMPLE_T_FINE));
}

/** The raw value runs the other way to the pressure: a higher raw value is a lower pressure. */
static void test_pressure_falls_as_the_raw_value_rises(void)
{
    TEST_ASSERT_EQUAL_UINT32(86043, bme280_compensate_pressure(500000, &example, EXAMPLE_T_FINE));
    TEST_ASSERT_EQUAL_UINT32(120597, bme280_compensate_pressure(300000, &example, EXAMPLE_T_FINE));
}

/** Calibration values that would divide by zero give 0, not a crash. */
static void test_pressure_is_zero_when_it_would_divide_by_zero(void)
{
    bme280_calibration_t unusable = example;
    unusable.p1 = 0;

    TEST_ASSERT_EQUAL_UINT32(0, bme280_compensate_pressure(415148, &unusable, EXAMPLE_T_FINE));
}

/** A raw humidity comes out in hundredths of a percent: 44.81 %. */
static void test_humidity_mid_range(void)
{
    TEST_ASSERT_EQUAL_UINT32(4481, bme280_compensate_humidity(28300, &example, EXAMPLE_T_FINE));
}

/** A higher raw value is a higher humidity: 54.28 %. */
static void test_humidity_rises_with_the_raw_value(void)
{
    TEST_ASSERT_EQUAL_UINT32(5428, bme280_compensate_humidity(30000, &example, EXAMPLE_T_FINE));
}

/** A raw value below the sensor's range gives 0 %, never a negative humidity. */
static void test_humidity_stops_at_zero(void)
{
    TEST_ASSERT_EQUAL_UINT32(0, bme280_compensate_humidity(20000, &example, EXAMPLE_T_FINE));
    TEST_ASSERT_EQUAL_UINT32(0, bme280_compensate_humidity(0, &example, EXAMPLE_T_FINE));
}

/** A raw value above the sensor's range gives 100 %, never more. */
static void test_humidity_stops_at_one_hundred_percent(void)
{
    TEST_ASSERT_EQUAL_UINT32(10000, bme280_compensate_humidity(40000, &example, EXAMPLE_T_FINE));
    TEST_ASSERT_EQUAL_UINT32(10000, bme280_compensate_humidity(UINT16_MAX, &example, EXAMPLE_T_FINE));
}

/** Runs every test in this file. */
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_temperature_at_room_temperature);
    RUN_TEST(test_temperature_hands_back_t_fine);
    RUN_TEST(test_temperature_below_freezing);
    RUN_TEST(test_temperature_hot);
    RUN_TEST(test_temperature_uses_the_calibration_values);

    RUN_TEST(test_pressure_near_sea_level);
    RUN_TEST(test_pressure_falls_as_the_raw_value_rises);
    RUN_TEST(test_pressure_is_zero_when_it_would_divide_by_zero);

    RUN_TEST(test_humidity_mid_range);
    RUN_TEST(test_humidity_rises_with_the_raw_value);
    RUN_TEST(test_humidity_stops_at_zero);
    RUN_TEST(test_humidity_stops_at_one_hundred_percent);

    return UNITY_END();
}
