#ifndef SENSOR_TIMER_H
#define SENSOR_TIMER_H

#include <stdbool.h>
#include <stdint.h>

#include "reading.h"

/** The times sensor_poll() works from. Kept by the caller, in its app_state_t. */
typedef struct {
    uint32_t last_read_ms;      // the time the last reading was taken
    uint32_t read_interval_ms;  // how often a reading is taken: starts as SENSOR_READ_INTERVAL_MS
} sensor_timer_t;

/**
 * @brief Takes a measurement and writes it to the debug output, once the read interval has passed.
 *
 * Call on every pass of the main loop. Does not wait when it is not time yet; taking the
 * measurement itself takes about 10 ms. Reads no clock: it is handed the time.
 *
 * @param timer   The sensor's times: its last read time is set to the time now when a reading is
 *                taken
 * @param now_ms  The time now
 * @param reading Filled in with the measurement in real units; left as it was if none was taken
 * @return true only on the pass a reading was taken
 */
bool sensor_poll(sensor_timer_t *timer, uint32_t now_ms, reading_t *reading);

#endif
