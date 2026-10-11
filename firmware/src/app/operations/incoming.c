#include "incoming.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "format_number.h"
#include "link_framing.h"
#include "message_fields.h"
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
 * @param awaiting_ack The awaiting flag: set to false, as the answer to the message that was
 *                     out has arrived
 * @param response     Filled in with the three fields
 * @return true if the response had all three fields
 */
bool read_response(const char *text, bool *awaiting_ack, response_t *response)
{
    *awaiting_ack = false;

    if (!get_field(text, PROTOCOL_FIELD_SEPARATOR, 0, response->letter, sizeof(response->letter))) {
        return false;
    }
    if (!get_field(text, PROTOCOL_FIELD_SEPARATOR, 1, response->first_field, sizeof(response->first_field))) {
        return false;
    }
    if (!get_field(text, PROTOCOL_FIELD_SEPARATOR, 2, response->status, sizeof(response->status))) {
        return false;
    }
    return true;
}

/**
 * @brief Reads a command coming in into its three fields: C,<id>,<command>,<value>.
 *
 * A field that is missing after the ID is left empty: a command with nothing after its ID can still be
 * answered, and not every command has a value.
 *
 * @param text    The command received
 * @param command Filled in with the three fields
 * @return true if the command has an ID that fits; false if it has none or one too long, as there
 *         is then nothing to answer with
 */
bool read_command(const char *text, command_t *command)
{
    if (!get_field(text, PROTOCOL_FIELD_SEPARATOR, 0, command->id, sizeof(command->id))) {
        return false;
    }
    if (command->id[0] == '\0') {
        return false;
    }
    if (!get_field(text, PROTOCOL_FIELD_SEPARATOR, 1, command->command, sizeof(command->command))) {
        command->command[0] = '\0';
    }
    if (!get_field(text, PROTOCOL_FIELD_SEPARATOR, 2, command->value, sizeof(command->value))) {
        command->value[0] = '\0';
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
 * @brief Removes an answered reply from the store, if the state is ok.
 *
 * Run after handle_status(), which has set the state from this response's status. Does nothing
 * when the state is not ok: the reply stays in the store.
 *
 * @param id            The first field of the response: the ID of the command the reply answers
 * @param current_state The state the program is in
 */
void remove_reply_if_ok(const char *id, state_key_t current_state)
{
    if (current_state != STATE_OK) {
        return;
    }
    reply_store_remove(id);
}
