#include "routing.h"

#include <stddef.h>

#include "config.h"
#include "protocol.h"
#include "handlers.h"
#include "incoming.h"
#include "outgoing.h"
#include "states.h"

// The table for messages coming in: the function for each letter sits at that letter's position.
const incoming_handler_t incoming_routes[INCOMING_ROUTES_SIZE] = {
    [PROTOCOL_LETTER_RESPONSE] = response_router,
    [PROTOCOL_LETTER_COMMAND]  = handle_incoming_command,
};

// The table for responses: the function for each second letter sits at that letter's position.
const incoming_handler_t response_routes[INCOMING_ROUTES_SIZE] = {
    [PROTOCOL_LETTER_HANDSHAKE] = handle_handshake_ack,
    [PROTOCOL_LETTER_READING]   = handle_weather_reading_ack,
    [PROTOCOL_LETTER_REPLY]     = handle_reply_ack,
};

// The table of states: the function for each state sits at that state's position.
const route_handler_t state_routing[STATE_COUNT] = {
    [STATE_OK]          = state_ok,
    [STATE_NO_RESPONSE] = state_no_response,
};

/**
 * @brief Calls the function a letter leads to in a table of routes, handing it the message.
 *
 * A letter with no function in the table is ignored: nothing is called.
 *
 * @param table  The table of routes, with INCOMING_ROUTES_SIZE positions
 * @param letter The letter that picks the route
 * @param text   The message received, handed on to the function
 */
void call_route(const incoming_handler_t table[], char letter, const char *text)
{
    unsigned char position = (unsigned char)letter;

    if (position >= INCOMING_ROUTES_SIZE) {
        return;
    }
    if (table[position] == NULL) {
        return;
    }
    table[position](text);
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

    if (!get_field(text, 0, letter, sizeof(letter))) {
        return;
    }
    call_route(response_routes, letter[0], text);
}
