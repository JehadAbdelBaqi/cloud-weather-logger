#include "sensor.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "stm32f4xx_hal.h"

#include "bme280_driver.h"
#include "bme280_calibration.h"
#include "bme280_compensation.h"
#include "board.h"
#include "config.h"
#include "debug.h"
#include "message_handlers.h"
#include "reading.h"
#include "time_interval.h"

// The sensor's calibration values, read once at start-up
static bme280_calibration_t calibration;
static bool calibration_read = false;

/**
 * @brief Finds the BME280 and writes its address and chip ID to the debug output.
 */
static void show_sensor(void)
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

    debug_print("BME280 at address ");
    debug_print_hex(bme280_address());
    debug_print(", chip ID ");
    debug_print_hex(chip_id);
    debug_print("\r\n");
}

/**
 * @brief Writes a list of numbers to the debug output on one line, after a label.
 *
 * @param label  Text to start the line with
 * @param values The numbers
 * @param count  How many numbers
 */
static void show_values(const char *label, const int32_t *values, size_t count)
{
    debug_print(label);
    for (size_t i = 0; i < count; i++) {
        debug_print(" ");
        debug_print_signed(values[i]);
    }
    debug_print("\r\n");
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

    const int32_t t[] = { c.t1, c.t2, c.t3 };
    const int32_t p[] = { c.p1, c.p2, c.p3, c.p4, c.p5, c.p6, c.p7, c.p8, c.p9 };
    const int32_t h[] = { c.h1, c.h2, c.h3, c.h4, c.h5, c.h6 };

    show_values("calibration T:", t, sizeof(t) / sizeof(t[0]));
    show_values("calibration P:", p, sizeof(p) / sizeof(p[0]));
    show_values("calibration H:", h, sizeof(h) / sizeof(h[0]));
}

/**
 * @brief Writes a measurement's raw values to the debug output.
 *
 * @param raw The measurement
 */
static void show_raw(const bme280_raw_t *raw)
{
    debug_print("raw: temperature ");
    debug_print_number(raw->temperature);
    debug_print(", pressure ");
    debug_print_number(raw->pressure);
    debug_print(", humidity ");
    debug_print_number(raw->humidity);
    debug_print("\r\n");
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

/**
 * @brief Writes a reading to the debug output.
 *
 * @param reading The reading
 */
static void show_reading(const reading_t *reading)
{
    debug_print("compensated: temperature ");
    debug_print_signed(reading->temperature);
    debug_print(" hundredths of a degree C, pressure ");
    debug_print_number(reading->pressure);
    debug_print(" Pa, humidity ");
    debug_print_number(reading->humidity);
    debug_print(" hundredths of a percent\r\n");
}

/**
 * @brief Sends a reading to the bridge, and says on the debug output if it could not be sent.
 *
 * @param reading The reading
 */
static void send_reading(const reading_t *reading)
{
    if (handle_reading(reading)) {
        return;
    }
    debug_print("reading not sent\r\n");
}

void sensor_init(void)
{
    show_sensor();
    load_calibration();
}

void sensor_poll(void)
{
    static uint32_t last_read_ms = 0;
    bme280_raw_t raw;

    if (!time_interval_passed(HAL_GetTick(), last_read_ms, SENSOR_READ_INTERVAL_MS)) {
        return;
    }
    last_read_ms = HAL_GetTick();

    if (!bme280_read_raw(&raw)) {
        debug_print("raw: the sensor could not be read\r\n");
        return;
    }

    show_raw(&raw);
    if (!calibration_read) {
        return;  // no real units without the calibration values
    }

    reading_t reading;
    compensate(&raw, &reading);
    show_reading(&reading);
    send_reading(&reading);
}
