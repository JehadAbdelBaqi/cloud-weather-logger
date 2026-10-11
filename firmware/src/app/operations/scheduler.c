#include "scheduler.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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
 * @param now_ms                 The time now
 * @param sent_ms                The time the message being answered went out
 * @param current_state          The state the program is in
 * @param awaiting_handshake_ack The flag for a handshake that is out
 * @param awaiting_reading_ack   The flag for a reading that is out
 * @param awaiting_reply_ack     The flag for a reply that is out
 */
void poll_uart(uint32_t now_ms, uint32_t sent_ms, state_key_t *current_state,
               bool *awaiting_handshake_ack, bool *awaiting_reading_ack, bool *awaiting_reply_ack)
{
    const char *text = link_read_line();

    if (text != NULL) {
        call_route(incoming_routes, text[0], text);
        return;
    }
    if (time_interval_passed(now_ms, sent_ms, HANDSHAKE_QUIET_LIMIT_MS)) {
        *current_state = STATE_NO_RESPONSE;
        *awaiting_handshake_ack = false;
        *awaiting_reading_ack = false;
        *awaiting_reply_ack = false;
    }
}
