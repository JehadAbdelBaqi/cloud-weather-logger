#include "incoming.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "format_number.h"
#include "link_framing.h"
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

    if (!get_field(text, 0, response->letter, sizeof(response->letter))) {
        return false;
    }
    if (!get_field(text, 1, response->first_field, sizeof(response->first_field))) {
        return false;
    }
    if (!get_field(text, 2, response->status, sizeof(response->status))) {
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
    if (!format_read_number(seq_text, &seq)) {
        return;
    }
    reading_store_remove(seq);
}

/**
 * @brief Gets one field out of a message, as text of its own.
 *
 * @param text  The message, e.g. L,W,12,ok
 * @param index Which field: 0 is the first one after the letter
 * @param field Filled in with the field, ending in '\0'
 * @param size  The size of the field buffer
 * @return true if the field was copied; false if the message has no such field or it does not fit
 */
bool get_field(const char *text, size_t index, char *field, size_t size)
{
    const char separator[] = { PROTOCOL_FIELD_SEPARATOR, '\0' };
    const char *start = text;

    // Step past the letter and each field before the one wanted: one separator each
    for (size_t i = 0; i <= index; i++) {
        start = strchr(start, PROTOCOL_FIELD_SEPARATOR);
        if (start == NULL) {
            return false;
        }
        start++;
    }

    // The field runs up to the next separator, or to the end of the message
    size_t length = strcspn(start, separator);

    if (length >= size) {
        return false;
    }
    memcpy(field, start, length);
    field[length] = '\0';
    return true;
}
