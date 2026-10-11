#include "outgoing.h"

#include <stdbool.h>
#include <stdint.h>

#include "link.h"

#include "message_build.h"
#include "protocol.h"

/**
 * @brief Sends a message going out: constructs it and sends it.
 *
 * Sends nothing while an earlier message, of any kind, is still waiting for its answer: one
 * message is out at a time.
 *
 * @param message      The message to send
 * @param awaiting_ack The awaiting flag: true while a message is out and not answered yet. Set
 *                     to true once the message is sent
 * @param now_ms       The time now
 * @param sent_ms      Set to the time now once the message is sent
 */
void send_outgoing_message(message_t message, bool *awaiting_ack, uint32_t now_ms, uint32_t *sent_ms)
{
    if (*awaiting_ack) {
        return;
    }
    char text[LINK_LINE_MAX_LEN + 1];

    construct_message(text, PROTOCOL_FIELD_SEPARATOR, message);
    link_send_line(text);
    *awaiting_ack = true;
    *sent_ms = now_ms;
}
