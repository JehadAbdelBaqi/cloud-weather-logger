#ifndef SENSOR_H
#define SENSOR_H

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

#endif
