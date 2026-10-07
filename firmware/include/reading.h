#ifndef READING_H
#define READING_H

#include <stdint.h>

/** One measurement in real units, with the scales of the protocol's "W" line. */
typedef struct {
    int32_t temperature;  // hundredths of a degree Celsius
    uint32_t pressure;    // pascals
    uint32_t humidity;    // hundredths of a percent of relative humidity
} reading_t;

#endif
