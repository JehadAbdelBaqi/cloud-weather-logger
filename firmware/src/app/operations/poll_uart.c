#include "poll_uart.h"

#include <stdbool.h>
#include <stddef.h>

#include "app_state.h"
#include "config.h"
#include "link.h"
#include "routing.h"
#include "time_interval.h"

/**
 * @brief Polls the UART for a message from the bridge and routes it by its letter.
 *
 * A message whose letter has no route is ignored.
 *
 * Does not wait: it returns straight away when no whole message has arrived yet. While nothing
 * has arrived, the state becomes no response once the quiet limit has passed since the last
 * message went out, and every awaiting flag is set to false, so messages can be sent again. That
 * holds whether or not an answer was being waited for: a link that has been quiet for the limit
 * is checked with a handshake.
 *
 * @param state The program's running state: the time now, the time the last message went out, the
 *              state the program is in and the awaiting flags
 */
void poll_uart(app_state_t *state)
{
    const char *text = link_read_line();

    if (text != NULL) {
        call_route(incoming_routes, text[0], state, text);
        return;
    }
    if (time_interval_passed(state->now_ms, state->sent_ms, HANDSHAKE_QUIET_LIMIT_MS)) {
        state->current_state = STATE_NO_RESPONSE;
        state->awaiting_handshake_ack = false;
        state->awaiting_reading_ack = false;
        state->awaiting_reply_ack = false;
    }
}
