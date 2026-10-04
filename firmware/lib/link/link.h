#ifndef LINK_H
#define LINK_H

#include <stdbool.h>

// Longest line in either direction, in characters, not counting the '\n' (same as the bridge)
#define LINK_LINE_MAX_LEN 127

/**
 * @brief Sets up the link to the Wi-Fi bridge, which carries the protocol lines.
 *
 * Call once before any other link_ function.
 *
 * @return true if the link's UART was set up
 */
bool link_init(void);

/**
 * @brief Sends one protocol line to the bridge.
 *
 * @param line The line, without a line ending; a '\n' is added after it
 * @return true if the whole line was sent, false if it timed out
 */
bool link_send_line(const char *line);

/**
 * @brief Takes the bytes that have arrived from the bridge and returns a line once one is complete.
 *
 * Does not wait. Call it often, every pass of the main loop: bytes are taken one at a time and one
 * not taken before the next arrives is lost. A line ends in '\n' ('\r\n' also accepted). An empty
 * line, or one longer than LINK_LINE_MAX_LEN, is dropped.
 *
 * @return The line without its line ending, valid until the next call; NULL if no line is complete yet
 */
const char *link_read_line(void);

#endif
