#ifndef SENSOR_H
#define SENSOR_H

#include <stdbool.h>

#include "reading.h"

/**
 * @brief Finds the BME280 and reads its calibration values, writing both to the debug output.
 *
 * Call once, after debug_init().
 */
void sensor_init(void);

/**
 * @brief Takes a measurement now and writes it to the debug output.
 *
 * Takes about 10 ms. The timed readings carry on as before.
 *
 * @param reading Filled in with the measurement in real units; left as it was if none was taken
 * @return true if the reading was taken
 */
bool sensor_read_now(reading_t *reading);

#endif
