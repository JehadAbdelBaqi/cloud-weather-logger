#ifndef SENSOR_TIMER_H
#define SENSOR_TIMER_H

#include <stdbool.h>
#include <stdint.h>

#include "reading.h"

/**
 * @brief Takes a measurement and writes it to the debug output, once the read interval has passed.
 *
 * Call on every pass of the main loop. Does not wait when it is not time yet; taking the
 * measurement itself takes about 10 ms.
 *
 * @param reading Filled in with the measurement in real units; left as it was if none was taken
 * @return true only on the pass a reading was taken
 */
bool sensor_poll(reading_t *reading);

/**
 * @brief Sets how often sensor_poll() takes a reading.
 *
 * @param seconds The interval, from SENSOR_READ_INTERVAL_MIN_S to SENSOR_READ_INTERVAL_MAX_S
 * @return true if the interval was set, false if it is outside those limits
 */
bool sensor_set_interval(uint32_t seconds);

#endif
