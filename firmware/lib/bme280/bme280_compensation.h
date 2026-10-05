#ifndef BME280_COMPENSATION_H
#define BME280_COMPENSATION_H

#include <stdint.h>

#include "bme280_calibration.h"

// Turns the sensor's raw values into real units, with the integer formulas of the BME280
// datasheet, section 4.2.3. Maths only: nothing here talks to the sensor.

/**
 * @brief Turns a raw temperature into hundredths of a degree Celsius.
 *
 * @param raw_temperature The raw temperature from bme280_read_raw()
 * @param calibration     The sensor's calibration values
 * @param t_fine          Filled in with the fine temperature value that the pressure and humidity
 *                        formulas need
 * @return The temperature in hundredths of a degree Celsius: 2403 is 24.03 degrees
 */
int32_t bme280_compensate_temperature(uint32_t raw_temperature,
                                      const bme280_calibration_t *calibration, int32_t *t_fine);

/**
 * @brief Turns a raw pressure into pascals.
 *
 * The maths is 64 bits wide. The part of a pascal after the decimal point is dropped.
 *
 * @param raw_pressure The raw pressure from bme280_read_raw()
 * @param calibration  The sensor's calibration values
 * @param t_fine       The fine temperature value from bme280_compensate_temperature(), for the
 *                     same measurement
 * @return The pressure in pascals: 100653 is 1006.53 hPa. 0 if the calibration values cannot
 *         be used
 */
uint32_t bme280_compensate_pressure(uint32_t raw_pressure, const bme280_calibration_t *calibration,
                                    int32_t t_fine);

/**
 * @brief Turns a raw humidity into hundredths of a percent of relative humidity.
 *
 * @param raw_humidity The raw humidity from bme280_read_raw()
 * @param calibration  The sensor's calibration values
 * @param t_fine       The fine temperature value from bme280_compensate_temperature(), for the
 *                     same measurement
 * @return The relative humidity in hundredths of a percent, from 0 to 10000: 4633 is 46.33 %
 */
uint32_t bme280_compensate_humidity(uint16_t raw_humidity, const bme280_calibration_t *calibration,
                                    int32_t t_fine);

#endif
