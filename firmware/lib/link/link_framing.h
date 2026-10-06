#ifndef LINK_FRAMING_H
#define LINK_FRAMING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Longest line in either direction, in characters, not counting the '\n' (same as the bridge)
#define LINK_LINE_MAX_LEN 127

/** A line being collected from received bytes. Start with every field zero. */
typedef struct {
    char line[LINK_LINE_MAX_LEN + 1];  // room for the ending '\0'
    size_t length;
    bool too_long;
} link_framing_t;

/**
 * @brief Adds one received byte to the line being collected.
 *
 * A line ends in '\n'. A '\r' is ignored, so '\r\n' ends one too. An empty line, or one longer
 * than LINK_LINE_MAX_LEN, is dropped.
 *
 * @param framing The line being collected
 * @param byte    The byte
 * @return true if this byte completed a usable line: it is in framing->line, without its line
 *         ending, until the next call
 */
bool link_framing_take_byte(link_framing_t *framing, uint8_t byte);

#endif
