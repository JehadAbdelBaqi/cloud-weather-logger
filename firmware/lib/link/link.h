#ifndef LINK_H
#define LINK_H

#include <stdbool.h>

#include "link_buffer.h"
#include "link_framing.h"
#include "uart_driver.h"

/**
 * @brief Sets up the link to the Wi-Fi bridge, which carries the protocol lines.
 *
 * Call once before any other link_ function. Switches on the receive interrupt of the link's
 * UART: its interrupt function has to call link_byte_arrived().
 *
 * @param config Which UART the bridge is on, its pins and its speed
 * @return true if the link's UART was set up
 */
bool link_init(const uart_config_t *config);

/**
 * @brief Sends one protocol line to the bridge.
 *
 * @param line The line, without a line ending; a '\n' is added after it
 * @return true if the whole line was sent, false if it timed out
 */
bool link_send_line(const char *line);

/**
 * @brief Takes the byte that has just arrived from the bridge and keeps it until it is read.
 *
 * Call it from the interrupt function of the link's UART, and from nowhere else. A byte that
 * arrives while LINK_BUFFER_SIZE bytes are already waiting to be read is dropped.
 */
void link_byte_arrived(void);

/**
 * @brief Takes the bytes that have arrived from the bridge and returns a line once one is complete.
 *
 * Does not wait. Call it on every pass of the main loop. The bytes are kept as they arrive, so a
 * pass that takes a while loses none, up to LINK_BUFFER_SIZE of them. A line ends in '\n'
 * ('\r\n' also accepted). An empty line, or one longer than LINK_LINE_MAX_LEN, is dropped.
 *
 * @return The line without its line ending, valid until the next call; NULL if no line is complete yet
 */
const char *link_read_line(void);

#endif
