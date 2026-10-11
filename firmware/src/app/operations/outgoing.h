#ifndef OUTGOING_H
#define OUTGOING_H

// The handlers for messages going out. Described in outgoing.c

#include <stdbool.h>
#include <stdint.h>

#include "message.h"

void send_outgoing_message(message_t message, bool *awaiting_ack, uint32_t now_ms, uint32_t *sent_ms);

#endif
