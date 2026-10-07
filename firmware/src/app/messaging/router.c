#include "router.h"

#include <stddef.h>

#include "debug.h"
#include "link.h"
#include "message_handlers.h"
#include "polling.h"
#include "protocol.h"

/** A function that deals with one type of line. */
typedef void (*line_handler_t)(const char *line);

/** One row of the routing table: a line starting with this letter goes to this handler. */
typedef struct {
    char letter;
    line_handler_t handler;
} route_t;

// The lines the Nucleo receives, and the handler for each
static const route_t routes[] = {
    { PROTOCOL_LETTER_HANDSHAKE, handle_handshake },
    { PROTOCOL_LETTER_COMMAND,   handle_command },
};

#define ROUTE_COUNT (sizeof(routes) / sizeof(routes[0]))

/**
 * @brief Finds the handler for a letter in the routing table.
 *
 * @param letter The first letter of a line
 * @return The handler, or NULL if the letter has no row
 */
static line_handler_t find_handler(char letter)
{
    for (size_t i = 0; i < ROUTE_COUNT; i++) {
        if (routes[i].letter == letter) {
            return routes[i].handler;
        }
    }
    return NULL;
}

/**
 * @brief Deals with a line whose letter has no row in the table: it is only shown.
 *
 * @param line The line received
 */
static void handle_unknown(const char *line)
{
    polling_note_heard();
    debug_print("down: ");
    debug_print(line);
    debug_print("\r\n");
}

void router_poll(void)
{
    const char *line = link_read_line();
    if (line == NULL) {
        return;
    }

    line_handler_t handler = find_handler(line[0]);
    if (handler == NULL) {
        handle_unknown(line);
        return;
    }
    handler(line);
}
