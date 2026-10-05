#ifndef HANDSHAKE_H
#define HANDSHAKE_H

#include <stdbool.h>

/**
 * @brief Starts the handshake with the bridge: from now on the connection counts as not made.
 *
 * Call once, after link_init() and debug_init(), then call handshake_poll() on every pass of
 * the main loop.
 */
void handshake_init(void);

/**
 * @brief Sends "H,request" to the bridge when one is due. Does not wait.
 *
 * Until the bridge answers, a request goes out every HANDSHAKE_REQUEST_INTERVAL_MS, for as long as
 * it takes. Once the connection is made, nothing is sent while lines keep arriving from the bridge;
 * after HANDSHAKE_QUIET_LIMIT_MS with none, requests go out again at the same interval, and after
 * HANDSHAKE_MISSED_LIMIT in a row go unanswered the connection counts as lost.
 */
void handshake_poll(void);

/**
 * @brief Takes note of a line from the bridge, and deals with it if it is a handshake line.
 *
 * Any line shows the bridge is still there. "H,request" is answered with "H,ack"; either of the
 * two makes the connection. Call it with every line received.
 *
 * @param line The line, ending in '\0'
 * @return true if the line was a handshake line and needs nothing more done with it
 */
bool handshake_handle_line(const char *line);

/**
 * @brief Says whether the connection to the bridge is made.
 *
 * @return true once the bridge has been heard from, false before that and after it is lost
 */
bool handshake_connected(void);

#endif
