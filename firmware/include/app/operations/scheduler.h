#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdbool.h>
#include <stdint.h>

#include "routing.h"

// Described in scheduler.c
void poll_uart(uint32_t now_ms, uint32_t sent_ms, state_key_t *current_state,
               bool *awaiting_handshake_ack, bool *awaiting_reading_ack, bool *awaiting_reply_ack);

#endif
