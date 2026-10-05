#ifndef ROUTER_H
#define ROUTER_H

/**
 * @brief Deals with a line that has arrived from the bridge, if one is complete.
 *
 * A handshake line goes to the handshake; any other line is written to the debug output.
 * Call on every pass of the main loop. Does not wait.
 */
void router_poll(void);

#endif
