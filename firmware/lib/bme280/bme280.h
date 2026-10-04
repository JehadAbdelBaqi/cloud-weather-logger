#ifndef BME280_H
#define BME280_H

#include <stdbool.h>
#include <stdint.h>

/** One measurement as the sensor gives it: raw values, not yet in real units. */
typedef struct {
    uint32_t pressure;     // 20 bits
    uint32_t temperature;  // 20 bits
    uint16_t humidity;     // 16 bits
} bme280_raw_t;

/**
 * The calibration values stored in each sensor at the factory, which turn raw values into
 * real units. Names and types as in the BME280 datasheet, Table 16.
 */
typedef struct {
    uint16_t t1;
    int16_t t2;
    int16_t t3;
    uint16_t p1;
    int16_t p2;
    int16_t p3;
    int16_t p4;
    int16_t p5;
    int16_t p6;
    int16_t p7;
    int16_t p8;
    int16_t p9;
    uint8_t h1;
    int16_t h2;
    uint8_t h3;
    int16_t h4;
    int16_t h5;
    int8_t h6;
} bme280_calibration_t;

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

/**
 * @brief Reads the sensor's calibration values. They never change, so once is enough.
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
