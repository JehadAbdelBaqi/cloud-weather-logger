#include "message_handlers.h"

#include <stdbool.h>
#include <stdint.h>

#include "app_state.h"
#include "incoming.h"
#include "outgoing.h"
#include "protocol.h"
#include "reading_store.h"

#include "messages.h"
#include "reading_to_message.h"

/**
 * @brief Handles the handshake: sends the handshake message.
 *
 * @param state The program's running state
 */
void handle_handshake(app_state_t *state)
{
    send_outgoing_message(handshake_message, &state->awaiting_ack, state->now_ms, &state->sent_ms);
}

/**
 * @brief Handles a reading going out: turns it into the reading message and sends it.
 *
 * Does nothing while an earlier message, of any kind, is still waiting for its answer.
 *
 * @param state  The program's running state
 * @param stored The stored reading to send
 */
void handle_reading(app_state_t *state, const stored_reading_t *stored)
{
    // Checked first, so that a pass spent waiting for an answer stays short: turning the reading
    // into text takes longer than one byte takes to arrive, and bytes of the answer would be lost
    if (state->awaiting_ack) {
        return;
    }
    char texts[PROTOCOL_FIELDS_READING][FORMAT_SIGNED_SIZE];

    reading_to_message(stored, &reading_message, texts);
    send_outgoing_message(reading_message, &state->awaiting_ack, state->now_ms, &state->sent_ms);
}

/**
 * @brief Handles the response to a weather reading: reads it, sets the state from its status and
 * removes the reading from the store if the state is ok.
 *
 * A response that is missing a field is ignored.
 *
 * @param state The program's running state
 * @param text  The response received
 */
void handle_weather_reading_ack(app_state_t *state, const char *text)
{
    response_t response;

    if (!read_response(text, &state->awaiting_ack, &response)) {
        return;
    }
    handle_status(response.status, &state->current_state);
    remove_reading_if_ok(response.first_field, state->current_state);
}

/**
 * @brief Handles the response to a handshake: reads it and sets the state from its status.
 *
 * A response that is missing a field is ignored.
 *
 * @param state The program's running state
 * @param text  The response received
 */
void handle_handshake_ack(app_state_t *state, const char *text)
{
    response_t response;

    if (!read_response(text, &state->awaiting_ack, &response)) {
        return;
    }
    handle_status(response.status, &state->current_state);
}

/**
 * @brief Handles a command coming in. A placeholder: whatever the command is, it sends the oldest
 * stored reading.
 *
 * Does nothing when the store is empty.
 *
 * @param state The program's running state
 * @param text  The command received: not looked at yet
 */
void handle_incoming_command(app_state_t *state, const char *text)
{
    // TODO: placeholder. Read the command out of the text and act on it
    (void)text;

    stored_reading_t stored;

    if (!reading_store_oldest(&stored)) {
        return;
    }
    handle_reading(state, &stored);
}

/**
 * @brief Handles a reply going out: the reply to a command. A placeholder: does nothing.
 */
void handle_reply(void)
{
    // TODO: placeholder. Construct the reply message and send it
    return;
}

/**
 * @brief Handles the response to a reply. A placeholder: does nothing.
 *
 * @param state The program's running state: not used yet
 * @param text  The response received: not looked at yet
 */
void handle_reply_ack(app_state_t *state, const char *text)
{
    // TODO: placeholder. Read the response and set the state from its status
    (void)state;
    (void)text;
    return;
}
