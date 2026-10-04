#ifndef BME280_H
#define BME280_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Sets up the I2C bus the BME280 is on and finds the sensor.
 *
 * The sensor answers at one of two addresses, depending on how its address pin is tied;
 * both are tried.
 *
 * @return true if the bus was set up and a device answered at one of the two addresses
 */
bool bme280_init(void);

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

#endif
