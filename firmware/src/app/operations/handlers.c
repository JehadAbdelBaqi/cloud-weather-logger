#include "handlers.h"

#include <stdbool.h>
#include <stdint.h>

#include "config.h"
#include "incoming.h"
#include "outgoing.h"
#include "protocol.h"
#include "routing.h"
#include "scheduler.h"
#include "sensor_timer.h"
#include "reading_store.h"

#include "../messaging_two/messages.h"
#include "../messaging_two/reading_to_message.h"

// One flag for each kind of message sent up: true while one is out and not answered yet
static bool awaiting_handshake_ack = false;  // H
static bool awaiting_reading_ack = false;    // W
static bool awaiting_reply_ack = false;      // R

// The time now, as a reading of the millisecond counter: the same for the whole of one pass
static uint32_t now_ms = 0;

// The time the last message went out
static uint32_t sent_ms = 0;

// The state the program is in: starts with no response, so a handshake is the first thing sent
static state_key_t current_state = STATE_NO_RESPONSE;

/**
 * @brief Handles the handshake: sends the handshake message.
 */
void handle_handshake(void)
{
    send_outgoing_message(handshake_message, &awaiting_handshake_ack, now_ms, &sent_ms);
}

/**
 * @brief Handles a reading going out: turns it into the reading message and sends it.
 *
 * @param stored The stored reading to send
 */
void handle_reading(const stored_reading_t *stored)
{
    char texts[PROTOCOL_FIELDS_READING][FORMAT_SIGNED_SIZE];

    reading_to_message(stored, &reading_message, texts);
    send_outgoing_message(reading_message, &awaiting_reading_ack, now_ms, &sent_ms);
}

/**
 * @brief Handles the response to a weather reading: reads it, sets the state from its status and
 * removes the reading from the store if the state is ok.
 *
 * A response that is missing a field is ignored.
 *
 * @param text The response received
 */
void handle_weather_reading_ack(const char *text)
{
    response_t response;

    if (!read_response(text, &awaiting_reading_ack, &response)) {
        return;
    }
    handle_status(response.status, &current_state);
    remove_reading_if_ok(response.first_field, current_state);
}

/**
 * @brief Handles the response to a handshake: reads it and sets the state from its status.
 *
 * A response that is missing a field is ignored.
 *
 * @param text The response received
 */
void handle_handshake_ack(const char *text)
{
    response_t response;

    if (!read_response(text, &awaiting_handshake_ack, &response)) {
        return;
    }
    handle_status(response.status, &current_state);
}

/**
 * @brief Handles a command coming in. A placeholder: whatever the command is, it sends the oldest
 * stored reading.
 *
 * Does nothing when the store is empty.
 *
 * @param text The command received: not looked at yet
 */
void handle_incoming_command(const char *text)
{
    // TODO: placeholder. Read the command out of the text and act on it
    (void)text;

    stored_reading_t stored;

    if (!reading_store_oldest(&stored)) {
        return;
    }
    handle_reading(&stored);
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
 * @param text The response received: not looked at yet
 */
void handle_reply_ack(const char *text)
{
    // TODO: placeholder. Read the response and set the state from its status
    (void)text;
    return;
}

/**
 * @brief Runs the state the program is in: the function mapped to the current state.
 */
void state_setter(void)
{
    state_routing[current_state]();
}

/**
 * @brief Runs one pass of the main loop: the things done whatever the state is, then the state.
 *
 * Takes a reading if one is due and stores it, polls the UART, then runs the current state. Reads
 * no clock: it is handed the time.
 *
 * @param time_ms The time now, as a reading of a millisecond counter such as HAL_GetTick()
 */
void run_pass(uint32_t time_ms)
{
    reading_t reading;

    now_ms = time_ms;
    if (sensor_poll(&reading)) {
        reading_store_put(&reading, READING_TIME_START + now_ms / 1000);
    }
    poll_uart(now_ms, sent_ms, &current_state, &awaiting_handshake_ack, &awaiting_reading_ack, &awaiting_reply_ack);
    state_setter();
}
