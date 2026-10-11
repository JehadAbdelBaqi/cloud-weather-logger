#include "sensor.h"

#include <stdbool.h>
#include <stdint.h>

#include "bme280_driver.h"
#include "bme280_calibration.h"
#include "bme280_compensation.h"
#include "board.h"
#include "debug.h"
#include "reading.h"
#include "sensor_debug.h"

// The sensor's calibration values, read once at start-up
static bme280_calibration_t calibration;
static bool calibration_read = false;

/**
 * @brief Finds the BME280 and writes its address and chip ID to the debug output.
 */
static void find_sensor(void)
{
    uint8_t chip_id = 0;

    if (!bme280_init(board_sensor_bus())) {
        debug_print("BME280 not found\r\n");
        return;
    }
    if (!bme280_read_chip_id(&chip_id)) {
        debug_print("BME280 found, but its chip ID could not be read\r\n");
        return;
    }

    sensor_debug_show_chip(bme280_address(), chip_id);
}

/**
 * @brief Reads the sensor's calibration values, keeps them for later readings and writes them to
 *        the debug output.
 */
static void load_calibration(void)
{
    bme280_calibration_t c;

    if (!bme280_read_calibration(&c)) {
        debug_print("calibration: could not be read\r\n");
        return;
    }
    calibration = c;
    calibration_read = true;

    sensor_debug_show_calibration(&calibration);
}

/**
 * @brief Turns a measurement's raw values into real units.
 *
 * @param raw     The measurement
 * @param reading Filled in with the temperature, pressure and humidity
 */
static void compensate(const bme280_raw_t *raw, reading_t *reading)
{
    int32_t t_fine = 0;

    reading->temperature = bme280_compensate_temperature(raw->temperature, &calibration, &t_fine);
    reading->pressure = bme280_compensate_pressure(raw->pressure, &calibration, t_fine);
    reading->humidity = bme280_compensate_humidity(raw->humidity, &calibration, t_fine);
}

void sensor_init(void)
{
    find_sensor();
    load_calibration();
}

bool sensor_read_now(reading_t *reading)
{
    bme280_raw_t raw;

    if (!bme280_read_raw(&raw)) {
        debug_print("raw: the sensor could not be read\r\n");
        return false;
    }

    sensor_debug_show_raw(&raw);
    if (!calibration_read) {
        return false;  // no real units without the calibration values
    }

    compensate(&raw, reading);
    sensor_debug_show_reading(reading);
    return true;
}
