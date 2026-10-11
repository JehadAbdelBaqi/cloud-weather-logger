#ifndef STATES_H
#define STATES_H

// The states the Nucleo can be in, with one function for each, and the statuses a response can
// carry, with the state each one leads to.

/** The statuses a response can carry. */
typedef enum {
    STATUS_OK,  // the line went through; for the handshake, the bridge can reach upstream
} status_t;

/** The states. */
typedef enum {
    STATE_NORMAL,       // the bridge is connected and upstream is up: read, store and send
    STATE_BRIDGE_DOWN,  // nothing back from the bridge: read, store and ask for a handshake
} state_t;

/**
 * @brief Gives the state a status leads to.
 *
 * @param status The status of a response
 * @return The state for that status
 */
state_t state_for_status(status_t status);

/**
 * @brief Normal behaviour: the bridge is connected and upstream is up.
 *
 * Sends what is in the store, takes a reading when one is due and puts it into the store, and
 * sends the oldest stored reading. Does not wait.
 */
void state_normal(void);

/**
 * @brief The bridge is down: the handshake request has had no response.
 *
 * Takes a reading when one is due and puts it into the store, and asks for a handshake again
 * each time the request interval has passed. Nothing else is sent. Does not wait.
 */
void state_bridge_down(void);

#endif
