#ifndef HANDLERS_H
#define HANDLERS_H

// The handlers. Described in handlers.c

#include <stdint.h>

#include "reading_store.h"

void handle_handshake(void);
void handle_reading(const stored_reading_t *stored);
void handle_weather_reading_ack(const char *text);
void handle_handshake_ack(const char *text);
void handle_incoming_command(const char *text);
void handle_reply(void);
void handle_reply_ack(const char *text);
void state_setter(void);
void run_pass(uint32_t time_ms);

#endif
