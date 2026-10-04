#ifndef LINK_H
#define LINK_H

#include <stdbool.h>

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

#endif
