#include "states.h"

#include "app_state.h"
#include "message_handlers.h"
#include "reading_store.h"
#include "routing.h"

/**
 * @brief The state when everything is answering: sends the oldest stored reading, if there is one.
 *
 * @param state The program's running state
 */
void state_ok(app_state_t *state)
{
    stored_reading_t stored;

    if (reading_store_oldest(&stored)) {
        handle_reading(state, &stored);
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
