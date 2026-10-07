#ifndef MESSAGE_HANDLERS_H
#define MESSAGE_HANDLERS_H

#include <stdbool.h>

#include "reading.h"

// One complete handler for each type of line: everything that is done with it.

/**
 * @brief Deals with a handshake line received from the bridge.
 *
 * A request from the bridge is answered with an ack, and the Nucleo asks straight back. Only an
 * ack from the bridge makes the connection. Any other handshake line is ignored.
 *
 * @param line The line received, starting with PROTOCOL_LETTER_HANDSHAKE
 */
void handle_handshake(const char *line);

/**
 * @brief Deals with a command received from the server: C,<id>,<command>.
 *
 * The command is carried out and answered with a reply line that carries the same ID. A command
 * with no ID cannot be answered, so it is ignored.
 *
 * @param line The line received, starting with PROTOCOL_LETTER_COMMAND
 */
void handle_command(const char *line);

/**
 * @brief Sends a reading to the bridge as a reading line: W,<seq>,<epoch>,<t>,<p>,<rh>.
 *
 * Gives the reading the next sequence number, which starts at 1 at power-on. Nothing is sent
 * while the connection to the bridge is not made, and no sequence number is used.
 *
 * @param reading The reading
 * @return true if the line was sent
 */
bool handle_reading(const reading_t *reading);

#endif
