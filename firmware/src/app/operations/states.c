#include "states.h"

#include "handlers.h"
#include "reading_store.h"

/**
 * @brief The state when everything is answering: sends the oldest stored reading, if there is one.
 */
void state_ok(void)
{
    stored_reading_t stored;

    if (reading_store_oldest(&stored)) {
        handle_reading(&stored);
    }
}

/**
 * @brief The state when a message went out and no response came back in time: sends a handshake.
 */
void state_no_response(void)
{
    handle_handshake();
}
