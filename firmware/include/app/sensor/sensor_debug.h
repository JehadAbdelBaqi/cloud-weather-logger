#ifndef SENSOR_DEBUG_H
#define SENSOR_DEBUG_H

#include <stdint.h>

#include "bme280_calibration.h"
#include "bme280_raw.h"
#include "reading.h"

/**
 * @brief Writes the sensor's address and chip ID to the debug output.
 *
 * @param address The sensor's I2C address
 * @param chip_id The chip ID read from the sensor
 */
void sensor_debug_show_chip(uint8_t address, uint8_t chip_id);

/**
 * @brief Writes the sensor's calibration values to the debug output, one line for each of
 *        temperature, pressure and humidity.
 *
 * @param calibration The calibration values
 */
void sensor_debug_show_calibration(const bme280_calibration_t *calibration);

/**
 * @brief Writes a measurement's raw values to the debug output.
 *
 * @param raw The measurement
 */
void sensor_debug_show_raw(const bme280_raw_t *raw);

/**
 * @brief Writes a reading to the debug output.
 *
 * @param reading The reading
 */
void sensor_debug_show_reading(const reading_t *reading);

#endif
