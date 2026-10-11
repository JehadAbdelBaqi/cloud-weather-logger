#include "message_handlers.h"

#include <stdbool.h>
#include <stdint.h>

#include "app_state.h"
#include "commands.h"
#include "incoming.h"
#include "message_fields.h"
#include "outgoing.h"
#include "protocol.h"
#include "reading_store.h"
#include "reply_store.h"

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
 * @brief Handles a command coming in: reads it, runs it, and puts the reply in the reply store.
 *
 * A command with no ID, or one too long, is ignored: there is nothing to answer with.
 *
 * @param state The program's running state
 * @param text  The command received
 */
void handle_incoming_command(app_state_t *state, const char *text)
{
    command_t command;
    char result[REPLY_RESULT_SIZE];

    if (!read_command(text, &command)) {
        return;
    }
    run_command(state, command.command, command.value, result);
    reply_store_put(command.id, result);
}

/**
 * @brief Handles a reply going out: fills in the reply message from a stored reply and sends it.
 *
 * Sends nothing while an earlier message, of any kind, is still waiting for its answer.
 *
 * @param state  The program's running state
 * @param stored The stored reply to send
 */
void handle_reply(app_state_t *state, const stored_reply_t *stored)
{
    const char *const values[] = { stored->id, stored->result };

    fill_message(&reply_message, values);
    send_outgoing_message(reply_message, &state->awaiting_ack, state->now_ms, &state->sent_ms);
}

/**
 * @brief Handles the response to a reply: reads it, sets the state from its status and removes
 * the reply from the store if the state is ok.
 *
 * A response that is missing a field is ignored.
 *
 * @param state The program's running state
 * @param text  The response received
 */
void handle_reply_ack(app_state_t *state, const char *text)
{
    response_t response;

    if (!read_response(text, &state->awaiting_ack, &response)) {
        return;
    }
    handle_status(response.status, &state->current_state);
    remove_reply_if_ok(response.first_field, state->current_state);
}
