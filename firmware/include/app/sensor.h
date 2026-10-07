#ifndef SENSOR_H
#define SENSOR_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Finds the BME280 and reads its calibration values, writing both to the debug output.
 *
 * Call once, after debug_init().
 */
void sensor_init(void);

/**
 * @brief Takes a measurement, writes it to the debug output and sends it to the bridge, once the
 *        read interval has passed.
 *
 * Call on every pass of the main loop. Does not wait when it is not time yet; taking the
 * measurement itself takes about 10 ms.
 */
void sensor_poll(void);

/**
 * @brief Takes a measurement now, writes it to the debug output and sends it to the bridge.
 *
 * Takes about 10 ms. The timed readings carry on as before.
 *
 * @return true if the reading was taken and sent
 */
bool sensor_read_now(void);

/**
 * @brief Sets how often sensor_poll() takes a reading.
 *
 * @param seconds The interval, from SENSOR_READ_INTERVAL_MIN_S to SENSOR_READ_INTERVAL_MAX_S
 * @return true if the interval was set, false if it is outside those limits
 */
bool sensor_set_interval(uint32_t seconds);

#endif
