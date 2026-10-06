#ifndef BME280_CALIBRATION_H
#define BME280_CALIBRATION_H

#include <stdint.h>

// The calibration values are in two runs of the sensor's registers (BME280 datasheet section 4.2.2)
#define BME280_CALIBRATION_A_LENGTH 26  // 0x88 to 0xA1: temperature, pressure and the first humidity value
#define BME280_CALIBRATION_B_LENGTH 7   // 0xE1 to 0xE7: the rest of the humidity values

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
 * @brief Joins the sensor's calibration bytes into its calibration values.
 *
 * @param a           The BME280_CALIBRATION_A_LENGTH bytes of registers 0x88 to 0xA1
 * @param b           The BME280_CALIBRATION_B_LENGTH bytes of registers 0xE1 to 0xE7
 * @param calibration Filled in with the values
 */
void bme280_calibration_from_bytes(const uint8_t *a, const uint8_t *b, bme280_calibration_t *calibration);

#endif
