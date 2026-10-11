#ifndef INCOMING_H
#define INCOMING_H

// The handlers for messages coming in. Described in incoming.c

#include <stdbool.h>
#include <stddef.h>

#include "link_framing.h"
#include "reply_store.h"
#include "states.h"

// A response that has been read: its three fields, each as text
typedef struct {
    char letter[LINK_LINE_MAX_LEN + 1];       // the letter of the message it answers: H, W or R
    char first_field[LINK_LINE_MAX_LEN + 1];  // the first field of that message
    char status[LINK_LINE_MAX_LEN + 1];       // the status, e.g. PROTOCOL_REPLY_OK
} response_t;

// A command that has been read: its three fields, each as text
typedef struct {
    char id[REPLY_ID_SIZE];               // the ID the sender gave it
    char command[LINK_LINE_MAX_LEN + 1];  // the command's number, as text, e.g. "3"
    char value[LINK_LINE_MAX_LEN + 1];    // the value after it, e.g. "10"; empty if there is none
} command_t;

void handle_status(const char *status, state_key_t *current_state);
bool read_response(const char *text, bool *awaiting_ack, response_t *response);
bool read_command(const char *text, command_t *command);
void remove_reading_if_ok(const char *seq_text, state_key_t current_state);
void remove_reply_if_ok(const char *id, state_key_t current_state);

#endif
