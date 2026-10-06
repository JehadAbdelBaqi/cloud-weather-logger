#ifndef BME280_RAW_H
#define BME280_RAW_H

#include <stdint.h>

// How many bytes one measurement takes in the sensor's registers: pressure (3), temperature (3),
// humidity (2)
#define BME280_RAW_LENGTH 8

/** One measurement as the sensor gives it: raw values, not yet in real units. */
typedef struct {
    uint32_t pressure;     // 20 bits
    uint32_t temperature;  // 20 bits
    uint16_t humidity;     // 16 bits
} bme280_raw_t;

/**
 * @brief Joins the bytes of one measurement into its raw values.
 *
 * @param data The BME280_RAW_LENGTH bytes of the sensor's data registers, in register order
 * @param raw  Filled in with the raw pressure, temperature and humidity
 */
void bme280_raw_from_bytes(const uint8_t *data, bme280_raw_t *raw);

#endif
