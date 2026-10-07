#ifndef BRIDGE_CONNECTION_H
#define BRIDGE_CONNECTION_H

#include <stdbool.h>

// The connection to the bridge: whether it is made. Knows nothing about any line type.
// Anything that sends a line checks here first.

/**
 * @brief Records whether the connection to the bridge is made, and says so on the debug output.
 *
 * Writes only when the state changes.
 *
 * @param now_connected true once the bridge has answered, false when it is lost
 */
void bridge_connection_set_connected(bool now_connected);

/**
 * @brief Says whether the connection to the bridge is made.
 *
 * @return true once the bridge has answered, false before that and after it is lost
 */
bool bridge_connection_is_connected(void);

#endif
