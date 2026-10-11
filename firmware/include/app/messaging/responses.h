#ifndef RESPONSES_H
#define RESPONSES_H

// Handling responses: what comes back for a line the Nucleo sent, from the bridge or the server.
// A handler takes the response apart and calls what router.c maps its status to. Which response
// goes to which handler is in router.c too.

/**
 * @brief Handles the response to the handshake request: L,H,<ID>,<status>.
 *
 * @param line The response received
 */
void handle_handshake_response(const char *line);

/**
 * @brief Handles the response to a reading: L,W,<sequence number>,<status>.
 *
 * @param line The response received
 */
void handle_reading_response(const char *line);

/**
 * @brief Handles the response to a reply to a command: L,R,<ID of the command>,<status>.
 *
 * A placeholder: nothing is done with it yet.
 *
 * @param line The response received
 */
void handle_reply_response(const char *line);

#endif
