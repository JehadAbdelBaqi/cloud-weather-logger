#ifndef ROUTER_H
#define ROUTER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "states.h"

// Routing: every mapping from something in an incoming line to the function that deals with it is
// in router.c. Only incoming lines are routed: a line the Nucleo sends is sent by the function
// that knows what it is.

/** A function that handles an incoming line. */
typedef void (*message_handler_t)(const char *line);

/** One row of a routing table: this letter goes to this handler. */
typedef struct {
    char letter;
    message_handler_t handler;
} message_route_t;

/** What is done when the handshake's response has a status. */
typedef void (*handshake_action_t)(void);

/** What is done with a reading when its response has a status. */
typedef void (*reading_action_t)(uint32_t seq);

// The status of the handshake's response, and what is done: handshake_actions[status]
extern const handshake_action_t handshake_actions[];

// The status of a reading's response, and what is done with that reading: reading_actions[status]
extern const reading_action_t reading_actions[];

/**
 * @brief Gives the status a word in a response stands for.
 *
 * @param word   The status as it is written in a response, e.g. "ok"
 * @param status Filled in with the status; left as it was if the word is not a status
 * @return true if the word is a status
 */
bool status_from_word(const char *word, status_t *status);

/**
 * @brief Routes a line that has arrived from the bridge to the function that handles it.
 *
 * A response (PROTOCOL_LETTER_RESPONSE) is routed by its second letter, and a command
 * (PROTOCOL_LETTER_COMMAND) goes to handle_command(). Any other line is shown on the debug output.
 *
 * @param line The line received, ending in '\0'
 */
void route_message(const char *line);

/**
 * @brief Polls the bridge: takes a line from it if a whole one has arrived, and routes it.
 *
 * The line is handed to route_message(). Does not wait: if no whole line has arrived yet, it
 * returns at once.
 */
void poll_bridge(void);

#endif
