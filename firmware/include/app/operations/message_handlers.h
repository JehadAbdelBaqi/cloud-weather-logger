#ifndef MESSAGE_HANDLERS_H
#define MESSAGE_HANDLERS_H

// The handlers for each kind of message, going out and coming in. Described in message_handlers.c

#include "app_state.h"
#include "reading_store.h"

void handle_handshake(app_state_t *state);
void handle_reading(app_state_t *state, const stored_reading_t *stored);
void handle_weather_reading_ack(app_state_t *state, const char *text);
void handle_handshake_ack(app_state_t *state, const char *text);
void handle_incoming_command(app_state_t *state, const char *text);
void handle_reply(void);
void handle_reply_ack(app_state_t *state, const char *text);

#endif
