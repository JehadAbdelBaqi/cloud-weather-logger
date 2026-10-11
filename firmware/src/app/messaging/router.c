#include "router.h"

#include <string.h>

#include "commands.h"
#include "debug.h"
#include "line.h"
#include "link.h"
#include "polling.h"
#include "protocol.h"
#include "responses.h"
#include "state_functions.h"

// A response (L,<letter>,...): the letter after the L says what kind of line it is a response
// to, and picks the handler
static const message_route_t response_routes[] = {
    { PROTOCOL_LETTER_HANDSHAKE, handle_handshake_response },
    { PROTOCOL_LETTER_READING,   handle_reading_response },
    { PROTOCOL_LETTER_REPLY,     handle_reply_response },
};

#define RESPONSE_ROUTE_COUNT (sizeof(response_routes) / sizeof(response_routes[0]))

// The word each status is written as in a response
static const char *const status_words[] = {
    [STATUS_OK] = PROTOCOL_REPLY_OK,
};

#define STATUS_COUNT (sizeof(status_words) / sizeof(status_words[0]))

const handshake_action_t handshake_actions[] = {
    [STATUS_OK] = connect_bridge,
};

const reading_action_t reading_actions[] = {
    [STATUS_OK] = confirm_reading,
};

/**
 * @brief Finds the handler for a letter in a routing table.
 *
 * @param routes The table
 * @param count  How many rows it has
 * @param letter The letter to look for
 * @return The handler, or NULL if the letter has no row
 */
static message_handler_t find_message_handler(const message_route_t *routes, size_t count, char letter)
{
    for (size_t i = 0; i < count; i++) {
        if (routes[i].letter == letter) {
            return routes[i].handler;
        }
    }
    return NULL;
}

bool status_from_word(const char *word, status_t *status)
{
    for (size_t i = 0; i < STATUS_COUNT; i++) {
        if (strcmp(status_words[i], word) == 0) {
            *status = (status_t)i;
            return true;
        }
    }
    return false;
}

/**
 * @brief Routes a response by its second letter: the one after the first separator.
 *
 * A response with no second letter, or one that has no row in the table, is shown on the debug
 * output.
 *
 * @param line The response received, ending in '\0'
 */
static void route_response(const char *line)
{
    char letter[2];  // one letter, then the ending '\0'

    if (!line_copy_field(line, 0, letter, sizeof(letter))) {
        debug_print("response ignored: no second letter to route it by\r\n");
        return;
    }

    message_handler_t handler = find_message_handler(response_routes, RESPONSE_ROUTE_COUNT, letter[0]);
    if (handler == NULL) {
        debug_print("response ignored: its second letter has no handler\r\n");
        return;
    }
    handler(line);
}

void route_message(const char *line)
{
    polling_note_heard();

    if (line[0] == PROTOCOL_LETTER_RESPONSE) {
        route_response(line);
        return;
    }
    if (line[0] == PROTOCOL_LETTER_COMMAND) {
        handle_command(line);
        return;
    }

    debug_print("line ignored: not a response or a command\r\n");
}

void poll_bridge(void)
{
    const char *line = link_read_line();

    if (line == NULL) {
        return;
    }
    route_message(line);
}
