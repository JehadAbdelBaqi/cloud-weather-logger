#include "routing.h"

#include <stddef.h>

#include "commands.h"
#include "config.h"
#include "protocol.h"
#include "message_handlers.h"
#include "incoming.h"
#include "message_fields.h"
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

// The table of commands: the function for each command sits at that command's number.
static const command_handler_t command_routes[] = {
    [PROTOCOL_COMMAND_HELP] = command_help,
    [PROTOCOL_COMMAND_READ] = command_read,
    [PROTOCOL_COMMAND_RATE] = command_rate,
};

#define COMMAND_ROUTES_SIZE (sizeof(command_routes) / sizeof(command_handler_t))

// The table of states: the function for each state sits at that state's position.
const route_handler_t state_routing[STATE_COUNT] = {
    [STATE_OK]          = state_ok,
    [STATE_NO_RESPONSE] = state_no_response,
};

/**
 * @brief Calls the function a letter leads to in a table of routes, handing it the program's
 *        running state and the message.
 *
 * A letter with no function in the table is ignored: nothing is called.
 *
 * @param table  The table of routes, with INCOMING_ROUTES_SIZE positions
 * @param letter The letter that picks the route
 * @param state  The program's running state, handed on to the function
 * @param text   The message received, handed on to the function
 */
void call_route(const incoming_handler_t table[], char letter, app_state_t *state, const char *text)
{
    unsigned char position = (unsigned char)letter;

    if (position >= INCOMING_ROUTES_SIZE) {
        return;
    }
    if (table[position] == NULL) {
        return;
    }
    table[position](state, text);
}

/**
 * @brief Routes a response by its second letter: the letter of the message it answers.
 *
 * A response with no second letter is ignored.
 *
 * @param state The program's running state, handed on to the handler
 * @param text  The response received: L,<letter>,<first field>,<status>
 */
void response_router(app_state_t *state, const char *text)
{
    char letter[2];

    if (!get_field(text, PROTOCOL_FIELD_SEPARATOR, 0, letter, sizeof(letter))) {
        return;
    }
    call_route(response_routes, letter[0], state, text);
}

/**
 * @brief Calls the function a command's number leads to in the table of commands, handing it the
 *        program's running state and the command's value.
 *
 * A number with no function in the table calls nothing.
 *
 * @param command The command's number, which picks the route
 * @param state   The program's running state, handed on to the function
 * @param value   The value that came with the command, handed on to the function
 * @param result  Filled in by the function with the result for the reply
 * @return true if a function was called, false if the number has none
 */
bool call_command_route(uint32_t command, app_state_t *state, const char *value, char *result)
{
    if (command >= COMMAND_ROUTES_SIZE) {
        return false;
    }
    if (command_routes[command] == NULL) {
        return false;
    }
    command_routes[command](state, value, result);
    return true;
}
