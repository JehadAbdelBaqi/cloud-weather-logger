#include "states.h"

#include "app_state.h"
#include "message_handlers.h"
#include "reading_store.h"
#include "reply_store.h"
#include "routing.h"

/**
 * @brief The state when everything is answering: sends the oldest stored reply, if there is one,
 *        and otherwise the oldest stored reading.
 *
 * A reply goes ahead of the readings that are waiting: someone is waiting for it.
 *
 * @param state The program's running state
 */
void state_ok(app_state_t *state)
{
    stored_reply_t reply;
    stored_reading_t reading;

    if (reply_store_oldest(&reply)) {
        handle_reply(state, &reply);
        return;
    }
    if (reading_store_oldest(&reading)) {
        handle_reading(state, &reading);
    }
}

/**
 * @brief The state when a message went out and no response came back in time: sends a handshake.
 *
 * @param state The program's running state
 */
void state_no_response(app_state_t *state)
{
    handle_handshake(state);
}

/**
 * @brief Runs the state the program is in: the function mapped to the current state.
 *
 * @param state The program's running state
 */
void state_setter(app_state_t *state)
{
    state_routing[state->current_state](state);
}
