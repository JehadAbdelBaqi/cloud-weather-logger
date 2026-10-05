#ifndef READING_H
#define READING_H

#include <stdbool.h>
#include <stdint.h>

/** One measurement in real units, with the scales of the protocol's "W" line. */
typedef struct {
    int32_t temperature;  // hundredths of a degree Celsius
    uint32_t pressure;    // pascals
    uint32_t humidity;    // hundredths of a percent of relative humidity
} reading_t;

/**
 * @brief Sends a reading to the bridge as a "W" line: W,<seq>,<epoch>,<t>,<p>,<rh>.
 *
 * Gives the reading the next sequence number, which starts at 1 at power-on. Nothing is sent
 * while the connection to the bridge is not made, and no sequence number is used.
 *
 * @param reading The reading
 * @return true if the line was sent
 */
bool reading_send(const reading_t *reading);

#endif
