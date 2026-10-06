#ifndef BME280_DRIVER_H
#define BME280_DRIVER_H

#include <stdbool.h>
#include <stdint.h>

#include "bme280_calibration.h"
#include "bme280_raw.h"
#include "i2c_driver.h"

/**
 * @brief Sets up the I2C bus the BME280 is on and finds the sensor.
 *
 * The sensor answers at one of two addresses, depending on how its address pin is tied;
 * both are tried.
 *
 * @param config Which I2C bus the sensor is on, its pins and its speed
 * @return true if the bus was set up and a device answered at one of the two addresses
 */
bool bme280_init(const i2c_config_t *config);

/**
 * @brief Gives the address the sensor was found at.
 *
 * @return The 7-bit address, or 0 if bme280_init() did not find the sensor
 */
uint8_t bme280_address(void);

/**
 * @brief Reads the sensor's chip ID, which is 0x60 for a BME280.
 *
 * @param chip_id Filled in with the ID read
 * @return true if the ID was read
 */
bool bme280_read_chip_id(uint8_t *chip_id);

/**
 * @brief Reads the sensor's calibration values. They never change, so once is enough.
 *
 * Call after bme280_init() has found the sensor.
 *
 * @param calibration Filled in with the values
 * @return true if they were read
 */
bool bme280_read_calibration(bme280_calibration_t *calibration);

/**
 * @brief Takes one measurement and reads its raw values.
 *
 * Asks the sensor for a single measurement of all three, waits until it is done (about 10 ms),
 * then reads the result.
 *
 * @param raw Filled in with the raw pressure, temperature and humidity
 * @return true if the measurement was taken and read
 */
bool bme280_read_raw(bme280_raw_t *raw);

#endif
