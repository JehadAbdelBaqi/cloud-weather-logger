#ifndef STATE_FUNCTIONS_H
#define STATE_FUNCTIONS_H

#include <stdint.h>

// The actions the states and the response handlers are built from. Each does one thing.

/**
 * @brief Takes a reading if one is due, and puts it into the store.
 */
void read_and_store(void);

/**
 * @brief Sends the oldest reading in the store to the bridge.
 */
void send_oldest_reading(void);

/**
 * @brief Sends the readings in the store, oldest first, until the store is empty.
 *
 * Not finished: the response to each reading is not handled inside the loop yet, so for now it
 * sends the oldest reading and stops.
 */
void send_stored_readings(void);

/**
 * @brief Asks the bridge whether it is there, with a handshake request, when one is due.
 *
 * A request is due at the start, and then each time the request interval has passed with no
 * response. Does not wait.
 */
void request_handshake(void);

/**
 * @brief The bridge has answered the handshake "ok": the connection to the bridge is made, and
 *        the Nucleo goes to the state that status leads to.
 */
void connect_bridge(void);

/**
 * @brief A reading has been answered "ok": it is removed from the store, and the Nucleo goes to
 *        the state that status leads to.
 *
 * @param seq The sequence number of the reading
 */
void confirm_reading(uint32_t seq);

#endif
