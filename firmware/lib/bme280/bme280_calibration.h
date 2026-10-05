#ifndef BME280_CALIBRATION_H
#define BME280_CALIBRATION_H

#include <stdbool.h>
#include <stdint.h>

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
 * @brief Reads the sensor's calibration values. They never change, so once is enough.
 *
 * Call after bme280_init() has found the sensor.
 *
 * @param calibration Filled in with the values
 * @return true if they were read
 */
bool bme280_read_calibration(bme280_calibration_t *calibration);

#endif
