#include "incoming.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "line.h"
#include "link.h"
#include "protocol.h"
#include "reading_store.h"

/**
 * @brief Handles the status of a response: sets the state from it.
 *
 * The state is ok for an ok status, and no response for anything else.
 *
 * @param status        The status of the response received, e.g. PROTOCOL_REPLY_OK
 * @param current_state The state the program is in: set from the status
 */
void handle_status(const char *status, state_key_t *current_state)
{
    if (strcmp(status, PROTOCOL_REPLY_OK) != 0) {
        *current_state = STATE_NO_RESPONSE;
        return;
    }
    *current_state = STATE_OK;
}

/**
 * @brief Reads a response coming in into its three fields: L,<letter>,<first field>,<status>.
 *
 * @param text         The response received
 * @param awaiting_ack The flag for the message this is the response to: set to false, as its
 *                     answer has arrived
 * @param response     Filled in with the three fields
 * @return true if the response had all three fields
 */
bool read_response(const char *text, bool *awaiting_ack, response_t *response)
{
    *awaiting_ack = false;

    if (!line_copy_field(text, 0, response->letter, sizeof(response->letter))) {
        return false;
    }
    if (!line_copy_field(text, 1, response->first_field, sizeof(response->first_field))) {
        return false;
    }
    if (!line_copy_field(text, 2, response->status, sizeof(response->status))) {
        return false;
    }
    return true;
}

/**
 * @brief Removes an answered reading from the store, if the state is ok.
 *
 * Run after handle_status(), which has set the state from this response's status. Does nothing
 * when the state is not ok: the reading stays in the store.
 *
 * @param seq_text      The first field of the response: the reading's sequence number, as text
 * @param current_state The state the program is in
 */
void remove_reading_if_ok(const char *seq_text, state_key_t current_state)
{
    uint32_t seq = 0;

    if (current_state != STATE_OK) {
        return;
    }
    if (!line_read_number(seq_text, &seq)) {
        return;
    }
    reading_store_remove(seq);
}

/**
 * @brief Routes a response by its second letter: the letter of the message it answers.
 *
 * A response with no second letter is ignored.
 *
 * @param text The response received: L,<letter>,<first field>,<status>
 */
void response_router(const char *text)
{
    char letter[2];

    if (!line_copy_field(text, 0, letter, sizeof(letter))) {
        return;
    }
    response_routes[letter[0]](text);
}
